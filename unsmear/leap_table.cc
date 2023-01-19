// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>
#include <string>

#include "absl/log/log.h"
#include "absl/memory/memory.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/substitute.h"
#include "unsmear/unsmear.h"

namespace unsmear {

// The earliest possible leap table expiration is Julian Day 2441347, which is
// the 24-hour period ending 1972-01-31 12:00 UTC.
constexpr int kMinJdn = 2441347;

// The latest accepted leap table expiration is Julian Day 5373483, which is the
// 24-hour period ending 9999-12-31 12:00 UTC.  Leap tables with later
// expirations are likely corrupt.
//
// Times in the far future (2.4 million years) have an additional problem:
// because leap seconds occur at the end of UTC months, it's theoretically
// possible that an entire month of seconds has been added, creating a new leap
// second opportunity.  Meanwhile, enough leap seconds might have been removed
// to eliminate a leap second opportunity.
constexpr int kMaxJdn = 5373483;

// The Unix epoch is JDN 2440587.5.
absl::Time JdnToTime(int jdn) {
  // Use 64-bit math to avoid integer underflow.
  return absl::UnixEpoch() + absl::Hours(12) +
         (jdn - 2440588LL) * absl::Hours(24);
}

namespace {
// Only valid for times at UTC noon.
int ToJdnInt(absl::Time t) {
  return static_cast<int>(absl::ToUnixSeconds(t - absl::Hours(12)) / 86400) +
         2440588;
}
}  // namespace

absl::Time LeapTable::expiration() const { return entries_.front().utc; }

std::unique_ptr<LeapTable> NewLeapTableFromProto(const LeapTableProto& proto) {
  int end_jdn = proto.end_jdn();
  if (end_jdn < kMinJdn || end_jdn > kMaxJdn) {
    LOG(ERROR)
        << "Failed validating leap table: end_jdn was not in valid range";
    return nullptr;
  }

  // The expiration must be at the end of the month, immediately before what
  // might be the start of a leap smear.
  absl::Time expiration = JdnToTime(proto.end_jdn() + 1);
  if (absl::ToTM(expiration + absl::Hours(24), absl::UTCTimeZone()).tm_mday !=
      1) {
    LOG(ERROR) << "Failed validating leap table: end_jdn must be at the end of "
                  "the month";
    return nullptr;
  }

  // We will have two entries for each leap second, plus one for each endpoint.
  // We can't use make_unique here because of the private constructor.
  std::unique_ptr<LeapTable> lt(new LeapTable(
      (proto.positive_leaps_size() + proto.negative_leaps_size()) * 2 + 2));
  auto& entries = lt->entries_;

  entries.front().utc = expiration;
  entries.front().smear = 0;
  entries.back().utc = ModernUtcEpoch();
  entries.back().tai = TaiModernUtcEpoch();

  // Fill in the leap table from the end, since that's the most expected order.
  size_t i = entries.size() - 2;
  for (const auto& jdn : proto.positive_leaps()) {
    if (jdn < kMinJdn || jdn > kMaxJdn) {
      LOG(ERROR) << absl::StrCat("Failed validating leap table: positive leap ",
                                 jdn, " was not in valid range");
      return nullptr;
    }
    entries[i].utc = JdnToTime(jdn);
    entries[i].smear = 0;
    --i;
    entries[i].utc = JdnToTime(jdn + 1);
    entries[i].smear = 1;
    --i;
  }
  for (const auto& jdn : proto.negative_leaps()) {
    if (jdn < kMinJdn || jdn > kMaxJdn) {
      LOG(ERROR) << absl::StrCat("Failed validating leap table: negative leap ",
                                 jdn, " was not in valid range");
      return nullptr;
    }
    entries[i].utc = JdnToTime(jdn);
    entries[i].smear = 0;
    --i;
    entries[i].utc = JdnToTime(jdn + 1);
    entries[i].smear = -1;
    --i;
  }

  std::sort(
      entries.begin(), entries.end(),
      [](const internal::LeapTableEntry& a,
         const internal::LeapTableEntry& b) -> bool { return a.utc > b.utc; });
  if (entries.front().utc != expiration || entries.front().smear != 0) {
    LOG(ERROR)
        << "Failed validating leap table: there are leap seconds after end_jdn";
    return nullptr;
  }
  if (entries.back().utc < ModernUtcEpoch()) {
    LOG(ERROR) << "Failed validating leap table: leap is before the epoch";
    return nullptr;
  }

  // Validate the table and fill in TAI for each entry.
  for (ssize_t i = entries.size() - 2; i >= 0; --i) {
    if (entries[i].utc == entries[i + 1].utc) {
      LOG(ERROR) << "Failed validating leap table: duplicate or conflicting "
                    "leap seconds";
      return nullptr;
    }
    if (entries[i].smear != 0 &&
        absl::ToTM(entries[i].utc, absl::UTCTimeZone()).tm_mon ==
            absl::ToTM(entries[i + 1].utc, absl::UTCTimeZone()).tm_mon) {
      LOG(ERROR)
          << "Failed validating leap table: leap second is not at end of month";
      return nullptr;
    }
    entries[i].tai =
        entries[i + 1].tai +
        Seconds(absl::ToInt64Seconds(entries[i].utc - entries[i + 1].utc)) +
        Seconds(entries[i].smear);
  }

  return lt;
}

void LeapTable::ToProto(LeapTableProto* proto) const {
  proto->Clear();
  for (size_t i = entries_.size() - 1; i > 0; --i) {
    const auto& entry = entries_[i];
    if (entry.smear == 1) {
      proto->add_positive_leaps(ToJdnInt(entry.utc) - 1);
    } else if (entry.smear == -1) {
      proto->add_negative_leaps(ToJdnInt(entry.utc) - 1);
    }
  }
  proto->set_end_jdn(ToJdnInt(entries_[0].utc) - 1);
}

std::string LeapTable::DebugString() const {
  std::string s = absl::StrCat("LeapTable expires ",
                               ::unsmear::FormatTime(expiration()), "\n");

  int tai_utc = 10;
  for (const auto& e : entries_) {
    tai_utc += e.smear;
  }
  for (const auto& e : entries_) {
    absl::SubstituteAndAppend(&s, "  $0  $1  smear $2  TAI-UTC $3\n",
                              ::unsmear::FormatTime(e.utc), FormatTime(e.tai),
                              e.smear, tai_utc);
    tai_utc -= e.smear;
  }

  return s;
}

bool LeapTable::operator==(const LeapTable& other) const {
  if (entries_.size() != other.entries_.size()) {
    return false;
  }
  for (size_t i = 0; i < entries_.size(); ++i) {
    if (entries_[i].utc != other.entries_[i].utc ||
        entries_[i].tai != other.entries_[i].tai ||
        entries_[i].smear != other.entries_[i].smear) {
      return false;
    }
  }
  return true;
}

namespace {

absl::Time Interpolate(const internal::LeapTableEntry& e, TaiTime tai) {
  Duration d = e.tai - tai;
  assert(d >= ZeroDuration());
  absl::Time t = e.utc - internal::GetRep(d);
  if (e.smear != 0) {
    double s = FDivDuration(d, Hours(24) + Seconds(e.smear));
    assert(s >= 0);
    assert(s <= 1);
    t += s * absl::Seconds(e.smear);
  }
  return t;
}

TaiTime Interpolate(const internal::LeapTableEntry& e, absl::Time utc) {
  absl::Duration d = e.utc - utc;
  assert(d >= absl::ZeroDuration());
  TaiTime t = e.tai - internal::MakeDuration(d);
  if (e.smear != 0) {
    double s = absl::FDivDuration(d, absl::Hours(24));
    assert(s >= 0);
    assert(s <= 1);
    t -= s * Seconds(e.smear);
  }
  return t;
}

// Returns true if t is within twelve hours before the end of the month.
bool IsJustBeforeMonthEnd(const struct tm& t) {
  if (t.tm_hour < 12) {
    return false;
  }
  assert(t.tm_mon >= 0 && t.tm_mon <= 11);
  switch (t.tm_mon) {
    case 0:
      return t.tm_mday == 31;
    case 1:
      if (t.tm_year % 4 == 0 &&
          (t.tm_year % 100 != 0 || t.tm_year % 400 == 0)) {
        return t.tm_mday == 29;
      } else {
        return t.tm_mday == 28;
      }
    case 2:
      return t.tm_mday == 31;
    case 3:
      return t.tm_mday == 30;
    case 4:
      return t.tm_mday == 31;
    case 5:
      return t.tm_mday == 30;
    case 6:
      return t.tm_mday == 31;
    case 7:
      return t.tm_mday == 31;
    case 8:
      return t.tm_mday == 30;
    case 9:
      return t.tm_mday == 31;
    case 10:
      return t.tm_mday == 30;
    case 11:
      return t.tm_mday == 31;
  }
  return false;  // unreachable
}

// Advances a leap table entry into the future, returning hypothetical leap
// table entries as if a leap second happens at every intervening month.
std::pair<internal::LeapTableEntry, internal::LeapTableEntry> Advance(
    internal::LeapTableEntry e, absl::Time t) {
  assert(t > e.utc);
  assert(e.smear == 0);

  struct tm e_tm = absl::ToTM(e.utc, absl::UTCTimeZone());
  struct tm t_tm = absl::ToTM(t, absl::UTCTimeZone());

  int leaps = (t_tm.tm_year - e_tm.tm_year) * 12 + (t_tm.tm_mon - e_tm.tm_mon);

  int year = t_tm.tm_year + 1900;
  int month = t_tm.tm_mon + 1;
  int day = t_tm.tm_mday;

  // Construct two entries, one if all upcoming leap seconds are negative and
  // one if all upcoming leap seconds are positive.
  internal::LeapTableEntry neg;
  internal::LeapTableEntry pos;
  if (IsJustBeforeMonthEnd(t_tm)) {
    // t is within the first half of a possible smear period.  It ends at noon
    // on the following day, the first day of a new month.
    ++leaps;
    neg.utc = absl::FromDateTime(month == 12 ? year + 1 : year,
                                 month == 12 ? 1 : month + 1, 1, 12, 0, 0,
                                 absl::UTCTimeZone());
    neg.smear = -1;
    pos.smear = 1;
  } else if (day == 1 && t_tm.tm_hour < 12) {
    // t is within the second half of a possible smear period.  It ends at noon
    // on the current day.
    neg.utc =
        absl::FromDateTime(year, month, day, 12, 0, 0, absl::UTCTimeZone());
    neg.smear = -1;
    pos.smear = 1;
  } else {
    // t is not within a smear period.  We can set expiration at noon on the
    // following day, which we know to be within the current month.
    neg.utc =
        absl::FromDateTime(year, month, day + 1, 12, 0, 0, absl::UTCTimeZone());
    neg.smear = 0;
    pos.smear = 0;
  }
  neg.tai = e.tai + internal::MakeDuration(neg.utc - e.utc) - Seconds(leaps);
  pos.utc = neg.utc;
  pos.tai = e.tai + internal::MakeDuration(neg.utc - e.utc) + Seconds(leaps);

  return {neg, pos};
}

}  // namespace

template <internal::TtBasedTimescale timescale>
absl::optional<absl::Time> LeapTable::Smear(
    internal::TtBasedTime<timescale> t) const {
  auto interval = FutureProofSmear(t);
  if (interval.first != interval.second) {
    return absl::nullopt;
  }
  return interval.first;
}

absl::optional<TaiTime> LeapTable::Unsmear(absl::Time utc) const {
  auto interval = FutureProofUnsmear(utc);
  if (interval.first != interval.second) {
    return absl::nullopt;
  }
  return interval.first;
}

absl::optional<GpsTime> LeapTable::UnsmearToGps(absl::Time utc) const {
  auto interval = FutureProofUnsmearToGps(utc);
  if (interval.first != interval.second) {
    return absl::nullopt;
  }
  return interval.first;
}

template <internal::TtBasedTimescale timescale>
std::pair<absl::Time, absl::Time> LeapTable::FutureProofSmear(
    internal::TtBasedTime<timescale> t) const {
  if (t == internal::TtBasedTime<timescale>::InfiniteFuture()) {
    return {absl::InfiniteFuture(), absl::InfiniteFuture()};
  }
  if (t == internal::TtBasedTime<timescale>::InfinitePast()) {
    return {absl::InfinitePast(), absl::InfinitePast()};
  }

  // If the time is before its own epoch, we cannot convert it.
  if (t < internal::TtBasedTime<timescale>()) {
    return {absl::InfinitePast(), absl::InfiniteFuture()};
  }

  // If the time is within the current leap table, we can convert it precisely.
  TaiTime tai = ToTaiTime(t);
  const auto& expiration = entries_.front();
  if (tai <= expiration.tai) {
    size_t i;
    for (i = 1; i < entries_.size(); ++i) {
      if (tai >= entries_[i].tai) break;
    }
    if (i == entries_.size()) {
      // We ran past the smear epoch.  The time is not convertible.
      return {absl::InfinitePast(), absl::InfiniteFuture()};
    }
    absl::Time smeared = Interpolate(entries_[i - 1], tai);
    return {smeared, smeared};
  }

  auto advanced = Advance(
      expiration, expiration.utc + internal::GetRep(tai - expiration.tai));
  return {Interpolate(advanced.second, tai), Interpolate(advanced.first, tai)};
}

// Explicit instantiations:
template absl::optional<absl::Time> LeapTable::Smear(TaiTime t) const;
template absl::optional<absl::Time> LeapTable::Smear(GpsTime t) const;
template std::pair<absl::Time, absl::Time> LeapTable::FutureProofSmear(
    TaiTime t) const;
template std::pair<absl::Time, absl::Time> LeapTable::FutureProofSmear(
    GpsTime t) const;

std::pair<TaiTime, TaiTime> LeapTable::FutureProofUnsmear(
    absl::Time utc) const {
  if (utc == absl::InfiniteFuture()) {
    return {TaiInfiniteFuture(), TaiInfiniteFuture()};
  }
  if (utc == absl::InfinitePast()) {
    return {TaiInfinitePast(), TaiInfinitePast()};
  }

  // If the time is within the current leap table, we can convert it precisely.
  const auto& expiration = entries_.front();
  if (utc <= expiration.utc) {
    size_t i;
    for (i = 1; i < entries_.size(); ++i) {
      if (utc >= entries_[i].utc) break;
    }
    if (i == entries_.size()) {
      // We ran past the smear epoch.  The time is not convertible.
      return {TaiInfinitePast(), TaiInfiniteFuture()};
    }
    TaiTime unsmeared = Interpolate(entries_[i - 1], utc);
    return {unsmeared, unsmeared};
  }

  auto advanced = Advance(entries_.front(), utc);
  return {Interpolate(advanced.first, utc), Interpolate(advanced.second, utc)};
}

std::pair<GpsTime, GpsTime> LeapTable::FutureProofUnsmearToGps(
    absl::Time utc) const {
  if (utc == absl::InfiniteFuture()) {
    return {GpsInfiniteFuture(), GpsInfiniteFuture()};
  }
  if (utc == absl::InfinitePast()) {
    return {GpsInfinitePast(), GpsInfinitePast()};
  }
  auto unsmeared = FutureProofUnsmear(utc);
  if (unsmeared.first < ToTaiTime(GpsEpoch())) {
    // It's not valid to unsmear times before the GPST epoch.
    return {GpsInfinitePast(), GpsInfiniteFuture()};
  }
  // GPST can always be converted to TAI.
  return {ToGpsTime(unsmeared.first), ToGpsTime(unsmeared.second)};
}

}  // namespace unsmear

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

#ifndef UNSMEAR_UNSMEAR_H
#define UNSMEAR_UNSMEAR_H

#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include "absl/time/time.h"
#include "absl/types/optional.h"
#include "unsmear/leap_table.pb.h"

namespace unsmear {

// The leap smear extends 12 smeared hours (43,200 smeared seconds) on either
// side of UTC midnight.
constexpr absl::Duration kSmearRadius = absl::Hours(12);

class Duration;  // Defined below.

namespace internal {
constexpr absl::Duration GetRep(Duration d);
constexpr Duration MakeDuration(absl::Duration rep);
template <typename T>
using IsFloatingPoint =
    typename std::enable_if<std::is_floating_point<T>::value, int>::type;
}  // namespace internal

// An unsmear::Duration represents a period in seconds of Terrestrial Time.
// Its API follows the example of absl::Duration, which ticks in smeared
// seconds.
//
// There are no exposed conversions between unsmear::Duration and smeared types
// such as absl::Duration, std::chrono, timespec, or timeval.  These must be
// converted with reference to a timescale.  For convenience, conversions are
// provided to and from double and int64_t, and to and from strings, but use
// these with great caution.
class Duration {
 public:
  constexpr Duration() {}  // Zero-length duration.

  Duration& operator+=(Duration d) {
    rep_ += d.rep_;
    return *this;
  }
  Duration& operator-=(Duration d) {
    rep_ -= d.rep_;
    return *this;
  }

  Duration& operator*=(int64_t r) {
    rep_ *= r;
    return *this;
  }
  Duration& operator*=(double r) {
    rep_ *= r;
    return *this;
  }
  Duration& operator/=(int64_t r) {
    rep_ /= r;
    return *this;
  }
  Duration& operator/=(double r) {
    rep_ /= r;
    return *this;
  }
  Duration& operator%=(Duration rhs) {
    rep_ %= rhs.rep_;
    return *this;
  }

  // Overloads that forward to either the int64_t or double overloads above.
  template <typename T>
  Duration& operator*=(T r) {
    int64_t x = r;
    return *this *= x;
  }
  template <typename T>
  Duration& operator/=(T r) {
    int64_t x = r;
    return *this /= x;
  }
  Duration& operator*=(float r) { return *this *= static_cast<double>(r); }
  Duration& operator/=(float r) { return *this /= static_cast<double>(r); }

 private:
  explicit constexpr Duration(absl::Duration rep) : rep_(rep) {}

  friend constexpr absl::Duration internal::GetRep(Duration d);
  friend constexpr Duration internal::MakeDuration(absl::Duration rep);
  friend int64_t IDivDuration(Duration num, Duration den, Duration* rem);
  friend bool ParseDuration(const std::string& s, Duration* d);

  // The internal representation is an absl::Duration, but this is never allowed
  // to escape to public APIs, and may change in the future.  We do not rely on
  // any internals of absl::Duration.
  absl::Duration rep_;
};

// Relational Operators
constexpr bool operator<(Duration lhs, Duration rhs) {
  return internal::GetRep(lhs) < internal::GetRep(rhs);
}
constexpr bool operator>(Duration lhs, Duration rhs) { return rhs < lhs; }
constexpr bool operator>=(Duration lhs, Duration rhs) { return !(lhs < rhs); }
constexpr bool operator<=(Duration lhs, Duration rhs) { return !(rhs < lhs); }
constexpr bool operator==(Duration lhs, Duration rhs) {
  return internal::GetRep(lhs) == internal::GetRep(rhs);
}
constexpr bool operator!=(Duration lhs, Duration rhs) { return !(lhs == rhs); }

// Additive Operators
constexpr Duration operator-(Duration d) {
  return internal::MakeDuration(-internal::GetRep(d));
}
inline Duration operator+(Duration lhs, Duration rhs) { return lhs += rhs; }
inline Duration operator-(Duration lhs, Duration rhs) { return lhs -= rhs; }

// Multiplicative Operators
template <typename T>
inline Duration operator*(Duration lhs, T rhs) {
  return lhs *= rhs;
}
template <typename T>
inline Duration operator*(T lhs, Duration rhs) {
  return rhs *= lhs;
}
template <typename T>
inline Duration operator/(Duration lhs, T rhs) {
  return lhs /= rhs;
}
inline int64_t operator/(Duration lhs, Duration rhs) {
  return IDivDuration(lhs, rhs, &rhs);
}
inline Duration operator%(Duration lhs, Duration rhs) { return lhs %= rhs; }

// Divisions; see documentation for absl::Duration's similar methods.
inline int64_t IDivDuration(Duration num, Duration den, Duration* rem) {
  return absl::IDivDuration(num.rep_, den.rep_, &rem->rep_);
}
inline double FDivDuration(Duration num, Duration den) {
  return absl::FDivDuration(internal::GetRep(num), internal::GetRep(den));
}

// Equivalent to the default constructor.
constexpr Duration ZeroDuration() { return Duration(); }

// Returns the absolute value of a duration.
inline Duration AbsDuration(Duration d) {
  return (d < ZeroDuration()) ? -d : d;
}

// Duration truncation (toward zero), flooring (largest not greater than),
// and ceiling (smallest not less than) to a multiple of a non-zero unit.
inline Duration Trunc(Duration d, Duration unit) {
  return internal::MakeDuration(
      absl::Trunc(internal::GetRep(d), internal::GetRep(unit)));
}
inline Duration Floor(Duration d, Duration unit) {
  return internal::MakeDuration(
      absl::Floor(internal::GetRep(d), internal::GetRep(unit)));
}
inline Duration Ceil(Duration d, Duration unit) {
  return internal::MakeDuration(
      absl::Ceil(internal::GetRep(d), internal::GetRep(unit)));
}

// Factories.
constexpr Duration Nanoseconds(int64_t n) {
  return internal::MakeDuration(absl::Nanoseconds(n));
}
constexpr Duration Microseconds(int64_t n) {
  return internal::MakeDuration(absl::Microseconds(n));
}
constexpr Duration Milliseconds(int64_t n) {
  return internal::MakeDuration(absl::Milliseconds(n));
}
constexpr Duration Seconds(int64_t n) {
  return internal::MakeDuration(absl::Seconds(n));
}
constexpr Duration Minutes(int64_t n) {
  return internal::MakeDuration(absl::Minutes(n));
}
constexpr Duration Hours(int64_t n) {
  return internal::MakeDuration(absl::Hours(n));
}
template <typename T, internal::IsFloatingPoint<T> = 0>
Duration Nanoseconds(T n) {
  return n * Nanoseconds(1);
}
template <typename T, internal::IsFloatingPoint<T> = 0>
Duration Microseconds(T n) {
  return n * Microseconds(1);
}
template <typename T, internal::IsFloatingPoint<T> = 0>
Duration Milliseconds(T n) {
  return n * Milliseconds(1);
}
template <typename T, internal::IsFloatingPoint<T> = 0>
Duration Seconds(T n) {
  return n * Seconds(1);
}
template <typename T, internal::IsFloatingPoint<T> = 0>
Duration Minutes(T n) {
  return n * Minutes(1);
}
template <typename T, internal::IsFloatingPoint<T> = 0>
Duration Hours(T n) {
  return n * Hours(1);
}

// Helpers.
inline int64_t ToInt64Nanoseconds(Duration d) {
  return absl::ToInt64Nanoseconds(internal::GetRep(d));
}
inline int64_t ToInt64Microseconds(Duration d) {
  return absl::ToInt64Microseconds(internal::GetRep(d));
}
inline int64_t ToInt64Milliseconds(Duration d) {
  return absl::ToInt64Milliseconds(internal::GetRep(d));
}
inline int64_t ToInt64Seconds(Duration d) {
  return absl::ToInt64Seconds(internal::GetRep(d));
}
inline int64_t ToInt64Minutes(Duration d) {
  return absl::ToInt64Minutes(internal::GetRep(d));
}
inline int64_t ToInt64Hours(Duration d) {
  return absl::ToInt64Hours(internal::GetRep(d));
}
inline double ToDoubleNanoseconds(Duration d) {
  return absl::ToDoubleNanoseconds(internal::GetRep(d));
}
inline double ToDoubleMicroseconds(Duration d) {
  return absl::ToDoubleMicroseconds(internal::GetRep(d));
}
inline double ToDoubleMilliseconds(Duration d) {
  return absl::ToDoubleMilliseconds(internal::GetRep(d));
}
inline double ToDoubleSeconds(Duration d) {
  return absl::ToDoubleSeconds(internal::GetRep(d));
}
inline double ToDoubleMinutes(Duration d) {
  return absl::ToDoubleMinutes(internal::GetRep(d));
}
inline double ToDoubleHours(Duration d) {
  return absl::ToDoubleHours(internal::GetRep(d));
}

// Returns an infinite Duration.  To get a Duration representing negative
// infinity, use -InfiniteDuration().
constexpr Duration InfiniteDuration() {
  return internal::MakeDuration(absl::InfiniteDuration());
}

// Returns a string representing the duration in the form "72h3m0.5s".
// Returns "inf" or "-inf" for +/- InfiniteDuration().
inline std::string FormatDuration(Duration d) {
  return absl::FormatDuration(internal::GetRep(d));
}

// Output stream operator.
inline std::ostream& operator<<(std::ostream& os, Duration d) {
  return os << internal::GetRep(d) << " TT";
}

// Parses a duration string consisting of a possibly signed sequence
// of decimal numbers, each with an optional fractional part and a
// unit suffix.  The valid suffixes are "ns", "us" "ms", "s", "m",
// and "h".  Simple examples include "300ms", "-1.5h", and "2h45m".
// Parses "inf" and "-inf" as +/- InfiniteDuration().
inline bool ParseDuration(const std::string& s, Duration* d) {
  return absl::ParseDuration(s, &d->rep_);
}

namespace internal {

constexpr absl::Duration GetRep(Duration d) { return d.rep_; }
constexpr Duration MakeDuration(absl::Duration rep) { return Duration(rep); }

// Template argument for TtBasedTime below.
enum TtBasedTimescale {
  TAI = 0,
  GPST = 1,
};

// An unsmear::internal::TtBasedTime is an template base class for representing
// a specific instant in time, in a timescale based on Terrestrial Time that
// begins at some epoch.  Times may exist in a proleptic timescale (i.e., before
// the epoch) for arithmetic convenience, but LeapTable's smearing and
// unsmearing will not work with these times.
//
// Infinite-past and infinite-future pseudo-times are also available.
template <TtBasedTimescale timescale>
class TtBasedTime {
 public:
  // Returns a time at the epoch of this timescale.  For example, TaiTime()
  // gives 1958-01-01 00:00:00 TAI.
  constexpr TtBasedTime() {}

  // Instead of these, prefer to use the named specializations such as
  // TaiInfiniteFuture().
  static TtBasedTime InfiniteFuture();
  static TtBasedTime InfinitePast();

  // Assignment operators.
  TtBasedTime& operator+=(Duration d) {
    rep_ += d;
    return *this;
  }
  TtBasedTime& operator-=(Duration d) {
    rep_ -= d;
    return *this;
  }

 private:
  friend bool operator<(TtBasedTime lhs, TtBasedTime rhs) {
    return lhs.rep_ < rhs.rep_;
  }
  friend bool operator==(TtBasedTime lhs, TtBasedTime rhs) {
    return lhs.rep_ == rhs.rep_;
  };
  friend Duration operator-(TtBasedTime lhs, TtBasedTime rhs) {
    return lhs.rep_ - rhs.rep_;
  };

  Duration rep_;
};

// Returns the offset between this timescale's epoch and the TAI epoch of
// 1958-01-01 00:00:00 TAI.
template <TtBasedTimescale timescale>
constexpr Duration TaiOffset(TtBasedTime<timescale> t);
template <>
constexpr Duration TaiOffset(TtBasedTime<TAI> t) {
  // The TAI epoch is tautologically equal to the TAI epoch.
  return ZeroDuration();
}
template <>
constexpr Duration TaiOffset(TtBasedTime<GPST> t) {
  // The GPS epoch of 1980-01-06 00:00:00 GPST is 8,040 days after the TAI
  // epoch, plus the 10 seconds of DUT1 at the start of UTC, plus 9 leap seconds
  // between 1970 and 1980.
  return MakeDuration(absl::Seconds(8040 * 86400 + 19));
}

// Relational operators.
template <TtBasedTimescale timescale>
inline bool operator>(TtBasedTime<timescale> lhs, TtBasedTime<timescale> rhs) {
  return rhs < lhs;
}
template <TtBasedTimescale timescale>
inline bool operator>=(TtBasedTime<timescale> lhs, TtBasedTime<timescale> rhs) {
  return !(lhs < rhs);
}
template <TtBasedTimescale timescale>
inline bool operator<=(TtBasedTime<timescale> lhs, TtBasedTime<timescale> rhs) {
  return !(rhs < lhs);
}
template <TtBasedTimescale timescale>
inline bool operator!=(TtBasedTime<timescale> lhs, TtBasedTime<timescale> rhs) {
  return !(lhs == rhs);
}

// Additive operators.
template <TtBasedTimescale timescale>
inline TtBasedTime<timescale> operator+(TtBasedTime<timescale> lhs,
                                        Duration rhs) {
  return lhs += rhs;
}
template <TtBasedTimescale timescale>
inline TtBasedTime<timescale> operator+(Duration lhs,
                                        TtBasedTime<timescale> rhs) {
  return rhs += lhs;
}
template <TtBasedTimescale timescale>
inline TtBasedTime<timescale> operator-(TtBasedTime<timescale> lhs,
                                        Duration rhs) {
  return lhs -= rhs;
}

template <TtBasedTimescale timescale>
inline TtBasedTime<timescale> TtBasedTime<timescale>::InfiniteFuture() {
  TtBasedTime<timescale> t;
  t.rep_ = InfiniteDuration();
  return t;
}

template <TtBasedTimescale timescale>
inline TtBasedTime<timescale> TtBasedTime<timescale>::InfinitePast() {
  TtBasedTime<timescale> t;
  t.rep_ = -InfiniteDuration();
  return t;
}

}  // namespace internal

// Converts from a Julian Day Number (not a Modified JDN) to an absl::Time.
absl::Time JdnToTime(int jdn);

using TaiTime = internal::TtBasedTime<internal::TAI>;
using GpsTime = internal::TtBasedTime<internal::GPST>;

// Returns 1958-01-01 00:00:00 TAI.
constexpr TaiTime TaiEpoch() { return TaiTime(); }
// Returns 1980-01-06 00:00:00 GPST == 1980-01-06 00:00:10 TAI.
constexpr GpsTime GpsEpoch() { return GpsTime(); }
inline TaiTime TaiGpsEpoch() { return TaiEpoch() + TaiOffset(GpsEpoch()); }
constexpr absl::Time UtcGpsEpoch() { return absl::FromUnixSeconds(315964800); }

// These conversions must be explicit.
template <internal::TtBasedTimescale timescale>
inline TaiTime ToTaiTime(internal::TtBasedTime<timescale> t) {
  return TaiEpoch() + TaiOffset(t) + (t - internal::TtBasedTime<timescale>());
}
template <internal::TtBasedTimescale timescale>
inline GpsTime ToGpsTime(internal::TtBasedTime<timescale> t) {
  return GpsEpoch() - TaiOffset(GpsTime()) + (ToTaiTime(t) - TaiEpoch());
}

// Times in the infinite future and past.
inline TaiTime TaiInfiniteFuture() { return TaiTime::InfiniteFuture(); }
inline TaiTime TaiInfinitePast() { return TaiTime::InfinitePast(); }
inline GpsTime GpsInfiniteFuture() { return GpsTime::InfiniteFuture(); }
inline GpsTime GpsInfinitePast() { return GpsTime::InfinitePast(); }

// Returns the earliest unsmearable time, 1972-01-01 00:00:00 UTC, the start of
// modern UTC, equal to 1972-01-01 00:00:10 TAI.
//
// UTC was not well-defined before 1970, and its definition was changed at the
// start of 1972 when a 107.758 ms discontinuity was introduced.  Conversions
// before that point are infeasible.
//
// There is no GpsModernUtcEpoch() because that time predates the start of the
// GPS timescale.  For the same reason, there is no TaiUnixEpoch() or
// GpsUnixEpoch().
constexpr absl::Time ModernUtcEpoch() {
  return absl::FromUnixSeconds(63072000);
}
inline TaiTime TaiModernUtcEpoch() {
  return TaiEpoch() + 5113 * Hours(24) + Seconds(10);
}

// Formats the given time as "2006-01-02 15:04:05.999999999 TAI".
template <internal::TtBasedTimescale timescale>
std::string FormatTime(internal::TtBasedTime<timescale> t);
std::string FormatTime(absl::Time t);

// Formats the given time with a user-specified format string.  absl::Time will
// be formatted in UTC.
template <internal::TtBasedTimescale timescale>
std::string FormatTime(const std::string& format,
                       internal::TtBasedTime<timescale> t);
std::string FormatTime(const std::string& format, absl::Time t);

namespace internal {

// Outputs to a stream.
template <TtBasedTimescale timescale>
inline std::ostream& operator<<(std::ostream& os, TtBasedTime<timescale> t) {
  return os << FormatTime(t);
}

struct LeapTableEntry {
  // Because we construct the table from timestamps specified as JDNs, UTC times
  // will always fall at noon.
  absl::Time utc;
  TaiTime tai;
  // The smear direction for times before this point.  0 for the first entry in
  // the leap table, indicating that no smearing is happening.  +1 at end of a
  // positive smear (insertion of a leap second), or -1 for an anti-leap.  0 for
  // the start of a smear, resetting to a no-smear period.
  int smear;
};

}  // namespace internal

// A LeapTable represents conversions between timescales with leap seconds,
// smeared or unsmeared, and timescales based on continuous seconds of
// Terrestrial Time.  It is immutable after construction, and can therefore be
// freely shared between threads.
class LeapTable {
 public:
  // Converts between smeared and unsmeared times, if the time is within the
  // validity range of this leap table.
  template <internal::TtBasedTimescale timescale>
  absl::optional<absl::Time> Smear(internal::TtBasedTime<timescale> t) const;
  absl::optional<TaiTime> Unsmear(absl::Time utc) const;
  absl::optional<GpsTime> UnsmearToGps(absl::Time utc) const;

  // Returns the earliest and latest possible smeared times.  For times within
  // the validity range of this leap table, the times will be equal.  Infinities
  // will be mapped to an interval with both endpoints being the infinity of the
  // new timescale with the same sign.
  template <internal::TtBasedTimescale timescale>
  std::pair<absl::Time, absl::Time> FutureProofSmear(
      internal::TtBasedTime<timescale> t) const;
  std::pair<TaiTime, TaiTime> FutureProofUnsmear(absl::Time utc) const;
  std::pair<GpsTime, GpsTime> FutureProofUnsmearToGps(absl::Time utc) const;

  // Returns the latest time that can be unambiguously converted.  The earliest
  // convertible time is always ModernUtcEpoch(), 1972-01-01 00:00:00 UTC.
  absl::Time expiration() const;

  // Writes the leap table data to a LeapTableProto, which can be serialized.
  // Calling NewLeapTableFromProto() on this protobuf will result in an
  // equivalent LeapTable.
  void ToProto(LeapTableProto* proto) const;

  // A human-readable string describing the contents of the leap table.  For
  // debugging only and subject to change.  Do not attempt to parse this.
  std::string DebugString() const;

  bool operator==(const LeapTable& other) const;
  bool operator!=(const LeapTable& other) const { return !(*this == other); }

 private:
  explicit LeapTable(size_t size) noexcept : entries_(size) {}

  friend std::unique_ptr<LeapTable> NewLeapTableFromProto(
      const LeapTableProto& proto);

  // A reverse-ordered (i.e., latest-first) series of times at which the smear
  // changed.  There will be an even number of entries: the expiration, one each
  // for the start and end of each smear, and finally the smear epoch.
  std::vector<internal::LeapTableEntry> entries_;
};

// Constructs a LeapTable from a protobuf with the leap second data, if it is
// valid.
std::unique_ptr<LeapTable> NewLeapTableFromProto(const LeapTableProto& proto);

}  // namespace unsmear

#endif  // UNSMEAR_UNSMEAR_H

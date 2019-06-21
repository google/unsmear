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

#include <string>
#include "absl/time/time.h"
#include "unsmear/unsmear.h"

namespace unsmear {

namespace {

// Construct strings at most once.  Static initialization is thread-safe in
// C++11.  Using a pointer avoids running the destructor, which would not be
// done thread-safely.
const std::string& FutureName(TaiTime t) {
  static auto* s = new std::string("tai-infinite-future");
  return *s;
}
const std::string& FutureName(GpsTime t) {
  static auto* s = new std::string("gpst-infinite-future");
  return *s;
}
const std::string& PastName(TaiTime t) {
  static auto* s = new std::string("tai-infinite-past");
  return *s;
}
const std::string& PastName(GpsTime t) {
  static auto* s = new std::string("gpst-infinite-past");
  return *s;
}
const std::string& ZoneName(TaiTime t) {
  static auto* s = new std::string("TAI");
  return *s;
}
const std::string& ZoneName(GpsTime t) {
  static auto* s = new std::string("GPST");
  return *s;
}

const std::string& DefaultFormat(TaiTime t) {
  static auto* s = new std::string("%Y-%m-%d %H:%M:%E*S TAI");
  return *s;
}
const std::string& DefaultFormat(GpsTime t) {
  static auto* s = new std::string("%Y-%m-%d %H:%M:%E*S GPST");
  return *s;
}
const std::string& DefaultFormat(absl::Time t) {
  static auto* s = new std::string("%Y-%m-%d %H:%M:%E*S UTC");
  return *s;
}

// Convert between the TAI and GPST timescales and the Unix timescale.  Because
// seconds are defined differently in these timescales, this is completely
// unsound, but it's just what we need for formatting purposes.
absl::Time ToUnixTime(TaiTime t) {
  return absl::FromUnixNanos((t - TaiEpoch()) / Nanoseconds(1)) -
         4383 * absl::Hours(24);
}
absl::Time ToUnixTime(GpsTime t) {
  return absl::FromUnixNanos((t - GpsEpoch()) / Nanoseconds(1)) +
         3657 * absl::Hours(24);
}

}  // namespace

// Formatting with the default format.  This doesn't simply delegate to the
// non-default version because we don't need to handle %Z.
template <internal::TtBasedTimescale timescale>
std::string FormatTime(internal::TtBasedTime<timescale> t) {
  if (t == t.InfiniteFuture()) {
    return FutureName(t);
  }
  if (t == t.InfinitePast()) {
    return PastName(t);
  }
  return absl::FormatTime(DefaultFormat(t), ToUnixTime(t), absl::UTCTimeZone());
}
// Explicit instantiation.
template std::string FormatTime(internal::TtBasedTime<internal::TAI> t);
template std::string FormatTime(internal::TtBasedTime<internal::GPST> t);

// Formatting with user-specified formats.
template <internal::TtBasedTimescale timescale>
std::string FormatTime(const std::string& format,
                       internal::TtBasedTime<timescale> t) {
  if (t == t.InfiniteFuture()) {
    return FutureName(t);
  }
  if (t == t.InfinitePast()) {
    return PastName(t);
  }

  // There is no absl::FixedTimeZoneWithName() to create a time zone with the
  // name we want, so we need to preprocess the format string to replace "%Z"
  // with the zone name.  But we shouldn't replace "%%Z" as would happen if we
  // used absl::StrReplaceAll().
  std::string s;
  s.reserve(format.size());
  bool saw_percent = false;
  for (char c : format) {
    if (saw_percent) {
      if (c == 'Z') {
        s.append(ZoneName(t));
      } else {
        s.push_back('%');
        s.push_back(c);
      }
      saw_percent = false;
    } else {
      if (c == '%') {
        saw_percent = true;
      } else {
        s.push_back(c);
      }
    }
  }
  if (saw_percent) {
    // Last char of string was '%'; unswallow it.
    s.push_back('%');
  }

  return absl::FormatTime(s, ToUnixTime(t), absl::UTCTimeZone());
}
// Explicit instantiation.
template std::string FormatTime(const std::string& format,
                                internal::TtBasedTime<internal::TAI> t);
template std::string FormatTime(const std::string& format,
                                internal::TtBasedTime<internal::GPST> t);

std::string FormatTime(absl::Time t) { return FormatTime(DefaultFormat(t), t); }
std::string FormatTime(const std::string& format, absl::Time t) {
  return absl::FormatTime(format, t, absl::UTCTimeZone());
}

}  // namespace unsmear

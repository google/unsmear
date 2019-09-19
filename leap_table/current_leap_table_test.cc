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

#include "unsmear/unsmear.h"

#include <fstream>

#include "gtest/gtest.h"

namespace unsmear {
namespace {

// Returns an absl::Time at UTC noon.
absl::Time Noon(int64_t y, int m, int d) {
  return absl::FromDateTime(y, m, d, 12, 0, 0, absl::UTCTimeZone());
}

// Expects that three times are precisely convertible between each other.
void ExpectPrecise(const LeapTable& lt, absl::Time utc, TaiTime tai,
                   GpsTime gps) {
  auto utc_tai = lt.Unsmear(utc);
  ASSERT_TRUE(utc_tai.has_value()) << utc;
  EXPECT_EQ(*utc_tai, tai) << utc;

  auto utc_gps = lt.UnsmearToGps(utc);
  ASSERT_TRUE(utc_gps.has_value()) << utc;
  EXPECT_EQ(*utc_gps, gps) << utc;

  auto tai_utc = lt.Smear(tai);
  ASSERT_TRUE(tai_utc.has_value()) << tai;
  EXPECT_EQ(*tai_utc, utc) << tai;

  auto gps_utc = lt.Smear(gps);
  ASSERT_TRUE(gps_utc.has_value()) << gps;
  EXPECT_EQ(*gps_utc, utc) << gps;

  EXPECT_EQ(*utc_tai, ToTaiTime(*utc_gps));
  EXPECT_EQ(*utc_gps, ToGpsTime(*utc_tai));
  EXPECT_EQ(*tai_utc, *gps_utc);

  EXPECT_EQ(lt.FutureProofUnsmear(utc), std::make_pair(tai, tai)) << utc;
  EXPECT_EQ(lt.FutureProofUnsmearToGps(utc), std::make_pair(gps, gps)) << utc;
  EXPECT_EQ(lt.FutureProofSmear(tai), std::make_pair(utc, utc)) << tai;
  EXPECT_EQ(lt.FutureProofSmear(gps), std::make_pair(utc, utc)) << gps;
}

TEST(CurrentLeapTableTest, CurrentLeapTable) {
  LeapTableProto pb;
  std::ifstream input("leap_table/leap_table.pb");
  ASSERT_TRUE(pb.ParseFromIstream(&input));
  auto lt = NewLeapTableFromProto(pb);
  ASSERT_TRUE(lt != nullptr);

  // A time not during a leap smear: the start time of Dr. Emmett Brown's first
  // temporal displacement test, 1985-10-26 01:20 PDT.
  auto utc = absl::FromDateTime(1985, 10, 26, 8, 20, 0, absl::UTCTimeZone());
  auto tai =
      TaiEpoch() + 10160 * Hours(24) + Hours(8) + Minutes(20) + Seconds(23);
  auto gps =
      GpsEpoch() + 2120 * Hours(24) + Hours(8) + Minutes(20) + Seconds(4);
  ExpectPrecise(*lt, utc, tai, gps);

  // A time during a leap smear: 2016-12-31 18:00 UTC.
  utc = Noon(2016, 12, 31) + absl::Hours(6);
  tai = TaiEpoch() + 21549 * Hours(24) + Hours(18) + Seconds(36) +
        Milliseconds(250);
  gps = GpsEpoch() + 13509 * Hours(24) + Hours(18) + Seconds(17) +
        Milliseconds(250);
  ExpectPrecise(*lt, utc, tai, gps);
}

}  // namespace
}  // namespace unsmear

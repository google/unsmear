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

#include "gtest/gtest.h"
#include "unsmear/unsmear.h"

namespace unsmear {
namespace {

TEST(TimeTest, EpochsAndFormatting) {
  EXPECT_EQ(TaiEpoch(), TaiTime());
  EXPECT_EQ(GpsEpoch(), GpsTime());

  EXPECT_EQ(FormatTime(TaiEpoch()), "1958-01-01 00:00:00 TAI");
  EXPECT_EQ(FormatTime(GpsEpoch()), "1980-01-06 00:00:00 GPST");
  EXPECT_EQ(ToTaiTime(GpsEpoch()), TaiGpsEpoch());
  EXPECT_EQ(::unsmear::FormatTime(ModernUtcEpoch()), "1972-01-01 00:00:00 UTC");
  EXPECT_EQ(::unsmear::FormatTime(TaiModernUtcEpoch()),
            "1972-01-01 00:00:10 TAI");

  EXPECT_EQ(FormatTime(TaiGpsEpoch()), "1980-01-06 00:00:19 TAI");
  EXPECT_EQ(::unsmear::FormatTime(UtcGpsEpoch()), "1980-01-06 00:00:00 UTC");
  EXPECT_EQ(FormatTime(ToGpsTime(TaiEpoch())), "1957-12-31 23:59:41 GPST");
  EXPECT_EQ(FormatTime(ToTaiTime(GpsEpoch())), "1980-01-06 00:00:19 TAI");

  EXPECT_EQ(::unsmear::FormatTime(absl::RFC1123_full, ModernUtcEpoch()),
            "Sat, 01 Jan 1972 00:00:00 +0000");
  EXPECT_EQ(::unsmear::FormatTime("%Y %V %Z %% %%% %%Z %", TaiModernUtcEpoch()),
            "1972 52 TAI % %% %Z %");
}

TEST(TimeTest, Infinities) {
  EXPECT_LT(TaiInfinitePast(), TaiEpoch());
  EXPECT_LT(TaiEpoch(), TaiInfiniteFuture());
  EXPECT_EQ(FormatTime(TaiInfinitePast()), "tai-infinite-past");
  EXPECT_EQ(FormatTime(TaiInfiniteFuture()), "tai-infinite-future");

  EXPECT_LT(GpsInfinitePast(), GpsEpoch());
  EXPECT_LT(GpsEpoch(), GpsInfiniteFuture());
  EXPECT_EQ(FormatTime(GpsInfinitePast()), "gpst-infinite-past");
  EXPECT_EQ(FormatTime(GpsInfiniteFuture()), "gpst-infinite-future");

  EXPECT_EQ(TaiInfinitePast(), ToTaiTime(GpsInfinitePast()));
  EXPECT_EQ(GpsInfinitePast(), ToGpsTime(TaiInfinitePast()));
  EXPECT_EQ(TaiInfiniteFuture(), ToTaiTime(GpsInfiniteFuture()));
  EXPECT_EQ(GpsInfiniteFuture(), ToGpsTime(TaiInfiniteFuture()));
}

TEST(TimeTest, Conversions) {
  auto tai = TaiEpoch() + 12345 * Hours(24) + Seconds(19);
  auto gps = GpsEpoch() + 4305 * Hours(24);

  EXPECT_EQ(tai, ToTaiTime(gps));
  EXPECT_EQ(tai, ToTaiTime(tai));
  EXPECT_EQ(tai, ToTaiTime(ToTaiTime(tai)));
  EXPECT_EQ(tai, ToTaiTime(ToGpsTime(tai)));

  EXPECT_EQ(gps, ToGpsTime(tai));
  EXPECT_EQ(gps, ToGpsTime(gps));
  EXPECT_EQ(gps, ToGpsTime(ToGpsTime(gps)));
  EXPECT_EQ(gps, ToGpsTime(ToTaiTime(gps)));
}

}  // namespace
}  // namespace unsmear

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

#include "google/protobuf/util/message_differencer.h"
#include "gtest/gtest.h"

namespace unsmear {
namespace {

// Returns an absl::Time at UTC noon.
absl::Time Noon(int64_t y, int m, int d) {
  return absl::FromDateTime(y, m, d, 12, 0, 0, absl::UTCTimeZone());
}

TEST(JdnToTimeTest, JdnToTime) {
  // Note that absl::Time uses the proleptic Gregorian calendar, not Julian.
  EXPECT_EQ(JdnToTime(-2147483648), Noon(-5884323, 5, 15));

  EXPECT_EQ(JdnToTime(-1), Noon(-4713, 11, 23));  // 4714 BCE
  EXPECT_EQ(JdnToTime(0), Noon(-4713, 11, 24));
  EXPECT_EQ(JdnToTime(1), Noon(-4713, 11, 25));

  EXPECT_EQ(JdnToTime(2400001), Noon(1858, 11, 17));  // MJD 0.5
  EXPECT_EQ(JdnToTime(2441318), Noon(1972, 1, 1));
  EXPECT_EQ(JdnToTime(2451545), Noon(2000, 1, 1));
  EXPECT_EQ(JdnToTime(2457300), Noon(2015, 10, 4));

  EXPECT_EQ(JdnToTime(2147483647), Noon(5874898, 6, 3));
}

class LeapTableTest : public ::testing::Test {
 protected:
  void SetUp() {
    // These are not real leap seconds.  However, this test table does extend
    // past the GPST epoch, and TAI-UTC is 19 s at that point.
    proto_.add_positive_leaps(2441499);  // 1972-06-30 12:00:00 UTC
    proto_.add_positive_leaps(2441864);  // 1973-06-30 12:00:00 UTC
    proto_.add_negative_leaps(2442048);  // 1973-12-31 12:00:00 UTC, negative
    proto_.add_positive_leaps(2442413);  // 1974-12-31 12:00:00 UTC
    proto_.add_positive_leaps(2442778);  // 1975-12-31 12:00:00 UTC
    proto_.add_positive_leaps(2443144);  // 1976-12-31 12:00:00 UTC
    proto_.add_positive_leaps(2443509);  // 1977-12-31 12:00:00 UTC
    proto_.add_positive_leaps(2443874);  // 1978-12-31 12:00:00 UTC
    proto_.add_positive_leaps(2443905);  // 1979-01-31 12:00:00 UTC
    proto_.add_positive_leaps(2443933);  // 1979-02-28 12:00:00 UTC
    proto_.add_positive_leaps(2443964);  // 1979-03-31 12:00:00 UTC
    proto_.set_end_jdn(2446065);         // 1984-12-30 12:00:00 UTC
    lt_ = NewLeapTableFromProto(proto_);
    ASSERT_TRUE(lt_ != nullptr);
  }

  TaiTime expiration_tai() const {
    return TaiEpoch() + 9861 * Hours(24) + Hours(12) + Seconds(19);
  }

  LeapTableProto proto_;
  std::unique_ptr<LeapTable> lt_;
};

TEST_F(LeapTableTest, DebugString) {
  EXPECT_EQ(lt_->DebugString(),
            R"(LeapTable expires 1984-12-31 12:00:00 UTC
  1984-12-31 12:00:00 UTC  1984-12-31 12:00:19 TAI  smear 0  TAI-UTC 19
  1979-04-01 12:00:00 UTC  1979-04-01 12:00:19 TAI  smear 1  TAI-UTC 19
  1979-03-31 12:00:00 UTC  1979-03-31 12:00:18 TAI  smear 0  TAI-UTC 18
  1979-03-01 12:00:00 UTC  1979-03-01 12:00:18 TAI  smear 1  TAI-UTC 18
  1979-02-28 12:00:00 UTC  1979-02-28 12:00:17 TAI  smear 0  TAI-UTC 17
  1979-02-01 12:00:00 UTC  1979-02-01 12:00:17 TAI  smear 1  TAI-UTC 17
  1979-01-31 12:00:00 UTC  1979-01-31 12:00:16 TAI  smear 0  TAI-UTC 16
  1979-01-01 12:00:00 UTC  1979-01-01 12:00:16 TAI  smear 1  TAI-UTC 16
  1978-12-31 12:00:00 UTC  1978-12-31 12:00:15 TAI  smear 0  TAI-UTC 15
  1978-01-01 12:00:00 UTC  1978-01-01 12:00:15 TAI  smear 1  TAI-UTC 15
  1977-12-31 12:00:00 UTC  1977-12-31 12:00:14 TAI  smear 0  TAI-UTC 14
  1977-01-01 12:00:00 UTC  1977-01-01 12:00:14 TAI  smear 1  TAI-UTC 14
  1976-12-31 12:00:00 UTC  1976-12-31 12:00:13 TAI  smear 0  TAI-UTC 13
  1976-01-01 12:00:00 UTC  1976-01-01 12:00:13 TAI  smear 1  TAI-UTC 13
  1975-12-31 12:00:00 UTC  1975-12-31 12:00:12 TAI  smear 0  TAI-UTC 12
  1975-01-01 12:00:00 UTC  1975-01-01 12:00:12 TAI  smear 1  TAI-UTC 12
  1974-12-31 12:00:00 UTC  1974-12-31 12:00:11 TAI  smear 0  TAI-UTC 11
  1974-01-01 12:00:00 UTC  1974-01-01 12:00:11 TAI  smear -1  TAI-UTC 11
  1973-12-31 12:00:00 UTC  1973-12-31 12:00:12 TAI  smear 0  TAI-UTC 12
  1973-07-01 12:00:00 UTC  1973-07-01 12:00:12 TAI  smear 1  TAI-UTC 12
  1973-06-30 12:00:00 UTC  1973-06-30 12:00:11 TAI  smear 0  TAI-UTC 11
  1972-07-01 12:00:00 UTC  1972-07-01 12:00:11 TAI  smear 1  TAI-UTC 11
  1972-06-30 12:00:00 UTC  1972-06-30 12:00:10 TAI  smear 0  TAI-UTC 10
  1972-01-01 00:00:00 UTC  1972-01-01 00:00:10 TAI  smear 0  TAI-UTC 10
)");
}

TEST_F(LeapTableTest, InfinitySmear) {
  EXPECT_EQ(lt_->Smear(TaiInfiniteFuture()), absl::InfiniteFuture());
  EXPECT_EQ(lt_->Smear(GpsInfiniteFuture()), absl::InfiniteFuture());
  EXPECT_EQ(lt_->Smear(TaiInfinitePast()), absl::InfinitePast());
  EXPECT_EQ(lt_->Smear(GpsInfinitePast()), absl::InfinitePast());
  EXPECT_EQ(lt_->Unsmear(absl::InfiniteFuture()), TaiInfiniteFuture());
  EXPECT_EQ(lt_->UnsmearToGps(absl::InfiniteFuture()), GpsInfiniteFuture());
  EXPECT_EQ(lt_->Unsmear(absl::InfinitePast()), TaiInfinitePast());
  EXPECT_EQ(lt_->UnsmearToGps(absl::InfinitePast()), GpsInfinitePast());

  EXPECT_EQ(lt_->FutureProofSmear(TaiInfiniteFuture()),
            std::make_pair(absl::InfiniteFuture(), absl::InfiniteFuture()));
  EXPECT_EQ(lt_->FutureProofSmear(GpsInfiniteFuture()),
            std::make_pair(absl::InfiniteFuture(), absl::InfiniteFuture()));
  EXPECT_EQ(lt_->FutureProofSmear(TaiInfinitePast()),
            std::make_pair(absl::InfinitePast(), absl::InfinitePast()));
  EXPECT_EQ(lt_->FutureProofSmear(GpsInfinitePast()),
            std::make_pair(absl::InfinitePast(), absl::InfinitePast()));
  EXPECT_EQ(lt_->FutureProofUnsmear(absl::InfiniteFuture()),
            std::make_pair(TaiInfiniteFuture(), TaiInfiniteFuture()));
  EXPECT_EQ(lt_->FutureProofUnsmearToGps(absl::InfiniteFuture()),
            std::make_pair(GpsInfiniteFuture(), GpsInfiniteFuture()));
  EXPECT_EQ(lt_->FutureProofUnsmear(absl::InfinitePast()),
            std::make_pair(TaiInfinitePast(), TaiInfinitePast()));
  EXPECT_EQ(lt_->FutureProofUnsmearToGps(absl::InfinitePast()),
            std::make_pair(GpsInfinitePast(), GpsInfinitePast()));
}

TEST_F(LeapTableTest, ModernUtcEpoch) {
  // Ok to convert to TAI at the modern UTC epoch.
  EXPECT_EQ(lt_->Smear(TaiModernUtcEpoch()), ModernUtcEpoch());
  EXPECT_EQ(lt_->Unsmear(ModernUtcEpoch()), TaiModernUtcEpoch());
  EXPECT_EQ(lt_->FutureProofSmear(TaiModernUtcEpoch()),
            std::make_pair(ModernUtcEpoch(), ModernUtcEpoch()));
  EXPECT_EQ(lt_->FutureProofUnsmear(ModernUtcEpoch()),
            std::make_pair(TaiModernUtcEpoch(), TaiModernUtcEpoch()));

  // Not ok to convert to GPST at the modern UTC epoch.
  EXPECT_EQ(lt_->UnsmearToGps(ModernUtcEpoch()), absl::nullopt);
  EXPECT_EQ(lt_->FutureProofUnsmearToGps(ModernUtcEpoch()),
            std::make_pair(GpsInfinitePast(), GpsInfiniteFuture()));

  // Not ok to convert to TAI before the modern UTC epoch.
  EXPECT_EQ(lt_->Smear(TaiModernUtcEpoch() - Seconds(1)), absl::nullopt);
  EXPECT_EQ(lt_->Unsmear(ModernUtcEpoch() - absl::Seconds(1)), absl::nullopt);
  EXPECT_EQ(lt_->FutureProofSmear(TaiModernUtcEpoch() - Seconds(1)),
            std::make_pair(absl::InfinitePast(), absl::InfiniteFuture()));
  EXPECT_EQ(lt_->FutureProofUnsmear(ModernUtcEpoch() - absl::Seconds(1)),
            std::make_pair(TaiInfinitePast(), TaiInfiniteFuture()));
}

TEST_F(LeapTableTest, GpsEpoch) {
  // Ok to convert at the GPST epoch.
  EXPECT_EQ(lt_->Smear(GpsEpoch()), UtcGpsEpoch());
  EXPECT_EQ(lt_->Smear(TaiGpsEpoch()), UtcGpsEpoch());
  EXPECT_EQ(lt_->Unsmear(UtcGpsEpoch()), TaiGpsEpoch());
  EXPECT_EQ(lt_->UnsmearToGps(UtcGpsEpoch()), GpsEpoch());
  EXPECT_EQ(lt_->FutureProofSmear(GpsEpoch()),
            std::make_pair(UtcGpsEpoch(), UtcGpsEpoch()));
  EXPECT_EQ(lt_->FutureProofSmear(TaiGpsEpoch()),
            std::make_pair(UtcGpsEpoch(), UtcGpsEpoch()));
  EXPECT_EQ(lt_->FutureProofUnsmear(UtcGpsEpoch()),
            std::make_pair(TaiGpsEpoch(), TaiGpsEpoch()));
  EXPECT_EQ(lt_->FutureProofUnsmearToGps(UtcGpsEpoch()),
            std::make_pair(GpsEpoch(), GpsEpoch()));

  // Not ok to convert before the GPST epoch.
  EXPECT_EQ(lt_->Smear(GpsEpoch() - Seconds(1)), absl::nullopt);
  EXPECT_EQ(lt_->UnsmearToGps(UtcGpsEpoch() - absl::Seconds(1)), absl::nullopt);
  EXPECT_EQ(lt_->FutureProofSmear(GpsEpoch() - Seconds(1)),
            std::make_pair(absl::InfinitePast(), absl::InfiniteFuture()));
  EXPECT_EQ(lt_->FutureProofUnsmearToGps(UtcGpsEpoch() - absl::Seconds(1)),
            std::make_pair(GpsInfinitePast(), GpsInfiniteFuture()));
}

TEST_F(LeapTableTest, RoundTrip) {
  // This range crosses a leap smear and is within the leap table validity.
  absl::Time start = Noon(1973, 6, 30) - absl::Minutes(1);
  absl::Time end = Noon(1973, 7, 1) + absl::Minutes(1);
  for (absl::Time t = start; t < end; t += absl::Seconds(10)) {
    SCOPED_TRACE(t);
    // Use ASSERT instead of EXPECT here to avoid having thousands of
    // test failure messages.

    auto unsmeared = lt_->Unsmear(t);
    ASSERT_TRUE(unsmeared.has_value());
    auto unsmeared_interval = lt_->FutureProofUnsmear(t);
    ASSERT_EQ(unsmeared_interval, std::make_pair(*unsmeared, *unsmeared));

    auto smeared = lt_->Smear(*unsmeared);
    ASSERT_TRUE(smeared.has_value());
    auto smeared_interval = lt_->FutureProofSmear(*unsmeared);
    ASSERT_EQ(smeared_interval, std::make_pair(t, t))
        << t << "unsmeared to " << *unsmeared;
  }
}

TEST_F(LeapTableTest, PastExpiration) {
  // The exact moment of expiration is precisely convertible.
  auto utc = lt_->expiration();
  auto tai = expiration_tai();
  EXPECT_EQ(lt_->Unsmear(utc), tai) << utc;
  EXPECT_EQ(lt_->Smear(tai), utc) << tai;
  EXPECT_EQ(lt_->FutureProofUnsmear(utc), std::make_pair(tai, tai)) << utc;
  EXPECT_EQ(lt_->FutureProofSmear(tai), std::make_pair(utc, utc)) << tai;

  // A smear follows immediately.  Every six hours during the smear adds ±250 ms
  // TT of uncertainty.
  for (int i = 1; i < 5; ++i) {
    SCOPED_TRACE(i);
    utc += absl::Hours(6);
    tai += Hours(6);
    EXPECT_EQ(lt_->Unsmear(utc), absl::nullopt) << utc;
    EXPECT_EQ(lt_->FutureProofUnsmear(utc),
              std::make_pair(tai - Milliseconds(i * 250),
                             tai + Milliseconds(i * 250)))
        << utc;
  }

  // Several days later, there is still 1 s added, since there has been no new
  // leap second opportunity.
  utc = lt_->expiration() + 3 * absl::Hours(24);
  tai = expiration_tai() + 3 * Hours(24);
  EXPECT_EQ(lt_->Unsmear(utc), absl::nullopt) << utc;
  EXPECT_EQ(lt_->FutureProofUnsmear(utc),
            std::make_pair(tai - Seconds(1), tai + Seconds(1)))
      << utc;

  // In the middle of the next month, we've passed another possible leap second.
  utc = lt_->expiration() + 45 * absl::Hours(24);
  tai = expiration_tai() + 45 * Hours(24);
  EXPECT_EQ(lt_->Unsmear(utc), absl::nullopt) << utc;
  EXPECT_EQ(lt_->FutureProofUnsmear(utc),
            std::make_pair(tai - Seconds(2), tai + Seconds(2)))
      << utc;
}

TEST_F(LeapTableTest, ToProto) {
  LeapTableProto proto2;
  lt_->ToProto(&proto2);

  std::string diffs;
  google::protobuf::util::MessageDifferencer differencer;
  differencer.set_report_matches(true);
  differencer.ReportDifferencesToString(&diffs);
  ASSERT_TRUE(differencer.Compare(proto_, proto2)) << diffs;

  auto lt2 = NewLeapTableFromProto(proto2);
  ASSERT_TRUE(lt2 != nullptr);
  ASSERT_EQ(*lt_, *lt2);
}

TEST_F(LeapTableTest, EqualityOperators) {
  EXPECT_TRUE(*lt_ == *lt_);
  EXPECT_FALSE(*lt_ != *lt_);

  LeapTableProto proto2;
  proto2.add_positive_leaps(2441499);  // 1972-06-30 12:00:00 UTC
  proto2.set_end_jdn(2442412);         // 1974-12-30 12:00:00 UTC
  auto lt2 = NewLeapTableFromProto(proto2);
  ASSERT_TRUE(lt2 != nullptr);

  EXPECT_FALSE(*lt_ == *lt2);
  EXPECT_FALSE(*lt2 == *lt_);
  EXPECT_TRUE(*lt_ != *lt2);
  EXPECT_TRUE(*lt2 != *lt_);
}

TEST(InvalidLeapTableTest, DuplicateLeap) {
  LeapTableProto proto;
  proto.add_positive_leaps(2441499);  // 1972-06-30 12:00:00 UTC
  proto.add_positive_leaps(2441499);  // 1972-06-30 12:00:00 UTC
  proto.set_end_jdn(2442412);         // 1974-12-30 12:00:00 UTC
  ASSERT_TRUE(NewLeapTableFromProto(proto) == nullptr);
}

TEST(InvalidLeapTableTest, ConflictingLeapSign) {
  LeapTableProto proto;
  proto.add_positive_leaps(2441499);  // 1972-06-30 12:00:00 UTC
  proto.add_negative_leaps(2441499);  // 1972-06-30 12:00:00 UTC
  proto.set_end_jdn(2442412);         // 1974-12-30 12:00:00 UTC
  ASSERT_TRUE(NewLeapTableFromProto(proto) == nullptr);
}

TEST(InvalidLeapTableTest, LeapNotAtEndOfMonth) {
  LeapTableProto proto;
  proto.add_positive_leaps(2441500);  // 1972-07-01 12:00:00 UTC
  proto.set_end_jdn(2442412);         // 1974-12-30 12:00:00 UTC
  ASSERT_TRUE(NewLeapTableFromProto(proto) == nullptr);
}

TEST(InvalidLeapTableTest, ExpirationNotAtEndOfMonth) {
  LeapTableProto proto;
  proto.add_positive_leaps(2441499);  // 1972-06-30 12:00:00 UTC
  proto.set_end_jdn(2442413);         // 1974-12-31 12:00:00 UTC
  ASSERT_TRUE(NewLeapTableFromProto(proto) == nullptr);
}

TEST(InvalidLeapTableTest, LeapAfterExpiration) {
  LeapTableProto proto;
  proto.add_positive_leaps(2442412);  // 1974-12-31 12:00:00 UTC
  proto.set_end_jdn(2441498);         // 1972-06-29 12:00:00 UTC
  ASSERT_TRUE(NewLeapTableFromProto(proto) == nullptr);
}

TEST(InvalidLeapTableTest, MissingExpiration) {
  LeapTableProto proto;
  proto.add_positive_leaps(2442412);  // 1974-12-30 12:00:00 UTC
  ASSERT_TRUE(NewLeapTableFromProto(proto) == nullptr);
}

TEST(InvalidLeapTableTest, ExpirationTooLate) {
  LeapTableProto proto;
  proto.add_positive_leaps(2442412);  // 1974-12-30 12:00:00 UTC
  proto.set_end_jdn(7654321);         // 16244-09-19 12:00:00 UTC
  ASSERT_TRUE(NewLeapTableFromProto(proto) == nullptr);
}

TEST(AdjacentLeapSecondsTest, AdjacentLeapSeconds) {
  LeapTableProto pb;
  pb.add_positive_leaps(2441348);  // 1972-01-31
  pb.add_positive_leaps(2441377);  // 1972-02-29
  pb.add_negative_leaps(2441408);  // 1972-03-31
  pb.add_positive_leaps(2441438);  // 1972-04-30
  pb.set_end_jdn(2441468);         // 1972-05-30
  auto lt = NewLeapTableFromProto(pb);
  ASSERT_TRUE(lt != nullptr);
}

}  // namespace
}  // namespace unsmear

// Copyright 2017 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// This test borrows heavily from absl/time/duration_test.cc.

#include "unsmear/unsmear.h"

#include <cmath>
#include <cstdint>
#include <string>
#include "absl/time/time.h"
#include "gtest/gtest.h"

namespace {

constexpr int64_t kint64max = std::numeric_limits<int64_t>::max();
constexpr int64_t kint64min = std::numeric_limits<int64_t>::min();

// Approximates the given number of years. This is only used to make some test
// code more readable.
unsmear::Duration ApproxYears(double n) { return unsmear::Hours(n) * 365 * 24; }

TEST(Duration, ValueSemantics) {
  // If this compiles, the test passes.
  constexpr unsmear::Duration a;      // Default construction
  constexpr unsmear::Duration b = a;  // Copy construction
  constexpr unsmear::Duration c(b);   // Copy construction (again)

  unsmear::Duration d;
  d = c;  // Assignment
}

TEST(Duration, Factories) {
  constexpr unsmear::Duration zero = unsmear::ZeroDuration();
  constexpr unsmear::Duration nano = unsmear::Nanoseconds(1);
  constexpr unsmear::Duration micro = unsmear::Microseconds(1);
  constexpr unsmear::Duration milli = unsmear::Milliseconds(1);
  constexpr unsmear::Duration sec = unsmear::Seconds(1);
  constexpr unsmear::Duration min = unsmear::Minutes(1);
  constexpr unsmear::Duration hour = unsmear::Hours(1);

  EXPECT_EQ(zero, unsmear::Duration());
  EXPECT_EQ(zero, unsmear::Seconds(0));
  EXPECT_EQ(nano, unsmear::Nanoseconds(1));
  EXPECT_EQ(micro, unsmear::Nanoseconds(1000));
  EXPECT_EQ(milli, unsmear::Microseconds(1000));
  EXPECT_EQ(sec, unsmear::Milliseconds(1000));
  EXPECT_EQ(min, unsmear::Seconds(60));
  EXPECT_EQ(hour, unsmear::Minutes(60));

  // Tests factory limits
  const unsmear::Duration inf = unsmear::InfiniteDuration();

  EXPECT_GT(inf, unsmear::Seconds(kint64max));
  EXPECT_LT(-inf, unsmear::Seconds(kint64min));
  EXPECT_LT(-inf, unsmear::Seconds(-kint64max));

  EXPECT_EQ(inf, unsmear::Minutes(kint64max));
  EXPECT_EQ(-inf, unsmear::Minutes(kint64min));
  EXPECT_EQ(-inf, unsmear::Minutes(-kint64max));
  EXPECT_GT(inf, unsmear::Minutes(kint64max / 60));
  EXPECT_LT(-inf, unsmear::Minutes(kint64min / 60));
  EXPECT_LT(-inf, unsmear::Minutes(-kint64max / 60));

  EXPECT_EQ(inf, unsmear::Hours(kint64max));
  EXPECT_EQ(-inf, unsmear::Hours(kint64min));
  EXPECT_EQ(-inf, unsmear::Hours(-kint64max));
  EXPECT_GT(inf, unsmear::Hours(kint64max / 3600));
  EXPECT_LT(-inf, unsmear::Hours(kint64min / 3600));
  EXPECT_LT(-inf, unsmear::Hours(-kint64max / 3600));
}

TEST(Duration, ToConversion) {
#define TEST_DURATION_CONVERSION(UNIT)                              \
  do {                                                              \
    const unsmear::Duration d = unsmear::UNIT(1.5);                 \
    const unsmear::Duration z = unsmear::ZeroDuration();            \
    const unsmear::Duration inf = unsmear::InfiniteDuration();      \
    const double dbl_inf = std::numeric_limits<double>::infinity(); \
    EXPECT_EQ(kint64min, unsmear::ToInt64##UNIT(-inf));             \
    EXPECT_EQ(-1, unsmear::ToInt64##UNIT(-d));                      \
    EXPECT_EQ(0, unsmear::ToInt64##UNIT(z));                        \
    EXPECT_EQ(1, unsmear::ToInt64##UNIT(d));                        \
    EXPECT_EQ(kint64max, unsmear::ToInt64##UNIT(inf));              \
    EXPECT_EQ(-dbl_inf, unsmear::ToDouble##UNIT(-inf));             \
    EXPECT_EQ(-1.5, unsmear::ToDouble##UNIT(-d));                   \
    EXPECT_EQ(0, unsmear::ToDouble##UNIT(z));                       \
    EXPECT_EQ(1.5, unsmear::ToDouble##UNIT(d));                     \
    EXPECT_EQ(dbl_inf, unsmear::ToDouble##UNIT(inf));               \
  } while (0)

  TEST_DURATION_CONVERSION(Nanoseconds);
  TEST_DURATION_CONVERSION(Microseconds);
  TEST_DURATION_CONVERSION(Milliseconds);
  TEST_DURATION_CONVERSION(Seconds);
  TEST_DURATION_CONVERSION(Minutes);
  TEST_DURATION_CONVERSION(Hours);

#undef TEST_DURATION_CONVERSION
}

template <int64_t N>
void TestToConversion() {
  constexpr unsmear::Duration nano = unsmear::Nanoseconds(N);
  EXPECT_EQ(N, unsmear::ToInt64Nanoseconds(nano));
  EXPECT_EQ(0, unsmear::ToInt64Microseconds(nano));
  EXPECT_EQ(0, unsmear::ToInt64Milliseconds(nano));
  EXPECT_EQ(0, unsmear::ToInt64Seconds(nano));
  EXPECT_EQ(0, unsmear::ToInt64Minutes(nano));
  EXPECT_EQ(0, unsmear::ToInt64Hours(nano));
  const unsmear::Duration micro = unsmear::Microseconds(N);
  EXPECT_EQ(N * 1000, unsmear::ToInt64Nanoseconds(micro));
  EXPECT_EQ(N, unsmear::ToInt64Microseconds(micro));
  EXPECT_EQ(0, unsmear::ToInt64Milliseconds(micro));
  EXPECT_EQ(0, unsmear::ToInt64Seconds(micro));
  EXPECT_EQ(0, unsmear::ToInt64Minutes(micro));
  EXPECT_EQ(0, unsmear::ToInt64Hours(micro));
  const unsmear::Duration milli = unsmear::Milliseconds(N);
  EXPECT_EQ(N * 1000 * 1000, unsmear::ToInt64Nanoseconds(milli));
  EXPECT_EQ(N * 1000, unsmear::ToInt64Microseconds(milli));
  EXPECT_EQ(N, unsmear::ToInt64Milliseconds(milli));
  EXPECT_EQ(0, unsmear::ToInt64Seconds(milli));
  EXPECT_EQ(0, unsmear::ToInt64Minutes(milli));
  EXPECT_EQ(0, unsmear::ToInt64Hours(milli));
  const unsmear::Duration sec = unsmear::Seconds(N);
  EXPECT_EQ(N * 1000 * 1000 * 1000, unsmear::ToInt64Nanoseconds(sec));
  EXPECT_EQ(N * 1000 * 1000, unsmear::ToInt64Microseconds(sec));
  EXPECT_EQ(N * 1000, unsmear::ToInt64Milliseconds(sec));
  EXPECT_EQ(N, unsmear::ToInt64Seconds(sec));
  EXPECT_EQ(0, unsmear::ToInt64Minutes(sec));
  EXPECT_EQ(0, unsmear::ToInt64Hours(sec));
  const unsmear::Duration min = unsmear::Minutes(N);
  EXPECT_EQ(N * 60 * 1000 * 1000 * 1000, unsmear::ToInt64Nanoseconds(min));
  EXPECT_EQ(N * 60 * 1000 * 1000, unsmear::ToInt64Microseconds(min));
  EXPECT_EQ(N * 60 * 1000, unsmear::ToInt64Milliseconds(min));
  EXPECT_EQ(N * 60, unsmear::ToInt64Seconds(min));
  EXPECT_EQ(N, unsmear::ToInt64Minutes(min));
  EXPECT_EQ(0, unsmear::ToInt64Hours(min));
  const unsmear::Duration hour = unsmear::Hours(N);
  EXPECT_EQ(N * 60 * 60 * 1000 * 1000 * 1000,
            unsmear::ToInt64Nanoseconds(hour));
  EXPECT_EQ(N * 60 * 60 * 1000 * 1000, unsmear::ToInt64Microseconds(hour));
  EXPECT_EQ(N * 60 * 60 * 1000, unsmear::ToInt64Milliseconds(hour));
  EXPECT_EQ(N * 60 * 60, unsmear::ToInt64Seconds(hour));
  EXPECT_EQ(N * 60, unsmear::ToInt64Minutes(hour));
  EXPECT_EQ(N, unsmear::ToInt64Hours(hour));
}

TEST(Duration, ToConversionDeprecated) {
  TestToConversion<43>();
  TestToConversion<1>();
  TestToConversion<0>();
  TestToConversion<-1>();
  TestToConversion<-43>();
}

// Used for testing the factory overloads.
template <typename T>
struct ImplicitlyConvertible {
  T n_;
  explicit ImplicitlyConvertible(T n) : n_(n) {}
  // Marking this conversion operator with 'explicit' will cause the test to
  // fail (as desired).
  operator T() { return n_; }
};

TEST(Duration, FactoryOverloads) {
#define TEST_FACTORY_OVERLOADS(NAME)                                        \
  EXPECT_EQ(1, NAME(static_cast<int8_t>(1)) / NAME(1));                     \
  EXPECT_EQ(1, NAME(static_cast<int16_t>(1)) / NAME(1));                    \
  EXPECT_EQ(1, NAME(static_cast<int32_t>(1)) / NAME(1));                    \
  EXPECT_EQ(1, NAME(static_cast<int64_t>(1)) / NAME(1));                    \
  EXPECT_EQ(1, NAME(static_cast<uint8_t>(1)) / NAME(1));                    \
  EXPECT_EQ(1, NAME(static_cast<uint16_t>(1)) / NAME(1));                   \
  EXPECT_EQ(1, NAME(static_cast<uint32_t>(1)) / NAME(1));                   \
  EXPECT_EQ(1, NAME(static_cast<uint64_t>(1)) / NAME(1));                   \
  EXPECT_EQ(1, NAME(ImplicitlyConvertible<int8_t>(1)) / NAME(1));           \
  EXPECT_EQ(1, NAME(ImplicitlyConvertible<int16_t>(1)) / NAME(1));          \
  EXPECT_EQ(1, NAME(ImplicitlyConvertible<int32_t>(1)) / NAME(1));          \
  EXPECT_EQ(1, NAME(ImplicitlyConvertible<int64_t>(1)) / NAME(1));          \
  EXPECT_EQ(1, NAME(ImplicitlyConvertible<uint8_t>(1)) / NAME(1));          \
  EXPECT_EQ(1, NAME(ImplicitlyConvertible<uint16_t>(1)) / NAME(1));         \
  EXPECT_EQ(1, NAME(ImplicitlyConvertible<uint32_t>(1)) / NAME(1));         \
  EXPECT_EQ(1, NAME(ImplicitlyConvertible<uint64_t>(1)) / NAME(1));         \
  EXPECT_EQ(NAME(1) / 2, NAME(static_cast<float>(0.5)));                    \
  EXPECT_EQ(NAME(1) / 2, NAME(static_cast<double>(0.5)));                   \
  EXPECT_EQ(1.5,                                                            \
            unsmear::FDivDuration(NAME(static_cast<float>(1.5)), NAME(1))); \
  EXPECT_EQ(1.5,                                                            \
            unsmear::FDivDuration(NAME(static_cast<double>(1.5)), NAME(1)));

  TEST_FACTORY_OVERLOADS(unsmear::Nanoseconds);
  TEST_FACTORY_OVERLOADS(unsmear::Microseconds);
  TEST_FACTORY_OVERLOADS(unsmear::Milliseconds);
  TEST_FACTORY_OVERLOADS(unsmear::Seconds);
  TEST_FACTORY_OVERLOADS(unsmear::Minutes);
  TEST_FACTORY_OVERLOADS(unsmear::Hours);

#undef TEST_FACTORY_OVERLOADS

  EXPECT_EQ(unsmear::Milliseconds(1500), unsmear::Seconds(1.5));
  EXPECT_LT(unsmear::Nanoseconds(1), unsmear::Nanoseconds(1.5));
  EXPECT_GT(unsmear::Nanoseconds(2), unsmear::Nanoseconds(1.5));

  const double dbl_inf = std::numeric_limits<double>::infinity();
  EXPECT_EQ(unsmear::InfiniteDuration(), unsmear::Nanoseconds(dbl_inf));
  EXPECT_EQ(unsmear::InfiniteDuration(), unsmear::Microseconds(dbl_inf));
  EXPECT_EQ(unsmear::InfiniteDuration(), unsmear::Milliseconds(dbl_inf));
  EXPECT_EQ(unsmear::InfiniteDuration(), unsmear::Seconds(dbl_inf));
  EXPECT_EQ(unsmear::InfiniteDuration(), unsmear::Minutes(dbl_inf));
  EXPECT_EQ(unsmear::InfiniteDuration(), unsmear::Hours(dbl_inf));
  EXPECT_EQ(-unsmear::InfiniteDuration(), unsmear::Nanoseconds(-dbl_inf));
  EXPECT_EQ(-unsmear::InfiniteDuration(), unsmear::Microseconds(-dbl_inf));
  EXPECT_EQ(-unsmear::InfiniteDuration(), unsmear::Milliseconds(-dbl_inf));
  EXPECT_EQ(-unsmear::InfiniteDuration(), unsmear::Seconds(-dbl_inf));
  EXPECT_EQ(-unsmear::InfiniteDuration(), unsmear::Minutes(-dbl_inf));
  EXPECT_EQ(-unsmear::InfiniteDuration(), unsmear::Hours(-dbl_inf));
}

TEST(Duration, InfinityExamples) {
  // These examples are used in the documentation in time.h. They are
  // written so that they can be copy-n-pasted easily.

  constexpr unsmear::Duration inf = unsmear::InfiniteDuration();
  constexpr unsmear::Duration d = unsmear::Seconds(1);  // Any finite duration

  EXPECT_TRUE(inf == inf + inf);
  EXPECT_TRUE(inf == inf + d);
  EXPECT_TRUE(inf == inf - inf);
  EXPECT_TRUE(-inf == d - inf);

  EXPECT_TRUE(inf == d * 1e100);
  EXPECT_TRUE(0 == d / inf);  // NOLINT(readability/check)

  // Division by zero returns infinity, or kint64min/MAX where necessary.
  EXPECT_TRUE(inf == d / 0);
  EXPECT_TRUE(kint64max == d / unsmear::ZeroDuration());
}

TEST(Duration, InfinityComparison) {
  const unsmear::Duration inf = unsmear::InfiniteDuration();
  const unsmear::Duration any_dur = unsmear::Seconds(1);

  // Equality
  EXPECT_EQ(inf, inf);
  EXPECT_EQ(-inf, -inf);
  EXPECT_NE(inf, -inf);
  EXPECT_NE(any_dur, inf);
  EXPECT_NE(any_dur, -inf);

  // Relational
  EXPECT_GT(inf, any_dur);
  EXPECT_LT(-inf, any_dur);
  EXPECT_LT(-inf, inf);
  EXPECT_GT(inf, -inf);
}

TEST(Duration, InfinityAddition) {
  const unsmear::Duration sec_max = unsmear::Seconds(kint64max);
  const unsmear::Duration sec_min = unsmear::Seconds(kint64min);
  const unsmear::Duration any_dur = unsmear::Seconds(1);
  const unsmear::Duration inf = unsmear::InfiniteDuration();

  // Addition
  EXPECT_EQ(inf, inf + inf);
  EXPECT_EQ(inf, inf + -inf);
  EXPECT_EQ(-inf, -inf + inf);
  EXPECT_EQ(-inf, -inf + -inf);

  EXPECT_EQ(inf, inf + any_dur);
  EXPECT_EQ(inf, any_dur + inf);
  EXPECT_EQ(-inf, -inf + any_dur);
  EXPECT_EQ(-inf, any_dur + -inf);

  // Interesting case
  unsmear::Duration almost_inf = sec_max + unsmear::Nanoseconds(999999999);
  EXPECT_GT(inf, almost_inf);
  almost_inf += -unsmear::Nanoseconds(999999999);
  EXPECT_GT(inf, almost_inf);

  // Addition overflow/underflow
  EXPECT_EQ(inf, sec_max + unsmear::Seconds(1));
  EXPECT_EQ(inf, sec_max + sec_max);
  EXPECT_EQ(-inf, sec_min + -unsmear::Seconds(1));
  EXPECT_EQ(-inf, sec_min + -sec_max);

  // For reference: IEEE 754 behavior
  const double dbl_inf = std::numeric_limits<double>::infinity();
  EXPECT_TRUE(std::isinf(dbl_inf + dbl_inf));
  EXPECT_TRUE(std::isnan(dbl_inf + -dbl_inf));  // We return inf
  EXPECT_TRUE(std::isnan(-dbl_inf + dbl_inf));  // We return inf
  EXPECT_TRUE(std::isinf(-dbl_inf + -dbl_inf));
}

TEST(Duration, InfinitySubtraction) {
  const unsmear::Duration sec_max = unsmear::Seconds(kint64max);
  const unsmear::Duration sec_min = unsmear::Seconds(kint64min);
  const unsmear::Duration any_dur = unsmear::Seconds(1);
  const unsmear::Duration inf = unsmear::InfiniteDuration();

  // Subtraction
  EXPECT_EQ(inf, inf - inf);
  EXPECT_EQ(inf, inf - -inf);
  EXPECT_EQ(-inf, -inf - inf);
  EXPECT_EQ(-inf, -inf - -inf);

  EXPECT_EQ(inf, inf - any_dur);
  EXPECT_EQ(-inf, any_dur - inf);
  EXPECT_EQ(-inf, -inf - any_dur);
  EXPECT_EQ(inf, any_dur - -inf);

  // Subtraction overflow/underflow
  EXPECT_EQ(inf, sec_max - -unsmear::Seconds(1));
  EXPECT_EQ(inf, sec_max - -sec_max);
  EXPECT_EQ(-inf, sec_min - unsmear::Seconds(1));
  EXPECT_EQ(-inf, sec_min - sec_max);

  // Interesting case
  unsmear::Duration almost_neg_inf = sec_min;
  EXPECT_LT(-inf, almost_neg_inf);
  almost_neg_inf -= -unsmear::Nanoseconds(1);
  EXPECT_LT(-inf, almost_neg_inf);

  // For reference: IEEE 754 behavior
  const double dbl_inf = std::numeric_limits<double>::infinity();
  EXPECT_TRUE(std::isnan(dbl_inf - dbl_inf));  // We return inf
  EXPECT_TRUE(std::isinf(dbl_inf - -dbl_inf));
  EXPECT_TRUE(std::isinf(-dbl_inf - dbl_inf));
  EXPECT_TRUE(std::isnan(-dbl_inf - -dbl_inf));  // We return inf
}

TEST(Duration, InfinityMultiplication) {
  const unsmear::Duration sec_max = unsmear::Seconds(kint64max);
  const unsmear::Duration sec_min = unsmear::Seconds(kint64min);
  const unsmear::Duration inf = unsmear::InfiniteDuration();

#define TEST_INF_MUL_WITH_TYPE(T)                                    \
  EXPECT_EQ(inf, inf* static_cast<T>(2));                            \
  EXPECT_EQ(-inf, inf* static_cast<T>(-2));                          \
  EXPECT_EQ(-inf, -inf* static_cast<T>(2));                          \
  EXPECT_EQ(inf, -inf* static_cast<T>(-2));                          \
  EXPECT_EQ(inf, inf* static_cast<T>(0));                            \
  EXPECT_EQ(-inf, -inf* static_cast<T>(0));                          \
  EXPECT_EQ(inf, sec_max* static_cast<T>(2));                        \
  EXPECT_EQ(inf, sec_min* static_cast<T>(-2));                       \
  EXPECT_EQ(inf, (sec_max / static_cast<T>(2)) * static_cast<T>(3)); \
  EXPECT_EQ(-inf, sec_max* static_cast<T>(-2));                      \
  EXPECT_EQ(-inf, sec_min* static_cast<T>(2));                       \
  EXPECT_EQ(-inf, (sec_min / static_cast<T>(2)) * static_cast<T>(3));

  TEST_INF_MUL_WITH_TYPE(int64_t);  // NOLINT(readability/function)
  TEST_INF_MUL_WITH_TYPE(double);   // NOLINT(readability/function)

#undef TEST_INF_MUL_WITH_TYPE

  const double dbl_inf = std::numeric_limits<double>::infinity();
  EXPECT_EQ(inf, inf * dbl_inf);
  EXPECT_EQ(-inf, -inf * dbl_inf);
  EXPECT_EQ(-inf, inf * -dbl_inf);
  EXPECT_EQ(inf, -inf * -dbl_inf);

  const unsmear::Duration any_dur = unsmear::Seconds(1);
  EXPECT_EQ(inf, any_dur * dbl_inf);
  EXPECT_EQ(-inf, -any_dur * dbl_inf);
  EXPECT_EQ(-inf, any_dur * -dbl_inf);
  EXPECT_EQ(inf, -any_dur * -dbl_inf);

  // Fixed-point multiplication will produce a finite value, whereas floating
  // point fuzziness will overflow to inf.
  EXPECT_NE(unsmear::InfiniteDuration(), unsmear::Seconds(1) * kint64max);
  EXPECT_EQ(inf, unsmear::Seconds(1) * static_cast<double>(kint64max));
  EXPECT_NE(-unsmear::InfiniteDuration(), unsmear::Seconds(1) * kint64min);
  EXPECT_EQ(-inf, unsmear::Seconds(1) * static_cast<double>(kint64min));

  // Note that sec_max * or / by 1.0 overflows to inf due to the 53-bit
  // limitations of double.
  EXPECT_NE(inf, sec_max);
  EXPECT_NE(inf, sec_max / 1);
  EXPECT_EQ(inf, sec_max / 1.0);
  EXPECT_NE(inf, sec_max * 1);
  EXPECT_EQ(inf, sec_max * 1.0);
}

TEST(Duration, InfinityDivision) {
  const unsmear::Duration sec_max = unsmear::Seconds(kint64max);
  const unsmear::Duration sec_min = unsmear::Seconds(kint64min);
  const unsmear::Duration inf = unsmear::InfiniteDuration();

  // Division of Duration by a double
#define TEST_INF_DIV_WITH_TYPE(T)            \
  EXPECT_EQ(inf, inf / static_cast<T>(2));   \
  EXPECT_EQ(-inf, inf / static_cast<T>(-2)); \
  EXPECT_EQ(-inf, -inf / static_cast<T>(2)); \
  EXPECT_EQ(inf, -inf / static_cast<T>(-2));

  TEST_INF_DIV_WITH_TYPE(int64_t);  // NOLINT(readability/function)
  TEST_INF_DIV_WITH_TYPE(double);   // NOLINT(readability/function)

#undef TEST_INF_DIV_WITH_TYPE

  // Division of Duration by a double overflow/underflow
  EXPECT_EQ(inf, sec_max / 0.5);
  EXPECT_EQ(inf, sec_min / -0.5);
  EXPECT_EQ(inf, ((sec_max / 0.5) + unsmear::Seconds(1)) / 0.5);
  EXPECT_EQ(-inf, sec_max / -0.5);
  EXPECT_EQ(-inf, sec_min / 0.5);
  EXPECT_EQ(-inf, ((sec_min / 0.5) - unsmear::Seconds(1)) / 0.5);

  const double dbl_inf = std::numeric_limits<double>::infinity();
  EXPECT_EQ(inf, inf / dbl_inf);
  EXPECT_EQ(-inf, inf / -dbl_inf);
  EXPECT_EQ(-inf, -inf / dbl_inf);
  EXPECT_EQ(inf, -inf / -dbl_inf);

  const unsmear::Duration any_dur = unsmear::Seconds(1);
  EXPECT_EQ(unsmear::ZeroDuration(), any_dur / dbl_inf);
  EXPECT_EQ(unsmear::ZeroDuration(), any_dur / -dbl_inf);
  EXPECT_EQ(unsmear::ZeroDuration(), -any_dur / dbl_inf);
  EXPECT_EQ(unsmear::ZeroDuration(), -any_dur / -dbl_inf);
}

TEST(Duration, InfinityModulus) {
  const unsmear::Duration sec_max = unsmear::Seconds(kint64max);
  const unsmear::Duration any_dur = unsmear::Seconds(1);
  const unsmear::Duration inf = unsmear::InfiniteDuration();

  EXPECT_EQ(inf, inf % inf);
  EXPECT_EQ(inf, inf % -inf);
  EXPECT_EQ(-inf, -inf % -inf);
  EXPECT_EQ(-inf, -inf % inf);

  EXPECT_EQ(any_dur, any_dur % inf);
  EXPECT_EQ(any_dur, any_dur % -inf);
  EXPECT_EQ(-any_dur, -any_dur % inf);
  EXPECT_EQ(-any_dur, -any_dur % -inf);

  EXPECT_EQ(inf, inf % -any_dur);
  EXPECT_EQ(inf, inf % any_dur);
  EXPECT_EQ(-inf, -inf % -any_dur);
  EXPECT_EQ(-inf, -inf % any_dur);

  // Remainder isn't affected by overflow.
  EXPECT_EQ(unsmear::ZeroDuration(), sec_max % unsmear::Seconds(1));
  EXPECT_EQ(unsmear::ZeroDuration(), sec_max % unsmear::Milliseconds(1));
  EXPECT_EQ(unsmear::ZeroDuration(), sec_max % unsmear::Microseconds(1));
  EXPECT_EQ(unsmear::ZeroDuration(), sec_max % unsmear::Nanoseconds(1));
  EXPECT_EQ(unsmear::ZeroDuration(), sec_max % unsmear::Nanoseconds(1) / 4);
}

TEST(Duration, InfinityIDiv) {
  const unsmear::Duration sec_max = unsmear::Seconds(kint64max);
  const unsmear::Duration any_dur = unsmear::Seconds(1);
  const unsmear::Duration inf = unsmear::InfiniteDuration();
  const double dbl_inf = std::numeric_limits<double>::infinity();

  // IDivDuration (int64_t return value + a remainer)
  unsmear::Duration rem = unsmear::ZeroDuration();
  EXPECT_EQ(kint64max, unsmear::IDivDuration(inf, inf, &rem));
  EXPECT_EQ(inf, rem);

  rem = unsmear::ZeroDuration();
  EXPECT_EQ(kint64max, unsmear::IDivDuration(-inf, -inf, &rem));
  EXPECT_EQ(-inf, rem);

  rem = unsmear::ZeroDuration();
  EXPECT_EQ(kint64max, unsmear::IDivDuration(inf, any_dur, &rem));
  EXPECT_EQ(inf, rem);

  rem = unsmear::ZeroDuration();
  EXPECT_EQ(0, unsmear::IDivDuration(any_dur, inf, &rem));
  EXPECT_EQ(any_dur, rem);

  rem = unsmear::ZeroDuration();
  EXPECT_EQ(kint64max, unsmear::IDivDuration(-inf, -any_dur, &rem));
  EXPECT_EQ(-inf, rem);

  rem = unsmear::ZeroDuration();
  EXPECT_EQ(0, unsmear::IDivDuration(-any_dur, -inf, &rem));
  EXPECT_EQ(-any_dur, rem);

  rem = unsmear::ZeroDuration();
  EXPECT_EQ(kint64min, unsmear::IDivDuration(-inf, inf, &rem));
  EXPECT_EQ(-inf, rem);

  rem = unsmear::ZeroDuration();
  EXPECT_EQ(kint64min, unsmear::IDivDuration(inf, -inf, &rem));
  EXPECT_EQ(inf, rem);

  rem = unsmear::ZeroDuration();
  EXPECT_EQ(kint64min, unsmear::IDivDuration(-inf, any_dur, &rem));
  EXPECT_EQ(-inf, rem);

  rem = unsmear::ZeroDuration();
  EXPECT_EQ(0, unsmear::IDivDuration(-any_dur, inf, &rem));
  EXPECT_EQ(-any_dur, rem);

  rem = unsmear::ZeroDuration();
  EXPECT_EQ(kint64min, unsmear::IDivDuration(inf, -any_dur, &rem));
  EXPECT_EQ(inf, rem);

  rem = unsmear::ZeroDuration();
  EXPECT_EQ(0, unsmear::IDivDuration(any_dur, -inf, &rem));
  EXPECT_EQ(any_dur, rem);

  // IDivDuration overflow/underflow
  rem = any_dur;
  EXPECT_EQ(kint64max,
            unsmear::IDivDuration(sec_max, unsmear::Nanoseconds(1) / 4, &rem));
  EXPECT_EQ(sec_max - unsmear::Nanoseconds(kint64max) / 4, rem);

  rem = any_dur;
  EXPECT_EQ(kint64max,
            unsmear::IDivDuration(sec_max, unsmear::Milliseconds(1), &rem));
  EXPECT_EQ(sec_max - unsmear::Milliseconds(kint64max), rem);

  rem = any_dur;
  EXPECT_EQ(kint64max,
            unsmear::IDivDuration(-sec_max, -unsmear::Milliseconds(1), &rem));
  EXPECT_EQ(-sec_max + unsmear::Milliseconds(kint64max), rem);

  rem = any_dur;
  EXPECT_EQ(kint64min,
            unsmear::IDivDuration(-sec_max, unsmear::Milliseconds(1), &rem));
  EXPECT_EQ(-sec_max - unsmear::Milliseconds(kint64min), rem);

  rem = any_dur;
  EXPECT_EQ(kint64min,
            unsmear::IDivDuration(sec_max, -unsmear::Milliseconds(1), &rem));
  EXPECT_EQ(sec_max + unsmear::Milliseconds(kint64min), rem);

  //
  // operator/(Duration, Duration) is a wrapper for IDivDuration().
  //

  // IEEE 754 says inf / inf should be nan, but int64_t doesn't have
  // nan so we'll return kint64max/kint64min instead.
  EXPECT_TRUE(std::isnan(dbl_inf / dbl_inf));
  EXPECT_EQ(kint64max, inf / inf);
  EXPECT_EQ(kint64max, -inf / -inf);
  EXPECT_EQ(kint64min, -inf / inf);
  EXPECT_EQ(kint64min, inf / -inf);

  EXPECT_TRUE(std::isinf(dbl_inf / 2.0));
  EXPECT_EQ(kint64max, inf / any_dur);
  EXPECT_EQ(kint64max, -inf / -any_dur);
  EXPECT_EQ(kint64min, -inf / any_dur);
  EXPECT_EQ(kint64min, inf / -any_dur);

  EXPECT_EQ(0.0, 2.0 / dbl_inf);
  EXPECT_EQ(0, any_dur / inf);
  EXPECT_EQ(0, any_dur / -inf);
  EXPECT_EQ(0, -any_dur / inf);
  EXPECT_EQ(0, -any_dur / -inf);
  EXPECT_EQ(0, unsmear::ZeroDuration() / inf);

  // Division of Duration by a Duration overflow/underflow
  EXPECT_EQ(kint64max, sec_max / unsmear::Milliseconds(1));
  EXPECT_EQ(kint64max, -sec_max / -unsmear::Milliseconds(1));
  EXPECT_EQ(kint64min, -sec_max / unsmear::Milliseconds(1));
  EXPECT_EQ(kint64min, sec_max / -unsmear::Milliseconds(1));
}

TEST(Duration, InfinityFDiv) {
  const unsmear::Duration any_dur = unsmear::Seconds(1);
  const unsmear::Duration inf = unsmear::InfiniteDuration();
  const double dbl_inf = std::numeric_limits<double>::infinity();

  EXPECT_EQ(dbl_inf, unsmear::FDivDuration(inf, inf));
  EXPECT_EQ(dbl_inf, unsmear::FDivDuration(-inf, -inf));
  EXPECT_EQ(dbl_inf, unsmear::FDivDuration(inf, any_dur));
  EXPECT_EQ(0.0, unsmear::FDivDuration(any_dur, inf));
  EXPECT_EQ(dbl_inf, unsmear::FDivDuration(-inf, -any_dur));
  EXPECT_EQ(0.0, unsmear::FDivDuration(-any_dur, -inf));

  EXPECT_EQ(-dbl_inf, unsmear::FDivDuration(-inf, inf));
  EXPECT_EQ(-dbl_inf, unsmear::FDivDuration(inf, -inf));
  EXPECT_EQ(-dbl_inf, unsmear::FDivDuration(-inf, any_dur));
  EXPECT_EQ(0.0, unsmear::FDivDuration(-any_dur, inf));
  EXPECT_EQ(-dbl_inf, unsmear::FDivDuration(inf, -any_dur));
  EXPECT_EQ(0.0, unsmear::FDivDuration(any_dur, -inf));
}

TEST(Duration, DivisionByZero) {
  const unsmear::Duration zero = unsmear::ZeroDuration();
  const unsmear::Duration inf = unsmear::InfiniteDuration();
  const unsmear::Duration any_dur = unsmear::Seconds(1);
  const double dbl_inf = std::numeric_limits<double>::infinity();
  const double dbl_denorm = std::numeric_limits<double>::denorm_min();

  // IEEE 754 behavior
  double z = 0.0, two = 2.0;
  EXPECT_TRUE(std::isinf(two / z));
  EXPECT_TRUE(std::isnan(z / z));  // We'll return inf

  // Operator/(Duration, double)
  EXPECT_EQ(inf, zero / 0.0);
  EXPECT_EQ(-inf, zero / -0.0);
  EXPECT_EQ(inf, any_dur / 0.0);
  EXPECT_EQ(-inf, any_dur / -0.0);
  EXPECT_EQ(-inf, -any_dur / 0.0);
  EXPECT_EQ(inf, -any_dur / -0.0);

  // Tests dividing by a number very close to, but not quite zero.
  EXPECT_EQ(zero, zero / dbl_denorm);
  EXPECT_EQ(zero, zero / -dbl_denorm);
  EXPECT_EQ(inf, any_dur / dbl_denorm);
  EXPECT_EQ(-inf, any_dur / -dbl_denorm);
  EXPECT_EQ(-inf, -any_dur / dbl_denorm);
  EXPECT_EQ(inf, -any_dur / -dbl_denorm);

  // IDiv
  unsmear::Duration rem = zero;
  EXPECT_EQ(kint64max, unsmear::IDivDuration(zero, zero, &rem));
  EXPECT_EQ(inf, rem);

  rem = zero;
  EXPECT_EQ(kint64max, unsmear::IDivDuration(any_dur, zero, &rem));
  EXPECT_EQ(inf, rem);

  rem = zero;
  EXPECT_EQ(kint64min, unsmear::IDivDuration(-any_dur, zero, &rem));
  EXPECT_EQ(-inf, rem);

  // Operator/(Duration, Duration)
  EXPECT_EQ(kint64max, zero / zero);
  EXPECT_EQ(kint64max, any_dur / zero);
  EXPECT_EQ(kint64min, -any_dur / zero);

  // FDiv
  EXPECT_EQ(dbl_inf, unsmear::FDivDuration(zero, zero));
  EXPECT_EQ(dbl_inf, unsmear::FDivDuration(any_dur, zero));
  EXPECT_EQ(-dbl_inf, unsmear::FDivDuration(-any_dur, zero));
}

TEST(Duration, Range) {
  const unsmear::Duration range = ApproxYears(100 * 1e9);
  const unsmear::Duration range_future = range;
  const unsmear::Duration range_past = -range;

  EXPECT_LT(range_future, unsmear::InfiniteDuration());
  EXPECT_GT(range_past, -unsmear::InfiniteDuration());

  const unsmear::Duration full_range = range_future - range_past;
  EXPECT_GT(full_range, unsmear::ZeroDuration());
  EXPECT_LT(full_range, unsmear::InfiniteDuration());

  const unsmear::Duration neg_full_range = range_past - range_future;
  EXPECT_LT(neg_full_range, unsmear::ZeroDuration());
  EXPECT_GT(neg_full_range, -unsmear::InfiniteDuration());

  EXPECT_LT(neg_full_range, full_range);
  EXPECT_EQ(neg_full_range, -full_range);
}

TEST(Duration, RelationalOperators) {
#define TEST_REL_OPS(UNIT)               \
  static_assert(UNIT(2) == UNIT(2), ""); \
  static_assert(UNIT(1) != UNIT(2), ""); \
  static_assert(UNIT(1) < UNIT(2), "");  \
  static_assert(UNIT(3) > UNIT(2), "");  \
  static_assert(UNIT(1) <= UNIT(2), ""); \
  static_assert(UNIT(2) <= UNIT(2), ""); \
  static_assert(UNIT(3) >= UNIT(2), ""); \
  static_assert(UNIT(2) >= UNIT(2), "");

  TEST_REL_OPS(unsmear::Nanoseconds);
  TEST_REL_OPS(unsmear::Microseconds);
  TEST_REL_OPS(unsmear::Milliseconds);
  TEST_REL_OPS(unsmear::Seconds);
  TEST_REL_OPS(unsmear::Minutes);
  TEST_REL_OPS(unsmear::Hours);

#undef TEST_REL_OPS
}

TEST(Duration, Addition) {
#define TEST_ADD_OPS(UNIT)                  \
  do {                                      \
    EXPECT_EQ(UNIT(2), UNIT(1) + UNIT(1));  \
    EXPECT_EQ(UNIT(1), UNIT(2) - UNIT(1));  \
    EXPECT_EQ(UNIT(0), UNIT(2) - UNIT(2));  \
    EXPECT_EQ(UNIT(-1), UNIT(1) - UNIT(2)); \
    EXPECT_EQ(UNIT(-2), UNIT(0) - UNIT(2)); \
    EXPECT_EQ(UNIT(-2), UNIT(1) - UNIT(3)); \
    unsmear::Duration a = UNIT(1);          \
    a += UNIT(1);                           \
    EXPECT_EQ(UNIT(2), a);                  \
    a -= UNIT(1);                           \
    EXPECT_EQ(UNIT(1), a);                  \
  } while (0)

  TEST_ADD_OPS(unsmear::Nanoseconds);
  TEST_ADD_OPS(unsmear::Microseconds);
  TEST_ADD_OPS(unsmear::Milliseconds);
  TEST_ADD_OPS(unsmear::Seconds);
  TEST_ADD_OPS(unsmear::Minutes);
  TEST_ADD_OPS(unsmear::Hours);

#undef TEST_ADD_OPS

  EXPECT_EQ(unsmear::Seconds(2),
            unsmear::Seconds(3) - 2 * unsmear::Milliseconds(500));
  EXPECT_EQ(unsmear::Seconds(2) + unsmear::Milliseconds(500),
            unsmear::Seconds(3) - unsmear::Milliseconds(500));

  EXPECT_EQ(unsmear::Seconds(1) + unsmear::Milliseconds(998),
            unsmear::Milliseconds(999) + unsmear::Milliseconds(999));

  EXPECT_EQ(unsmear::Milliseconds(-1),
            unsmear::Milliseconds(998) - unsmear::Milliseconds(999));

  // Tests fractions of a nanoseconds. These are implementation details only.
  EXPECT_GT(unsmear::Nanoseconds(1), unsmear::Nanoseconds(1) / 2);
  EXPECT_EQ(unsmear::Nanoseconds(1),
            unsmear::Nanoseconds(1) / 2 + unsmear::Nanoseconds(1) / 2);
  EXPECT_GT(unsmear::Nanoseconds(1) / 4, unsmear::Nanoseconds(0));
  EXPECT_EQ(unsmear::Nanoseconds(1) / 8, unsmear::Nanoseconds(0));

  // Tests subtraction that will cause wrap around of the rep_lo_ bits.
  unsmear::Duration d_7_5 = unsmear::Seconds(7) + unsmear::Milliseconds(500);
  unsmear::Duration d_3_7 = unsmear::Seconds(3) + unsmear::Milliseconds(700);
  unsmear::Duration ans_3_8 = unsmear::Seconds(3) + unsmear::Milliseconds(800);
  EXPECT_EQ(ans_3_8, d_7_5 - d_3_7);

  // Subtracting min_duration
  unsmear::Duration min_dur = unsmear::Seconds(kint64min);
  EXPECT_EQ(unsmear::Seconds(0), min_dur - min_dur);
  EXPECT_EQ(unsmear::Seconds(kint64max), unsmear::Seconds(-1) - min_dur);
}

TEST(Duration, Negation) {
  // By storing negations of various values in constexpr variables we
  // verify that the initializers are constant expressions.
  constexpr unsmear::Duration negated_zero_duration = -unsmear::ZeroDuration();
  EXPECT_EQ(negated_zero_duration, unsmear::ZeroDuration());

  constexpr unsmear::Duration negated_infinite_duration =
      -unsmear::InfiniteDuration();
  EXPECT_NE(negated_infinite_duration, unsmear::InfiniteDuration());
  EXPECT_EQ(-negated_infinite_duration, unsmear::InfiniteDuration());

  EXPECT_LT(negated_infinite_duration, unsmear::ZeroDuration());
}

TEST(Duration, AbsoluteValue) {
  EXPECT_EQ(unsmear::ZeroDuration(),
            unsmear::AbsDuration(unsmear::ZeroDuration()));
  EXPECT_EQ(unsmear::Seconds(1), unsmear::AbsDuration(unsmear::Seconds(1)));
  EXPECT_EQ(unsmear::Seconds(1), unsmear::AbsDuration(unsmear::Seconds(-1)));

  EXPECT_EQ(unsmear::InfiniteDuration(),
            unsmear::AbsDuration(unsmear::InfiniteDuration()));
  EXPECT_EQ(unsmear::InfiniteDuration(),
            unsmear::AbsDuration(-unsmear::InfiniteDuration()));
}

TEST(Duration, Multiplication) {
#define TEST_MUL_OPS(UNIT)                                       \
  do {                                                           \
    EXPECT_EQ(UNIT(5), UNIT(2) * 2.5);                           \
    EXPECT_EQ(UNIT(2), UNIT(5) / 2.5);                           \
    EXPECT_EQ(UNIT(-5), UNIT(-2) * 2.5);                         \
    EXPECT_EQ(UNIT(-5), -UNIT(2) * 2.5);                         \
    EXPECT_EQ(UNIT(-5), UNIT(2) * -2.5);                         \
    EXPECT_EQ(UNIT(-2), UNIT(-5) / 2.5);                         \
    EXPECT_EQ(UNIT(-2), -UNIT(5) / 2.5);                         \
    EXPECT_EQ(UNIT(-2), UNIT(5) / -2.5);                         \
    EXPECT_EQ(UNIT(2), UNIT(11) % UNIT(3));                      \
    unsmear::Duration a = UNIT(2);                               \
    a *= 2.5;                                                    \
    EXPECT_EQ(UNIT(5), a);                                       \
    a /= 2.5;                                                    \
    EXPECT_EQ(UNIT(2), a);                                       \
    a %= UNIT(1);                                                \
    EXPECT_EQ(UNIT(0), a);                                       \
    unsmear::Duration big = UNIT(1000000000);                    \
    big *= 3;                                                    \
    big /= 3;                                                    \
    EXPECT_EQ(UNIT(1000000000), big);                            \
    EXPECT_EQ(-UNIT(2), -UNIT(2));                               \
    EXPECT_EQ(-UNIT(2), UNIT(2) * -1);                           \
    EXPECT_EQ(-UNIT(2), -1 * UNIT(2));                           \
    EXPECT_EQ(-UNIT(-2), UNIT(2));                               \
    EXPECT_EQ(2, UNIT(2) / UNIT(1));                             \
    unsmear::Duration rem;                                       \
    EXPECT_EQ(2, unsmear::IDivDuration(UNIT(2), UNIT(1), &rem)); \
    EXPECT_EQ(2.0, unsmear::FDivDuration(UNIT(2), UNIT(1)));     \
  } while (0)

  TEST_MUL_OPS(unsmear::Nanoseconds);
  TEST_MUL_OPS(unsmear::Microseconds);
  TEST_MUL_OPS(unsmear::Milliseconds);
  TEST_MUL_OPS(unsmear::Seconds);
  TEST_MUL_OPS(unsmear::Minutes);
  TEST_MUL_OPS(unsmear::Hours);

#undef TEST_MUL_OPS

  // Ensures that multiplication and division by 1 with a maxed-out durations
  // doesn't lose precision.
  unsmear::Duration max_dur =
      unsmear::Seconds(kint64max) +
      (unsmear::Seconds(1) - unsmear::Nanoseconds(1) / 4);
  unsmear::Duration min_dur = unsmear::Seconds(kint64min);
  EXPECT_EQ(max_dur, max_dur * 1);
  EXPECT_EQ(max_dur, max_dur / 1);
  EXPECT_EQ(min_dur, min_dur * 1);
  EXPECT_EQ(min_dur, min_dur / 1);

  // Tests division on a Duration with a large number of significant digits.
  // Tests when the digits span hi and lo as well as only in hi.
  unsmear::Duration sigfigs =
      unsmear::Seconds(2000000000) + unsmear::Nanoseconds(3);
  EXPECT_EQ(unsmear::Seconds(666666666) + unsmear::Nanoseconds(666666667) +
                unsmear::Nanoseconds(1) / 2,
            sigfigs / 3);
  sigfigs = unsmear::Seconds(7000000000LL);
  EXPECT_EQ(unsmear::Seconds(2333333333) + unsmear::Nanoseconds(333333333) +
                unsmear::Nanoseconds(1) / 4,
            sigfigs / 3);

  EXPECT_EQ(unsmear::Seconds(7) + unsmear::Milliseconds(500),
            unsmear::Seconds(3) * 2.5);
  EXPECT_EQ(unsmear::Seconds(8) * -1 + unsmear::Milliseconds(300),
            (unsmear::Seconds(2) + unsmear::Milliseconds(200)) * -3.5);
  EXPECT_EQ(-unsmear::Seconds(8) + unsmear::Milliseconds(300),
            (unsmear::Seconds(2) + unsmear::Milliseconds(200)) * -3.5);
  EXPECT_EQ(unsmear::Seconds(1) + unsmear::Milliseconds(875),
            (unsmear::Seconds(7) + unsmear::Milliseconds(500)) / 4);
  EXPECT_EQ(unsmear::Seconds(30),
            (unsmear::Seconds(7) + unsmear::Milliseconds(500)) / 0.25);
  EXPECT_EQ(unsmear::Seconds(3),
            (unsmear::Seconds(7) + unsmear::Milliseconds(500)) / 2.5);

  // Tests division remainder.
  EXPECT_EQ(unsmear::Nanoseconds(0),
            unsmear::Nanoseconds(7) % unsmear::Nanoseconds(1));
  EXPECT_EQ(unsmear::Nanoseconds(0),
            unsmear::Nanoseconds(0) % unsmear::Nanoseconds(10));
  EXPECT_EQ(unsmear::Nanoseconds(2),
            unsmear::Nanoseconds(7) % unsmear::Nanoseconds(5));
  EXPECT_EQ(unsmear::Nanoseconds(2),
            unsmear::Nanoseconds(2) % unsmear::Nanoseconds(5));

  EXPECT_EQ(unsmear::Nanoseconds(1),
            unsmear::Nanoseconds(10) % unsmear::Nanoseconds(3));
  EXPECT_EQ(unsmear::Nanoseconds(1),
            unsmear::Nanoseconds(10) % unsmear::Nanoseconds(-3));
  EXPECT_EQ(unsmear::Nanoseconds(-1),
            unsmear::Nanoseconds(-10) % unsmear::Nanoseconds(3));
  EXPECT_EQ(unsmear::Nanoseconds(-1),
            unsmear::Nanoseconds(-10) % unsmear::Nanoseconds(-3));

  EXPECT_EQ(unsmear::Milliseconds(100),
            unsmear::Seconds(1) % unsmear::Milliseconds(300));
  EXPECT_EQ(unsmear::Milliseconds(300),
            (unsmear::Seconds(3) + unsmear::Milliseconds(800)) %
                unsmear::Milliseconds(500));

  EXPECT_EQ(unsmear::Nanoseconds(1),
            unsmear::Nanoseconds(1) % unsmear::Seconds(1));
  EXPECT_EQ(unsmear::Nanoseconds(-1),
            unsmear::Nanoseconds(-1) % unsmear::Seconds(1));
  EXPECT_EQ(0, unsmear::Nanoseconds(-1) / unsmear::Seconds(1));  // Actual -1e-9

  // Tests identity a = (a/b)*b + a%b
#define TEST_MOD_IDENTITY(a, b) EXPECT_EQ((a), ((a) / (b)) * (b) + ((a) % (b)))

  TEST_MOD_IDENTITY(unsmear::Seconds(0), unsmear::Seconds(2));
  TEST_MOD_IDENTITY(unsmear::Seconds(1), unsmear::Seconds(1));
  TEST_MOD_IDENTITY(unsmear::Seconds(1), unsmear::Seconds(2));
  TEST_MOD_IDENTITY(unsmear::Seconds(2), unsmear::Seconds(1));

  TEST_MOD_IDENTITY(unsmear::Seconds(-2), unsmear::Seconds(1));
  TEST_MOD_IDENTITY(unsmear::Seconds(2), unsmear::Seconds(-1));
  TEST_MOD_IDENTITY(unsmear::Seconds(-2), unsmear::Seconds(-1));

  TEST_MOD_IDENTITY(unsmear::Nanoseconds(0), unsmear::Nanoseconds(2));
  TEST_MOD_IDENTITY(unsmear::Nanoseconds(1), unsmear::Nanoseconds(1));
  TEST_MOD_IDENTITY(unsmear::Nanoseconds(1), unsmear::Nanoseconds(2));
  TEST_MOD_IDENTITY(unsmear::Nanoseconds(2), unsmear::Nanoseconds(1));

  TEST_MOD_IDENTITY(unsmear::Nanoseconds(-2), unsmear::Nanoseconds(1));
  TEST_MOD_IDENTITY(unsmear::Nanoseconds(2), unsmear::Nanoseconds(-1));
  TEST_MOD_IDENTITY(unsmear::Nanoseconds(-2), unsmear::Nanoseconds(-1));

  // Mixed seconds + subseconds
  unsmear::Duration mixed_a = unsmear::Seconds(1) + unsmear::Nanoseconds(2);
  unsmear::Duration mixed_b = unsmear::Seconds(1) + unsmear::Nanoseconds(3);

  TEST_MOD_IDENTITY(unsmear::Seconds(0), mixed_a);
  TEST_MOD_IDENTITY(mixed_a, mixed_a);
  TEST_MOD_IDENTITY(mixed_a, mixed_b);
  TEST_MOD_IDENTITY(mixed_b, mixed_a);

  TEST_MOD_IDENTITY(-mixed_a, mixed_b);
  TEST_MOD_IDENTITY(mixed_a, -mixed_b);
  TEST_MOD_IDENTITY(-mixed_a, -mixed_b);

#undef TEST_MOD_IDENTITY
}

TEST(Duration, Truncation) {
  const unsmear::Duration d = unsmear::Nanoseconds(1234567890);
  const unsmear::Duration inf = unsmear::InfiniteDuration();
  for (int unit_sign : {1, -1}) {  // sign shouldn't matter
    EXPECT_EQ(unsmear::Nanoseconds(1234567890),
              Trunc(d, unit_sign * unsmear::Nanoseconds(1)));
    EXPECT_EQ(unsmear::Microseconds(1234567),
              Trunc(d, unit_sign * unsmear::Microseconds(1)));
    EXPECT_EQ(unsmear::Milliseconds(1234),
              Trunc(d, unit_sign * unsmear::Milliseconds(1)));
    EXPECT_EQ(unsmear::Seconds(1), Trunc(d, unit_sign * unsmear::Seconds(1)));
    EXPECT_EQ(inf, Trunc(inf, unit_sign * unsmear::Seconds(1)));

    EXPECT_EQ(unsmear::Nanoseconds(-1234567890),
              Trunc(-d, unit_sign * unsmear::Nanoseconds(1)));
    EXPECT_EQ(unsmear::Microseconds(-1234567),
              Trunc(-d, unit_sign * unsmear::Microseconds(1)));
    EXPECT_EQ(unsmear::Milliseconds(-1234),
              Trunc(-d, unit_sign * unsmear::Milliseconds(1)));
    EXPECT_EQ(unsmear::Seconds(-1), Trunc(-d, unit_sign * unsmear::Seconds(1)));
    EXPECT_EQ(-inf, Trunc(-inf, unit_sign * unsmear::Seconds(1)));
  }
}

TEST(Duration, Flooring) {
  const unsmear::Duration d = unsmear::Nanoseconds(1234567890);
  const unsmear::Duration inf = unsmear::InfiniteDuration();
  for (int unit_sign : {1, -1}) {  // sign shouldn't matter
    EXPECT_EQ(unsmear::Nanoseconds(1234567890),
              unsmear::Floor(d, unit_sign * unsmear::Nanoseconds(1)));
    EXPECT_EQ(unsmear::Microseconds(1234567),
              unsmear::Floor(d, unit_sign * unsmear::Microseconds(1)));
    EXPECT_EQ(unsmear::Milliseconds(1234),
              unsmear::Floor(d, unit_sign * unsmear::Milliseconds(1)));
    EXPECT_EQ(unsmear::Seconds(1),
              unsmear::Floor(d, unit_sign * unsmear::Seconds(1)));
    EXPECT_EQ(inf, unsmear::Floor(inf, unit_sign * unsmear::Seconds(1)));

    EXPECT_EQ(unsmear::Nanoseconds(-1234567890),
              unsmear::Floor(-d, unit_sign * unsmear::Nanoseconds(1)));
    EXPECT_EQ(unsmear::Microseconds(-1234568),
              unsmear::Floor(-d, unit_sign * unsmear::Microseconds(1)));
    EXPECT_EQ(unsmear::Milliseconds(-1235),
              unsmear::Floor(-d, unit_sign * unsmear::Milliseconds(1)));
    EXPECT_EQ(unsmear::Seconds(-2),
              unsmear::Floor(-d, unit_sign * unsmear::Seconds(1)));
    EXPECT_EQ(-inf, unsmear::Floor(-inf, unit_sign * unsmear::Seconds(1)));
  }
}

TEST(Duration, Ceiling) {
  const unsmear::Duration d = unsmear::Nanoseconds(1234567890);
  const unsmear::Duration inf = unsmear::InfiniteDuration();
  for (int unit_sign : {1, -1}) {  // // sign shouldn't matter
    EXPECT_EQ(unsmear::Nanoseconds(1234567890),
              unsmear::Ceil(d, unit_sign * unsmear::Nanoseconds(1)));
    EXPECT_EQ(unsmear::Microseconds(1234568),
              unsmear::Ceil(d, unit_sign * unsmear::Microseconds(1)));
    EXPECT_EQ(unsmear::Milliseconds(1235),
              unsmear::Ceil(d, unit_sign * unsmear::Milliseconds(1)));
    EXPECT_EQ(unsmear::Seconds(2),
              unsmear::Ceil(d, unit_sign * unsmear::Seconds(1)));
    EXPECT_EQ(inf, unsmear::Ceil(inf, unit_sign * unsmear::Seconds(1)));

    EXPECT_EQ(unsmear::Nanoseconds(-1234567890),
              unsmear::Ceil(-d, unit_sign * unsmear::Nanoseconds(1)));
    EXPECT_EQ(unsmear::Microseconds(-1234567),
              unsmear::Ceil(-d, unit_sign * unsmear::Microseconds(1)));
    EXPECT_EQ(unsmear::Milliseconds(-1234),
              unsmear::Ceil(-d, unit_sign * unsmear::Milliseconds(1)));
    EXPECT_EQ(unsmear::Seconds(-1),
              unsmear::Ceil(-d, unit_sign * unsmear::Seconds(1)));
    EXPECT_EQ(-inf, unsmear::Ceil(-inf, unit_sign * unsmear::Seconds(1)));
  }
}

TEST(Duration, RoundTripUnits) {
  const int kRange = 100000;

#define ROUND_TRIP_UNIT(U, LOW, HIGH)                \
  do {                                               \
    for (int64_t i = LOW; i < HIGH; ++i) {           \
      unsmear::Duration d = unsmear::U(i);           \
      if (d == unsmear::InfiniteDuration())          \
        EXPECT_EQ(kint64max, d / unsmear::U(1));     \
      else if (d == -unsmear::InfiniteDuration())    \
        EXPECT_EQ(kint64min, d / unsmear::U(1));     \
      else                                           \
        EXPECT_EQ(i, unsmear::U(i) / unsmear::U(1)); \
    }                                                \
  } while (0)

  ROUND_TRIP_UNIT(Nanoseconds, kint64min, kint64min + kRange);
  ROUND_TRIP_UNIT(Nanoseconds, -kRange, kRange);
  ROUND_TRIP_UNIT(Nanoseconds, kint64max - kRange, kint64max);

  ROUND_TRIP_UNIT(Microseconds, kint64min, kint64min + kRange);
  ROUND_TRIP_UNIT(Microseconds, -kRange, kRange);
  ROUND_TRIP_UNIT(Microseconds, kint64max - kRange, kint64max);

  ROUND_TRIP_UNIT(Milliseconds, kint64min, kint64min + kRange);
  ROUND_TRIP_UNIT(Milliseconds, -kRange, kRange);
  ROUND_TRIP_UNIT(Milliseconds, kint64max - kRange, kint64max);

  ROUND_TRIP_UNIT(Seconds, kint64min, kint64min + kRange);
  ROUND_TRIP_UNIT(Seconds, -kRange, kRange);
  ROUND_TRIP_UNIT(Seconds, kint64max - kRange, kint64max);

  ROUND_TRIP_UNIT(Minutes, kint64min / 60, kint64min / 60 + kRange);
  ROUND_TRIP_UNIT(Minutes, -kRange, kRange);
  ROUND_TRIP_UNIT(Minutes, kint64max / 60 - kRange, kint64max / 60);

  ROUND_TRIP_UNIT(Hours, kint64min / 3600, kint64min / 3600 + kRange);
  ROUND_TRIP_UNIT(Hours, -kRange, kRange);
  ROUND_TRIP_UNIT(Hours, kint64max / 3600 - kRange, kint64max / 3600);

#undef ROUND_TRIP_UNIT
}

TEST(Duration, SmallConversions) {
  // Special tests for conversions of small durations.

  EXPECT_EQ(unsmear::ZeroDuration(), unsmear::Seconds(0));
  // TODO(bww): Is the next one OK?
  EXPECT_EQ(unsmear::ZeroDuration(), unsmear::Seconds(0.124999999e-9));
  EXPECT_EQ(unsmear::Nanoseconds(1) / 4, unsmear::Seconds(0.125e-9));
  EXPECT_EQ(unsmear::Nanoseconds(1) / 4, unsmear::Seconds(0.250e-9));
  EXPECT_EQ(unsmear::Nanoseconds(1) / 2, unsmear::Seconds(0.375e-9));
  EXPECT_EQ(unsmear::Nanoseconds(1) / 2, unsmear::Seconds(0.500e-9));
  EXPECT_EQ(unsmear::Nanoseconds(3) / 4, unsmear::Seconds(0.625e-9));
  EXPECT_EQ(unsmear::Nanoseconds(3) / 4, unsmear::Seconds(0.750e-9));
  EXPECT_EQ(unsmear::Nanoseconds(1), unsmear::Seconds(0.875e-9));
  EXPECT_EQ(unsmear::Nanoseconds(1), unsmear::Seconds(1.000e-9));
}

TEST(Duration, FormatDuration) {
  // Example from Go's docs.
  EXPECT_EQ("72h3m0.5s",
            unsmear::FormatDuration(unsmear::Hours(72) + unsmear::Minutes(3) +
                                    unsmear::Milliseconds(500)));
  // Go's largest time: 2540400h10m10.000000000s
  EXPECT_EQ("2540400h10m10s", unsmear::FormatDuration(unsmear::Hours(2540400) +
                                                      unsmear::Minutes(10) +
                                                      unsmear::Seconds(10)));

  EXPECT_EQ("0", unsmear::FormatDuration(unsmear::ZeroDuration()));
  EXPECT_EQ("0", unsmear::FormatDuration(unsmear::Seconds(0)));
  EXPECT_EQ("0", unsmear::FormatDuration(unsmear::Nanoseconds(0)));

  EXPECT_EQ("1ns", unsmear::FormatDuration(unsmear::Nanoseconds(1)));
  EXPECT_EQ("1us", unsmear::FormatDuration(unsmear::Microseconds(1)));
  EXPECT_EQ("1ms", unsmear::FormatDuration(unsmear::Milliseconds(1)));
  EXPECT_EQ("1s", unsmear::FormatDuration(unsmear::Seconds(1)));
  EXPECT_EQ("1m", unsmear::FormatDuration(unsmear::Minutes(1)));
  EXPECT_EQ("1h", unsmear::FormatDuration(unsmear::Hours(1)));

  EXPECT_EQ("1h1m",
            unsmear::FormatDuration(unsmear::Hours(1) + unsmear::Minutes(1)));
  EXPECT_EQ("1h1s",
            unsmear::FormatDuration(unsmear::Hours(1) + unsmear::Seconds(1)));
  EXPECT_EQ("1m1s",
            unsmear::FormatDuration(unsmear::Minutes(1) + unsmear::Seconds(1)));

  EXPECT_EQ("1h0.25s", unsmear::FormatDuration(unsmear::Hours(1) +
                                               unsmear::Milliseconds(250)));
  EXPECT_EQ("1m0.25s", unsmear::FormatDuration(unsmear::Minutes(1) +
                                               unsmear::Milliseconds(250)));
  EXPECT_EQ("1h1m0.25s",
            unsmear::FormatDuration(unsmear::Hours(1) + unsmear::Minutes(1) +
                                    unsmear::Milliseconds(250)));
  EXPECT_EQ("1h0.0005s", unsmear::FormatDuration(unsmear::Hours(1) +
                                                 unsmear::Microseconds(500)));
  EXPECT_EQ("1h0.0000005s", unsmear::FormatDuration(unsmear::Hours(1) +
                                                    unsmear::Nanoseconds(500)));

  // Subsecond special case.
  EXPECT_EQ("1.5ns", unsmear::FormatDuration(unsmear::Nanoseconds(1) +
                                             unsmear::Nanoseconds(1) / 2));
  EXPECT_EQ("1.25ns", unsmear::FormatDuration(unsmear::Nanoseconds(1) +
                                              unsmear::Nanoseconds(1) / 4));
  EXPECT_EQ("1ns", unsmear::FormatDuration(unsmear::Nanoseconds(1) +
                                           unsmear::Nanoseconds(1) / 9));
  EXPECT_EQ("1.2us", unsmear::FormatDuration(unsmear::Microseconds(1) +
                                             unsmear::Nanoseconds(200)));
  EXPECT_EQ("1.2ms", unsmear::FormatDuration(unsmear::Milliseconds(1) +
                                             unsmear::Microseconds(200)));
  EXPECT_EQ("1.0002ms", unsmear::FormatDuration(unsmear::Milliseconds(1) +
                                                unsmear::Nanoseconds(200)));
  EXPECT_EQ("1.00001ms", unsmear::FormatDuration(unsmear::Milliseconds(1) +
                                                 unsmear::Nanoseconds(10)));
  EXPECT_EQ("1.000001ms", unsmear::FormatDuration(unsmear::Milliseconds(1) +
                                                  unsmear::Nanoseconds(1)));

  // Negative durations.
  EXPECT_EQ("-1ns", unsmear::FormatDuration(unsmear::Nanoseconds(-1)));
  EXPECT_EQ("-1us", unsmear::FormatDuration(unsmear::Microseconds(-1)));
  EXPECT_EQ("-1ms", unsmear::FormatDuration(unsmear::Milliseconds(-1)));
  EXPECT_EQ("-1s", unsmear::FormatDuration(unsmear::Seconds(-1)));
  EXPECT_EQ("-1m", unsmear::FormatDuration(unsmear::Minutes(-1)));
  EXPECT_EQ("-1h", unsmear::FormatDuration(unsmear::Hours(-1)));

  EXPECT_EQ("-1h1m", unsmear::FormatDuration(
                         -(unsmear::Hours(1) + unsmear::Minutes(1))));
  EXPECT_EQ("-1h1s", unsmear::FormatDuration(
                         -(unsmear::Hours(1) + unsmear::Seconds(1))));
  EXPECT_EQ("-1m1s", unsmear::FormatDuration(
                         -(unsmear::Minutes(1) + unsmear::Seconds(1))));

  EXPECT_EQ("-1ns", unsmear::FormatDuration(unsmear::Nanoseconds(-1)));
  EXPECT_EQ("-1.2us", unsmear::FormatDuration(-(unsmear::Microseconds(1) +
                                                unsmear::Nanoseconds(200))));
  EXPECT_EQ("-1.2ms", unsmear::FormatDuration(-(unsmear::Milliseconds(1) +
                                                unsmear::Microseconds(200))));
  EXPECT_EQ("-1.0002ms", unsmear::FormatDuration(-(unsmear::Milliseconds(1) +
                                                   unsmear::Nanoseconds(200))));
  EXPECT_EQ("-1.00001ms", unsmear::FormatDuration(-(unsmear::Milliseconds(1) +
                                                    unsmear::Nanoseconds(10))));
  EXPECT_EQ("-1.000001ms", unsmear::FormatDuration(-(unsmear::Milliseconds(1) +
                                                     unsmear::Nanoseconds(1))));

  //
  // Interesting corner cases.
  //

  const unsmear::Duration qns = unsmear::Nanoseconds(1) / 4;
  const unsmear::Duration max_dur =
      unsmear::Seconds(kint64max) + (unsmear::Seconds(1) - qns);
  const unsmear::Duration min_dur = unsmear::Seconds(kint64min);

  EXPECT_EQ("0.25ns", unsmear::FormatDuration(qns));
  EXPECT_EQ("-0.25ns", unsmear::FormatDuration(-qns));
  EXPECT_EQ("2562047788015215h30m7.99999999975s",
            unsmear::FormatDuration(max_dur));
  EXPECT_EQ("-2562047788015215h30m8s", unsmear::FormatDuration(min_dur));

  // Tests printing full precision from units that print using FDivDuration
  EXPECT_EQ("55.00000000025s",
            unsmear::FormatDuration(unsmear::Seconds(55) + qns));
  EXPECT_EQ("55.00000025ms",
            unsmear::FormatDuration(unsmear::Milliseconds(55) + qns));
  EXPECT_EQ("55.00025us",
            unsmear::FormatDuration(unsmear::Microseconds(55) + qns));
  EXPECT_EQ("55.25ns", unsmear::FormatDuration(unsmear::Nanoseconds(55) + qns));

  // Formatting infinity
  EXPECT_EQ("inf", unsmear::FormatDuration(unsmear::InfiniteDuration()));
  EXPECT_EQ("-inf", unsmear::FormatDuration(-unsmear::InfiniteDuration()));

  // Formatting approximately +/- 100 billion years
  const unsmear::Duration huge_range = ApproxYears(100000000000);
  EXPECT_EQ("876000000000000h", unsmear::FormatDuration(huge_range));
  EXPECT_EQ("-876000000000000h", unsmear::FormatDuration(-huge_range));

  EXPECT_EQ("876000000000000h0.999999999s",
            unsmear::FormatDuration(
                huge_range + (unsmear::Seconds(1) - unsmear::Nanoseconds(1))));
  EXPECT_EQ(
      "876000000000000h0.9999999995s",
      unsmear::FormatDuration(
          huge_range + (unsmear::Seconds(1) - unsmear::Nanoseconds(1) / 2)));
  EXPECT_EQ(
      "876000000000000h0.99999999975s",
      unsmear::FormatDuration(
          huge_range + (unsmear::Seconds(1) - unsmear::Nanoseconds(1) / 4)));

  EXPECT_EQ("-876000000000000h0.999999999s",
            unsmear::FormatDuration(
                -huge_range - (unsmear::Seconds(1) - unsmear::Nanoseconds(1))));
  EXPECT_EQ(
      "-876000000000000h0.9999999995s",
      unsmear::FormatDuration(
          -huge_range - (unsmear::Seconds(1) - unsmear::Nanoseconds(1) / 2)));
  EXPECT_EQ(
      "-876000000000000h0.99999999975s",
      unsmear::FormatDuration(
          -huge_range - (unsmear::Seconds(1) - unsmear::Nanoseconds(1) / 4)));
}

TEST(Duration, ParseDuration) {
  unsmear::Duration d;

  // No specified unit. Should only work for zero and infinity.
  EXPECT_TRUE(unsmear::ParseDuration("0", &d));
  EXPECT_EQ(unsmear::ZeroDuration(), d);
  EXPECT_TRUE(unsmear::ParseDuration("+0", &d));
  EXPECT_EQ(unsmear::ZeroDuration(), d);
  EXPECT_TRUE(unsmear::ParseDuration("-0", &d));
  EXPECT_EQ(unsmear::ZeroDuration(), d);

  EXPECT_TRUE(unsmear::ParseDuration("inf", &d));
  EXPECT_EQ(unsmear::InfiniteDuration(), d);
  EXPECT_TRUE(unsmear::ParseDuration("+inf", &d));
  EXPECT_EQ(unsmear::InfiniteDuration(), d);
  EXPECT_TRUE(unsmear::ParseDuration("-inf", &d));
  EXPECT_EQ(-unsmear::InfiniteDuration(), d);
  EXPECT_FALSE(unsmear::ParseDuration("infBlah", &d));

  // Illegal input forms.
  EXPECT_FALSE(unsmear::ParseDuration("", &d));
  EXPECT_FALSE(unsmear::ParseDuration("0.0", &d));
  EXPECT_FALSE(unsmear::ParseDuration(".0", &d));
  EXPECT_FALSE(unsmear::ParseDuration(".", &d));
  EXPECT_FALSE(unsmear::ParseDuration("01", &d));
  EXPECT_FALSE(unsmear::ParseDuration("1", &d));
  EXPECT_FALSE(unsmear::ParseDuration("-1", &d));
  EXPECT_FALSE(unsmear::ParseDuration("2", &d));
  EXPECT_FALSE(unsmear::ParseDuration("2 s", &d));
  EXPECT_FALSE(unsmear::ParseDuration(".s", &d));
  EXPECT_FALSE(unsmear::ParseDuration("-.s", &d));
  EXPECT_FALSE(unsmear::ParseDuration("s", &d));
  EXPECT_FALSE(unsmear::ParseDuration(" 2s", &d));
  EXPECT_FALSE(unsmear::ParseDuration("2s ", &d));
  EXPECT_FALSE(unsmear::ParseDuration(" 2s ", &d));
  EXPECT_FALSE(unsmear::ParseDuration("2mt", &d));

  // One unit type.
  EXPECT_TRUE(unsmear::ParseDuration("1ns", &d));
  EXPECT_EQ(unsmear::Nanoseconds(1), d);
  EXPECT_TRUE(unsmear::ParseDuration("1us", &d));
  EXPECT_EQ(unsmear::Microseconds(1), d);
  EXPECT_TRUE(unsmear::ParseDuration("1ms", &d));
  EXPECT_EQ(unsmear::Milliseconds(1), d);
  EXPECT_TRUE(unsmear::ParseDuration("1s", &d));
  EXPECT_EQ(unsmear::Seconds(1), d);
  EXPECT_TRUE(unsmear::ParseDuration("2m", &d));
  EXPECT_EQ(unsmear::Minutes(2), d);
  EXPECT_TRUE(unsmear::ParseDuration("2h", &d));
  EXPECT_EQ(unsmear::Hours(2), d);

  // Multiple units.
  EXPECT_TRUE(unsmear::ParseDuration("2h3m4s", &d));
  EXPECT_EQ(unsmear::Hours(2) + unsmear::Minutes(3) + unsmear::Seconds(4), d);
  EXPECT_TRUE(unsmear::ParseDuration("3m4s5us", &d));
  EXPECT_EQ(
      unsmear::Minutes(3) + unsmear::Seconds(4) + unsmear::Microseconds(5), d);
  EXPECT_TRUE(unsmear::ParseDuration("2h3m4s5ms6us7ns", &d));
  EXPECT_EQ(unsmear::Hours(2) + unsmear::Minutes(3) + unsmear::Seconds(4) +
                unsmear::Milliseconds(5) + unsmear::Microseconds(6) +
                unsmear::Nanoseconds(7),
            d);

  // Multiple units out of order.
  EXPECT_TRUE(unsmear::ParseDuration("2us3m4s5h", &d));
  EXPECT_EQ(unsmear::Hours(5) + unsmear::Minutes(3) + unsmear::Seconds(4) +
                unsmear::Microseconds(2),
            d);

  // Fractional values of units.
  EXPECT_TRUE(unsmear::ParseDuration("1.5ns", &d));
  EXPECT_EQ(1.5 * unsmear::Nanoseconds(1), d);
  EXPECT_TRUE(unsmear::ParseDuration("1.5us", &d));
  EXPECT_EQ(1.5 * unsmear::Microseconds(1), d);
  EXPECT_TRUE(unsmear::ParseDuration("1.5ms", &d));
  EXPECT_EQ(1.5 * unsmear::Milliseconds(1), d);
  EXPECT_TRUE(unsmear::ParseDuration("1.5s", &d));
  EXPECT_EQ(1.5 * unsmear::Seconds(1), d);
  EXPECT_TRUE(unsmear::ParseDuration("1.5m", &d));
  EXPECT_EQ(1.5 * unsmear::Minutes(1), d);
  EXPECT_TRUE(unsmear::ParseDuration("1.5h", &d));
  EXPECT_EQ(1.5 * unsmear::Hours(1), d);

  // Negative durations.
  EXPECT_TRUE(unsmear::ParseDuration("-1s", &d));
  EXPECT_EQ(unsmear::Seconds(-1), d);
  EXPECT_TRUE(unsmear::ParseDuration("-1m", &d));
  EXPECT_EQ(unsmear::Minutes(-1), d);
  EXPECT_TRUE(unsmear::ParseDuration("-1h", &d));
  EXPECT_EQ(unsmear::Hours(-1), d);

  EXPECT_TRUE(unsmear::ParseDuration("-1h2s", &d));
  EXPECT_EQ(-(unsmear::Hours(1) + unsmear::Seconds(2)), d);
  EXPECT_FALSE(unsmear::ParseDuration("1h-2s", &d));
  EXPECT_FALSE(unsmear::ParseDuration("-1h-2s", &d));
  EXPECT_FALSE(unsmear::ParseDuration("-1h -2s", &d));
}

TEST(Duration, FormatParseRoundTrip) {
#define TEST_PARSE_ROUNDTRIP(d)                   \
  do {                                            \
    std::string s = unsmear::FormatDuration(d);   \
    unsmear::Duration dur;                        \
    EXPECT_TRUE(unsmear::ParseDuration(s, &dur)); \
    EXPECT_EQ(d, dur);                            \
  } while (0)

  TEST_PARSE_ROUNDTRIP(unsmear::Nanoseconds(1));
  TEST_PARSE_ROUNDTRIP(unsmear::Microseconds(1));
  TEST_PARSE_ROUNDTRIP(unsmear::Milliseconds(1));
  TEST_PARSE_ROUNDTRIP(unsmear::Seconds(1));
  TEST_PARSE_ROUNDTRIP(unsmear::Minutes(1));
  TEST_PARSE_ROUNDTRIP(unsmear::Hours(1));
  TEST_PARSE_ROUNDTRIP(unsmear::Hours(1) + unsmear::Nanoseconds(2));

  TEST_PARSE_ROUNDTRIP(unsmear::Nanoseconds(-1));
  TEST_PARSE_ROUNDTRIP(unsmear::Microseconds(-1));
  TEST_PARSE_ROUNDTRIP(unsmear::Milliseconds(-1));
  TEST_PARSE_ROUNDTRIP(unsmear::Seconds(-1));
  TEST_PARSE_ROUNDTRIP(unsmear::Minutes(-1));
  TEST_PARSE_ROUNDTRIP(unsmear::Hours(-1));

  TEST_PARSE_ROUNDTRIP(unsmear::Hours(-1) + unsmear::Nanoseconds(2));
  TEST_PARSE_ROUNDTRIP(unsmear::Hours(1) + unsmear::Nanoseconds(-2));
  TEST_PARSE_ROUNDTRIP(unsmear::Hours(-1) + unsmear::Nanoseconds(-2));

  TEST_PARSE_ROUNDTRIP(unsmear::Nanoseconds(1) +
                       unsmear::Nanoseconds(1) / 4);  // 1.25ns

  const unsmear::Duration huge_range = ApproxYears(100000000000);
  TEST_PARSE_ROUNDTRIP(huge_range);
  TEST_PARSE_ROUNDTRIP(huge_range +
                       (unsmear::Seconds(1) - unsmear::Nanoseconds(1)));

#undef TEST_PARSE_ROUNDTRIP
}

}  // namespace

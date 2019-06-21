# Unsmear

This C++ library converts precisely between timestamps in the timescale of
[UTC with smeared leap seconds](https://developers.google.com/time/smear) and
the unsmeared [TAI](https://en.wikipedia.org/wiki/International_Atomic_Time) and
GPS timescales.

This allows smeared time to be stored and distributed exclusively, then
converted to and from other timescales in applications where the smear's 11.6
ppm frequency change is consequential. No parallel time distribution systems are
required, nor is any mechanism necessary for a system to record what timescale
its system clock is using.

In other words, the smear is not a way of approximately smudging the clock; it
is a defined, precise, and reversible conversion. Rather than having seconds of
constant length and minutes with a variable number of seconds, there are always
60 seconds in a minute and the length of a second sometimes differs from those
of TAI.

Unsmear is used for timekeeping on every Google production machine. Although it
is stable and mature, we may change the details of the API based on feedback.
Although this is not an officially supported Google product, you can reach us on
the
[unsmear-discuss mailing list](https://groups.google.com/forum/#!forum/unsmear-discuss).

## API overview

Unsmear's API works with and closely parallels the
[Abseil time library](https://github.com/abseil/abseil-cpp/blob/master/absl/time/time.h).
It uses `absl::Time` for timestamps in the smeared timescale, and
`absl::Duration` for intervals of possibly-smeared seconds. `unsmear::TaiTime`
and `unsmear::GpsTime` represent times in TAI and GPST. `unsmear::Duration`
measures intervals of constant-length SI seconds in
[Terrestrial Time](https://en.wikipedia.org/wiki/Terrestrial_Time). These new
types offer the benefits of the Abseil time types while also allowing for
correct and type-safe conversions between smeared and unsmeared timescales. The
resolution of these types is 1 ns or better.

An `unsmear::LeapTable` tracks the leap seconds that have been added to UTC,
allowing for smearing and unsmearing conversions between the three supported
timescales. Leap tables can be serialized and deserialized using the
[protocol buffer](https://developers.google.com/protocol-buffers/) in
`unsmear/leap_table.proto`. A current table of leap seconds is provided in
`leap_table.textpb`.

We strongly recommend distributing the leap table to your applications in binary
protobuf format. Protobuf text format is intended primarily for checked-in
configurations and human-readable debugging output.

During the period for which leap seconds are already announced, the methods
`Smear()`, `Unsmear()`, and `UnsmearToGps()` will convert precisely between
smeared UTC and the unsmeared timescales. This period extends some amount past
the last leap second, since
[IERS Bulletin C](https://www.iers.org/IERS/EN/Publications/Bulletins/bulletins.html)
announces when leap seconds will not happen as well as when they will.

The methods `FutureProofSmear()`, `FutureProofUnsmear()`, and
`FutureProofUnsmearToGps()` will convert times after the expiration of the leap
table by returning an interval of possible times.

Unsmear is built using the
[Bazel build system](https://docs.bazel.build/versions/master/getting-started.html).

## Example

To perform conversions, construct a `unsmear::LeapTable` from the binary
protobuf data:

```c++
std::ifstream stream;
stream.open("leap_table.pb", std::ios::binary);
unsmear::LeapTableProto pb;
if (!pb.ParseFromIstream(&stream)) { /* error... */ }
std::unique_ptr<unsmear::LeapTable> lt = unsmear::NewLeapTableFromProto(pb);
```

The `Unsmear()` method returns an `absl::optional`, which will be
`absl::nullopt` if outside the valid range of the leap table. Leap seconds have
already been determined for 2017, so this will succeed:

```c++
absl::Time utc =
    absl::FromDateTime(2017, 1, 15, 10, 0, 0, absl::UTCTimeZone());
unsmear::TaiTime tai = *lt->Unsmear(utc);  // Note '*' to de-optionalize.
```

We can reverse the conversion using `Smear()`:

```c++
absl::Time utc2 = *lt->Smear(tai);
assert(utc == utc2);
```

Conversions beyond the end of the leap table will return a range of times based
on how many possible leap seconds may have occurred.

```c++
absl::Time utc = lt->expiration();
std::pair<unsmear::TaiTime, unsmear::TaiTime> tai_beyond =
    lt->FutureProofUnsmear(utc + absl::Hours(48));
```

Leap seconds may be inserted at the end of any UTC month, and may be positive or
negative (i.e., seconds added or inserted). This results in the converted range
expanding by one second in each direction for the 24-hour period crossing every
possible smear. In this example, for a time 48 hours past the end of the leap
table, there may have been one negative leap second, no leap second, or one
positive leap second.

```c++
assert(tai_beyond.second - tai_beyond.first == unsmear::Seconds(2));
unsmear::TaiTime tai = *lt->Unsmear(utc);
assert(tai_beyond.first - tai == unsmear::Hours(48) - unsmear::Seconds(1));
assert(tai_beyond.second - tai == unsmear::Hours(48) + unsmear::Seconds(1));
```

This allows storage of future timestamps in the very popular representation as a
count of seconds or nanoseconds since an epoch. As the time approaches, the leap
seconds will be determined, and the window of conversion uncertainty will reduce
to zero.

## Additional API details

### Valid ranges of times

The supported timescales are defined from these starting times:

<!-- mdformat off(don't wrap to multi-line table) -->
| TAI                     | Modern UTC              | GPST                     |
| :---------------------: | :---------------------: | :----------------------: |
| **1958-01-01 00:00:00 TAI** | n/a | n/a |
| 1972-01-01 00:00:10 TAI | **1972-01-01 00:00:00 UTC** | n/a |
| 1980-01-06 00:00:19 TAI | 1980-01-06 00:00:00 UTC | **1980-01-06 00:00:00 GPST** |
<!-- mdformat on -->

Like `absl::Time`, `unsmear::TaiTime` and `unsmear::GpsTime` times may be
created with any value, including times before the beginning of the timescale.
This allows ordinary laws of arithmetic to hold; for example:

```c++
unsmear::Duration d = unsmear::Seconds(1);
unsmear::TaiTime epoch = unsmear::TaiEpoch();
// Ok even though (epoch - d) is not a well-defined time:
unsmear::TaiTime t1 = epoch - d + d;
```

Active conversions using a `LeapTable` do not consider the timescales to be
proleptic, and will only convert times after 1972-01-01 00:00:00 UTC, the start
of modern UTC with leap seconds. When converting smeared UTC to or from GPST,
the time must also not be before that epoch, 1980-01-06 00:00:00 GPST.

In particular, the Unix epoch of 1970-01-01 00:00:00 UTC predates modern UTC,
and so it cannot be smeared or unsmeared. During the period between the Unix
epoch and the modern UTC epoch, UTC seconds were not equal to TAI seconds, and a
discontinuity of 107.758 ms was introduced. Smearing during this period is
infeasible.

`Smear()` and `Unsmear()` report errors by returning an empty `absl::optional`.
The `FutureProof` methods return an interval from the infinite past to the
infinite future.

A leap table is not necessary to convert from TAI to GPST or the reverse. Use
the `ToTaiTime()` and `ToGpsTime()` functions. There are no restrictions on what
times may be converted in this way.

### Formatting and parsing

`unsmear::FormatTime()` and `unsmear::FormatDuration()` will convert times and
durations to strings.

The default format is ISO 8601, e.g., `2006-01-02 15:04:05.999999999 TAI`. You
can also provide a custom format string for times, supporting the same
specifiers used by `absl::FormatTime()` (roughly, those used by `strftime(3)`).
For convenience, `unsmear::FormatTime()` will also accept an `absl::Time` to be
formatted as UTC.

The time and duration types can also be output to a stream:

```c++
std::cout << "Duration since TAI epoch is " << now - TaiEpoch();
```

`unsmear::ParseDuration()` will convert a string to an `unsmear::Duration`.

```c++
unsmear::Duration d;
if (!unsmear::ParseDuration(s, &d)) {
  std::cerr << "Invalid duration string: " << s << "\n";
}
```

### Conveniences

`JdnToTime()` will convert an integer
[Julian Day Number](https://en.wikipedia.org/wiki/Julian_day) to an
`absl::Time`. Julian days begin at noon UTC.

```c++
absl::Time t = unsmear::JdnToTime(2451545);  // 2000-01-01 12:00:00 UTC
```

The command-line `leap_table_tool` will convert the provided text-format
`LeapTableProto` in `leap_table.textpb` to a binary protobuf, to JSON, or to a
human-readable debugging output.

```sh
leap_table_tool --output=proto leap_table.textpb > leap_table.pb
leap_table_tool --output=json leap_table.textpb > leap_table.json
leap_table_tool --input=proto --output=debug leap_table.pb
```

## Smear details

The smear implemented here is from noon to noon UTC. The details and rationale
are described
[at developers.google.com](https://developers.google.com/time/smear).

This smear is identical to the smear used by
[Google Public NTP](https://developers.google.com/time/guides), by all other
Google services, and by
[Amazon Web Services](https://aws.amazon.com/blogs/aws/look-before-you-leap-the-coming-leap-second-and-aws/).
This consistency is what allows the smear to be used interoperably and to be
reliably reversed.

The concept of a "future-proof" API that provides a range of possible times
based on unknown future leap seconds was described in the paper
["Leap-smeared representation of time for high-accuracy applications"](https://www.tdcommons.org/dpubs_series/339/),
published by Google in 2016 as a
[defensive publication](https://en.wikipedia.org/wiki/Defensive_publication).

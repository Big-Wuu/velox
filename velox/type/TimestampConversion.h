/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <cstdint>
#include "velox/common/base/Status.h"
#include "velox/type/Timestamp.h"

namespace facebook::velox::tz {
class TimeZone;
}

namespace facebook::velox::util {

constexpr const int32_t kHoursPerDay{24};
constexpr const int32_t kMinsPerHour{60};
constexpr const int32_t kSecsPerMinute{60};
constexpr const int64_t kMsecsPerSec{1000};

constexpr const int64_t kMillisPerSecond{1000};
constexpr const int64_t kMillisPerMinute{kMillisPerSecond * kSecsPerMinute};
constexpr const int64_t kMillisPerHour{kMillisPerMinute * kMinsPerHour};

constexpr const int64_t kMicrosPerMsec{1000};
constexpr const int64_t kMicrosPerSec{kMicrosPerMsec * kMsecsPerSec};
constexpr const int64_t kMicrosPerMinute{kMicrosPerSec * kSecsPerMinute};
constexpr const int64_t kMicrosPerHour{kMicrosPerMinute * kMinsPerHour};

constexpr const int64_t kNanosPerMicro{1000};

constexpr const int32_t kSecsPerHour{kSecsPerMinute * kMinsPerHour};
constexpr const int32_t kSecsPerDay{kSecsPerHour * kHoursPerDay};

// Max and min year correspond to Joda datetime min and max
constexpr const int32_t kMinYear{-292275055};
constexpr const int32_t kMaxYear{292278994};

constexpr const int32_t kYearInterval{400};
constexpr const int32_t kDaysPerYearInterval{146097};

/// Enum to dictate parsing modes for date strings.
enum class ParseMode {
  // For date string conversion, align with DuckDB's implementation.
  kStrict,

  // For timestamp string conversion, align with DuckDB's implementation.
  kNonStrict,

  // Accepts complete ISO 8601 format, i.e. [+-](YYYY-MM-DD). Allows leading and
  // trailing spaces.
  // Aligned with Presto casting conventions.
  kPrestoCast,

  // ISO-8601 format with optional leading or trailing spaces. Optional trailing
  // 'T' is also allowed.
  // Aligned with Spark SQL casting conventions.
  //
  // [+-][Y]Y*
  // [+-][Y]Y*-[M]M
  // [+-][Y]Y*-[M]M*-[D]D
  // [+-][Y]Y*-[M]M*-[D]D *
  // [+-][Y]Y*-[M]M*-[D]DT*
  kSparkCast,

  // ISO-8601 format. No leading or trailing spaces allowed.
  //
  // [+-][Y]Y*
  // [+-][Y]Y*-[M]M
  // [+-][Y]Y*-[M]M*-[D]D
  kIso8601
};

// Returns true if leap year, false otherwise
bool isLeapYear(int32_t year);

// Returns true if year, month, day corresponds to valid date, false otherwise
bool isValidDate(int32_t year, int32_t month, int32_t day);

// Returns true if yday of year is valid for given year
bool isValidDayOfYear(int32_t year, int32_t dayOfYear);

// Returns max day of month for inputted month of inputted year
int32_t getMaxDayOfMonth(int32_t year, int32_t month);

/// Computes the last day of month since unix epoch (1970-01-01).
/// Returns UserError status if the date is invalid.
Expected<int64_t> lastDayOfMonthSinceEpochFromDate(const std::tm& dateTime);

/// Date conversions.

/// Computes the (signed) number of days since unix epoch (1970-01-01).
/// Returns UserError status if the date is invalid.
Expected<int64_t>
daysSinceEpochFromDate(int32_t year, int32_t month, int32_t day);

/// Computes the (signed) number of days since unix epoch (1970-01-01).
/// Returns UserError status if the date is invalid.
Expected<int64_t> daysSinceEpochFromWeekDate(
    int32_t weekYear,
    int32_t weekOfYear,
    int32_t dayOfWeek);

/// Computes the signed number of days since the Unix epoch (1970-01-01). To
/// align with Spark's SimpleDateFormat behavior, this function offers two
/// modes: lenient and non-lenient. For non-lenient mode, dates before Jan 1, 1
/// are not supported, and it returns an error status if the date is invalid.
/// For lenient mode, it accepts a wider range of arguments.
/// @param year Year. For non-lenient mode, it should be in the range [1,
/// 292278994]. e.g: 1996, 2024. For lenient mode, it should be in the range
/// [-292275055, 292278994].
/// @param month Month of year. For non-lenient mode, it should be in the range
/// [1, 12]. For example, 1 is January, 7 is July. For lenient mode, values
/// greater than 12 wrap around to the start of the year, and values less than 1
/// count backward from December. For example, 13 corresponds to January of the
/// following year and -1 corresponds to November of the previous year.
/// @param weekOfMonth Week of the month. For non-lenient mode, it should be in
/// the range [1, depends on month]. For example, 1 is 1st week, 3 is 3rd week.
/// For lenient mode, we consider days of the previous or next months as part of
/// the specified weekOfMonth. For example, if weekOfMonth is 5 but the current
/// month only has 4 weeks (such as February), the first week of March will be
/// considered as the 5th week of February.
/// @param dayOfWeek Day number of week. For non-lenient mode, it should be in
/// the range [1, depends on month]. For example, 1 is Monday, 7 is Sunday. For
/// lenient mode, we consider days of the previous or next months as part of the
/// specified dayOfWeek.For example, if weekOfMonth is 1 and dayOfWeek is 1 but
/// the month's first day is Saturday, the Monday of the last week of the
/// previous month will be used.
Expected<int64_t> daysSinceEpochFromWeekOfMonthDate(
    int32_t year,
    int32_t month,
    int32_t weekOfMonth,
    int32_t dayOfWeek,
    bool lenient);

/// Computes the (signed) number of days since unix epoch (1970-01-01).
/// Returns UserError status if the date is invalid.
Expected<int64_t> daysSinceEpochFromDayOfYear(int32_t year, int32_t dayOfYear);

/// Cast string to date. Supported date formats vary, depending on input
/// ParseMode. Refer to ParseMode enum for further info.
///
/// Returns Unexpected with UserError status if the format or date is invalid.
Expected<int32_t> fromDateString(const char* buf, size_t len, ParseMode mode);

inline Expected<int32_t> fromDateString(const StringView& str, ParseMode mode) {
  return fromDateString(str.data(), str.size(), mode);
}

// Extracts the day of the week from the number of days since epoch
int32_t extractISODayOfTheWeek(int64_t daysSinceEpoch);

/// Time conversions.

/// Returns the cumulative number of microseconds.
/// Does not perform any sanity checks.
int64_t
fromTime(int32_t hour, int32_t minute, int32_t second, int32_t microseconds);

// Timestamp conversion

enum class TimestampParseMode {
  /// Accepted syntax:
  // clang-format off
  ///   datetime          = time | date-opt-time
  ///   time              = 'T' time-element [offset]
  ///   date-opt-time     = date-element ['T' [time-element] [offset]]
  ///   date-element      = yyyy ['-' MM ['-' dd]]
  ///   time-element      = HH [minute-element] | [fraction]
  ///   minute-element    = ':' mm [second-element] | [fraction]
  ///   second-element    = ':' ss [fraction]
  ///   fraction          = ('.' | ',') digit+
  ///   offset            = 'Z' | (('+' | '-') HH [':' mm [':' ss [('.' | ',') SSS]]])
  // clang-format on
  kIso8601,

  /// Accepted syntax:
  // clang-format off
  ///   date-opt-time     = date-element [' ' [time-element] [[' '] [offset]]]
  ///   date-element      = yyyy ['-' MM ['-' dd]]
  ///   time-element      = HH [minute-element] | [fraction]
  ///   minute-element    = ':' mm [second-element] | [fraction]
  ///   second-element    = ':' ss [fraction]
  ///   fraction          = '.' digit+
  ///   offset            = 'Z' | ZZZ
  // clang-format on
  // Allows leading and trailing spaces.
  kPrestoCast,

  // Same as kPrestoCast, but allows 'T' separator between date and time.
  kLegacyCast,

  /// A Spark-compatible timestamp string. A mix of the above. Accepts T and
  /// space as separator between date and time. Allows leading and trailing
  /// spaces.
  kSparkCast,
};

/// Parses a timestamp string using specified TimestampParseMode.
///
/// This function does not accept any timezone information in the string (e.g.
/// UTC, Z, or a timezone offsets). This is because the returned timestamp does
/// not contain timezone information; therefore, it would either be required for
/// this function to convert the parsed timestamp (but we don't know the
/// original timezone), or ignore the timezone information, which would be
/// incorecct.
///
/// For a timezone-aware version of this function, check
/// `fromTimestampWithTimezoneString()` below.
Expected<Timestamp>
fromTimestampString(const char* buf, size_t len, TimestampParseMode parseMode);

inline Expected<Timestamp> fromTimestampString(
    const StringView& str,
    TimestampParseMode parseMode) {
  return fromTimestampString(str.data(), str.size(), parseMode);
}

struct ParsedTimestampWithTimeZone {
  Timestamp timestamp;
  const tz::TimeZone* timeZone;
  std::optional<int64_t> offsetMillis;

  // For ease of testing purposes.
  bool operator==(const ParsedTimestampWithTimeZone& other) const {
    return timestamp == other.timestamp && timeZone == other.timeZone &&
        offsetMillis == other.offsetMillis;
  }
};

/// Parses a timestamp string using specified TimestampParseMode.
///
/// This is a timezone-aware version of the function above
/// `fromTimestampString()` which returns both the parsed timestamp and the
/// TimeZone pointer. It is up to the client to do the expected conversion based
/// on these two values.
///
/// The timezone information at the end of the string may contain a timezone
/// name (as defined in velox/type/tz/*), such as "UTC" or
/// "America/Los_Angeles", or a timezone offset, like "+06:00" or "-09:30". The
/// white space between the hour definition and timestamp is optional.
///
/// `nullptr` means the timezone was not recognized as a valid time zone or
/// was not present. In this case offsetMillis may be set with the milliseconds
/// timezone offset if an offset was found but was not a valid timezone.
///
/// Returns Unexpected with UserError status in case of parsing errors.
Expected<ParsedTimestampWithTimeZone> fromTimestampWithTimezoneString(
    const char* buf,
    size_t len,
    TimestampParseMode parseMode);

inline Expected<ParsedTimestampWithTimeZone> fromTimestampWithTimezoneString(
    const StringView& str,
    TimestampParseMode parseMode) {
  return fromTimestampWithTimezoneString(str.data(), str.size(), parseMode);
}

/// Converts ParsedTimestampWithTimeZone to Timestamp according to the
/// timezone-based adjustment. If no timezone information is available
/// in the first argument, respects the session timezone if configured.
Timestamp fromParsedTimestampWithTimeZone(
    ParsedTimestampWithTimeZone parsed,
    const tz::TimeZone* sessionTimeZone);

Timestamp fromDatetime(int64_t daysSinceEpoch, int64_t microsSinceMidnight);

/// Returns the number of days since epoch for a given timestamp and optional
/// time zone.
int32_t toDate(const Timestamp& timestamp, const tz::TimeZone* timeZone);

} // namespace facebook::velox::util

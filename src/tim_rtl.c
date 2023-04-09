/********************************************************************/
/*                                                                  */
/*  tim_rtl.c     Time access using the C capabilities.             */
/*  Copyright (C) 1989 - 2015, 2018, 2020  Thomas Mertes            */
/*                                                                  */
/*  This file is part of the Seed7 Runtime Library.                 */
/*                                                                  */
/*  The Seed7 Runtime Library is free software; you can             */
/*  redistribute it and/or modify it under the terms of the GNU     */
/*  Lesser General Public License as published by the Free Software */
/*  Foundation; either version 2.1 of the License, or (at your      */
/*  option) any later version.                                      */
/*                                                                  */
/*  The Seed7 Runtime Library is distributed in the hope that it    */
/*  will be useful, but WITHOUT ANY WARRANTY; without even the      */
/*  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR */
/*  PURPOSE.  See the GNU Lesser General Public License for more    */
/*  details.                                                        */
/*                                                                  */
/*  You should have received a copy of the GNU Lesser General       */
/*  Public License along with this program; if not, write to the    */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/tim_rtl.c                                       */
/*  Changes: 2009, 2013 - 2015, 2018, 2020  Thomas Mertes           */
/*  Content: Time access using the C capabilities.                  */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include "string.h"
#include "errno.h"

#include "common.h"
#include "big_drv.h"
#include "tim_drv.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "tim_rtl.h"


#if TIME_T_SIZE == 32
#if TIME_T_SIGNED
#define STRUCT_TM_MIN_YEAR 1902
#else
#define STRUCT_TM_MIN_YEAR 1970
#endif
#define STRUCT_TM_MAX_YEAR 2037
#elif TIME_T_SIZE == 64
#if   defined INT64TYPE_SUFFIX_LL
#define STRUCT_TM_MIN_YEAR (-292277022655LL)
#define STRUCT_TM_MAX_YEAR   292277026595LL
#elif defined INT64TYPE_SUFFIX_L
#define STRUCT_TM_MIN_YEAR (-292277022655L)
#define STRUCT_TM_MAX_YEAR   292277026595L
#else
#define STRUCT_TM_MIN_YEAR (-292277022655)
#define STRUCT_TM_MAX_YEAR   292277026595
#endif
#endif

#define SECONDS_PER_DAY 86400
#define SECONDS_PER_YEAR (SECONDS_PER_DAY * 365)
/* Days between 0-01-01 and 1900-01-01 */
#define DAYS_FROM_0_TO_1900 693595
/* Days between 0-01-01 and 1970-01-01 */
#define DAYS_FROM_0_TO_1970 719162
#define MINIMUM_TIMESTAMP_YEAR -292277022657
#define MAXIMUM_TIMESTAMP_YEAR  292277026596

static const int monthDays[2][12] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

static const time_t yearDays[2][13] = {
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 366},
    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 367}};



/**
 *  Convert a broken down time in '*timeptr' to a timestamp.
 *  The timestamp is expressed in seconds since the Unix Epoch.
 *  The Unix Epoch (1970-01-01 00:00:00 UTC) corresponds to 0.
 *  The date/time in '*timeptr' is not checked for correctness.
 *  @return the timestamp that corresponds to the time.
 */
time_t unchecked_mkutc (struct tm *timeptr)

  {
    int real_year;
    int leap_year;
    int year_before;
    time_t timestamp;

  /* unchecked_mkutc */
    logFunction(printf("unchecked_mkutc([%d, %d, %d, %d, %d, %d])\n",
                       timeptr->tm_year, timeptr->tm_mon, timeptr->tm_mday,
                       timeptr->tm_hour, timeptr->tm_min, timeptr->tm_sec););
    real_year = 1900 + timeptr->tm_year;
    if ((real_year % 4 == 0 && real_year % 100 != 0) || real_year % 400 == 0) {
      leap_year = 1;
    } else {
      leap_year = 0;
    } /* if */
    year_before = real_year - 1;
    if (year_before < 0) {
      /* Year_before is divided and truncated towards minus infinite. */
      /* This is done with: quotient = (dividend + 1) / divisor - 1;  */
      timestamp = (time_t) year_before * 365 +
                  ((time_t) real_year / 4 - 1) -
                  ((time_t) real_year / 100 - 1) +
                  ((time_t) real_year / 400 - 1);
    } else {
      timestamp = (time_t) year_before * 365 +
                  (time_t) year_before / 4 -
                  (time_t) year_before / 100 +
                  (time_t) year_before / 400;
    } /* if */
    timestamp = (((timestamp -
                (time_t) DAYS_FROM_0_TO_1970 +
                yearDays[leap_year][timeptr->tm_mon] +
                (time_t) timeptr->tm_mday - 1) * 24 +
                (time_t) timeptr->tm_hour) * 60 +
                (time_t) timeptr->tm_min) * 60 +
                (time_t) timeptr->tm_sec;
    logFunction(printf("unchecked_mkutc --> " FMT_T "\n", timestamp););
    return timestamp;
  } /* unchecked_mkutc */



/**
 *  Convert a timestamp into a time from the local time zone.
 *  The timestamp is expressed in seconds since the Unix Epoch.
 *  The Unix Epoch (1970-01-01 00:00:00 UTC) corresponds to 0.
 *  @param timestamp Unix timestamp to be converted.
 *  @return the local time that corresponds to the timestamp.
 */
void timFromTimestamp (time_t timestamp,
    intType *year, intType *month, intType *day, intType *hour,
    intType *minute, intType *second, intType *micro_sec, intType *timeZone,
    boolType *isDst)

  {
#if defined USE_LOCALTIME_R || defined USE_LOCALTIME_S
    struct tm tm_result;
#endif
    struct tm *local_time;

  /* timFromTimestamp */
    logFunction(printf("timFromTimestamp(" FMT_T ")\n", timestamp););
#if defined USE_LOCALTIME_R
    local_time = localtime_r(&timestamp, &tm_result);
#elif defined USE_LOCALTIME_S
    if (localtime_s(&tm_result, &timestamp) != 0) {
      local_time = NULL;
    } else {
      local_time = &tm_result;
    } /* if */
#else
    local_time = localtime(&timestamp);
#endif
    if (unlikely(local_time == NULL)) {
      logError(printf("timFromTimestamp(" FMT_T "): "
                      "One of localtime/localtime_r/localtime_s failed:\n"
                      "errno=%d\nerror: %s\n",
                      timestamp, errno, strerror(errno)););
      raise_error(RANGE_ERROR);
    } else {
      *year      = local_time->tm_year + 1900;
      *month     = local_time->tm_mon + 1;
      *day       = local_time->tm_mday;
      *hour      = local_time->tm_hour;
      *minute    = local_time->tm_min;
      *second    = local_time->tm_sec;
      *micro_sec = 0;
      *timeZone  = ((intType) unchecked_mkutc(local_time) - (intType) timestamp) / 60;
      /* Correct timeZone values that are outside of the allowed range. */
      /* Under Linux this never happens, but Windows has this problem.  */
      if (unlikely(*timeZone < -12 * 60)) {
        *timeZone += 24 * 60;
        if (*day < monthDays[((*year % 4 == 0 && *year % 100 != 0) || *year % 400 == 0)][*month - 1]) {
          (*day)++;
        } else if (*month < 12) {
          (*month)++;
          *day = 1;
        } else {
          (*year)++;
          *month = 1;
          *day = 1;
        } /* if */
      } else if (unlikely(*timeZone > 14 * 60)) {
        *timeZone -= 24 * 60;
        if (*day > 1) {
          (*day)--;
        } else if (*month > 1) {
          (*month)--;
          *day = monthDays[((*year % 4 == 0 && *year % 100 != 0) || *year % 400 == 0)][*month - 1];
        } else {
          (*year)--;
          *month = 12;
          *day = 31;
        } /* if */
      } /* if */
      *isDst     = local_time->tm_isdst > 0;
    } /* if */
    logFunction(printf("timFromTimestamp(" FMT_T ", "
                       F_D(04) "-" F_D(02) "-" F_D(02) " "
                       F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) " "
                       FMT_D " %d) -->\n",
                       timestamp, *year, *month, *day, *hour, *minute, *second,
                       *micro_sec, *timeZone, *isDst););
  } /* timFromTimestamp */



/**
 *  Convert a timestamp into a time from the local time zone.
 *  The timestamp is expressed in seconds since the Unix Epoch.
 *  The Unix Epoch (1970-01-01 00:00:00 UTC) corresponds to 0.
 *  @param timestamp Unix timestamp to be converted.
 *  @return the local time that corresponds to the timestamp.
 */
void timFromIntTimestamp (intType timestamp,
    intType *year, intType *month, intType *day, intType *hour,
    intType *minute, intType *second, intType *micro_sec, intType *timeZone,
    boolType *isDst)

  { /* timFromIntTimestamp */
    if (!inTimeTRange(timestamp)) {
      logError(printf("timFromIntTimestamp(" FMT_D "): "
                      "Timestamp not in allowed range of time_t.\n",
                      timestamp););
      raise_error(RANGE_ERROR);
    } else {
      timFromTimestamp((time_t) timestamp,
          year, month, day, hour, minute, second, micro_sec, timeZone, isDst);
    } /* if */
  } /* timFromIntTimestamp */



/**
 *  Convert a timestamp into an UTC time.
 *  The timestamp is expressed in seconds since the Unix Epoch.
 *  The Unix Epoch (1970-01-01 00:00:00 UTC) corresponds to 0.
 *  @param timestamp Unix timestamp to be converted.
 */
void timUtcFromTimestamp (timeStampType timestamp,
    intType *year, intType *month, intType *day,
    intType *hour, intType *minute, intType *second)

  {
    int64Type daysSince1970_01_01;
    int64Type currentYear;
    int64Type previousYear;
    int64Type dayJan1;
    int64Type yearDay;
    int32Type secondsSinceMidnight;
    int leapYear;
    int monthIdx;

  /* timUtcFromTimestamp */
    logFunction(printf("timUtcFromTimestamp(" FMT_D64 ")\n", timestamp););
    if (timestamp < 0) {
      daysSince1970_01_01 = (timestamp + 1) / SECONDS_PER_DAY - 1;
      secondsSinceMidnight = (int32Type) (timestamp - daysSince1970_01_01 * SECONDS_PER_DAY);
      currentYear = 1969 + timestamp / SECONDS_PER_YEAR;
      do {
        previousYear = currentYear - 1;
        if (previousYear < 0) {
          /* previousYear is divided and truncated towards minus infinite. */
          /* This is done with: quotient = (dividend + 1) / divisor - 1; */
          dayJan1 = previousYear * 365 +
                    (currentYear / 4 - 1) -
                    (currentYear / 100 - 1) +
                    (currentYear / 400 - 1) -
                    DAYS_FROM_0_TO_1970;
        } else {
          dayJan1 = previousYear * 365 +
                    previousYear / 4 -
                    previousYear / 100 +
                    previousYear / 400 -
                    DAYS_FROM_0_TO_1970;
        } /* if */
        yearDay = daysSince1970_01_01 - dayJan1;
        if ((currentYear % 4 == 0 && currentYear % 100 != 0) || currentYear % 400 == 0) {
          leapYear = 1;
        } else {
          leapYear = 0;
        } /* if */
        if (yearDay >= (leapYear ? 366 : 365)) {
          /* Add at least one year. Underestimate the added number */
          /* of years, by assuming that every year is a leap year. */
          currentYear += (yearDay - (leapYear ? 366 : 365)) / 366 + 1;
        } /* if */
      } while (yearDay >= (leapYear ? 366 : 365));
    } else {
      daysSince1970_01_01 = timestamp / SECONDS_PER_DAY;
      secondsSinceMidnight = (int32Type) (timestamp % SECONDS_PER_DAY);
      currentYear = 1970 + timestamp / SECONDS_PER_YEAR;
      do {
        previousYear = currentYear - 1;
        dayJan1 = previousYear * 365 +
                  previousYear / 4 -
                  previousYear / 100 +
                  previousYear / 400 -
                  DAYS_FROM_0_TO_1970;
        yearDay = daysSince1970_01_01 - dayJan1;
        if (yearDay < 0) {
          /* Subtract at least one year. Underestimate the subtracted     */
          /* number of years, by assuming that every year is a leap year. */
          currentYear -= (-yearDay - 1) / 366 + 1;
        } /* if */
      } while (yearDay < 0);
    } /* if */
    if ((currentYear % 4 == 0 && currentYear % 100 != 0) || currentYear % 400 == 0) {
      leapYear = 1;
    } else {
      leapYear = 0;
    } /* if */
    for (monthIdx = 0; monthIdx < 12 && yearDay >= yearDays[leapYear][monthIdx]; monthIdx++) ;
    *year = currentYear;
    *month = monthIdx;
    *day = yearDay - (int64Type) yearDays[leapYear][monthIdx - 1] + 1;
    *hour = secondsSinceMidnight / 3600;
    *minute = (secondsSinceMidnight / 60) % 60;
    *second = secondsSinceMidnight % 60;
    logFunction(printf("timUtcFromTimestamp -> "
                       F_D(04) "-" F_D(02) "-" F_D(02) " "
                       F_D(02) ":" F_D(02) ":" F_D(02) "\n",
                       *year, *month, *day, *hour, *minute, *second););
  } /* timUtcFromTimestamp */



/**
 *  Convert a time with timeZone to a timestamp.
 *  The timestamp is expressed in seconds since the Unix Epoch.
 *  The Unix Epoch (1970-01-01 00:00:00 UTC) corresponds to 0.
 *  @return the timestamp that corresponds to the time, or
 *          TIMESTAMPTYPE_MIN if a value is not in the allowed range.
 */
timeStampType timToTimestamp (intType year, intType month, intType day,
    intType hour, intType minute, intType second, intType timeZone)

  {
    int leap_year;
    timeStampType previousYear;
    timeStampType timestamp;

  /* timToTimestamp */
    logFunction(printf("timToTimestamp(" F_D(04) "-" F_D(02) "-" F_D(02) " "
                                         F_D(02) ":" F_D(02) ":" F_D(02) " "
                                         FMT_D ")\n",
                       year, month, day, hour, minute, second, timeZone););
    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
      leap_year = 1;
    } else {
      leap_year = 0;
    } /* if */
    if (unlikely(year <= MINIMUM_TIMESTAMP_YEAR ||
                 year >= MAXIMUM_TIMESTAMP_YEAR ||
                 month < 1 || month > 12 || day < 1 ||
                 day > monthDays[leap_year][month - 1] ||
                 hour < 0 || hour >= 24 ||
                 minute < 0 || minute >= 60 ||
                 second < 0 || second >= 60)) {
      logError(printf("timToTimestamp(" F_D(04) "-" F_D(02) "-" F_D(02) " "
                                        F_D(02) ":" F_D(02) ":" F_D(02) " "
                                        FMT_D "): Not in allowed range.\n",
                       year, month, day, hour, minute, second, timeZone););
      timestamp = TIMESTAMPTYPE_MIN;
    } else {
      previousYear = (timeStampType) year - 1;
      if (previousYear < 0) {
        /* previousYear is divided and truncated towards minus infinite. */
        /* This is done with: quotient = (dividend + 1) / divisor - 1;  */
        timestamp = previousYear * 365 +
                    ((timeStampType) year / 4 - 1) -
                    ((timeStampType) year / 100 - 1) +
                    ((timeStampType) year / 400 - 1);
      } else {
        timestamp = previousYear * 365 +
                    previousYear / 4 -
                    previousYear / 100 +
                    previousYear / 400;
        /* printf("timestamp: " FMT_D64 "\n", timestamp - DAYS_FROM_0_TO_1970); */
      } /* if */
      timestamp = (((timestamp -
                  (timeStampType) DAYS_FROM_0_TO_1970 +
                  yearDays[leap_year][month - 1] +
                  (timeStampType) day - 1) * 24 +
                  (timeStampType) hour) * 60 +
                  (timeStampType) minute) * 60 +
                  (timeStampType) second;
      if (unlikely(timeZone < -1500 || timeZone > 1500)) {
        logError(printf("timToTimestamp("
                        F_D(04) "-" F_D(02) "-" F_D(02) " "
                        F_D(02) ":" F_D(02) ":" F_D(02) " " FMT_D
                        "): timeZone not in allowed range.\n",
                        year, month, day, hour, minute, second, timeZone););
        timestamp = TIMESTAMPTYPE_MIN;
      } else {
        timestamp -= (timeStampType) timeZone * 60;
      } /* if */
    } /* if */
    logFunction(printf("timToTimestamp --> " FMT_D64 "\n", timestamp););
    return timestamp;
  } /* timToTimestamp */



#if TIME_T_SIZE != TIMESTAMPTYPE_SIZE || !TIME_T_SIGNED
/**
 *  Convert a time with timeZone to a timestamp.
 *  The timestamp is expressed in seconds since the Unix Epoch.
 *  The Unix Epoch (1970-01-01 00:00:00 UTC) corresponds to 0.
 *  @return the timestamp that corresponds to the time, or
 *          TIME_T_ERROR if a value is not in the allowed range.
 */
time_t timToOsTimestamp (intType year, intType month, intType day,
    intType hour, intType minute, intType second, intType timeZone)

  {
    timeStampType timestamp;
    time_t osTimestamp;

  /* timToOsTimestamp */
    logFunction(printf("timToOsTimestamp(" F_D(04) "-" F_D(02) "-" F_D(02) " "
                                           F_D(02) ":" F_D(02) ":" F_D(02) " "
                                           FMT_D ")\n",
                       year, month, day, hour, minute, second, timeZone););
    timestamp = timToTimestamp(year, month, day, hour, minute, second,
                               timeZone);
    if (unlikely(timestamp == TIMESTAMPTYPE_MIN || !inTimeTRange(timestamp))) {
      osTimestamp = TIME_T_ERROR;
    } else {
      osTimestamp = (time_t) timestamp;
    } /* if */
    logFunction(printf("timToOsTimestamp --> " FMT_T "\n", timestamp););
    return osTimestamp;
  } /* timToOsTimestamp */
#endif



/**
 *  Sets timeZone and daylightSavingTime for a given time.
 *  @return the time in the local time zone.
 */
void timSetLocalTZ (intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType *timeZone, boolType *isDst)

  {
    time_t timestamp;
#if defined USE_LOCALTIME_R || defined USE_LOCALTIME_S
    struct tm tm_result;
#endif
    struct tm *local_time;
    time_t timeZoneReference;

  /* timSetLocalTZ */
    logFunction(printf("timSetLocalTZ(" F_D(04) "-" F_D(02) "-" F_D(02) " "
                                        F_D(02) ":" F_D(02) ":" F_D(02) ")\n",
                       year, month, day, hour, minute, second););
    timestamp = 0;
#if defined USE_LOCALTIME_R
    local_time = localtime_r(&timestamp, &tm_result);
#elif defined USE_LOCALTIME_S
    if (localtime_s(&tm_result, &timestamp) != 0) {
      local_time = NULL;
    } else {
      local_time = &tm_result;
    } /* if */
#else
    local_time = localtime(&timestamp);
#endif
    if (unlikely(local_time == NULL)) {
      logError(printf("timSetLocalTZ: "
                      "One of localtime/localtime_r/localtime_s(" FMT_T ") failed:\n"
                      "errno=%d\nerror: %s\n",
                      timestamp, errno, strerror(errno)););
      raise_error(RANGE_ERROR);
    } else {
      timeZoneReference = unchecked_mkutc(local_time) / 60;
      /* printf("timeZoneReference: %ld\n", timeZoneReference); */
      timestamp = timToOsTimestamp(year, month, day, hour, minute, second, 0);
      if (unlikely(timestamp == TIME_T_ERROR)) {
        *timeZone = timeZoneReference;
        *isDst    = 0;
      } else {
        /* printf("timestamp: %ld\n", timestamp); */
        timestamp -= timeZoneReference * 60;
#if !LOCALTIME_WORKS_SIGNED
        if (timestamp < 0) {
          *timeZone = timeZoneReference;
          *isDst    = 0;
        } else {
#endif
#if defined USE_LOCALTIME_R
          local_time = localtime_r(&timestamp, &tm_result);
#elif defined USE_LOCALTIME_S
          if (localtime_s(&tm_result, &timestamp) != 0) {
            local_time = NULL;
          } else {
            local_time = &tm_result;
          } /* if */
#else
          local_time = localtime(&timestamp);
#endif
          if (unlikely(local_time == NULL)) {
            logError(printf("timSetLocalTZ: One of "
                            "localtime/localtime_r/localtime_s(" FMT_T ") failed:\n"
                            "errno=%d\nerror: %s\n",
                            timestamp, errno, strerror(errno)););
            raise_error(RANGE_ERROR);
          } else {
            *timeZone = ((intType) unchecked_mkutc(local_time) - (intType) timestamp) / 60;
            /* Correct timeZone values that are outside of the allowed range. */
            /* Under Linux this never happens, but Windows has this problem.  */
            if (unlikely(*timeZone < -12 * 60)) {
              *timeZone += 24 * 60;
            } else if (unlikely(*timeZone > 14 * 60)) {
              *timeZone -= 24 * 60;
            } /* if */
            *isDst    = local_time->tm_isdst > 0;
          } /* if */
#if !LOCALTIME_WORKS_SIGNED
        } /* if */
#endif
      } /* if */
    } /* if */
    logFunction(printf("timSetLocalTZ(" F_D(04) "-" F_D(02) "-" F_D(02) " "
                       F_D(02) ":" F_D(02) ":" F_D(02) " " FMT_D " %d) -->\n",
                       year, month, day, hour, minute, second,
                       *timeZone, *isDst););
  } /* timSetLocalTZ */



/**
 *  Assign year, month and day from the number of days since 1900-01-01.
 *  This functions works correct for the whole range of daysSince1900_01_01.
 */
void dateFromDaysSince1900 (int32Type daysSince1900_01_01,
    intType *year, intType *month, intType *day)

  {
    int32Type currentYear;
    int32Type previousYear;
    int32Type dayJan1;
    int32Type yearDay;
    int leapYear;
    int monthIdx;

  /* dateFromDaysSince1900 */
    logFunction(printf("dateFromDaysSince1900(" FMT_D32 ")\n", daysSince1900_01_01););
    if (daysSince1900_01_01 < 0) {
      currentYear = 1899 + daysSince1900_01_01 / 365;
      do {
        previousYear = currentYear - 1;
        if (previousYear < 0) {
          /* previousYear is divided and truncated towards minus infinite. */
          /* This is done with: quotient = (dividend + 1) / divisor - 1; */
          dayJan1 = previousYear * 365 +
                    (currentYear / 4 - 1) -
                    (currentYear / 100 - 1) +
                    (currentYear / 400 - 1) -
                    DAYS_FROM_0_TO_1900;
        } else {
          dayJan1 = previousYear * 365 +
                    previousYear / 4 -
                    previousYear / 100 +
                    previousYear / 400 -
                    DAYS_FROM_0_TO_1900;
        } /* if */
        yearDay = daysSince1900_01_01 - dayJan1;
        if ((currentYear % 4 == 0 && currentYear % 100 != 0) || currentYear % 400 == 0) {
          leapYear = 1;
        } else {
          leapYear = 0;
        } /* if */
        if (yearDay >= (leapYear ? 366 : 365)) {
          /* Add at least one year. Underestimate the added number */
          /* of years, by assuming that every year is a leap year. */
          currentYear += (yearDay - (leapYear ? 366 : 365)) / 366 + 1;
        } /* if */
      } while (yearDay >= (leapYear ? 366 : 365));
    } else {
      currentYear = 1900 + daysSince1900_01_01 / 365;
      do {
        previousYear = currentYear - 1;
        dayJan1 = previousYear * 365 +
                  previousYear / 4 -
                  previousYear / 100 +
                  previousYear / 400 -
                  DAYS_FROM_0_TO_1900;
        yearDay = daysSince1900_01_01 - dayJan1;
        if (yearDay < 0) {
          /* Subtract at least one year. Underestimate the subtracted     */
          /* number of years, by assuming that every year is a leap year. */
          currentYear -= (-yearDay - 1) / 366 + 1;
        } /* if */
      } while (yearDay < 0);
    } /* if */
    if ((currentYear % 4 == 0 && currentYear % 100 != 0) || currentYear % 400 == 0) {
      leapYear = 1;
    } else {
      leapYear = 0;
    } /* if */
    for (monthIdx = 0; monthIdx < 12 && yearDay >= yearDays[leapYear][monthIdx]; monthIdx++) ;
    *year = currentYear;
    *month = monthIdx;
    *day = yearDay - (int32Type) yearDays[leapYear][monthIdx - 1] + 1;
    logFunction(printf("dateFromDaysSince1900 -> " F_D(04) "-" F_D(02) "-" F_D(02) "\n",
                       *year, *month, *day););
  } /* dateFromDaysSince1900 */



/**
 *  Assign date and time from a string with the given 'isoDate'.
 *  Possible formats for 'isoData' are "yyyy-mm-dd hh:mm:ss.uuuuuu",
 *  "yyyy-mm-dd hh:mm:ss", "yyyy-mm-dd hh:mm", "yyyy-mm-dd hh",
 *  "yyyy-mm-dd", "yyyy-mm", "hh:mm:ss.uuuuuu", "hh:mm:ss" and "hh:mm".
 *  @param isoDate String with an ISO date/time.
 */
boolType assignTime (const_cstriType isoDate, intType *year, intType *month,
    intType *day, intType *hour, intType *minute, intType *second,
    intType *microSecond, boolType *isTime)

  {
    int numAssigned;
    memSizeType microsecStriLen;
    char microsecStri[6 + NULL_TERMINATION_LEN];
    boolType okay = FALSE;

  /* assignTime */
    numAssigned = sscanf(isoDate,
                         FMT_D "-" FMT_U "-" FMT_U " "
                         FMT_U ":" FMT_U ":" FMT_U ".%6s",
                         year, month, day, hour, minute, second,
                         microsecStri);
    if (numAssigned >= 2) {
      *isTime = FALSE;
      if (numAssigned == 7) {
        microsecStriLen = (memSizeType) strlen(microsecStri);
        if (microsecStriLen < 6) {
          memset(&microsecStri[microsecStriLen], '0', 6 - microsecStriLen);
          microsecStri[6] = '\0';
        } /* if */
        okay = sscanf((const char *) microsecStri, FMT_D, microSecond) == 1;
      } else {
        okay = TRUE;
        *microSecond = 0;
        if (numAssigned <= 5) {
          *second = 0;
          if (numAssigned <= 4) {
            *minute = 0;
            if (numAssigned <= 3) {
              *hour = 0;
              if (numAssigned <= 2) {
                *day = 1;
              } /* if */
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } else {
      numAssigned = sscanf(isoDate,
                           FMT_U ":" FMT_U ":" FMT_U ".%6s",
                           hour, minute, second, microsecStri);
      if (numAssigned >= 2) {
        *year        = 0;
        *month       = 1;
        *day         = 1;
        *microSecond = 0;
        *isTime = TRUE;
        if (numAssigned == 4) {
          microsecStriLen = (memSizeType) strlen(microsecStri);
          if (microsecStriLen < 6) {
            memset(&microsecStri[microsecStriLen], '0', 6 - microsecStriLen);
            microsecStri[6] = '\0';
          } /* if */
          okay = sscanf((const char *) microsecStri, FMT_D, microSecond) == 1;
        } else {
          okay = TRUE;
          *microSecond = 0;
          if (numAssigned <= 2) {
            *second = 0;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("assignTime(" F_D(04) "-" F_D(02) "-" F_D(02) " "
                                     F_D(02) ":" F_D(02) ":" F_D(02) "."
                                     F_D(06) ", %d) --> %d\n",
                       *year, *month, *day, *hour, *minute, *second,
                       *microSecond, *isTime, okay););
    return okay;
  } /* assignTime */

/********************************************************************/
/*                                                                  */
/*  tim_rtl.c     Time access using the C capabilities.             */
/*  Copyright (C) 1989 - 2013  Thomas Mertes                        */
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
/*  Changes: 2009, 2013  Thomas Mertes                              */
/*  Content: Time access using the C capabilities.                  */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

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

/* Days between 0-01-01 and 1970-01-01 */
#define DAYS_FROM_0_TO_1970 719162

static const int month_days[2][12] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

static const time_t year_days[2][12] = {
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}};



/**
 *  Convert a broken down time in '*timeptr' to a timestamp.
 *  The timestamp is expressed in seconds since the Unix Epoch.
 *  The Unix Epoch (1970-01-01 00:00:00 UTC) corresponds to 0.
 *  @return the timestamp that corresponds to the time, or
 *          -1 if the date/time in '*timeptr' is not correct.
 */
time_t mkutc (struct tm *timeptr)

  {
    int real_year;
    int leap_year;
    int year_before;
    time_t timestamp;

  /* mkutc */
    logFunction(printf("mkutc([%d, %d, %d, %d, %d, %d])\n",
                       timeptr->tm_year, timeptr->tm_mon, timeptr->tm_mday,
                       timeptr->tm_hour, timeptr->tm_min, timeptr->tm_sec););
    real_year = 1900 + timeptr->tm_year;
    if ((real_year % 4 == 0 && real_year % 100 != 0) || real_year % 400 == 0) {
      leap_year = 1;
    } else {
      leap_year = 0;
    } /* if */
    if (unlikely(
        real_year < STRUCT_TM_MIN_YEAR || real_year > STRUCT_TM_MAX_YEAR ||
        timeptr->tm_mon < 0 || timeptr->tm_mon > 11 || timeptr->tm_mday < 1 ||
        timeptr->tm_mday > month_days[leap_year][timeptr->tm_mon] ||
        timeptr->tm_hour < 0 || timeptr->tm_hour >= 24 ||
        timeptr->tm_min < 0 || timeptr->tm_min >= 60 ||
        timeptr->tm_sec < 0 || timeptr->tm_sec >= 60)) {
      timestamp = (time_t) -1;
    } else {
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
                  year_days[leap_year][timeptr->tm_mon] +
                  (time_t) timeptr->tm_mday - 1) * 24 +
                  (time_t) timeptr->tm_hour) * 60 +
                  (time_t) timeptr->tm_min) * 60 +
                  (time_t) timeptr->tm_sec;
    } /* if */
    logFunction(printf("mkutc --> " FMT_T "\n", timestamp););
    return timestamp;
  } /* mkutc */



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
                year_days[leap_year][timeptr->tm_mon] +
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
 *  @return the local time that corresponds to the timestamp.
 */
void timFromTimestamp (time_t timestamp,
    intType *year, intType *month, intType *day, intType *hour,
    intType *min, intType *sec, intType *micro_sec, intType *time_zone,
    boolType *is_dst)

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
      *min       = local_time->tm_min;
      *sec       = local_time->tm_sec;
      *micro_sec = 0;
      *time_zone = (intType) (unchecked_mkutc(local_time) - timestamp) / 60;
      *is_dst    = local_time->tm_isdst > 0;
    } /* if */
    logFunction(printf("timFromTimestamp(%ld, " F_D(04) "-" F_D(02) "-" F_D(02),
                       timestamp, *year, *month, *day);
                printf(" " F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) " " FMT_D " %d) -->\n",
                       *hour, *min, *sec, *micro_sec, *time_zone, *is_dst););
  } /* timFromTimestamp */



/**
 *  Convert a timestamp into a time from the local time zone.
 *  The timestamp is expressed in seconds since the Unix Epoch.
 *  The Unix Epoch (1970-01-01 00:00:00 UTC) corresponds to 0.
 *  @return the local time that corresponds to the timestamp.
 */
void timFromIntTimestamp (intType timestamp,
    intType *year, intType *month, intType *day, intType *hour,
    intType *min, intType *sec, intType *micro_sec, intType *time_zone,
    boolType *is_dst)

  { /* timFromIntTimestamp */
    if (!inTimeTRange(timestamp)) {
      logError(printf("timFromIntTimestamp(" FMT_D "): "
                      "Timestamp not in allowed range of time_t.\n",
                      timestamp););
      raise_error(RANGE_ERROR);
    } else {
      timFromTimestamp((time_t) timestamp,
          year, month, day, hour, min, sec, micro_sec, time_zone, is_dst);
    } /* if */
  } /* timFromIntTimestamp */



void timUtcFromTimestamp (time_t timestamp,
    intType *year, intType *month, intType *day, intType *hour,
    intType *min, intType *sec, intType *micro_sec, intType *time_zone,
    boolType *is_dst)

  {
#if defined HAS_GMTIME_R || defined HAS_GMTIME_S
    struct tm tm_result;
#endif
    struct tm *gm_time;

  /* timUtcFromTimestamp */
    logFunction(printf("timUtcFromTimestamp(" FMT_T ")\n", timestamp););
#if defined HAS_GMTIME_R
    gm_time = gmtime_r(&timestamp, &tm_result);
#elif defined HAS_GMTIME_S
    if (gmtime_s(&tm_result, &timestamp) != 0) {
      gm_time = NULL;
    } else {
      gm_time = &tm_result;
    } /* if */
#else
    gm_time = gmtime(&timestamp);
#endif
    if (unlikely(gm_time == NULL)) {
      logError(printf("timUtcFromTimestamp(" FMT_T "): "
                      "One of gmtime/gmtime_r/gmtime_s failed:\n"
                      "errno=%d\nerror: %s\n",
                      timestamp, errno, strerror(errno)););
      raise_error(RANGE_ERROR);
    } else {
      *year      = gm_time->tm_year + 1900;
      *month     = gm_time->tm_mon + 1;
      *day       = gm_time->tm_mday;
      *hour      = gm_time->tm_hour;
      *min       = gm_time->tm_min;
      *sec       = gm_time->tm_sec;
      *micro_sec = 0;
      *time_zone = (intType) (unchecked_mkutc(gm_time) - timestamp) / 60;
      *is_dst    = gm_time->tm_isdst > 0;
    } /* if */
    logFunction(printf("timUtcFromTimestamp(%ld, " F_D(04) "-" F_D(02) "-" F_D(02),
                       timestamp, *year, *month, *day);
                printf(" " F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) " " FMT_D " %d) -->\n",
                       *hour, *min, *sec, *micro_sec, *time_zone, *is_dst););
  } /* timUtcFromTimestamp */



/**
 *  Convert a time with time_zone to a timestamp.
 *  The timestamp is expressed in seconds since the Unix Epoch.
 *  The Unix Epoch (1970-01-01 00:00:00 UTC) corresponds to 0.
 *  @return the timestamp that corresponds to the time.
 */
time_t timToTimestamp (intType year, intType month, intType day, intType hour,
    intType min, intType sec, intType micro_sec, intType time_zone)

  {
    struct tm tm_time;
    time_t timestamp;

  /* timToTimestamp */
    logFunction(printf("timToTimestamp(" F_D(04) "-" F_D(02) "-" F_D(02),
                       year, month, day);
                printf(" " F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) " " FMT_D ") -->\n",
                       hour, min, sec, micro_sec, time_zone););
    tm_time.tm_year  = (int) year - 1900;
    tm_time.tm_mon   = (int) month - 1;
    tm_time.tm_mday  = (int) day;
    tm_time.tm_hour  = (int) hour;
    tm_time.tm_min   = (int) min;
    tm_time.tm_sec   = (int) sec;
    tm_time.tm_isdst = 0;
    timestamp = mkutc(&tm_time);
    if (likely(timestamp != (time_t) -1)) {
      if (unlikely(time_zone < -1500 || time_zone > 1500)) {
        timestamp = (time_t) -1;
      } else {
        timestamp -= (time_t) time_zone * 60;
      } /* if */
    } /* if */
    logFunction(printf("timToTimestamp --> " FMT_T "\n", timestamp););
    return timestamp;
  } /* timToTimestamp */



/**
 *  Sets timeZone and daylightSavingTime for a given time.
 *  @return the time in the local time zone.
 */
void timSetLocalTZ (intType year, intType month, intType day, intType hour,
    intType min, intType sec, intType *time_zone, boolType *is_dst)

  {
    struct tm tm_time;
    time_t timestamp;
#if defined USE_LOCALTIME_R || defined USE_LOCALTIME_S
    struct tm tm_result;
#endif
    struct tm *local_time;
    time_t time_zone_reference;

  /* timSetLocalTZ */
    logFunction(printf("timSetLocalTZ(" F_D(04) "-" F_D(02) "-" F_D(02),
                       year, month, day);
                printf(" " F_D(02) ":" F_D(02) ":" F_D(02) ")\n",
                       hour, min, sec););
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
      time_zone_reference = unchecked_mkutc(local_time) / 60;
      /* printf("time_zone_reference: %ld\n", time_zone_reference); */
      tm_time.tm_year  = (int) year - 1900;
      tm_time.tm_mon   = (int) month - 1;
      tm_time.tm_mday  = (int) day;
      tm_time.tm_hour  = (int) hour;
      tm_time.tm_min   = (int) min;
      tm_time.tm_sec   = (int) sec;
      tm_time.tm_isdst = 0;
      timestamp = mkutc(&tm_time);
      if (unlikely(timestamp == (time_t) -1)) {
        *time_zone = time_zone_reference;
        *is_dst    = 0;
      } else {
        /* printf("timestamp: %ld\n", timestamp); */
        timestamp -= time_zone_reference * 60;
#if !LOCALTIME_WORKS_SIGNED
        if (timestamp < 0) {
          *time_zone = time_zone_reference;
          *is_dst    = 0;
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
            *time_zone = (intType) (unchecked_mkutc(local_time) - timestamp) / 60;
            *is_dst    = local_time->tm_isdst > 0;
          } /* if */
#if !LOCALTIME_WORKS_SIGNED
        } /* if */
#endif
      } /* if */
    } /* if */
    logFunction(printf("timSetLocalTZ(" F_D(04) "-" F_D(02) "-" F_D(02),
                       year, month, day);
                printf(" " F_D(02) ":" F_D(02) ":" F_D(02) " " FMT_D " %d) -->\n",
                       hour, min, sec, *time_zone, *is_dst););
  } /* timSetLocalTZ */



/**
 *  Convert a bigInteger timestamp into a time from the local time zone.
 *  The timestamp is expressed in seconds since the Unix Epoch.
 *  The Unix Epoch (1970-01-01 00:00:00 UTC) corresponds to 0.
 *  @return the local time that corresponds to the timestamp.
 */
void timFromBigTimestamp (const const_bigIntType timestamp,
    intType *year, intType *month, intType *day, intType *hour,
    intType *min, intType *sec, intType *micro_sec, intType *time_zone,
    boolType *is_dst)

  {
    time_t os_timestamp;

  /* timFromBigTimestamp */
    if (sizeof(time_t) == 8) {
      os_timestamp = (time_t) bigToInt64(timestamp, NULL);
    } else {
      os_timestamp = (time_t) bigToInt32(timestamp, NULL);
    } /* if */
    timFromTimestamp(os_timestamp,
        year, month, day, hour, min, sec, micro_sec, time_zone, is_dst);
  } /* timFromBigTimestamp */



/**
 *  Convert a time with time_zone to a bigInteger timestamp.
 *  The timestamp is expressed in seconds since the Unix Epoch.
 *  The Unix Epoch (1970-01-01 00:00:00 UTC) corresponds to 0.
 *  @return the timestamp that corresponds to the time.
 */
bigIntType timToBigTimestamp (intType year, intType month, intType day, intType hour,
    intType min, intType sec, intType micro_sec, intType time_zone)

  {
    time_t os_timestamp;
    bigIntType timestamp;

  /* timToBigTimestamp */
    os_timestamp = timToTimestamp(year, month, day, hour, min, sec, micro_sec, time_zone);
    if (sizeof(time_t) == 8) {
#ifdef INT64TYPE
      timestamp = bigFromInt64((int64Type) os_timestamp);
#else
      if (os_timestamp > INT32TYPE_MAX || os_timestamp < INT32TYPE_MIN) {
        logError(printf("timToBigTimestamp: Size of time_t too big " FMT_T ").\n",
                        os_timestamp););
        raise_error(RANGE_ERROR);
        timestamp = NULL;
      } else {
        timestamp = bigFromInt32((int32Type) os_timestamp);
      } /* if */
#endif
    } else {
      timestamp = bigFromInt32((int32Type) os_timestamp);
    } /* if */
    return timestamp;
  } /* timToBigTimestamp */

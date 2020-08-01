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

static const int month_days[2][12] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

static const time_t year_days[2][12] = {
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}};



time_t mkutc (struct tm *timeptr)

  {
    int real_year;
    int leap_year;
    int year_before;
    time_t result;

  /* mkutc */
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
      result = (time_t) -1;
    } else {
      year_before = real_year - 1;
      result = ((((time_t) year_before * 365 +
                           year_before / 4 -
                           year_before / 100 +
                           year_before / 400 -
               (time_t) 719162 +
               year_days[leap_year][timeptr->tm_mon] + timeptr->tm_mday - 1) * 24 +
               (time_t) timeptr->tm_hour) * 60 +
               (time_t) timeptr->tm_min) * 60 +
               (time_t) timeptr->tm_sec;
    } /* if */
    return result;
  } /* mkutc */



time_t unchecked_mkutc (struct tm *timeptr)

  {
    int real_year;
    int leap_year;
    int year_before;
    time_t result;

  /* unchecked_mkutc */
    real_year = 1900 + timeptr->tm_year;
    if ((real_year % 4 == 0 && real_year % 100 != 0) || real_year % 400 == 0) {
      leap_year = 1;
    } else {
      leap_year = 0;
    } /* if */
    year_before = real_year - 1;
    result = ((((time_t) year_before * 365 +
                         year_before / 4 -
                         year_before / 100 +
                         year_before / 400 -
             (time_t) 719162 +
             year_days[leap_year][timeptr->tm_mon] + timeptr->tm_mday - 1) * 24 +
             (time_t) timeptr->tm_hour) * 60 +
             (time_t) timeptr->tm_min) * 60 +
             (time_t) timeptr->tm_sec;
    return result;
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
    logFunction(printf("timFromTimestamp(%ld)\n", timestamp););
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
      raise_error(RANGE_ERROR);
    } else {
      timFromTimestamp((time_t) timestamp,
          year, month, day, hour, min, sec, micro_sec, time_zone, is_dst);
    } /* if */
  } /* timFromIntTimestamp */



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
    time_t result;

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
    result = mkutc(&tm_time);
    if (likely(result != (time_t) -1)) {
      if (unlikely(time_zone < -1500 || time_zone > 1500)) {
        result = (time_t) -1;
      } else {
        result -= (time_t) time_zone * 60;
      } /* if */
    } /* if */
    logFunction(printf("timToTimestamp --> " FMT_T "\n", result););
    return result;
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
      os_timestamp = (time_t) bigToInt64(timestamp);
    } else {
      os_timestamp = (time_t) bigToInt32(timestamp);
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
    bigIntType result;

  /* timToBigTimestamp */
    os_timestamp = timToTimestamp(year, month, day, hour, min, sec, micro_sec, time_zone);
    if (sizeof(time_t) == 8) {
#ifdef INT64TYPE
      result = bigFromInt64((int64Type) os_timestamp);
#else
      if (os_timestamp > INT32TYPE_MAX || os_timestamp < INT32TYPE_MIN) {
        raise_error(RANGE_ERROR);
        result = NULL;
      } else {
        result = bigFromInt32((int32Type) os_timestamp);
      } /* if */
#endif
    } else {
      result = bigFromInt32((int32Type) os_timestamp);
    } /* if */
    return result;
  } /* timToBigTimestamp */

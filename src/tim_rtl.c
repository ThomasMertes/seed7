/********************************************************************/
/*                                                                  */
/*  tim_rtl.c     Time access using the C capabilities.             */
/*  Copyright (C) 1989 - 2009  Thomas Mertes                        */
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
/*  Changes: 2009  Thomas Mertes                                    */
/*  Content: Time access using the C capabilities.                  */
/*                                                                  */
/********************************************************************/

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

#undef TRACE_TIM_RTL


#if TIME_T_SIZE == 32
#ifdef TIME_T_SIGNED
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


static time_t month_days[2][12] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

static time_t year_days[2][12] = {
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}};



#ifdef ANSI_C

time_t mkutc (struct tm *timeptr)
#else

time_t mkutc (timeptr)
struct tm *timeptr;
#endif

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



#ifdef ANSI_C

time_t unchecked_mkutc (struct tm *timeptr)
#else

time_t unchecked_mkutc (timeptr)
struct tm *timeptr;
#endif

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



#ifdef ANSI_C

void timFromTimestamp (time_t timestamp,
    inttype *year, inttype *month, inttype *day, inttype *hour,
    inttype *min, inttype *sec, inttype *micro_sec, inttype *time_zone,
    booltype *is_dst)
#else

void timFromTimestamp (timestamp,
    year, month, day, hour, min, sec, micro_sec, time_zone, is_dst)
time_t timestamp;
inttype *year;
inttype *month;
inttype *day;
inttype *hour;
inttype *min;
inttype *sec;
inttype *micro_sec;
inttype *time_zone;
booltype *is_dst;
#endif

  {
#ifdef USE_LOCALTIME_R
    struct tm tm_result;
#endif
    struct tm *local_time;

  /* timFromTimestamp */
#ifdef TRACE_TIM_RTL
    printf("BEGIN timFromTimestamp(%ld)\n", timestamp);
#endif
#ifdef USE_LOCALTIME_R
    local_time = localtime_r(&timestamp, &tm_result);
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
      *time_zone = (unchecked_mkutc(local_time) - timestamp) / 60;
      *is_dst    = local_time->tm_isdst > 0;
    } /* if */
#ifdef TRACE_TIM_RTL
    printf("END timFromTimestamp(%ld, %04ld-%02ld-%02ld %02ld:%02ld:%02ld.%06ld %ld %d)\n",
        timestamp, *year, *month, *day, *hour, *min, *sec,
        *micro_sec, *time_zone, *is_dst);
#endif
  } /* timFromTimestamp */



#ifdef ANSI_C

void timFromIntTimestamp (inttype timestamp,
    inttype *year, inttype *month, inttype *day, inttype *hour,
    inttype *min, inttype *sec, inttype *micro_sec, inttype *time_zone,
    booltype *is_dst)
#else

void timFromIntTimestamp (timestamp,
    year, month, day, hour, min, sec, micro_sec, time_zone, is_dst)
inttype timestamp;
inttype *year;
inttype *month;
inttype *day;
inttype *hour;
inttype *min;
inttype *sec;
inttype *micro_sec;
inttype *time_zone;
booltype *is_dst;
#endif

  { /* timFromIntTimestamp */
#ifdef TIME_T_SIGNED
#if TIME_T_SIZE < INTTYPE_SIZE
#if TIME_T_SIZE == 32
    if (unlikely(timestamp < INT32TYPE_MIN || timestamp > INT32TYPE_MAX)) {
#elif TIME_T_SIZE == 64
    if (unlikely(timestamp < INT64TYPE_MIN || timestamp > INT64TYPE_MAX)) {
#else
    if (TRUE) {
#endif
      raise_error(RANGE_ERROR);
    } else {
#endif
      timFromTimestamp((time_t) timestamp,
          year, month, day, hour, min, sec, micro_sec, time_zone, is_dst);
#if TIME_T_SIZE < INTTYPE_SIZE
    } /* if */
#endif

#else

    if (unlikely(timestamp < 0)) {
      raise_error(RANGE_ERROR);
    } else {
#if TIME_T_SIZE < INTTYPE_SIZE
#if TIME_T_SIZE == 32
      if (unlikely(timestamp > UINT32TYPE_MAX)) {
#elif TIME_T_SIZE == 64
      if (unlikely(timestamp > UINT64TYPE_MAX)) {
#else
      if (TRUE) {
#endif
        raise_error(RANGE_ERROR);
      } else {
#endif
        timFromTimestamp((time_t) timestamp,
            year, month, day, hour, min, sec, micro_sec, time_zone, is_dst);
#if TIME_T_SIZE < INTTYPE_SIZE
      } /* if */
#endif
    } /* if */
#endif
  } /* timFromIntTimestamp */



#ifdef ANSI_C

time_t timToTimestamp (inttype year, inttype month, inttype day, inttype hour,
    inttype min, inttype sec, inttype micro_sec, inttype time_zone)
#else

time_t timToTimestamp (year, month, day, hour, min, sec, micro_sec, time_zone)
inttype year;
inttype month;
inttype day;
inttype hour;
inttype min;
inttype sec;
inttype micro_sec;
inttype time_zone;
#endif

  {
    struct tm tm_time;
    time_t result;

  /* timToTimestamp */
#ifdef TRACE_TIM_RTL
    printf("BEGIN timToTimestamp(%04ld-%02ld-%02ld %02ld:%02ld:%02ld.%06ld %ld)\n",
        year, month, day, hour, min, sec, micro_sec, time_zone);
#endif
    tm_time.tm_year  = (int) year - 1900;
    tm_time.tm_mon   = (int) month - 1;
    tm_time.tm_mday  = (int) day;
    tm_time.tm_hour  = (int) hour;
    tm_time.tm_min   = (int) min;
    tm_time.tm_sec   = (int) sec;
    tm_time.tm_isdst = 0;
    result = mkutc(&tm_time);
    if (likely(result != (time_t) -1)) {
      result -= time_zone * 60;
    } /* if */
#ifdef TRACE_TIM_RTL
    printf("END timToTimestamp ==> %lu\n", result);
#endif
    return result;
  } /* timToTimestamp */



#ifdef ANSI_C

void timSetLocalTZ (inttype year, inttype month, inttype day, inttype hour,
    inttype min, inttype sec, inttype *time_zone, booltype *is_dst)
#else

void timSetLocalTZ (year, month, day, hour, min, sec, time_zone, is_dst)
inttype year;
inttype month;
inttype day;
inttype hour;
inttype min;
inttype sec;
inttype *time_zone;
booltype *is_dst;
#endif

  {
    struct tm tm_time;
    time_t timestamp;
#ifdef USE_LOCALTIME_R
    struct tm tm_result;
#endif
    struct tm *local_time;
    inttype time_zone_reference;

  /* timSetLocalTZ */
#ifdef TRACE_TIM_RTL
    printf("BEGIN timSetLocalTZ(%04ld-%02ld-%02ld %02ld:%02ld:%02ld)\n",
        year, month, day, hour, min, sec);
#endif
    timestamp = 0;
#ifdef USE_LOCALTIME_R
    local_time = localtime_r(&timestamp, &tm_result);
#else
    local_time = localtime(&timestamp);
#endif
    if (unlikely(local_time == NULL)) {
      raise_error(RANGE_ERROR);
    } else {
      time_zone_reference = unchecked_mkutc(local_time) / 60;
      tm_time.tm_year  = (int) year - 1900;
      tm_time.tm_mon   = (int) month - 1;
      tm_time.tm_mday  = (int) day;
      tm_time.tm_hour  = (int) hour;
      tm_time.tm_min   = (int) min;
      tm_time.tm_sec   = (int) sec;
      tm_time.tm_isdst = 0;
      timestamp = mkutc(&tm_time);
      if (unlikely(timestamp == (time_t) -1)) {
        raise_error(RANGE_ERROR);
      } else {
        timestamp -= time_zone_reference * 60;
#ifdef USE_LOCALTIME_R
        local_time = localtime_r(&timestamp, &tm_result);
#else
        local_time = localtime(&timestamp);
#endif
        if (unlikely(local_time == NULL)) {
          raise_error(RANGE_ERROR);
        } else {
          *time_zone = (unchecked_mkutc(local_time) - timestamp) / 60;
          *is_dst    = local_time->tm_isdst > 0;
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_TIM_RTL
    printf("END timSetLocalTZ (%04ld-%02ld-%02ld %02ld:%02ld:%02ld %ld %d)\n",
        year, month, day, hour, min, sec, *time_zone, *is_dst);
#endif
  } /* timSetLocalTZ */



#ifdef ANSI_C

void timFromBigTimestamp (const const_biginttype timestamp,
    inttype *year, inttype *month, inttype *day, inttype *hour,
    inttype *min, inttype *sec, inttype *micro_sec, inttype *time_zone,
    booltype *is_dst)
#else

void timFromBigTimestamp (timestamp,
    year, month, day, hour, min, sec, micro_sec, time_zone, is_dst)
biginttype timestamp;
inttype *year;
inttype *month;
inttype *day;
inttype *hour;
inttype *min;
inttype *sec;
inttype *micro_sec;
inttype *time_zone;
booltype *is_dst;
#endif

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



#ifdef ANSI_C

biginttype timToBigTimestamp (inttype year, inttype month, inttype day, inttype hour,
    inttype min, inttype sec, inttype micro_sec, inttype time_zone)
#else

biginttype timToBigTimestamp (year, month, day, hour, min, sec, micro_sec, time_zone)
inttype year;
inttype month;
inttype day;
inttype hour;
inttype min;
inttype sec;
inttype micro_sec;
inttype time_zone;
#endif

  {
    time_t os_timestamp;
    biginttype result;

  /* timToBigTimestamp */
    os_timestamp = timToTimestamp(year, month, day, hour, min, sec, micro_sec, time_zone);
    if (sizeof(time_t) == 8) {
      result = bigFromInt64((int64type) os_timestamp);
    } else {
      result = bigFromInt32((int32type) os_timestamp);
    } /* if */
    return result;
  } /* timToBigTimestamp */

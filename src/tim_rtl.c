/********************************************************************/
/*                                                                  */
/*  tim_rtl.c     Time access using the C capabilitys.              */
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
/*  Content: Time access using the C capabilitys.                   */
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
    return(result);
  } /* mkutc */



#ifdef ANSI_C

void timFromTimestamp (time_t timestamp,
    inttype *year, inttype *month, inttype *day, inttype *hour,
    inttype *min, inttype *sec, inttype *mycro_sec, inttype *time_zone,
    booltype *is_dst)
#else

void timFromTimestamp (timestamp,
    year, month, day, hour, min, sec, mycro_sec, time_zone, is_dst)
time_t timestamp;
inttype *year;
inttype *month;
inttype *day;
inttype *hour;
inttype *min;
inttype *sec;
inttype *mycro_sec;
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
    if (local_time == NULL) {
      raise_error(RANGE_ERROR);
    } else {
      *year      = local_time->tm_year + 1900;
      *month     = local_time->tm_mon + 1;
      *day       = local_time->tm_mday;
      *hour      = local_time->tm_hour;
      *min       = local_time->tm_min;
      *sec       = local_time->tm_sec;
      *mycro_sec = 0;
      *time_zone = (mkutc(local_time) - timestamp) / 60;
      *is_dst    = local_time->tm_isdst;
    } /* if */
#ifdef TRACE_TIM_RTL
    printf("END timFromTimestamp(%ld, %04ld-%02ld-%02ld %02ld:%02ld:%02ld.%06ld %ld %d)\n",
        timestamp, *year, *month, *day, *hour, *min, *sec,
        *mycro_sec, *time_zone, *is_dst);
#endif
  } /* timFromTimestamp */



#ifdef ANSI_C

time_t timToTimestamp (inttype year, inttype month, inttype day, inttype hour,
    inttype min, inttype sec, inttype mycro_sec, inttype time_zone)
#else

time_t timToTimestamp (year, month, day, hour, min, sec, mycro_sec, time_zone)
inttype year;
inttype month;
inttype day;
inttype hour;
inttype min;
inttype sec;
inttype mycro_sec;
inttype time_zone;
#endif

  {
    struct tm tm_time;
    time_t result;

  /* timToTimestamp */
#ifdef TRACE_TIM_RTL
    printf("BEGIN timToTimestamp(%04ld-%02ld-%02ld %02ld:%02ld:%02ld.%06ld %ld)\n",
        year, month, day, hour, min, sec, mycro_sec, time_zone);
#endif
    if (year < 1970 || year >= 2038 || month < 1 || month > 12 || day < 1 || day > 31 ||
        hour < 0 || hour >= 24 || min < 0 || min >= 60 || sec < 0 || sec >= 60) {
      raise_error(RANGE_ERROR);
      result = (time_t) -1;
    } else {
      tm_time.tm_year  = (int) year - 1900;
      tm_time.tm_mon   = (int) month - 1;
      tm_time.tm_mday  = (int) day;
      tm_time.tm_hour  = (int) hour;
      tm_time.tm_min   = (int) min;
      tm_time.tm_sec   = (int) sec;
      tm_time.tm_isdst = 0;
      result = mkutc(&tm_time) - time_zone * 60;
    } /* if */
#ifdef TRACE_TIM_RTL
    printf("END timToTimestamp ==> %lu\n", result);
#endif
    return(result);
  } /* timToTimestamp */



#ifdef ANSI_C

void timFromBigTimestamp (biginttype timestamp,
    inttype *year, inttype *month, inttype *day, inttype *hour,
    inttype *min, inttype *sec, inttype *mycro_sec, inttype *time_zone,
    booltype *is_dst)
#else

void timFromBigTimestamp (timestamp,
    year, month, day, hour, min, sec, mycro_sec, time_zone, is_dst)
biginttype timestamp;
inttype *year;
inttype *month;
inttype *day;
inttype *hour;
inttype *min;
inttype *sec;
inttype *mycro_sec;
inttype *time_zone;
booltype *is_dst;
#endif

  {
    time_t os_timestamp;

  /* timFromBigTimestamp */
    if (sizeof(time_t) == 8) {
      os_timestamp = bigToInt64(timestamp);
    } else {
      os_timestamp = bigToInt32(timestamp);
    } /* if */
    timFromTimestamp(os_timestamp,
        year, month, day, hour, min, sec, mycro_sec, time_zone, is_dst);
  } /* timFromBigTimestamp */



#ifdef ANSI_C

biginttype timToBigTimestamp (inttype year, inttype month, inttype day, inttype hour,
    inttype min, inttype sec, inttype mycro_sec, inttype time_zone)
#else

biginttype timToBigTimestamp (year, month, day, hour, min, sec, mycro_sec, time_zone)
inttype year;
inttype month;
inttype day;
inttype hour;
inttype min;
inttype sec;
inttype mycro_sec;
inttype time_zone;
#endif

  {
    time_t os_timestamp;
    biginttype result;

  /* timToBigTimestamp */
    os_timestamp = timToTimestamp (year, month, day, hour, min, sec, mycro_sec, time_zone);
    if (sizeof(time_t) == 8) {
      result = bigFromInt64(os_timestamp);
    } else {
      result = bigFromInt32(os_timestamp);
    } /* if */
    return(result);
  } /* timToBigTimestamp */

/********************************************************************/
/*                                                                  */
/*  tim_dos.c     Time functions which call the Dos API.            */
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
/*  File: seed7/src/tim_dos.c                                       */
/*  Changes: 1992, 1993, 1994, 2009  Thomas Mertes                  */
/*  Content: Time functions which call the Dos API.                 */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include "sys/types.h"
#include "sys/timeb.h"
#include "string.h"
#include "errno.h"

#include "common.h"
#include "tim_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "tim_drv.h"



/**
 *  Wait until the given time is reached
 *  @param time_zone Difference to UTC in minutes (for UTC+1 it is 60).
 *                   The time_zone includes the effect of a daylight saving time.
 */
void timAwait (intType year, intType month, intType day, intType hour,
    intType min, intType sec, intType micro_sec, intType time_zone)

  {
    struct tm tm_time;
    struct timeb tstruct;
    time_t await_second;

  /* timAwait */
    logFunction(printf("timAwait(%04ld-%02ld-%02ld %02ld:%02ld:%02ld.%06ld %ld)\n",
                       year, month, day, hour, min, sec, micro_sec, time_zone););
    await_second = timToOsTimestamp(year, month, day, hour, min, sec,
                                    time_zone);
    if (unlikely(await_second == (time_t) TIME_T_ERROR)) {
      logError(printf("timAwait: Timestamp not in allowed range.\n"););
      raise_error(RANGE_ERROR);
    } else {
      ftime(&tstruct);
      /* printf("%ld %d %d %d\n",
             tstruct.time, tstruct.millitm, tstruct.timezone, tstruct.dstflag);
         printf("%ld %ld %ld\n",
             await_second, micro_sec, time_zone);
         printf("tstruct.time < await_second: %d < %d\n",
             tstruct.time, await_second); */
      if (tstruct.time < await_second) {
        do {
          ftime(&tstruct);
/*        printf("%ld ?= %ld\n", tstruct.time, await_second); */
        } while (tstruct.time < await_second);
      } /* if */
      if (micro_sec != 0) {
        do {
          ftime(&tstruct);
/*        printf("%ld.%ld000 ?= %ld.%ld\n",
              tstruct.time, (long) tstruct.millitm,
              await_second, micro_sec); */
        } while (tstruct.time <= await_second &&
            1000 * ((long) tstruct.millitm) < micro_sec);
      } /* if */
    } /* if */
    logFunction(printf("timAwait -->\n"););
  } /* timAwait */



/**
 *  Return the current clock time.
 *  This function is only used to initialize the random number
 *  generator, so overflows can be ignored.
 */
intType timMicroSec (void)

  {
    intType micro_sec;

  /* timMicroSec */
    micro_sec = clock();
    logFunction(printf("timMicroSec() --> " FMT_U "\n", micro_sec););
    return micro_sec;
  } /* timMicroSec */



/**
 *  Determine the current local time.
 *  @param time_zone Difference to UTC in minutes (for UTC+1 it is 60).
 *                   The time_zone includes the effect of a daylight saving time.
 *  @param is_dst Is TRUE, if a daylight saving time is currently in effect.
 */
void timNow (intType *year, intType *month, intType *day, intType *hour,
    intType *min, intType *sec, intType *micro_sec, intType *time_zone,
    boolType *is_dst)

  {
    struct timeb tstruct;
#if defined USE_LOCALTIME_R || defined USE_LOCALTIME_S
    struct tm tm_result;
#endif
    struct tm *local_time;

  /* timNow */
    logFunction(printf("timNow\n"););
    ftime(&tstruct);
#if defined USE_LOCALTIME_R
    local_time = localtime_r(&tstruct.time, &tm_result);
#elif defined USE_LOCALTIME_S
    if (localtime_s(&tm_result, &tstruct.time) != 0) {
      local_time = NULL;
    } else {
      local_time = &tm_result;
    } /* if */
#else
    local_time = localtime(&tstruct.time);
#endif
    if (unlikely(local_time == NULL)) {
      logError(printf("timNow: One of "
                      "localtime/localtime_r/localtime_s(" FMT_T ") failed:\n"
                      "errno=%d\nerror: %s\n",
                      tstruct.time, errno, strerror(errno)););
      raise_error(RANGE_ERROR);
    } else {
      *year      = local_time->tm_year + 1900;
      *month     = local_time->tm_mon + 1;
      *day       = local_time->tm_mday;
      *hour      = local_time->tm_hour;
      *min       = local_time->tm_min;
      *sec       = local_time->tm_sec;
      *micro_sec = 1000 * ((long) tstruct.millitm);
#ifdef TAKE_TIMEZONE_FROM_TSTRUCT
      *time_zone = - (intType) tstruct.timezone;
      if (tstruct.dstflag) {
        *time_zone += 60;
      } /* if */
      *is_dst    = tstruct.dstflag;
#else
      *time_zone = (intType) (unchecked_mkutc(local_time) - tstruct.time) / 60;
      *is_dst    = local_time->tm_isdst > 0;
#endif
    } /* if */
    logFunction(printf("timNow(%04ld-%02ld-%02ld %02ld:%02ld:%02ld.%06ld %ld %d)\n",
                       *year, *month, *day, *hour, *min, *sec,
                       *micro_sec, *time_zone, *is_dst););
  } /* timNow */

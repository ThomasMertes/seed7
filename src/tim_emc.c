/********************************************************************/
/*                                                                  */
/*  tim_emc.c     Time functions for Emscripten.                    */
/*  Copyright (C) 2023  Thomas Mertes                               */
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
/*  File: seed7/src/tim_emc.c                                       */
/*  Changes: 2023  Thomas Mertes                                    */
/*  Content: Time functions for Emscripten.                         */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdio.h"
#include "time.h"
#include "sys/time.h"
#include "errno.h"
#include "emscripten.h"

#include "common.h"
#include "tim_rtl.h"
#include "emc_utl.h"
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
    time_t await_second;
    struct timeval time_val;
    time_t wait_sec;
    intType wait_milli_sec;
    boolType timeReached = FALSE;

  /* timAwait */
    logFunction(printf("timAwait(" F_D(04) "-" F_D(02) "-" F_D(02) " "
                                   F_D(02) ":" F_D(02) ":" F_D(02) "."
                                   F_D(06) ", " FMT_D ")\n",
                       year, month, day, hour, min, sec, micro_sec,
                       time_zone););
    await_second = timToOsTimestamp(year, month, day, hour, min, sec,
                                    time_zone);
    if (unlikely(await_second == (time_t) TIME_T_ERROR)) {
      logError(printf("timAwait: Timestamp not in allowed range.\n"););
      raise_error(RANGE_ERROR);
    } else if (doWaitOrPushKey == NULL) {
      logError(printf("timAwait: Await not synchronized with keyboard.\n"););
      raise_error(RANGE_ERROR);
    } else {
      do {
        gettimeofday(&time_val, NULL);
        if (time_val.tv_sec > await_second ||
            (time_val.tv_sec == await_second &&
            time_val.tv_usec >= micro_sec)) {
          timeReached = TRUE;
        } else {
          wait_sec = await_second - time_val.tv_sec;
          if (micro_sec >= time_val.tv_usec) {
            wait_milli_sec = (micro_sec - time_val.tv_usec) / 1000;
          } else {
            wait_milli_sec = (1000000 - time_val.tv_usec + micro_sec) / 1000;
            wait_sec--;
          } /* if */
          wait_milli_sec += wait_sec * 1000;
          if (wait_milli_sec != 0) {
            doWaitOrPushKey(wait_milli_sec);
          } else {
            timeReached = TRUE;
          } /* if */
        } /* if */
      } while (!timeReached);
    } /* if */
    logFunction(printf("timAwait -->\n"););
  } /* timAwait */



/**
 *  Return the current time in microseconds since the epoch of the system.
 *  This function is used to initialize the random number generator and
 *  for the simple profiling.
 */
intType timMicroSec (void)

  {
    struct timeval time_val;
    intType micro_sec;

  /* timMicroSec */
    gettimeofday(&time_val, NULL);
    micro_sec = 1000000 * (intType) time_val.tv_sec + (intType) time_val.tv_usec;
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
    struct timeval time_val;
#if defined USE_LOCALTIME_R || defined USE_LOCALTIME_S
    struct tm tm_result;
#endif
    struct tm *local_time;

  /* timNow */
    logFunction(printf("timNow\n"););
    gettimeofday(&time_val, NULL);
#if defined USE_LOCALTIME_R
    local_time = localtime_r(&time_val.tv_sec, &tm_result);
#elif defined USE_LOCALTIME_S
    if (localtime_s(&tm_result, &time_val.tv_sec) != 0) {
      local_time = NULL;
    } else {
      local_time = &tm_result;
    } /* if */
#else
    local_time = localtime(&time_val.tv_sec);
#endif
    if (unlikely(local_time == NULL)) {
      logError(printf("timNow: One of "
                      "localtime/localtime_r/localtime_s(" FMT_T ") failed:\n"
                      "errno=%d\nerror: %s\n",
                      time_val.tv_sec, errno, strerror(errno)););
      raise_error(RANGE_ERROR);
    } else {
      *year      = local_time->tm_year + 1900;
      *month     = local_time->tm_mon + 1;
      *day       = local_time->tm_mday;
      *hour      = local_time->tm_hour;
      *min       = local_time->tm_min;
      *sec       = local_time->tm_sec;
      *micro_sec = (intType) time_val.tv_usec;
      *time_zone = ((intType) unchecked_mkutc(local_time) - (intType) time_val.tv_sec) / 60;
      *is_dst    = local_time->tm_isdst > 0;
    } /* if */
    logFunction(printf("timNow(" F_D(04) "-" F_D(02) "-" F_D(02) " "
                                 F_D(02) ":" F_D(02) ":" F_D(02) "."
                                 F_D(06) ", " FMT_D ", %d) -->\n",
                       *year, *month, *day, *hour, *min, *sec,
                       *micro_sec, *time_zone, *is_dst););
  } /* timNow */

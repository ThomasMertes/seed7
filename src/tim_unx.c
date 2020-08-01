/********************************************************************/
/*                                                                  */
/*  tim_unx.c     Time functions which call the Unix API.           */
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
/*  File: seed7/src/tim_unx.c                                       */
/*  Changes: 1992, 1993, 1994, 2009, 2013  Thomas Mertes            */
/*  Content: Time functions which call the Unix API.                */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#ifdef AWAIT_WITH_PPOLL
#define _GNU_SOURCE
#endif

#include "stdio.h"
#include "string.h"
#include "time.h"
#include "sys/time.h"

#if defined AWAIT_WITH_POLL || defined AWAIT_WITH_PPOLL
#include "poll.h"
#elif defined AWAIT_WITH_SIGACTION || defined AWAIT_WITH_SIGNAL
#include "signal.h"
#include "setjmp.h"
#elif defined AWAIT_WITH_SELECT
#include "sys/select.h"
#endif

#include "errno.h"

#include "common.h"
#include "tim_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "tim_drv.h"


#ifdef C_PLUS_PLUS

extern "C" int pause (void);

#else

int pause (void);

#endif


#if defined AWAIT_WITH_SIGACTION || defined AWAIT_WITH_SIGNAL
longjmpPosition waitFinished;
#endif



#if defined AWAIT_WITH_POLL



/**
 *  Wait until the given time is reached
 *  @param time_zone Difference to UTC in minutes (for UTC+1 it is 60).
 *                   The time_zone includes the effect of a daylight saving time.
 */
void timAwait (intType year, intType month, intType day, intType hour,
    intType min, intType sec, intType micro_sec, intType time_zone)

  {
    struct tm tm_time;
    time_t await_second;
    struct timeval time_val;
    int timeout_value;
    int poll_result;

  /* timAwait */
    logFunction(printf("timAwait(" F_D(04) "-" F_D(02) "-" F_D(02) " "
                                   F_D(02) ":" F_D(02) ":" F_D(02) "."
                                   F_D(06) ", " FMT_D ")\n",
                       year, month, day, hour, min, sec, micro_sec, time_zone););
    tm_time.tm_year  = (int) year - 1900;
    tm_time.tm_mon   = (int) month - 1;
    tm_time.tm_mday  = (int) day;
    tm_time.tm_hour  = (int) hour;
    tm_time.tm_min   = (int) min;
    tm_time.tm_sec   = (int) sec;
    tm_time.tm_isdst = 0;
    await_second = mkutc(&tm_time);
    if (unlikely(await_second == (time_t) -1)) {
      logError(printf("timAwait: mkutc() failed.\n"););
      raise_error(RANGE_ERROR);
    } else {
      await_second -= time_zone * 60;
      do {
        gettimeofday(&time_val, NULL);
        if (time_val.tv_sec > await_second ||
            (time_val.tv_sec == await_second &&
            time_val.tv_usec >= micro_sec)) {
          /* The point in time has been reached.   */
          /* Act as if poll() has been successful. */
          poll_result = 0;
        } else {
          timeout_value = (await_second - time_val.tv_sec) * 1000 +
                          (micro_sec - (intType) time_val.tv_usec) / 1000;
          poll_result = poll(NULL, 0, timeout_value);
        } /* if */
      } while (unlikely(poll_result == -1 && errno == EINTR));
    } /* if */
    logFunction(printf("timAwait -->\n"););
  } /* timAwait */



#elif defined AWAIT_WITH_PPOLL



/**
 *  Wait until the given time is reached
 *  @param time_zone Difference to UTC in minutes (for UTC+1 it is 60).
 *                   The time_zone includes the effect of a daylight saving time.
 */
void timAwait (intType year, intType month, intType day, intType hour,
    intType min, intType sec, intType micro_sec, intType time_zone)

  {
    struct tm tm_time;
    time_t await_second;
    struct timeval time_val;
    struct timespec timeout_value;
    int ppoll_result;

  /* timAwait */
    logFunction(printf("timAwait(" F_D(04) "-" F_D(02) "-" F_D(02) " "
                                   F_D(02) ":" F_D(02) ":" F_D(02) "."
                                   F_D(06) ", " FMT_D ")\n",
                       year, month, day, hour, min, sec, micro_sec, time_zone););
    tm_time.tm_year  = (int) year - 1900;
    tm_time.tm_mon   = (int) month - 1;
    tm_time.tm_mday  = (int) day;
    tm_time.tm_hour  = (int) hour;
    tm_time.tm_min   = (int) min;
    tm_time.tm_sec   = (int) sec;
    tm_time.tm_isdst = 0;
    await_second = mkutc(&tm_time);
    if (unlikely(await_second == (time_t) -1)) {
      logError(printf("timAwait: mkutc() failed.\n"););
      raise_error(RANGE_ERROR);
    } else {
      await_second -= time_zone * 60;
      do {
        gettimeofday(&time_val, NULL);
        if (time_val.tv_sec > await_second ||
            (time_val.tv_sec == await_second &&
            time_val.tv_usec >= micro_sec)) {
          /* The point in time has been reached.    */
          /* Act as if ppoll() has been successful. */
          ppoll_result = 0;
        } else {
          timeout_value.tv_sec = await_second - time_val.tv_sec;
          if (micro_sec >= time_val.tv_usec) {
            timeout_value.tv_nsec = (micro_sec - time_val.tv_usec) * 1000;
          } else {
            timeout_value.tv_nsec = (1000000 - time_val.tv_usec + micro_sec) * 1000;
            timeout_value.tv_sec--;
          } /* if */
          ppoll_result = ppoll(NULL, 0, &timeout_value, NULL);
        } /* if */
      } while (unlikely(ppoll_result == -1 && errno == EINTR));
    } /* if */
    logFunction(printf("timAwait -->\n"););
  } /* timAwait */



#elif defined AWAIT_WITH_SELECT



/**
 *  Wait until the given time is reached
 *  @param time_zone Difference to UTC in minutes (for UTC+1 it is 60).
 *                   The time_zone includes the effect of a daylight saving time.
 */
void timAwait (intType year, intType month, intType day, intType hour,
    intType min, intType sec, intType micro_sec, intType time_zone)

  {
    struct tm tm_time;
    time_t await_second;
    struct timeval time_val;
    struct timeval timeout_value;
    int select_result;

  /* timAwait */
    logFunction(printf("timAwait(" F_D(04) "-" F_D(02) "-" F_D(02) " "
                                   F_D(02) ":" F_D(02) ":" F_D(02) "."
                                   F_D(06) ", " FMT_D ")\n",
                       year, month, day, hour, min, sec, micro_sec, time_zone););
    tm_time.tm_year  = (int) year - 1900;
    tm_time.tm_mon   = (int) month - 1;
    tm_time.tm_mday  = (int) day;
    tm_time.tm_hour  = (int) hour;
    tm_time.tm_min   = (int) min;
    tm_time.tm_sec   = (int) sec;
    tm_time.tm_isdst = 0;
    await_second = mkutc(&tm_time);
    if (unlikely(await_second == (time_t) -1)) {
      logError(printf("timAwait: mkutc() failed.\n"););
      raise_error(RANGE_ERROR);
    } else {
      await_second -= time_zone * 60;
      do {
        gettimeofday(&time_val, NULL);
        if (time_val.tv_sec > await_second ||
            (time_val.tv_sec == await_second &&
            time_val.tv_usec >= micro_sec)) {
          /* The point in time has been reached.     */
          /* Act as if select() has been successful. */
          select_result = 0;
        } else {
          timeout_value.tv_sec = await_second - time_val.tv_sec;
          if (micro_sec >= time_val.tv_usec) {
            timeout_value.tv_usec = micro_sec - time_val.tv_usec;
          } else {
            timeout_value.tv_usec = 1000000 - time_val.tv_usec + micro_sec;
            timeout_value.tv_sec--;
          } /* if */
          select_result = select(0, NULL, NULL, NULL, &timeout_value);
        } /* if */
      } while (unlikely(select_result == -1 && errno == EINTR));
#if 0
      if (unlikely(select_result == -1)) {
        logError(printf("timAwait: select(0, NULL, NULL, NULL, *) failed:\n"
                        "errno=%d\nerror: %s\n",
                        errno, strerror(errno)););
      } /* if */
#endif
    } /* if */
    logFunction(printf("timAwait -->\n"););
  } /* timAwait */



#elif defined AWAIT_WITH_SIGACTION



static void alarm_signal_handler (int sig_num)

  { /* alarm_signal_handler */
    logFunction(printf("alarm_signal_handler\n"););
    do_longjmp(waitFinished, 1);
    logFunction(printf("alarm_signal_handler -->\n"););
  } /* alarm_signal_handler */



/**
 *  Wait until the given time is reached
 *  @param time_zone Difference to UTC in minutes (for UTC+1 it is 60).
 *                   The time_zone includes the effect of a daylight saving time.
 */
void timAwait (intType year, intType month, intType day, intType hour,
    intType min, intType sec, intType micro_sec, intType time_zone)

  {
    struct tm tm_time;
    time_t await_second;
    struct timeval time_val;
    struct itimerval timer_value;
    struct sigaction sigAct;

  /* timAwait */
    logFunction(printf("timAwait(" F_D(04) "-" F_D(02) "-" F_D(02) " "
                                   F_D(02) ":" F_D(02) ":" F_D(02) "."
                                   F_D(06) ", " FMT_D ")\n",
                       year, month, day, hour, min, sec, micro_sec, time_zone););
    tm_time.tm_year  = (int) year - 1900;
    tm_time.tm_mon   = (int) month - 1;
    tm_time.tm_mday  = (int) day;
    tm_time.tm_hour  = (int) hour;
    tm_time.tm_min   = (int) min;
    tm_time.tm_sec   = (int) sec;
    tm_time.tm_isdst = 0;
    await_second = mkutc(&tm_time);
    if (unlikely(await_second == (time_t) -1)) {
      logError(printf("timAwait: mkutc() failed.\n"););
      raise_error(RANGE_ERROR);
    } else {
      await_second -= time_zone * 60;
      gettimeofday(&time_val, NULL);
      if (time_val.tv_sec < await_second ||
          (time_val.tv_sec == await_second &&
          time_val.tv_usec < micro_sec)) {
        timer_value.it_value.tv_sec = await_second - time_val.tv_sec;
        if (micro_sec >= time_val.tv_usec) {
          timer_value.it_value.tv_usec = micro_sec - time_val.tv_usec;
        } else {
          timer_value.it_value.tv_usec = 1000000 - time_val.tv_usec + micro_sec;
          timer_value.it_value.tv_sec--;
        } /* if */
        timer_value.it_interval.tv_sec = 0;
        timer_value.it_interval.tv_usec = 0;
        sigAct.sa_handler = &alarm_signal_handler;
        sigemptyset(&sigAct.sa_mask);
        sigAct.sa_flags = 0;
        if (sigaction(SIGALRM, &sigAct, NULL) == 0) {
          if (do_setjmp(waitFinished) == 0) {
            if (setitimer(ITIMER_REAL, &timer_value, NULL) == 0) {
              pause();
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("timAwait -->\n"););
  } /* timAwait */



#elif defined AWAIT_WITH_SIGNAL



static void alarm_signal_handler (int sig_num)

  { /* alarm_signal_handler */
    logFunction(printf("alarm_signal_handler\n"););
    do_longjmp(waitFinished, 1);
    logFunction(printf("alarm_signal_handler -->\n"););
  } /* alarm_signal_handler */



/**
 *  Wait until the given time is reached
 *  @param time_zone Difference to UTC in minutes (for UTC+1 it is 60).
 *                   The time_zone includes the effect of a daylight saving time.
 */
void timAwait (intType year, intType month, intType day, intType hour,
    intType min, intType sec, intType micro_sec, intType time_zone)

  {
    struct tm tm_time;
    time_t await_second;
    struct timeval time_val;
    struct itimerval timer_value;

  /* timAwait */
    logFunction(printf("timAwait(" F_D(04) "-" F_D(02) "-" F_D(02) " "
                                   F_D(02) ":" F_D(02) ":" F_D(02) "."
                                   F_D(06) ", " FMT_D ")\n",
                       year, month, day, hour, min, sec, micro_sec, time_zone););
    tm_time.tm_year  = (int) year - 1900;
    tm_time.tm_mon   = (int) month - 1;
    tm_time.tm_mday  = (int) day;
    tm_time.tm_hour  = (int) hour;
    tm_time.tm_min   = (int) min;
    tm_time.tm_sec   = (int) sec;
    tm_time.tm_isdst = 0;
    await_second = mkutc(&tm_time);
    if (unlikely(await_second == (time_t) -1)) {
      logError(printf("timAwait: mkutc() failed.\n"););
      raise_error(RANGE_ERROR);
    } else {
      await_second -= time_zone * 60;
      gettimeofday(&time_val, NULL);
      if (time_val.tv_sec < await_second ||
          (time_val.tv_sec == await_second &&
          time_val.tv_usec < micro_sec)) {
        timer_value.it_value.tv_sec = await_second - time_val.tv_sec;
        if (micro_sec >= time_val.tv_usec) {
          timer_value.it_value.tv_usec = micro_sec - time_val.tv_usec;
        } else {
          timer_value.it_value.tv_usec = 1000000 - time_val.tv_usec + micro_sec;
          timer_value.it_value.tv_sec--;
        } /* if */
        timer_value.it_interval.tv_sec = 0;
        timer_value.it_interval.tv_usec = 0;
        if (signal(SIGALRM, alarm_signal_handler) != SIG_ERR) {
          if (do_setjmp(waitFinished) == 0) {
            if (setitimer(ITIMER_REAL, &timer_value, NULL) == 0) {
              pause();
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("timAwait -->\n"););
  } /* timAwait */



#endif



/**
 *  Return the current time in microseconds.
 *  This function is only used to initialize the random number
 *  generator, so overflows can be ignored.
 */
intType timMicroSec (void)

  {
    struct timeval time_val;
    intType micro_sec;

  /* timMicroSec */
    gettimeofday(&time_val, NULL);
    micro_sec = (intType) time_val.tv_usec;
    logFunction(printf("timMicroSec() --> " FMT_U "\n", micro_sec););
    return micro_sec;
  } /* timMicroSec */



/**
 *  Determine the current local time.
 *  @param time_zone Difference to UTC in minutes (for UTC+1 it is 60).
 *                   The time_zone includes the effect of a daylight saving time.
 *  @param is_dst Is TRUE, when a daylight saving time is currently in effect.
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
      *time_zone = (intType) (unchecked_mkutc(local_time) - time_val.tv_sec) / 60;
      *is_dst    = local_time->tm_isdst > 0;
    } /* if */
    logFunction(printf("timNow(" F_D(04) "-" F_D(02) "-" F_D(02) " "
                                 F_D(02) ":" F_D(02) ":" F_D(02) "."
                                 F_D(06) ", " FMT_D ", %d) -->\n",
                       *year, *month, *day, *hour, *min, *sec,
                       *micro_sec, *time_zone, *is_dst););
  } /* timNow */

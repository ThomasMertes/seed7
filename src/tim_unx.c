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

#include "version.h"

#ifdef AWAIT_WITH_PPOLL
#define _GNU_SOURCE
#endif

#include "stdio.h"
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

#undef TRACE_TIM_UNX


#ifdef C_PLUS_PLUS

extern "C" int pause (void);

#else

int pause (void);

#endif


#if defined AWAIT_WITH_SIGACTION || defined AWAIT_WITH_SIGNAL
#ifdef HAS_SIGSETJMP
#define do_setjmp(env)        sigsetjmp(env, 1)
#define do_longjmp(env, val)  siglongjmp(env, val);
sigjmp_buf wait_finished;
#else
#define do_setjmp(env)        setjmp(env)
#define do_longjmp(env, val)  longjmp(env, val);
jmp_buf wait_finished;
#endif
#endif



#if defined AWAIT_WITH_POLL



/**
 *  Wait until the given time is reached
 *  @param time_zone Difference to UTC in minutes (for UTC+1 it is 60).
 *                   The time_zone includes the effect of a daylight saving time.
 */
void timAwait (inttype year, inttype month, inttype day, inttype hour,
    inttype min, inttype sec, inttype micro_sec, inttype time_zone)

  {
    struct tm tm_time;
    time_t await_second;
    struct timeval time_val;
    int timeout_value;
    int poll_result;

  /* timAwait */
#ifdef TRACE_TIM_UNX
    printf("BEGIN timAwait(%04ld-%02ld-%02ld %02ld:%02ld:%02ld.%06ld %ld)\n",
        year, month, day, hour, min, sec, micro_sec, time_zone);
#endif
    tm_time.tm_year  = (int) year - 1900;
    tm_time.tm_mon   = (int) month - 1;
    tm_time.tm_mday  = (int) day;
    tm_time.tm_hour  = (int) hour;
    tm_time.tm_min   = (int) min;
    tm_time.tm_sec   = (int) sec;
    tm_time.tm_isdst = 0;
    await_second = mkutc(&tm_time);
    if (unlikely(await_second == (time_t) -1)) {
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
                          (micro_sec - (inttype) time_val.tv_usec) / 1000;
#ifdef TRACE_TIM_UNX
          fprintf(stderr, "%ld %ld %ld %ld %ld %ld %ld\n",
              time_val.tv_sec,
              await_second,
              await_second - time_val.tv_sec,
              time_val.tv_usec,
              micro_sec,
              micro_sec - time_val.tv_usec,
              timeout_value);
#endif
          poll_result = poll(NULL, 0, timeout_value);
        } /* if */
      } while (unlikely(poll_result == -1 && errno == EINTR));
    } /* if */
#ifdef TRACE_TIM_UNX
    printf("END timAwait\n");
#endif
  } /* timAwait */



#elif defined AWAIT_WITH_PPOLL



/**
 *  Wait until the given time is reached
 *  @param time_zone Difference to UTC in minutes (for UTC+1 it is 60).
 *                   The time_zone includes the effect of a daylight saving time.
 */
void timAwait (inttype year, inttype month, inttype day, inttype hour,
    inttype min, inttype sec, inttype micro_sec, inttype time_zone)

  {
    struct tm tm_time;
    time_t await_second;
    struct timeval time_val;
    struct timespec timeout_value;
    int ppoll_result;

  /* timAwait */
#ifdef TRACE_TIM_UNX
    printf("BEGIN timAwait(%04ld-%02ld-%02ld %02ld:%02ld:%02ld.%06ld %ld)\n",
        year, month, day, hour, min, sec, micro_sec, time_zone);
#endif
    tm_time.tm_year  = (int) year - 1900;
    tm_time.tm_mon   = (int) month - 1;
    tm_time.tm_mday  = (int) day;
    tm_time.tm_hour  = (int) hour;
    tm_time.tm_min   = (int) min;
    tm_time.tm_sec   = (int) sec;
    tm_time.tm_isdst = 0;
    await_second = mkutc(&tm_time);
    if (unlikely(await_second == (time_t) -1)) {
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
#ifdef TRACE_TIM_UNX
          fprintf(stderr, "%ld %ld %ld %ld %ld %ld %ld %ld\n",
              time_val.tv_sec,
              await_second,
              await_second - time_val.tv_sec,
              time_val.tv_usec,
              micro_sec,
              micro_sec - time_val.tv_usec,
              timeout_value.tv_sec,
              timeout_value.tv_nsec);
#endif
          ppoll_result = ppoll(NULL, 0, &timeout_value, NULL);
        } /* if */
      } while (unlikely(ppoll_result == -1 && errno == EINTR));
    } /* if */
#ifdef TRACE_TIM_UNX
    printf("END timAwait\n");
#endif
  } /* timAwait */



#elif defined AWAIT_WITH_SELECT



/**
 *  Wait until the given time is reached
 *  @param time_zone Difference to UTC in minutes (for UTC+1 it is 60).
 *                   The time_zone includes the effect of a daylight saving time.
 */
void timAwait (inttype year, inttype month, inttype day, inttype hour,
    inttype min, inttype sec, inttype micro_sec, inttype time_zone)

  {
    struct tm tm_time;
    time_t await_second;
    struct timeval time_val;
    struct timeval timeout_value;
    int select_result;

  /* timAwait */
#ifdef TRACE_TIM_UNX
    printf("BEGIN timAwait(%04ld-%02ld-%02ld %02ld:%02ld:%02ld.%06ld %ld)\n",
        year, month, day, hour, min, sec, micro_sec, time_zone);
#endif
    tm_time.tm_year  = (int) year - 1900;
    tm_time.tm_mon   = (int) month - 1;
    tm_time.tm_mday  = (int) day;
    tm_time.tm_hour  = (int) hour;
    tm_time.tm_min   = (int) min;
    tm_time.tm_sec   = (int) sec;
    tm_time.tm_isdst = 0;
    await_second = mkutc(&tm_time);
    if (unlikely(await_second == (time_t) -1)) {
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
#ifdef TRACE_TIM_UNX
          fprintf(stderr, "%ld %ld %ld %ld %ld %ld %ld %ld\n",
              time_val.tv_sec,
              await_second,
              await_second - time_val.tv_sec,
              time_val.tv_usec,
              micro_sec,
              micro_sec - time_val.tv_usec,
              timeout_value.tv_sec,
              timeout_value.tv_usec);
#endif
          select_result = select(0, NULL, NULL, NULL, &timeout_value);
        } /* if */
      } while (unlikely(select_result == -1 && errno == EINTR));
    } /* if */
#ifdef TRACE_TIM_UNX
    printf("END timAwait\n");
#endif
  } /* timAwait */



#elif defined AWAIT_WITH_SIGACTION



static void alarm_signal_handler (int sig_num)

  { /* alarm_signal_handler */
#ifdef TRACE_TIM_UNX
    printf("BEGIN alarm_signal_handler\n");
#endif
    do_longjmp(wait_finished, 1);
#ifdef TRACE_TIM_UNX
    printf("END alarm_signal_handler\n");
#endif
  } /* alarm_signal_handler */



/**
 *  Wait until the given time is reached
 *  @param time_zone Difference to UTC in minutes (for UTC+1 it is 60).
 *                   The time_zone includes the effect of a daylight saving time.
 */
void timAwait (inttype year, inttype month, inttype day, inttype hour,
    inttype min, inttype sec, inttype micro_sec, inttype time_zone)

  {
    struct tm tm_time;
    time_t await_second;
    struct timeval time_val;
    struct itimerval timer_value;
    struct sigaction action;

  /* timAwait */
#ifdef TRACE_TIM_UNX
    printf("BEGIN timAwait(%04ld-%02ld-%02ld %02ld:%02ld:%02ld.%06ld %ld)\n",
        year, month, day, hour, min, sec, micro_sec, time_zone);
#endif
    tm_time.tm_year  = (int) year - 1900;
    tm_time.tm_mon   = (int) month - 1;
    tm_time.tm_mday  = (int) day;
    tm_time.tm_hour  = (int) hour;
    tm_time.tm_min   = (int) min;
    tm_time.tm_sec   = (int) sec;
    tm_time.tm_isdst = 0;
    await_second = mkutc(&tm_time);
    if (unlikely(await_second == (time_t) -1)) {
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
#ifdef TRACE_TIM_UNX
        fprintf(stderr, "%ld %ld %ld %ld %ld %ld %ld %ld\n",
            time_val.tv_sec,
            await_second,
            await_second - time_val.tv_sec,
            time_val.tv_usec,
            micro_sec,
            micro_sec - time_val.tv_usec,
            timer_value.it_value.tv_sec,
            timer_value.it_value.tv_usec);
#endif
        timer_value.it_interval.tv_sec = 0;
        timer_value.it_interval.tv_usec = 0;
        action.sa_handler = &alarm_signal_handler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        if (sigaction(SIGALRM, &action, NULL) == 0) {
          if (do_setjmp(wait_finished) == 0) {
            if (setitimer(ITIMER_REAL, &timer_value, NULL) == 0) {
              pause();
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_TIM_UNX
    printf("END timAwait\n");
#endif
  } /* timAwait */



#elif defined AWAIT_WITH_SIGNAL



static void alarm_signal_handler (int sig_num)

  { /* alarm_signal_handler */
#ifdef TRACE_TIM_UNX
    printf("BEGIN alarm_signal_handler\n");
#endif
    do_longjmp(wait_finished, 1);
#ifdef TRACE_TIM_UNX
    printf("END alarm_signal_handler\n");
#endif
  } /* alarm_signal_handler */



/**
 *  Wait until the given time is reached
 *  @param time_zone Difference to UTC in minutes (for UTC+1 it is 60).
 *                   The time_zone includes the effect of a daylight saving time.
 */
void timAwait (inttype year, inttype month, inttype day, inttype hour,
    inttype min, inttype sec, inttype micro_sec, inttype time_zone)

  {
    struct tm tm_time;
    time_t await_second;
    struct timeval time_val;
    struct itimerval timer_value;

  /* timAwait */
#ifdef TRACE_TIM_UNX
    printf("BEGIN timAwait(%04ld-%02ld-%02ld %02ld:%02ld:%02ld.%06ld %ld)\n",
        year, month, day, hour, min, sec, micro_sec, time_zone);
#endif
    tm_time.tm_year  = (int) year - 1900;
    tm_time.tm_mon   = (int) month - 1;
    tm_time.tm_mday  = (int) day;
    tm_time.tm_hour  = (int) hour;
    tm_time.tm_min   = (int) min;
    tm_time.tm_sec   = (int) sec;
    tm_time.tm_isdst = 0;
    await_second = mkutc(&tm_time);
    if (unlikely(await_second == (time_t) -1)) {
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
#ifdef TRACE_TIM_UNX
        fprintf(stderr, "%ld %ld %ld %ld %ld %ld %ld %ld\n",
            time_val.tv_sec,
            await_second,
            await_second - time_val.tv_sec,
            time_val.tv_usec,
            micro_sec,
            micro_sec - time_val.tv_usec,
            timer_value.it_value.tv_sec,
            timer_value.it_value.tv_usec);
#endif
        timer_value.it_interval.tv_sec = 0;
        timer_value.it_interval.tv_usec = 0;
        if (signal(SIGALRM, alarm_signal_handler) != SIG_ERR) {
          if (do_setjmp(wait_finished) == 0) {
            if (setitimer(ITIMER_REAL, &timer_value, NULL) == 0) {
              pause();
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_TIM_UNX
    printf("END timAwait\n");
#endif
  } /* timAwait */



#endif



/**
 *  Return the current time in microseconds.
 *  This function is only used to initialize the random number
 *  generator, so overflows can be ignored.
 */
inttype timMicroSec (void)

  {
    struct timeval time_val;
    inttype micro_sec;

  /* timMicroSec */
    gettimeofday(&time_val, NULL);
    micro_sec = (inttype) time_val.tv_usec;
    /* printf("timMicroSec() ==> %lu\n", micro_sec); */
    return micro_sec;
  } /* timMicroSec */



/**
 *  Determine the current local time.
 *  @param time_zone Difference to UTC in minutes (for UTC+1 it is 60).
 *                   The time_zone includes the effect of a daylight saving time.
 *  @param is_dst Is TRUE, when a daylight saving time is currently in effect.
 */
void timNow (inttype *year, inttype *month, inttype *day, inttype *hour,
    inttype *min, inttype *sec, inttype *micro_sec, inttype *time_zone,
    booltype *is_dst)

  {
    struct timeval time_val;
#if defined USE_LOCALTIME_R || defined USE_LOCALTIME_S
    struct tm tm_result;
#endif
    struct tm *local_time;

  /* timNow */
#ifdef TRACE_TIM_UNX
    printf("BEGIN timNow\n");
#endif
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
      raise_error(RANGE_ERROR);
    } else {
      *year      = local_time->tm_year + 1900;
      *month     = local_time->tm_mon + 1;
      *day       = local_time->tm_mday;
      *hour      = local_time->tm_hour;
      *min       = local_time->tm_min;
      *sec       = local_time->tm_sec;
      *micro_sec = (inttype) time_val.tv_usec;
      *time_zone = (inttype) (unchecked_mkutc(local_time) - time_val.tv_sec) / 60;
      *is_dst    = local_time->tm_isdst > 0;
    } /* if */
#ifdef TRACE_TIM_UNX
    printf("END timNow(%04ld-%02ld-%02ld %02ld:%02ld:%02ld.%06ld %ld %d)\n",
        *year, *month, *day, *hour, *min, *sec,
        *micro_sec, *time_zone, *is_dst);
#endif
  } /* timNow */

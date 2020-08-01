/********************************************************************/
/*                                                                  */
/*  tim_unx.c     Time access using the unix capabilitys.           */
/*  Copyright (C) 1989 - 2006  Thomas Mertes                        */
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
/*  Free Software Foundation, Inc., 59 Temple Place, Suite 330,     */
/*  Boston, MA 02111-1307 USA                                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/tim_unx.c                                       */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: Time access using the unix capabilitys.                */
/*                                                                  */
/********************************************************************/

#include "stdio.h"
#include "time.h"
#include "sys/time.h"
#include "signal.h"
#include "setjmp.h"

#include "version.h"
#include "common.h"

#undef EXTERN
#define EXTERN
#include "tim_drv.h"

#undef TRACE_TIM_UNX
#define USE_SIGACTION


#ifdef ANSI_C
#ifdef C_PLUS_PLUS

extern "C" int pause (void);

#else

int pause (void);

#endif
#else

int pause ();

#endif


#ifdef USE_SIGACTION
sigjmp_buf wait_finished;
#else
jmp_buf wait_finished;
#endif



#ifdef USE_SIGACTION



#ifdef ANSI_C

static void alarm_signal_handler (int sig_num)
#else

static void alarm_signal_handler (sig_num)
int sig_num;
#endif

  { /* alarm_signal_handler */
#ifdef TRACE_TIM_UNX
    printf("BEGIN alarm_signal_handler\n");
#endif
    siglongjmp(wait_finished, 1);
#ifdef TRACE_TIM_UNX
    printf("END alarm_signal_handler\n");
#endif
  } /* alarm_signal_handler */



#ifdef ANSI_C

void timAwait (inttype year, inttype month, inttype day, inttype hour,
    inttype min, inttype sec, inttype mycro_sec, inttype time_zone)
#else

void timAwait (year, month, day, hour, min, sec, mycro_sec, time_zone)
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
    static struct tm *local_time = NULL;
    time_t await_second;
    struct timeval time_val;
    struct timezone this_time_zone;
    struct itimerval timer_value;
    struct sigaction action;

  /* timAwait */
#ifdef TRACE_TIM_UNX
    printf("BEGIN timAwait\n");
#endif
    /* fprintf(stderr, "await     %04ld-%02ld-%02ld %02ld:%02ld:%02ld %7ld %ld\n",
        year,
        month,
        day,
        hour,
        min,
        sec,
        mycro_sec,
        time_zone); */
    if (local_time == NULL) {
      gettimeofday(&time_val, &this_time_zone);
      local_time = localtime(&time_val.tv_sec);
    } /* if */
    local_time->tm_year = (int) year - 1900;
    local_time->tm_mon  = (int) month - 1;
    local_time->tm_mday = (int) day;
    local_time->tm_hour = (int) hour;
    local_time->tm_min  = (int) min;
    local_time->tm_sec  = (int) sec;
    await_second = mktime(local_time);
    gettimeofday(&time_val, &this_time_zone);
    await_second += (time_zone - this_time_zone.tz_minuteswest) * 60;
    if (time_val.tv_sec < await_second ||
        (time_val.tv_sec == await_second &&
        time_val.tv_usec < mycro_sec)) {
      timer_value.it_value.tv_sec = await_second - time_val.tv_sec;
      if (mycro_sec >= time_val.tv_usec) {
        timer_value.it_value.tv_usec = mycro_sec - time_val.tv_usec;
      } else {
        timer_value.it_value.tv_usec = 1000000 - time_val.tv_usec + mycro_sec;
        timer_value.it_value.tv_sec--;
      } /* if */
#ifdef TRACE_TIM_UNX
    fprintf(stderr, "%d %ld %ld %ld %ld %ld %ld %ld %ld\n",
        this_time_zone.tz_minuteswest,
        time_val.tv_sec,
        await_second,
        await_second - time_val.tv_sec,
        time_val.tv_usec,
        mycro_sec,
        mycro_sec - time_val.tv_usec,
        timer_value.it_value.tv_sec,
        timer_value.it_value.tv_usec);
#endif
      timer_value.it_interval.tv_sec = 0;
      timer_value.it_interval.tv_usec = 0;
      action.sa_handler = &alarm_signal_handler;
      sigemptyset(&action.sa_mask);
      action.sa_flags = 0;
      if (sigaction(SIGALRM, &action, NULL) != SIG_ERR) {
        if (sigsetjmp(wait_finished, 1) == 0) {
          if (setitimer(ITIMER_REAL, &timer_value, NULL) == 0) {
            pause();
          } /* if */
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_TIM_UNX
    printf("END timAwait\n");
#endif
  } /* timAwait */



#else



#ifdef ANSI_C

static void alarm_signal_handler (int sig_num)
#else

static void alarm_signal_handler (sig_num)
int sig_num;
#endif

  { /* alarm_signal_handler */
#ifdef TRACE_TIM_UNX
    printf("BEGIN alarm_signal_handler\n");
#endif
    longjmp(wait_finished, 1);
#ifdef TRACE_TIM_UNX
    printf("END alarm_signal_handler\n");
#endif
  } /* alarm_signal_handler */



#ifdef ANSI_C

void timAwait (inttype year, inttype month, inttype day, inttype hour,
    inttype min, inttype sec, inttype mycro_sec, inttype time_zone)
#else

void timAwait (year, month, day, hour, min, sec, mycro_sec, time_zone)
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
    static struct tm *local_time = NULL;
    time_t await_second;
    struct timeval time_val;
    struct timezone this_time_zone;
    struct itimerval timer_value;

  /* timAwait */
#ifdef TRACE_TIM_UNX
    printf("BEGIN timAwait\n");
#endif
    /* fprintf(stderr, "await     %04ld-%02ld-%02ld %02ld:%02ld:%02ld %7ld %ld\n",
        year,
        month,
        day,
        hour,
        min,
        sec,
        mycro_sec,
        time_zone); */
    if (local_time == NULL) {
      gettimeofday(&time_val, &this_time_zone);
      local_time = localtime(&time_val.tv_sec);
    } /* if */
    local_time->tm_year = (int) year - 1900;
    local_time->tm_mon  = (int) month - 1;
    local_time->tm_mday = (int) day;
    local_time->tm_hour = (int) hour;
    local_time->tm_min  = (int) min;
    local_time->tm_sec  = (int) sec;
    await_second = mktime(local_time);
    gettimeofday(&time_val, &this_time_zone);
    await_second += (time_zone - this_time_zone.tz_minuteswest) * 60;
    if (time_val.tv_sec < await_second ||
        (time_val.tv_sec == await_second &&
        time_val.tv_usec < mycro_sec)) {
      timer_value.it_value.tv_sec = await_second - time_val.tv_sec;
      if (mycro_sec >= time_val.tv_usec) {
        timer_value.it_value.tv_usec = mycro_sec - time_val.tv_usec;
      } else {
        timer_value.it_value.tv_usec = 1000000 - time_val.tv_usec + mycro_sec;
        timer_value.it_value.tv_sec--;
      } /* if */
#ifdef TRACE_TIM_UNX
      fprintf(stderr, "%d %ld %ld %ld %ld %ld %ld %ld %ld\n",
          this_time_zone.tz_minuteswest,
          time_val.tv_sec,
          await_second,
          await_second - time_val.tv_sec,
          time_val.tv_usec,
          mycro_sec,
          mycro_sec - time_val.tv_usec,
          timer_value.it_value.tv_sec,
          timer_value.it_value.tv_usec);
#endif
      timer_value.it_interval.tv_sec = 0;
      timer_value.it_interval.tv_usec = 0;
      if (signal(SIGALRM, alarm_signal_handler) != SIG_ERR) {
        if (setjmp(wait_finished) == 0) {
          if (setitimer(ITIMER_REAL, &timer_value, NULL) == 0) {
            pause();
          } /* if */
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_TIM_UNX
    printf("END timAwait\n");
#endif
  } /* timAwait */



#endif



#ifdef ANSI_C

void timNow (inttype *year, inttype *month, inttype *day, inttype *hour,
    inttype *min, inttype *sec, inttype *mycro_sec, inttype *time_zone)
#else

void timNow (year, month, day, hour, min, sec, mycro_sec, time_zone)
inttype *year;
inttype *month;
inttype *day;
inttype *hour;
inttype *min;
inttype *sec;
inttype *mycro_sec;
inttype *time_zone;
#endif

  {
    struct timeval time_val;
    struct timezone this_time_zone;
    struct tm *local_time;

  /* timNow */
#ifdef TRACE_TIM_UNX
    printf("BEGIN timNow\n");
#endif
    gettimeofday(&time_val, &this_time_zone);
    local_time = localtime(&time_val.tv_sec);
    *year      = local_time->tm_year + 1900;
    *month     = local_time->tm_mon + 1;
    *day       = local_time->tm_mday;
    *hour      = local_time->tm_hour;
    *min       = local_time->tm_min;
    *sec       = local_time->tm_sec;
    *mycro_sec = time_val.tv_usec;
    *time_zone = this_time_zone.tz_minuteswest;
#ifdef TRACE_TIM_UNX
    printf("END timNow(%d, %d, %d, %d, %d, %d, %d, %d)\n",
	*year, *month, *day, *hour, *min, *sec, *mycro_sec, *time_zone);
#endif
  } /* timNow */

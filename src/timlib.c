/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2000  Thomas Mertes                        */
/*                                                                  */
/*  This program is free software; you can redistribute it and/or   */
/*  modify it under the terms of the GNU General Public License as  */
/*  published by the Free Software Foundation; either version 2 of  */
/*  the License, or (at your option) any later version.             */
/*                                                                  */
/*  This program is distributed in the hope that it will be useful, */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   */
/*  GNU General Public License for more details.                    */
/*                                                                  */
/*  You should have received a copy of the GNU General Public       */
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/timlib.c                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions to do time operations.           */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "time.h"

#include "version.h"
#include "common.h"
#include "data.h"
#include "syvarutl.h"
#include "runerr.h"
#include "memory.h"
#include "tim_drv.h"

#undef EXTERN
#define EXTERN
#include "timlib.h"



#ifdef ANSI_C

objecttype tim_await (listtype arguments)
#else

objecttype tim_await (arguments)
listtype arguments;
#endif

  {
    long year;
    long month;
    long day;
    long hour;
    long minute;
    long second;
    long mycro_second;
    static struct tm *local_time = NULL;
    time_t calendar_time;
    long time_zone;
/*  time_t time_now; */
    long mycro_secs;

  /* tim_await */
    isit_int(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    isit_int(arg_7(arguments));
    year =         take_int(arg_1(arguments));
    month =        take_int(arg_2(arguments));
    day =          take_int(arg_3(arguments));
    hour =         take_int(arg_4(arguments));
    minute =       take_int(arg_5(arguments));
    second =       take_int(arg_6(arguments));
    mycro_second = take_int(arg_7(arguments));

/*  fprintf(stderr, "await    %04ld/%02ld/%02ld %02ld:%02ld:%02ld %7ld\n",
        year, month, day, hour, minute, second, mycro_second); */
    if (local_time == NULL) {
      get_time(&calendar_time, &mycro_secs, &time_zone);
      local_time = localtime(&calendar_time);
    } /* if */
/*  time_now = mktime(local_time);
    printf("%d %d\n", calendar_time, time_now); */
    local_time->tm_year = (int) year - 1900;
    local_time->tm_mon = (int) month - 1;
    local_time->tm_mday = (int) day;
    local_time->tm_hour = (int) hour;
    local_time->tm_min = (int) minute;
    local_time->tm_sec = (int) second;
    calendar_time = mktime(local_time);
/*  printf("%ld < %ld ?\n", time(NULL), calendar_time);
    LOC_TIME = localtime(&calendar_time); */
/*  fprintf(stderr, "await2   %04d/%02d/%02d %02d:%02d:%02d %7ld\n",
        local_time->tm_year + 1900,
        local_time->tm_mon + 1,
        local_time->tm_mday,
        local_time->tm_hour,
        local_time->tm_min,
        local_time->tm_sec,
        mycro_second); */
    await_time(calendar_time, mycro_second);
    return(SYS_EMPTY_OBJECT);
  } /* tim_await */



#ifdef ANSI_C

objecttype tim_now (listtype arguments)
#else

objecttype tim_now (arguments)
listtype arguments;
#endif

  { /* tim_now */
    isit_int(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    isit_int(arg_7(arguments));
    isit_int(arg_8(arguments));
    timNow(&arg_1(arguments)->value.intvalue,
	   &arg_2(arguments)->value.intvalue,
	   &arg_3(arguments)->value.intvalue,
	   &arg_4(arguments)->value.intvalue,
	   &arg_5(arguments)->value.intvalue,
	   &arg_6(arguments)->value.intvalue,
	   &arg_7(arguments)->value.intvalue,
	   &arg_8(arguments)->value.intvalue);
/*  fprintf(stderr, "now      %04ld/%02ld/%02ld %02ld:%02ld:%02ld %7ld\n",
        arg_1(arguments)->value.intvalue,
        arg_2(arguments)->value.intvalue,
        arg_3(arguments)->value.intvalue,
        arg_4(arguments)->value.intvalue,
        arg_5(arguments)->value.intvalue,
        arg_6(arguments)->value.intvalue,
        arg_7(arguments)->value.intvalue); */
    return(SYS_EMPTY_OBJECT);
  } /* tim_now */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

void tim_await (long year, long month, long day,
    long hour, long minute, long second, long mycro_second)
#else

void tim_await (year, month, day, hour, minute, second, mycro_second)
long year;
long month;
long day;
long hour;
long minute;
long second;
long mycro_second;
#endif

  {
    struct tm local_time;
    time_t calendar_time;

  /* tim_await */
/*  printf("%ld/%ld/%ld %ld:%ld:%ld\n", year, month, day, hour, minute, second); */
    local_time.tm_year = (int) year - 1900;
    local_time.tm_mon = (int) month - 1;
    local_time.tm_mday = (int) day;
    local_time.tm_hour = (int) hour;
    local_time.tm_min = (int) minute;
    local_time.tm_sec = (int) second;
    calendar_time = mktime(&local_time);
/*  printf("%ld < %ld ?\n", time(NULL), calendar_time);
    LOC_TIME = localtime(&calendar_time);
    printf("%d/%d/%d %d:%d:%d\n", LOC_TIME->tm_year + 1900,
        LOC_TIME->tm_mon + 1, LOC_TIME->tm_mday, LOC_TIME->tm_hour,
        LOC_TIME->tm_min, LOC_TIME->tm_sec); */
    do {
      ;
/*    printf("%ld ?= %ld\n", time(NULL), calendar_time); */
    } while (time(NULL) < calendar_time);
  } /* tim_await */



#ifdef ANSI_C

void tim_await (long year, long month, long day,
    long hour, long minute, long second, long mycro_second)
#else

void tim_await (year, month, day, hour, minute, second, mycro_second)
long year;
long month;
long day;
long hour;
long minute;
long second;
long mycro_second;
#endif

  {
    struct tm *local_time;
    time_t calendar_time;

  /* tim_await */
/*  printf("%ld/%ld/%ld %ld:%ld:%ld\n", year, month, day, hour, minute, second); */
    local_time = localtime(&calendar_time);
    local_time->tm_year = (int) year - 1900;
    local_time->tm_mon = (int) month - 1;
    local_time->tm_mday = (int) day;
    local_time->tm_hour = (int) hour;
    local_time->tm_min = (int) minute;
    local_time->tm_sec = (int) second;
    calendar_time = mktime(local_time);
/*  printf("%ld < %ld ?\n", time(NULL), calendar_time);
    LOC_TIME = localtime(&calendar_time);
    printf("%d/%d/%d %d:%d:%d\n", LOC_TIME->tm_year + 1900,
        LOC_TIME->tm_mon + 1, LOC_TIME->tm_mday, LOC_TIME->tm_hour,
        LOC_TIME->tm_min, LOC_TIME->tm_sec); */
    do {
      ;
/*    printf("%ld ?= %ld\n", time(NULL), calendar_time); */
    } while (time(NULL) < calendar_time);
  } /* tim_await */
#endif

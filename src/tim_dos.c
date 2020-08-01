/********************************************************************/
/*                                                                  */
/*  tim_dos.c     Time access using the dos capabilitys.            */
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
/*  File: seed7/src/tim_dos.c                                       */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: Time access using the dos capabilitys.                 */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include "sys/types.h"
#include "sys/timeb.h"

#include "version.h"
#include "common.h"

#undef EXTERN
#define EXTERN
#include "tim_drv.h"

#undef TRACE_TIM_DOS



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
    struct timeb tstruct;
    time_t await_second;

  /* timAwait */
#ifdef TRACE_TIM_DOS
    printf("BEGIN timAwait\n");
#endif
    if (local_time == NULL) {
      ftime(&tstruct);
      local_time = localtime(&tstruct.time);
    } /* if */
    local_time->tm_year = (int) year - 1900;
    local_time->tm_mon = (int) month - 1;
    local_time->tm_mday = (int) day;
    local_time->tm_hour = (int) hour;
    local_time->tm_min = (int) min;
    local_time->tm_sec = (int) sec;
    await_second = mktime(local_time);
    await_second += time_zone * 60;

    ftime(&tstruct);
/*  printf("%d %d %d\n", tstruct.time, tstruct.timezone, tstruct.dstflag); */
    await_second -= 60 * (int) tstruct.timezone;
    if (tstruct.dstflag) {
      await_second += 3600;
    } /* if */
/*  printf("%d %d %d %d\n",
        tstruct.time, tstruct.millitm, await_second, mycro_sec); */
/*  printf("tstruct.time < await_second: %d < %d\n", tstruct.time, await_second); */
    if (tstruct.time < await_second) {
      do {
        ftime(&tstruct);
/*      printf("%ld ?= %ld\n", tstruct.time, await_second); */
      } while (tstruct.time < await_second);
    } /* if */
    if (mycro_sec != 0) {
      do {
        ftime(&tstruct);
/*      printf("%ld.%ld000 ?= %ld.%ld\n",
            tstruct.time, (long) tstruct.millitm,
            await_second, mycro_sec); */
      } while (tstruct.time <= await_second &&
          1000 * ((long) tstruct.millitm) < mycro_sec);
    } /* if */
#ifdef TRACE_TIM_DOS
    printf("END timAwait\n");
#endif
  } /* timAwait */



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
    struct timeb tstruct;
    struct tm *local_time;

  /* timNow */
#ifdef TRACE_TIM_DOS
    printf("BEGIN timNow\n");
#endif
    ftime(&tstruct);
    local_time = localtime(&tstruct.time);
    *year      = local_time->tm_year + 1900;
    *month     = local_time->tm_mon + 1;
    *day       = local_time->tm_mday;
    *hour      = local_time->tm_hour;
    *min       = local_time->tm_min;
    *sec       = local_time->tm_sec;
    *mycro_sec = 1000 * ((long) tstruct.millitm);
    *time_zone = (long) tstruct.timezone;
    if (tstruct.dstflag) {
      *time_zone -= 60;
    } /* if */
#ifdef TRACE_TIM_DOS
    printf("END timNow(%d, %d, %d, %d, %d, %d, %d, %d)\n",
	*year, *month, *day, *hour, *min, *sec, *mycro_sec, *time_zone);
#endif
  } /* timNow */

/********************************************************************/
/*                                                                  */
/*  tim_win.c     Time access using the windows capabilitys.        */
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
/*  File: seed7/src/tim_win.c                                       */
/*  Changes: 2006  Thomas Mertes                                    */
/*  Content: Time access using the windows capabilitys.             */
/*                                                                  */
/********************************************************************/

#include "stdio.h"
#include "string.h"
#include "windows.h"

#include "version.h"
#include "common.h"

#undef EXTERN
#define EXTERN
#include "tim_drv.h"

#undef TRACE_TIM_WIN



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
    FILETIME system_time;
    FILETIME local_time;
    SYSTEMTIME time_struct;
    ULARGE_INTEGER local_time_64;
    TIME_ZONE_INFORMATION this_time_zone;

  /* timNow */
#ifdef TRACE_TIM_WIN
    printf("BEGIN timNow\n");
#endif
    GetSystemTimeAsFileTime(&system_time);
    FileTimeToLocalFileTime(&system_time, &local_time);
    FileTimeToSystemTime(&local_time, &time_struct);
    *year  = time_struct.wYear;
    *month = time_struct.wMonth;
    *day   = time_struct.wDay;
    *hour  = time_struct.wHour;
    *min   = time_struct.wMinute;
    *sec   = time_struct.wSecond;
    memcpy(&local_time_64, &local_time, sizeof(ULARGE_INTEGER));
    *mycro_sec = (local_time_64.QuadPart / 10) % 1000000;
    GetTimeZoneInformation(&this_time_zone);
    *time_zone = this_time_zone.Bias;
#ifdef TRACE_TIM_WIN
    printf("END timNow(%d, %d, %d, %d, %d, %d, %d, %d)\n",
	*year, *month, *day, *hour, *min, *sec, *mycro_sec, *time_zone);
#endif
  } /* timNow */



#ifdef ANSI_C

void get_time (time_t *calendar_time, long *mycro_secs, long *time_zone)
#else

void get_time (calendar_time, mycro_secs, time_zone)
time_t *calendar_time;
long *mycro_secs;
long *time_zone;
#endif

  {
    FILETIME system_time;
    ULARGE_INTEGER system_time_64;
    TIME_ZONE_INFORMATION this_time_zone;

  /* get_time */
#ifdef TRACE_TIM_WIN
    printf("BEGIN get_time\n");
#endif
    /* printf("time before %ld\n", time(NULL)); */
    GetSystemTimeAsFileTime(&system_time);
    /* printf("time after %ld\n", time(NULL)); */
    memcpy(&system_time_64, &system_time, sizeof(ULARGE_INTEGER));
    *calendar_time =  system_time_64.QuadPart / 10000000 - 3054539008LU;
    *mycro_secs = (system_time_64.QuadPart / 10) % 1000000;
    GetTimeZoneInformation(&this_time_zone);
    *time_zone = this_time_zone.Bias;
#ifdef TRACE_TIM_WIN
    printf("END get_time(%lu, %ld, %ld)\n",
        *calendar_time, *mycro_secs, *time_zone);
#endif
  } /* get_time */



#ifdef ANSI_C

void await_time (time_t calendar_time, long mycro_secs)
#else

void await_time (calendar_time, mycro_secs)
time_t calendar_time;
long mycro_secs;
#endif

  {
    FILETIME system_time;
    ULARGE_INTEGER system_time_64;
    time_t current_time;
    long current_mycro_secs;
    unsigned long wait_milliseconds;

  /* await_time */
#ifdef TRACE_TIM_WIN
    printf("BEGIN await_time(%lu, %ld)\n",
        calendar_time, mycro_secs);
#endif
    GetSystemTimeAsFileTime(&system_time);
    memcpy(&system_time_64, &system_time, sizeof(ULARGE_INTEGER));
    current_time =  system_time_64.QuadPart / 10000000 - 3054539008LU;
    current_mycro_secs = (system_time_64.QuadPart / 10) % 1000000;
    if (current_time < calendar_time ||
        (current_time == calendar_time &&
        current_mycro_secs < mycro_secs)) {
      wait_milliseconds = (calendar_time - current_time) * 1000;
      if (mycro_secs >= current_mycro_secs) {
        wait_milliseconds += (mycro_secs - current_mycro_secs) / 1000;
      } else {
        wait_milliseconds -= (current_mycro_secs - mycro_secs) / 1000;
      } /* if */
#ifdef TRACE_TIM_WIN
      printf("%lu %lu < %lu %lu Sleep(%lu)\n",
	  current_time, current_mycro_secs, calendar_time, mycro_secs,
	  wait_milliseconds);
#endif
      Sleep(wait_milliseconds);
    } /* if */
#ifdef TRACE_TIM_WIN
    printf("END await_time\n");
#endif
  } /* await_time */

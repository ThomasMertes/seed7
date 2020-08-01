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
    SYSTEMTIME await_time_struct;
    FILETIME await_file_time;
    FILETIME current_time;
    ULARGE_INTEGER time_64;
    long await_second;
    long current_second;
    long current_mycro_sec;
    unsigned long wait_milliseconds;

  /* timAwait */
#ifdef TRACE_TIM_WIN
    printf("BEGIN timAwait\n");
#endif
    await_time_struct.wYear         = year;
    await_time_struct.wMonth        = month;
    await_time_struct.wDay          = day;
    await_time_struct.wHour         = hour;
    await_time_struct.wMinute       = min;
    await_time_struct.wSecond       = sec;
    SystemTimeToFileTime(&await_time_struct, &await_file_time);
    memcpy(&time_64, &await_file_time, sizeof(ULARGE_INTEGER));
    await_second = time_64.QuadPart / 10000000 - 3054539008LU;
    await_second +=  time_zone * 60;
    GetSystemTimeAsFileTime(&current_time);
    memcpy(&time_64, &current_time, sizeof(ULARGE_INTEGER));
    current_second =  time_64.QuadPart / 10000000 - 3054539008LU;
    current_mycro_sec = (time_64.QuadPart / 10) % 1000000;
    if (current_second < await_second ||
        (current_second == await_second &&
        current_mycro_sec < mycro_sec)) {
      wait_milliseconds = (await_second - current_second) * 1000;
      if (mycro_sec >= current_mycro_sec) {
        wait_milliseconds += (mycro_sec - current_mycro_sec) / 1000;
      } else {
        wait_milliseconds -= (current_mycro_sec - mycro_sec) / 1000;
      } /* if */
#ifdef TRACE_TIM_WIN
      printf("%lu %lu < %lu %lu Sleep(%lu)\n",
          current_second, current_mycro_sec, await_second, mycro_sec,
          wait_milliseconds);
#endif
      Sleep(wait_milliseconds);
    } /* if */
#ifdef TRACE_TIM_WIN
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
    FILETIME system_time;
    FILETIME local_time;
    SYSTEMTIME time_struct;
    ULARGE_INTEGER time_64;
    long utc_seconds;
    long local_seconds;

  /* timNow */
#ifdef TRACE_TIM_WIN
    printf("BEGIN timNow\n");
#endif
    GetSystemTimeAsFileTime(&system_time);
    memcpy(&time_64, &system_time, sizeof(ULARGE_INTEGER));
    utc_seconds = time_64.QuadPart / 10000000 - 3054539008LU;
    FileTimeToLocalFileTime(&system_time, &local_time);
    FileTimeToSystemTime(&local_time, &time_struct);
    *year  = time_struct.wYear;
    *month = time_struct.wMonth;
    *day   = time_struct.wDay;
    *hour  = time_struct.wHour;
    *min   = time_struct.wMinute;
    *sec   = time_struct.wSecond;
    memcpy(&time_64, &local_time, sizeof(ULARGE_INTEGER));
    local_seconds = time_64.QuadPart / 10000000 - 3054539008LU;
    *mycro_sec = (time_64.QuadPart / 10) % 1000000;
    *time_zone = (utc_seconds - local_seconds) / 60;
#ifdef TRACE_TIM_WIN
    printf("timNow %lu %lu %lu %lu %ld %d %d\n",
        system_time.dwHighDateTime, system_time.dwLowDateTime,
        local_time.dwHighDateTime, local_time.dwLowDateTime,
        *time_zone, utc_seconds, local_seconds);
    printf("END timNow(%d, %d, %d, %d, %d, %d, %d, %d)\n",
        *year, *month, *day, *hour, *min, *sec, *mycro_sec, *time_zone);
#endif
  } /* timNow */

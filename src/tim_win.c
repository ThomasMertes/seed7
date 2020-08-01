/********************************************************************/
/*                                                                  */
/*  tim_win.c     Time access using the windows capabilitys.        */
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
/*  File: seed7/src/tim_win.c                                       */
/*  Changes: 2006, 2009  Thomas Mertes                              */
/*  Content: Time access using the windows capabilitys.             */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdio.h"
#include "string.h"
#include "time.h"
#include "windows.h"
#include "sys/stat.h"
#ifdef INCLUDE_SYS_UTIME
#include "sys/utime.h"
#else
#include "utime.h"
#endif
#include "errno.h"

#include "common.h"
#include "tim_rtl.h"
#include "fil_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "tim_drv.h"

#undef TRACE_TIM_WIN


/* Seconds between 1601-01-01 and 1970-01-01 */
#ifdef INT64TYPE_SUFFIX_LL
#define SECONDS_1601_1970 11644473600ULL
#else
#define SECONDS_1601_1970 11644473600
#endif



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
    union {
      uint64type nanosecs100; /*time since 1 Jan 1601 in 100ns units */
      FILETIME filetime;
    } await_file_time, current_time;
    long await_second;
    long current_second;
    long current_mycro_sec;
    unsigned long wait_milliseconds;

  /* timAwait */
#ifdef TRACE_TIM_WIN
    printf("BEGIN timAwait(%04ld-%02ld-%02ld %02ld:%02ld:%02ld.%06ld %ld)\n",
        year, month, day, hour, min, sec, mycro_sec, time_zone);
#endif
    await_time_struct.wYear         = year;
    await_time_struct.wMonth        = month;
    await_time_struct.wDay          = day;
    await_time_struct.wHour         = hour;
    await_time_struct.wMinute       = min;
    await_time_struct.wSecond       = sec;
    await_time_struct.wMilliseconds = 0;
    SystemTimeToFileTime(&await_time_struct, &await_file_time.filetime);
    await_second = await_file_time.nanosecs100 / 10000000 - SECONDS_1601_1970;
    await_second -= time_zone * 60;

    GetSystemTimeAsFileTime(&current_time.filetime);
    current_second = current_time.nanosecs100 / 10000000 - SECONDS_1601_1970;
    current_mycro_sec = (current_time.nanosecs100 / 10) % 1000000;
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
    inttype *min, inttype *sec, inttype *mycro_sec, inttype *time_zone,
    booltype *is_dst)
#else

void timNow (year, month, day, hour, min, sec, mycro_sec, time_zone, is_dst)
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
    union {
      uint64type nanosecs100; /*time since 1 Jan 1601 in 100ns units */
      FILETIME filetime;
    } utc_time;
    time_t utc_seconds;
#ifdef USE_LOCALTIME_R
    struct tm tm_result;
#endif
    struct tm *local_time;

  /* timNow */
#ifdef TRACE_TIM_WIN
    printf("BEGIN timNow\n");
#endif
    GetSystemTimeAsFileTime(&utc_time.filetime);
    utc_seconds = utc_time.nanosecs100 / 10000000 - SECONDS_1601_1970;
#ifdef USE_LOCALTIME_R
    local_time = localtime_r(&utc_seconds, &tm_result);
#else
    local_time = localtime(&utc_seconds);
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
      *mycro_sec = (utc_time.nanosecs100 / 10) % 1000000;
      *time_zone = (mkutc(local_time) - utc_seconds) / 60;
      *is_dst    = local_time->tm_isdst;
    } /* if */
#ifdef TRACE_TIM_WIN
    printf("END timNow(%04ld-%02ld-%02ld %02ld:%02ld:%02ld.%06ld %ld %d)\n",
        *year, *month, *day, *hour, *min, *sec,
        *mycro_sec, *time_zone, *is_dst);
#endif
  } /* timNow */



#ifdef USE_ALTERNATE_LOCALTIME_R
#undef localtime_r



#ifdef ANSI_C

struct tm *alternate_localtime_r (time_t *utc_seconds, struct tm *tm_result)
#else

struct tm *alternate_localtime_r (utc_seconds, tm_result)
time_t *utc_seconds;
struct tm *tm_result;
#endif

  {
    union {
      uint64type nanosecs100; /*time since 1 Jan 1601 in 100ns units */
      FILETIME filetime;
    } utc_time, time_zone_delta;
    SYSTEMTIME utc_time_struct;
    SYSTEMTIME local_time_struct;
    static time_t time_zone_seconds = 1;

  /* alternate_localtime_r */
#ifdef TRACE_TIM_WIN
    printf("BEGIN alternate_localtime_r(%ld)\n", *utc_seconds);
#endif
    if (time_zone_seconds == 1) {
      utc_time.nanosecs100 = SECONDS_1601_1970 * 10000000;
      /* FileTimeToLocalFileTime(&utc_time.filetime, &time_zone_delta.filetime);
      time_zone_seconds = time_zone_delta.nanosecs100 / 10000000 - SECONDS_1601_1970; */
      FileTimeToSystemTime(&utc_time.filetime, &utc_time_struct);
      SystemTimeToTzSpecificLocalTime(NULL, &utc_time_struct, &local_time_struct);
      tm_result->tm_year  = local_time_struct.wYear - 1900;
      tm_result->tm_mon   = local_time_struct.wMonth - 1;
      tm_result->tm_mday  = local_time_struct.wDay;
      tm_result->tm_hour  = local_time_struct.wHour;
      tm_result->tm_min   = local_time_struct.wMinute;
      tm_result->tm_sec   = local_time_struct.wSecond;
      time_zone_seconds = mkutc(tm_result);
      /* printf("%ld\n", time_zone_seconds); */
    } /* if */
    utc_time.nanosecs100 = (*utc_seconds + SECONDS_1601_1970) * 10000000;
    FileTimeToSystemTime(&utc_time.filetime, &utc_time_struct);
    SystemTimeToTzSpecificLocalTime(NULL, &utc_time_struct, &local_time_struct);
    tm_result->tm_year  = local_time_struct.wYear - 1900;
    tm_result->tm_mon   = local_time_struct.wMonth - 1;
    tm_result->tm_mday  = local_time_struct.wDay;
    tm_result->tm_hour  = local_time_struct.wHour;
    tm_result->tm_min   = local_time_struct.wMinute;
    tm_result->tm_sec   = local_time_struct.wSecond;
    tm_result->tm_isdst = mkutc(tm_result) - *utc_seconds != time_zone_seconds;
    return(tm_result);
  } /* alternate_localtime_r */
#endif



#ifdef USE_ALTERNATE_UTIME
#ifdef ANSI_C

int alternate_utime (wchar_t *os_path, os_utimbuf_struct *utime_buf)
#else

int alternate_utime (os_path)
wchar_t *os_path;
os_utimbuf_struct *utime_buf;
#endif

  {
    os_stat_struct stat_buf;
    HANDLE filehandle;
    union {
      uint64type nanosecs100; /*time since 1 Jan 1601 in 100ns units */
      FILETIME filetime;
    } actime, modtime;
    int result;

  /* alternate_utime */
    /* printf("alternate_utime: actime=%ld\n", utime_buf->actime);
       printf("alternate_utime: modtime=%ld\n", utime_buf->modtime); */
    result = os_utime_orig(os_path, utime_buf);
#ifdef UTIME_ORIG_BUGGY_FOR_FAT_FILES
    /* A FAT filesystem has a mtime resolution of 2 seconds. */
    /* The error causes an even mtime to be set 2 seconds too big. */
    if (result == 0 && (utime_buf->modtime & 1) == 0 &&
        os_stat(os_path, &stat_buf) == 0) {
      /* printf("new atime=%ld\n", stat_buf.st_atime);
         printf("new mtime=%ld\n", stat_buf.st_mtime); */
      if (stat_buf.st_mtime - utime_buf->modtime == 2) {
        utime_buf->modtime -= 2;
        result = os_utime_orig(os_path, utime_buf);
        utime_buf->modtime += 2;
      } /* if */
    } /* if */
#endif
    if (result != 0 && errno == EACCES &&
        os_stat(os_path, &stat_buf) == 0 && S_ISDIR(stat_buf.st_mode)) {
      /* printf("old atime=%ld\n", stat_buf.st_atime);
         printf("old mtime=%ld\n", stat_buf.st_mtime); */
      filehandle = CreateFileW(os_path, GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_DELETE, NULL,
                               OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
      if (filehandle != INVALID_HANDLE_VALUE) {
        /* The case of utime_buf == NULL is not considered,   */
        /* since alternate_utime will never be used this way. */
        actime.nanosecs100 = (utime_buf->actime + SECONDS_1601_1970) * 10000000;
        modtime.nanosecs100 = (utime_buf->modtime + SECONDS_1601_1970) * 10000000;
        /* printf("actime=%ld %Ld\n", utime_buf->actime, actime.nanosecs100);
           printf("modtime=%ld %Ld\n", utime_buf->modtime, modtime.nanosecs100); */
        if (SetFileTime(filehandle, NULL, &actime.filetime, &modtime.filetime) != 0) {
          result = 0;
        } /* if */
        CloseHandle(filehandle);
      } /* if */
    } /* if */
    return(result);
  } /* alternate_utime */
#endif

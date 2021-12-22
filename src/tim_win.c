/********************************************************************/
/*                                                                  */
/*  tim_win.c     Time functions which call the Windows API.        */
/*  Copyright (C) 1989 - 2011  Thomas Mertes                        */
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
/*  Content: Time functions which call the Windows API.             */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdio.h"
#include "string.h"
#include "time.h"
#include "windows.h"
#include "sys/stat.h"
#include "sys/types.h"
#ifdef INCLUDE_SYS_UTIME
#include "sys/utime.h"
#else
#include "utime.h"
#endif
#include "errno.h"

#include "common.h"
#include "os_decls.h"
#include "striutl.h"
#include "tim_rtl.h"
#include "fil_rtl.h"
#include "stat_drv.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "tim_drv.h"


/* Seconds between 1601-01-01 and 1970-01-01 */
#define SECONDS_FROM_1601_TO_1970 INT64_SUFFIX(11644473600)
#define WINDOWS_TICK UINT64_SUFFIX(10000000)

#define USE_UTIME_ORIG 0



/**
 *  Wait until the given time is reached
 *  @param time_zone Difference to UTC in minutes (for UTC+1 it is 60).
 *                   The time_zone includes the effect of a daylight saving time.
 */
void timAwait (intType year, intType month, intType day, intType hour,
    intType min, intType sec, intType micro_sec, intType time_zone)

  {
    SYSTEMTIME await_time_struct;
    union {
      uint64Type nanosecs100; /*time since 1 Jan 1601 in 100ns units */
      FILETIME filetime;
    } await_file_time, current_time;
    uint64Type await_second;
    uint64Type current_second;
    intType current_micro_sec;
    unsigned long wait_milliseconds;

  /* timAwait */
    logFunction(printf("timAwait(" F_D(04) "-" F_D(02) "-" F_D(02) " "
                                   F_D(02) ":" F_D(02) ":" F_D(02) "."
                                   F_D(06) ", " FMT_D ")\n",
                       year, month, day, hour, min, sec, micro_sec, time_zone););
    await_time_struct.wYear         = (WORD) year;
    await_time_struct.wMonth        = (WORD) month;
    await_time_struct.wDay          = (WORD) day;
    await_time_struct.wHour         = (WORD) hour;
    await_time_struct.wMinute       = (WORD) min;
    await_time_struct.wSecond       = (WORD) sec;
    await_time_struct.wMilliseconds = 0;
    if (unlikely(SystemTimeToFileTime(
        &await_time_struct, &await_file_time.filetime) == 0)) {
      logError(printf("timAwait: SystemTimeToFileTime() failed.\n"););
      raise_error(RANGE_ERROR);
    } else {
      await_second = await_file_time.nanosecs100 / 10000000;
      await_second = (uint64Type) ((int64Type) await_second - time_zone * 60);

      GetSystemTimeAsFileTime(&current_time.filetime);
      current_second = current_time.nanosecs100 / 10000000;
      current_micro_sec = (intType) ((current_time.nanosecs100 / 10) % 1000000);
      if (current_second < await_second ||
          (current_second == await_second &&
          current_micro_sec < micro_sec)) {
        wait_milliseconds = (unsigned long) (await_second - current_second) * 1000;
        if (micro_sec >= current_micro_sec) {
          wait_milliseconds += (unsigned long) ((micro_sec - current_micro_sec) / 1000);
        } else {
          wait_milliseconds -= (unsigned long) ((current_micro_sec - micro_sec) / 1000);
        } /* if */
        Sleep(wait_milliseconds);
      } /* if */
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
    union {
      uint64Type nanosecs100; /*time since 1 Jan 1601 in 100ns units */
      FILETIME filetime;
    } utc_time;
    intType micro_sec;

  /* timMicroSec */
    GetSystemTimeAsFileTime(&utc_time.filetime);
    micro_sec = (intType) (utc_time.nanosecs100 / 10);
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
    union {
      uint64Type nanosecs100; /*time since 1 Jan 1601 in 100ns units */
      FILETIME filetime;
    } utc_time;
    time_t utc_seconds;
#if defined USE_LOCALTIME_R || defined USE_LOCALTIME_S
    struct tm tm_result;
#endif
    struct tm *local_time;

  /* timNow */
    logFunction(printf("timNow\n"););
    GetSystemTimeAsFileTime(&utc_time.filetime);
    utc_seconds = (time_t) ((int64Type) (utc_time.nanosecs100 / WINDOWS_TICK) -
        SECONDS_FROM_1601_TO_1970);
#if defined USE_LOCALTIME_R
    local_time = localtime_r(&utc_seconds, &tm_result);
#elif defined USE_LOCALTIME_S
    if (localtime_s(&tm_result, &utc_seconds) != 0) {
      local_time = NULL;
    } else {
      local_time = &tm_result;
    } /* if */
#else
    local_time = localtime(&utc_seconds);
#endif
    if (unlikely(local_time == NULL)) {
      logError(printf("timNow: One of "
                      "localtime/localtime_r/localtime_s(" FMT_T ") failed:\n"
                      "errno=%d\nerror: %s\n",
                      utc_seconds, errno, strerror(errno)););
      raise_error(RANGE_ERROR);
    } else {
      *year      = local_time->tm_year + 1900;
      *month     = local_time->tm_mon + 1;
      *day       = local_time->tm_mday;
      *hour      = local_time->tm_hour;
      *min       = local_time->tm_min;
      *sec       = local_time->tm_sec;
      *micro_sec = (intType) ((utc_time.nanosecs100 / 10) % 1000000);
      *time_zone = (unchecked_mkutc(local_time) - utc_seconds) / 60;
      *is_dst    = local_time->tm_isdst > 0;
    } /* if */
    logFunction(printf("timNow(" F_D(04) "-" F_D(02) "-" F_D(02) " "
                                 F_D(02) ":" F_D(02) ":" F_D(02) "."
                                 F_D(06) ", " FMT_D ", %d) -->\n",
                       *year, *month, *day, *hour, *min, *sec,
                       *micro_sec, *time_zone, *is_dst););
  } /* timNow */



#ifdef USE_ALTERNATE_LOCALTIME_R
#undef localtime_r



struct tm *alternate_localtime_r (time_t *utc_seconds, struct tm *tm_result)

  {
    union {
      uint64Type nanosecs100; /*time since 1 Jan 1601 in 100ns units */
      FILETIME filetime;
    } utc_time, time_zone_delta;
    SYSTEMTIME utc_time_struct;
    SYSTEMTIME local_time_struct;
    static time_t time_zone_seconds = 1;

  /* alternate_localtime_r */
    logFunction(printf("alternate_localtime_r(%ld)\n", *utc_seconds););
    if (time_zone_seconds == 1) {
      utc_time.nanosecs100 = (uint64Type) SECONDS_FROM_1601_TO_1970 * WINDOWS_TICK;
      /* FileTimeToLocalFileTime(&utc_time.filetime, &time_zone_delta.filetime);
      time_zone_seconds = time_zone_delta.nanosecs100 / WINDOWS_TICK - SECONDS_FROM_1601_TO_1970; */
      if (unlikely(FileTimeToSystemTime(&utc_time.filetime, &utc_time_struct) == 0)) {
        return NULL;
      } else if (unlikely(SystemTimeToTzSpecificLocalTime(
          NULL, &utc_time_struct, &local_time_struct) == 0)) {
        return NULL;
      } else {
        tm_result->tm_year  = local_time_struct.wYear - 1900;
        tm_result->tm_mon   = local_time_struct.wMonth - 1;
        tm_result->tm_mday  = local_time_struct.wDay;
        tm_result->tm_hour  = local_time_struct.wHour;
        tm_result->tm_min   = local_time_struct.wMinute;
        tm_result->tm_sec   = local_time_struct.wSecond;
        time_zone_seconds = unchecked_mkutc(tm_result);
        /* printf("%ld\n", time_zone_seconds); */
      } /* if */
    } /* if */
    utc_time.nanosecs100 = (uint64Type) (
        (int64Type) *utc_seconds + SECONDS_FROM_1601_TO_1970) * WINDOWS_TICK;
    if (unlikely(FileTimeToSystemTime(&utc_time.filetime, &utc_time_struct) == 0)) {
      return NULL;
    } else if (unlikely(SystemTimeToTzSpecificLocalTime(
        NULL, &utc_time_struct, &local_time_struct) == 0)) {
      return NULL;
    } else {
      tm_result->tm_year  = local_time_struct.wYear - 1900;
      tm_result->tm_mon   = local_time_struct.wMonth - 1;
      tm_result->tm_mday  = local_time_struct.wDay;
      tm_result->tm_hour  = local_time_struct.wHour;
      tm_result->tm_min   = local_time_struct.wMinute;
      tm_result->tm_sec   = local_time_struct.wSecond;
      tm_result->tm_isdst = unchecked_mkutc(tm_result) - *utc_seconds != time_zone_seconds;
    } /* if */
    return tm_result;
  } /* alternate_localtime_r */
#endif



#ifdef USE_ALTERNATE_UTIME
/**
 *  Change the access and modification times of a file.
 *  Since a directory is a form of file the utime function should
 *  work for directories also. Unfortunately the windows function
 *  utime does not behave this way. Under windows utime fails with
 *  EACCES if it is called for a directory. In this case
 *  alternate_utime uses win32 functions to work for directories.
 *  If the windows function utime is called for a volume it
 *  fails with ENOENT. This case is recognized and alternate_utime
 *  uses win32 functions to work for volumes.
 */
int alternate_utime (const wchar_t *os_path, os_utimbuf_struct *utime_buf)

  {
#if USE_UTIME_ORIG && defined os_utime_orig
    os_stat_struct stat_buf;
#endif
    HANDLE filehandle;
    union {
      uint64Type nanosecs100; /*time since 1 Jan 1601 in 100ns units */
      FILETIME filetime;
    } actime, modtime;
    int result;

  /* alternate_utime */
    logFunction(printf("alternate_utime(\"%ls\", "
                       "{actime=" FMT_T ", modtime=" FMT_T "})\n",
                       os_path, utime_buf->actime, utime_buf->modtime););
#if USE_UTIME_ORIG && defined os_utime_orig
    result = os_utime_orig(os_path, utime_buf);
    logErrorIfTrue(result != 0,
                   printf("alternate_utime: os_utime_orig(\"%ls\", ...) failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_path, errno, strerror(errno)););
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
    if (result != 0 &&
        ((errno == EACCES &&
          os_stat(os_path, &stat_buf) == 0 && S_ISDIR(stat_buf.st_mode)) ||
        (errno == ENOENT &&
          ((os_path[PREFIX_LEN] >= 'a' && os_path[PREFIX_LEN] <= 'z') ||
           (os_path[PREFIX_LEN] >= 'A' && os_path[PREFIX_LEN] <= 'Z')) &&
          os_path[PREFIX_LEN + 1] == ':' && os_path[PREFIX_LEN + 2] == '\\' &&
          os_path[PREFIX_LEN + 3] == '\0'))) {
      /* printf("old atime=" FMT_T "\n", stat_buf.st_atime);
         printf("old mtime=" FMT_T "\n", stat_buf.st_mtime); */
#endif
      filehandle = CreateFileW(os_path, FILE_WRITE_ATTRIBUTES,
                               FILE_SHARE_READ | FILE_SHARE_DELETE, NULL,
                               OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
      if (unlikely(filehandle == INVALID_HANDLE_VALUE)) {
        logError(printf("alternate_utime: CreateFileW(\"%ls\", ...) failed:\n"
                        "GetLastError=" FMT_U32 "\n",
                        os_path, (uint32Type) GetLastError()););
      } else {
        /* The case of utime_buf == NULL is not considered,   */
        /* since alternate_utime will never be used this way. */
#if TIME_T_SIGNED
        actime.nanosecs100 = (uint64Type) (
            (int64Type) utime_buf->actime + SECONDS_FROM_1601_TO_1970) * WINDOWS_TICK;
        modtime.nanosecs100 = (uint64Type) (
            (int64Type) utime_buf->modtime + SECONDS_FROM_1601_TO_1970) * WINDOWS_TICK;
#else
        actime.nanosecs100 = ((uint64Type) utime_buf->actime +
            (uint64Type) SECONDS_FROM_1601_TO_1970) * WINDOWS_TICK;
        modtime.nanosecs100 = ((uint64Type) utime_buf->modtime +
            (uint64Type) SECONDS_FROM_1601_TO_1970) * WINDOWS_TICK;
#endif
        /* printf("actime=" FMT_T " " FMT_U64 "\n",
                  utime_buf->actime, actime.nanosecs100);
           printf("modtime=" FMT_T " " FMT_U64 "\n",
                  utime_buf->modtime, modtime.nanosecs100); */
        if (unlikely(SetFileTime(filehandle, NULL, &actime.filetime,
                                 &modtime.filetime) == 0)) {
          logError(printf("alternate_utime(\"%ls\", ...): SetFileTime() failed:\n"
                          "GetLastError=" FMT_U32 "\n",
                          os_path, (uint32Type) GetLastError()););
        } else {
          result = 0;
        } /* if */
        CloseHandle(filehandle);
      } /* if */
#if USE_UTIME_ORIG && defined os_utime_orig
    } /* if */
#endif
    logFunction(printf("alternate_utime --> %d\n", result););
    return result;
  } /* alternate_utime */
#endif

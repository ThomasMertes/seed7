/********************************************************************/
/*                                                                  */
/*  stat_win.c    Define functions used by os_stat macros.          */
/*  Copyright (C) 2015 - 2016  Thomas Mertes                        */
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
/*  File: seed7/src/stat_win.c                                      */
/*  Changes: 2015 - 2016  Thomas Mertes                             */
/*  Content: Define functions used by os_stat macros.               */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdio.h"
#include "string.h"
#include "sys/stat.h"
#include "sys/types.h"
#include "windows.h"
#include "io.h"
#ifdef OS_STRI_WCHAR
#include "wchar.h"
#endif
#include "time.h"
#include "errno.h"

#include "common.h"
#include "os_decls.h"
#include "striutl.h"
#include "fil_rtl.h"
#include "tim_drv.h"

#include "stat_drv.h"


/* Seconds between 1601-01-01 and 1970-01-01 */
#define SECONDS_1601_1970 INT64_SUFFIX(11644473600)
#define WINDOWS_TICK UINT64_SUFFIX(10000000)

#ifdef DEFINE_OS_STAT_ORIG_PROTOTYPE
#ifdef C_PLUS_PLUS
extern "C" int __cdecl os_stat_orig (const_os_striType path, os_stat_struct *statBuf);
#else
extern int __cdecl os_stat_orig (const_os_striType path, os_stat_struct *statBuf);
#endif
#endif



/**
 *  Convert a Windows FILETIME to a Unix time_t time.
 *  A Windows FILETIME is measured in 100 nanosecond units from
 *  1601-01-01 00:00:00. Actually FILETIME is a structure holding
 *  holding the high and low bits of a 64-bit value.
 *  A Unix time_t time is measured in seconds since
 *  1970-01-01 00:00:00. If time_t is 32-bit or 64-bit depends
 *  on the implementation of the C runtime library.
 */
static time_t fileTime2UnixTime (const FILETIME *fileTime)

  {
    uint64Type nanosecs100; /* Time since 1 Jan 1601 in 100ns units. */
    int64Type unixTime;

  /* fileTime2UnixTime */
    nanosecs100 = ((uint64Type) fileTime->dwHighDateTime << 32) | fileTime->dwLowDateTime;
    unixTime = (int64Type) (nanosecs100 / WINDOWS_TICK) - SECONDS_1601_1970;
    if (!inTimeTRange(unixTime)) {
      unixTime = 0;
    } /* if */
    return (time_t) unixTime;
  } /* fileTime2UnixTime */



#ifdef os_stat_orig
/**
 *  Undo the time adjustments done by a windows stat() function.
 *  The windows stat() functions (_wstati64, ...) return adjusted
 *  access, modification and status change times of a file. E.g.:
 *  During summer time a modification time from summer is returned
 *  unchanged. During winter time the same modification time is
 *  returned, without daylight saving adjustment, in winter time.
 *  So for the same file you get different modification times in
 *  summer and in winter. The same applies for a modification time
 *  from winter. This adjustments of Daylight Saving Time can be
 *  switched on and off in the Time Zone settings of Windows. But
 *  getting access, modification and status change times of a file
 *  should neither depend on the time when the request is done, nor
 *  on some strange settings. This function corrects this Daylight
 *  Saving Timetime adjustments.
 */
static time_t correctAdjustedUnixTime (time_t aTime)

  {
#if defined USE_LOCALTIME_R || defined USE_LOCALTIME_S
    struct tm tmResult;
#endif
    struct tm *localTime;
    SYSTEMTIME systemTime;
    FILETIME localFileTime;
    FILETIME fileTime;

  /* correctAdjustedUnixTime */
#if defined USE_LOCALTIME_R
    localTime = localtime_r(&aTime, &tmResult);
#elif defined USE_LOCALTIME_S
    if (localtime_s(&tmResult, &aTime) != 0) {
      localTime = NULL;
    } else {
      localTime = &tmResult;
    } /* if */
#else
    localTime = localtime(&aTime);
#endif
    systemTime.wYear =      (WORD) (localTime->tm_year + 1900);
    systemTime.wMonth =     (WORD) (localTime->tm_mon + 1);
    systemTime.wDayOfWeek = (WORD) localTime->tm_wday;
    systemTime.wDay =       (WORD) localTime->tm_mday;
    systemTime.wHour =      (WORD) localTime->tm_hour;
    systemTime.wMinute =    (WORD) localTime->tm_min;
    systemTime.wSecond =    (WORD) localTime->tm_sec;
    systemTime.wMilliseconds = 0;
    if (SystemTimeToFileTime(&systemTime, &localFileTime) == 0 ||
        LocalFileTimeToFileTime(&localFileTime, &fileTime) == 0) {
      return 0;
    } /* if */
    return fileTime2UnixTime(&fileTime);
  } /* correctAdjustedUnixTime */
#endif


/**
 *  Determine a Unix file mode from Windows attributes and file name.
 */
static short unsigned int fileAttr2UnixMode (DWORD attr, const wchar_t *path)

  {
    unsigned int mode;

  /* fileAttr2UnixMode */
    logFunction(printf("fileAttr2UnixMode(" FMT_X32 ", \"%ls\")\n",
                       attr, path != NULL ? path : L"**NULL**"););
    mode = S_IRUSR | S_IRGRP | S_IROTH;
    if ((attr & FILE_ATTRIBUTE_READONLY) == 0) {
      mode |= S_IWUSR | S_IWGRP | S_IWOTH;
    } /* if */
    if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
      mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
    } else {
      mode |= S_IFREG;
    } /* if */
    if (path != NULL && (mode & S_IFREG)) {
      memSizeType len = wcslen(path);
      if (len >= 4) {
        const WCHAR *end = &path[len - 4];
        if (_wcsicmp(end, L".bat") == 0 ||
            _wcsicmp(end, L".cmd") == 0 ||
            _wcsicmp(end, L".com") == 0 ||
            _wcsicmp(end, L".exe") == 0) {
          mode |= S_IXUSR | S_IXGRP | S_IXOTH;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("fileAttr2UnixMode --> 0%o\n", mode););
    return (short unsigned int) mode;
  } /* fileAttr2UnixMode */



#ifdef DEFINE_WSTATI64_EXT
/**
 *  Stat() function for a wide character path and a 64 bit file size.
 *  The windows stat() functions (_wstati64, ...) have a lot of issues:
 *  - They don't work for extended length paths (paths longer than MAX_PATH).
 *  - For devices they set all times to 1980-01-01 00:00:00.
 *  - They adjust times (In summer and winter different times are returned).
 *  Wstati64Ext avoids this problems.
 */
int wstati64Ext (const wchar_t *path, os_stat_struct *statBuf)

  {
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    int result = 0;

  /* wstati64Ext */
    logFunction(printf("wstati64Ext(\"%ls\", *)", path);
                fflush(stdout););
    if (likely(GetFileAttributesExW(path, GetFileExInfoStandard, &fileInfo) != 0)) {
      /* The function os_stat_orig() fails with ENOENT, if the path is     */
      /* longer than MAX_PATH. So GetFileAttributesExW(), which works with */
      /* an extended length path, is used.                                 */
      memset(statBuf, 0, sizeof(os_stat_struct));
      statBuf->st_nlink = 1;
      statBuf->st_mode = fileAttr2UnixMode(fileInfo.dwFileAttributes, path);
      /* For devices os_stat_orig() sets all times to 1980-01-01 00:00:00. */
      /* For daylight saving time os_stat_orig() returns adjusted times.   */
      /* To get correct times the times from GetFileAttributesExW() are    */
      /* used instead of the times from os_stat_orig().                    */
      statBuf->st_atime = fileTime2UnixTime(&fileInfo.ftLastAccessTime);
      statBuf->st_mtime = fileTime2UnixTime(&fileInfo.ftLastWriteTime);
      statBuf->st_ctime = fileTime2UnixTime(&fileInfo.ftCreationTime);
      if (!S_ISDIR(statBuf->st_mode)) {
        statBuf->st_size = ((int64Type) fileInfo.nFileSizeHigh << 32) |
                                        fileInfo.nFileSizeLow;
      } /* if */
      if (path[PREFIX_LEN] >= 'a' && path[PREFIX_LEN] <= 'z') {
        statBuf->st_dev = (wchar_t) (path[PREFIX_LEN] - 'a');
      } else if (path[PREFIX_LEN] >= 'A' && path[PREFIX_LEN] <= 'Z') {
        statBuf->st_dev = (wchar_t) (path[PREFIX_LEN] - 'A');
      } /* if */
      statBuf->st_rdev = statBuf->st_dev;
    } else {
      /* GetFileAttributesExW fails with ERROR_SHARING_VIOLATION, if the */
      /* file is currently in use, by some other program. This happens   */
      /* e.g. with c:\swapfile.sys or c:\hiberfil.sys. Interestingly     */
      /* os_stat_orig() succeeds for these files.                        */
#ifdef os_stat_orig
      DWORD lastError = GetLastError();
      logMessage(printf("wstati64Ext: GetFileAttributesExW(\"%ls\", *) failed:\n"
                        "GetLastError=" FMT_U32 "\n",
                        path, (uint32Type) lastError););
      if (lastError == ERROR_FILE_NOT_FOUND || lastError == ERROR_PATH_NOT_FOUND) {
        logError(printf("wstati64Ext: GetFileAttributesExW(\"%ls\", *) failed:\n"
                        "GetLastError=" FMT_U32 "\n",
                        path, (uint32Type) GetLastError()););
        errno = ENOENT;
        result = -1;
      } else if (os_stat_orig(&path[USE_EXTENDED_LENGTH_PATH * PREFIX_LEN], statBuf) == 0) {
        /* Os_stat_orig does not work with an extended length path. */
        /* printf("os_stat_orig(\"%ls\", *) succeeded.\n",
            &path[USE_EXTENDED_LENGTH_PATH * PREFIX_LEN]); */
        /* Undo the effect of time adjustments done by os_stat_orig(). */
        statBuf->st_atime = correctAdjustedUnixTime(statBuf->st_atime);
        statBuf->st_mtime = correctAdjustedUnixTime(statBuf->st_mtime);
        statBuf->st_ctime = correctAdjustedUnixTime(statBuf->st_ctime);
      } else {
        logError(printf("wstati64Ext: os_stat_orig(\"%ls\", *) failed:\n"
                        "errno=%d\nerror: %s\n",
                        &path[USE_EXTENDED_LENGTH_PATH * PREFIX_LEN],
                        errno, strerror(errno)););
        result = -1;
      } /* if */
#else
      logError(printf("wstati64Ext: GetFileAttributesExW(\"%ls\", *) failed:\n"
                      "GetLastError=" FMT_U32 "\n",
                      path, (uint32Type) GetLastError()););
      errno = ENOENT;
      result = -1;
#endif
    } /* if */
    logFunctionResult(printf("%d\n", result););
    return result;
  } /* wstati64Ext */
#endif



#ifdef DEFINE_FSTATI64_EXT
int fstati64Ext (int fd, os_fstat_struct *statBuf)

  {
    HANDLE fileHandle;
    BY_HANDLE_FILE_INFORMATION fileInfo;
#ifdef os_fstat_orig
#ifdef os_fstat_struct_orig
    os_fstat_struct_orig tmp;
#else
    struct stat tmp;
#endif
#endif
    int result;

  /* fstati64Ext */
    logFunction(printf("fstati64Ext(%d, *)", fd);
                fflush(stdout););
    fileHandle = (HANDLE) _get_osfhandle(fd);
#ifdef os_fstat_orig
    result = os_fstat_orig(fd, &tmp);
    if (result == 0) {
      tmp.st_mode &= ~(S_IWGRP | S_IWOTH);
      statBuf->st_dev   =             tmp.st_dev;
      statBuf->st_ino   =             tmp.st_ino;
      statBuf->st_mode  =             tmp.st_mode;
      statBuf->st_nlink =             tmp.st_nlink;
      statBuf->st_uid   =             tmp.st_uid;
      statBuf->st_gid   =             tmp.st_gid;
      statBuf->st_rdev  =             tmp.st_rdev;
      statBuf->st_size  = (int64Type) tmp.st_size;
      statBuf->st_atime =             tmp.st_atime;
      statBuf->st_mtime =             tmp.st_mtime;
      statBuf->st_ctime =             tmp.st_ctime;
      if (likely(GetFileInformationByHandle(fileHandle, &fileInfo) != 0)) {
        if (!(fileInfo.dwFileAttributes & FILE_ATTRIBUTE_READONLY)) {
          statBuf->st_mode |= S_IWUSR;
        } /* if */
        statBuf->st_size = ((int64Type) fileInfo.nFileSizeHigh << 32) |
                                        fileInfo.nFileSizeLow;
      } else {
        logError(printf("fstati64Ext(%d, *): "
                        "GetFileInformationByHandle(\"" FMT_U_MEM "\", *) failed.\n"
                        "GetLastError=" FMT_U32 "\n",
                        fd, (memSizeType) fileHandle, (uint32Type) GetLastError()););
        errno = EACCES;
        result = -1;
      } /* if */
    } else
#endif
    if (likely(GetFileInformationByHandle(fileHandle, &fileInfo) != 0)) {
      memset(statBuf, 0, sizeof(os_fstat_struct));
      statBuf->st_dev   =             0; /* Not assigned */
      statBuf->st_mode = fileAttr2UnixMode(fileInfo.dwFileAttributes, NULL);
      statBuf->st_nlink =             fileInfo.nNumberOfLinks;
      statBuf->st_rdev  =             0; /* Not assigned */
      statBuf->st_size = ((int64Type) fileInfo.nFileSizeHigh << 32) |
                                      fileInfo.nFileSizeLow;
      statBuf->st_atime = fileTime2UnixTime(&fileInfo.ftLastAccessTime);
      statBuf->st_mtime = fileTime2UnixTime(&fileInfo.ftLastWriteTime);
      statBuf->st_ctime = fileTime2UnixTime(&fileInfo.ftCreationTime);
    } else {
      logError(printf("fstati64Ext(%d, *): "
                      "GetFileInformationByHandle(\"" FMT_U_MEM "\", *) failed:\n"
                      "GetLastError=" FMT_U32 "\n",
                      fd, (memSizeType) fileHandle, (uint32Type) GetLastError()););
      errno = ENOENT;
      result = -1;
    } /* if */
    logFunctionResult(printf("%d\n", result););
    return result;
  } /* fstati64Ext */
#endif

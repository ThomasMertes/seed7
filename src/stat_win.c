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
#include "sys/stat.h"
#include "windows.h"
#include "io.h"
#include "errno.h"

#include "common.h"
#include "striutl.h"
#include "fil_rtl.h"

#include "stat_drv.h"


/* Seconds between 1601-01-01 and 1970-01-01 */
#define SECONDS_1601_1970 UINT64_SUFFIX(11644473600)

typedef int64Type stat_time64_t;

#ifdef DEFINE_OS_STAT_ORIG_PROTOTYPE
extern C int __cdecl os_stat_orig (const_os_striType path, os_stat_struct *stat_buf);
#endif



static int64Type filetime_to_unixtime (const FILETIME *fileTime)

  {
    union {
      uint64Type nanosecs100; /*time since 1 Jan 1601 in 100ns units */
      FILETIME filetime;
    } tmp;
    int64Type micro_sec;

  /* filetime_to_unixtime */
    memcpy(&tmp.filetime, fileTime, sizeof(FILETIME));
    micro_sec = (int64Type) (tmp.nanosecs100 / 10);
    micro_sec -= SECONDS_1601_1970 * 1000000;
    return micro_sec / 1000000;
  } /* filetime_to_unixtime */



static unsigned int fileattr_to_unixmode (DWORD attr, const wchar_t *path)

  {
    unsigned int mode = 0;

  /* fileattr_to_unixmode */
    logFunction(printf("fileattr_to_unixmode(" FMT_X32 ", \"%ls\")\n",
                       attr, path););
    if (attr & FILE_ATTRIBUTE_READONLY) {
      mode |= S_IRUSR;
    } else {
      mode |= S_IRUSR | S_IWUSR;
    } /* if */
    if (attr & FILE_ATTRIBUTE_DIRECTORY) {
      mode |= S_IFDIR | S_IEXEC;
    } else {
      mode |= S_IFREG;
    } /* if */
    if (path && (mode & S_IFREG)) {
      memSizeType len = wcslen(path);
      if (len >= 4) {
        const WCHAR *end = &path[len - 4];
        if (memcmp(end, L".bat", STRLEN(L".bat") * sizeof(WCHAR)) == 0 ||
            memcmp(end, L".cmd", STRLEN(L".cmd") * sizeof(WCHAR)) == 0 ||
            memcmp(end, L".com", STRLEN(L".com") * sizeof(WCHAR)) == 0 ||
            memcmp(end, L".exe", STRLEN(L".exe") * sizeof(WCHAR)) == 0) {
          mode |= S_IEXEC;
        } /* if */
      } /* if */
    } /* if */
    mode |= (mode & 0700) >> 3;
    mode |= (mode & 0700) >> 6;
    logFunction(printf("fileattr_to_unixmode --> 0%o\n", mode););
    return mode;
  } /* fileattr_to_unixmode */



/**
 *  Stat() function for a wide character path and a 64 bit file size.
 */
int wstati64Ext (const wchar_t *path, os_stat_struct *stat_buf)

  {
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    int result = 0;

  /* wstati64Ext */
    logFunction(printf("wstati64Ext(\"%ls\", *)", path);
                fflush(stdout););
#ifdef os_stat_orig
    /* Os_stat_orig does not work with an extended length path. */
    if (os_stat_orig(&path[USE_EXTENDED_LENGTH_PATH * PREFIX_LEN], stat_buf) == 0) {
      /* printf("os_stat_orig(\"%ls\", *) succeeded.\n",
          &path[USE_EXTENDED_LENGTH_PATH * PREFIX_LEN]); */
      if (wcslen(path) == PREFIX_LEN + 3 && path[PREFIX_LEN + 1] == ':' &&
          (path[PREFIX_LEN + 2] == '/' || path[PREFIX_LEN + 2] == '\\') &&
          ((path[PREFIX_LEN] >= 'a' && path[PREFIX_LEN] <= 'z') ||
           (path[PREFIX_LEN] >= 'A' && path[PREFIX_LEN] <= 'Z')) &&
          GetFileAttributesExW(path, GetFileExInfoStandard, &fileInfo)) {
        /* For devices os_stat_orig sets all times to 1980-01-01 00:00:00. */
        /* This is corrected by values from GetFileAttributesExW.          */
        stat_buf->st_atime = filetime_to_unixtime(&fileInfo.ftLastAccessTime);
        stat_buf->st_mtime = filetime_to_unixtime(&fileInfo.ftLastWriteTime);
        stat_buf->st_ctime = filetime_to_unixtime(&fileInfo.ftCreationTime);
      } /* if */
    } else
#endif
    if (wcspbrk(&path[USE_EXTENDED_LENGTH_PATH * PREFIX_LEN], L"?*")) {
      logError(printf("wstati64Ext(\"%ls\", *): Wildcards in path.\n", path););
      errno = ENOENT;
      result = -1;
    } else if (GetFileAttributesExW(path, GetFileExInfoStandard, &fileInfo)) {
      memset(stat_buf, 0, sizeof(os_stat_struct));
      stat_buf->st_nlink = 1;
      stat_buf->st_mode = fileattr_to_unixmode(fileInfo.dwFileAttributes, path);
      stat_buf->st_atime = filetime_to_unixtime(&fileInfo.ftLastAccessTime);
      stat_buf->st_mtime = filetime_to_unixtime(&fileInfo.ftLastWriteTime);
      stat_buf->st_ctime = filetime_to_unixtime(&fileInfo.ftCreationTime);
      stat_buf->st_size = ((int64Type) fileInfo.nFileSizeHigh << 32) | fileInfo.nFileSizeLow;
      if (path[PREFIX_LEN] >= 'a' && path[PREFIX_LEN] <= 'z') {
        stat_buf->st_dev = path[PREFIX_LEN] - 'a';
      } else if (path[PREFIX_LEN] >= 'A' && path[PREFIX_LEN] <= 'Z') {
        stat_buf->st_dev = path[PREFIX_LEN] - 'A';
      } /* if */
      stat_buf->st_rdev = stat_buf->st_dev;
    } else {
      logError(printf("wstati64Ext: GetFileAttributesExW(\"%ls\", *) failed:\n"
                      "GetLastError=" FMT_U32 "\n",
                      path, (uint32Type) GetLastError()););
      errno = ENOENT;
      result = -1;
    } /* if */
    logFunctionResult(printf("%d\n", result););
    return result;
  } /* wstati64Ext */



int fstati64 (int fd, os_stat_struct *stat_buf)

  {
    BY_HANDLE_FILE_INFORMATION info;
    struct stat tmp;
    int result;

  /* fstati64 */
    logFunction(printf("fstati64(%d, *)", fd);
                fflush(stdout););
    result = fstat(fd, &tmp);
    if (result == 0) {
      tmp.st_mode &= ~(S_IWGRP | S_IWOTH);
      stat_buf->st_dev   =             tmp.st_dev;
      stat_buf->st_ino   =             tmp.st_ino;
      stat_buf->st_mode  =             tmp.st_mode;
      stat_buf->st_nlink =             tmp.st_nlink;
      stat_buf->st_uid   =             tmp.st_uid;
      stat_buf->st_gid   =             tmp.st_gid;
      stat_buf->st_rdev  =             tmp.st_rdev;
      stat_buf->st_size  = (int64Type) tmp.st_size;
      stat_buf->st_atime =             tmp.st_atime;
      stat_buf->st_mtime =             tmp.st_mtime;
      stat_buf->st_ctime =             tmp.st_ctime;
      if (!GetFileInformationByHandle((HANDLE) _get_osfhandle(fd), &info)) {
        logError(printf("fstati64(%d, *): "
                        "GetFileInformationByHandle(\"" FMT_U_MEM "\", *) failed.\n",
                        fd, (memSizeType) _get_osfhandle(fd)););
        errno = EACCES;
        result = -1;
      } else {
        if (!(info.dwFileAttributes & FILE_ATTRIBUTE_READONLY)) {
          stat_buf->st_mode |= S_IWUSR;
        } /* if */
        stat_buf->st_size = ((int64Type) info.nFileSizeHigh << 32) | info.nFileSizeLow;
      } /* if */
    } /* if */
    logFunctionResult(printf("%d\n", result););
    return result;
  } /* fstati64 */

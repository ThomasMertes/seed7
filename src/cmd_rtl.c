/********************************************************************/
/*                                                                  */
/*  cmd_rtl.c     Directory, file and other system functions.       */
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
/*  File: seed7/src/cmd_rtl.c                                       */
/*  Changes: 1994, 2006, 2009  Thomas Mertes                        */
/*  Content: Directory, file and other system functions.            */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "limits.h"
#include "time.h"
#include "sys/types.h"
#include "sys/stat.h"
#ifdef INCLUDE_SYS_UTIME
#include "sys/utime.h"
#else
#include "utime.h"
#endif
#ifdef OS_STRI_WCHAR
#include "wchar.h"
#ifdef OS_WIDE_DIR_INCLUDE_DIR_H
#include "dir.h"
#endif
#ifdef OS_CHMOD_INCLUDE_IO_H
#include "io.h"
#endif
#endif
#include "errno.h"

#ifdef UNISTD_H_PRESENT
#include "unistd.h"
#endif

#include "dir_drv.h"

#include "common.h"
#include "data_rtl.h"
#include "os_decls.h"
#include "heaputl.h"
#include "striutl.h"
#include "str_rtl.h"
#include "fil_rtl.h"
#include "dir_rtl.h"
#include "set_rtl.h"
#include "tim_rtl.h"
#include "tim_drv.h"
#include "big_drv.h"
#include "cmd_drv.h"
#include "rtl_err.h"

#ifdef USE_MMAP
#include "sys/mman.h"
#endif

#undef EXTERN
#define EXTERN
#include "cmd_rtl.h"

#undef TRACE_CMD_RTL
#undef VERBOSE_EXCEPTIONS


#define MAX_CSTRI_BUFFER_LEN 40

#ifndef CPLUSPLUS_COMPILER
#define CPLUSPLUS_COMPILER C_COMPILER
#endif
#ifndef EXECUTABLE_FILE_EXTENSION
#define EXECUTABLE_FILE_EXTENSION ""
#endif
#ifndef CC_FLAGS
#define CC_FLAGS ""
#endif
#ifndef REDIRECT_C_ERRORS
#define REDIRECT_C_ERRORS ""
#endif
#ifndef LINKER_OPT_DEBUG_INFO
#define LINKER_OPT_DEBUG_INFO ""
#endif
#ifndef LINKER_OPT_NO_DEBUG_INFO
#define LINKER_OPT_NO_DEBUG_INFO ""
#endif
#ifndef LINKER_OPT_OUTPUT_FILE
#define LINKER_OPT_OUTPUT_FILE ""
#endif
#ifndef LINKER_FLAGS
#define LINKER_FLAGS ""
#endif

#ifndef INT64TYPE
#define INT64TYPE_STRI ""
#define UINT64TYPE_STRI ""
#define INT32TYPE_LITERAL_SUFFIX ""
#define INT64TYPE_LITERAL_SUFFIX ""
#endif

#ifndef INT128TYPE
#define INT128TYPE_STRI ""
#define UINT128TYPE_STRI ""
#endif

#ifndef OVERFLOW_SIGNAL
#define OVERFLOW_SIGNAL ""
#endif

#ifndef PATH_MAX
#define PATH_MAX 2048
#endif

#define SIZE_NORMAL_BUFFER   32768
#define SIZE_RESERVE_BUFFER   2048

#define PRESERVE_NOTHING    0x00
#define PRESERVE_MODE       0x01
#define PRESERVE_OWNERSHIP  0x02
#define PRESERVE_TIMESTAMPS 0x04
#define PRESERVE_SYMLINKS   0x08
#define PRESERVE_ALL        0xFF

#define USR_BITS_NORMAL (S_IRUSR == 0400 && S_IWUSR == 0200 && S_IXUSR == 0100)
#define GRP_BITS_NORMAL (S_IRGRP == 0040 && S_IWGRP == 0020 && S_IXGRP == 0010)
#define OTH_BITS_NORMAL (S_IROTH == 0004 && S_IWOTH == 0002 && S_IXOTH == 0001)
#define MODE_BITS_NORMAL (USR_BITS_NORMAL && GRP_BITS_NORMAL && OTH_BITS_NORMAL)

#define INITAL_ARRAY_SIZE  256
#define ARRAY_SIZE_DELTA   256

#ifdef DEFINE_OS_ENVIRON
extern os_striType *os_environ;
#endif

extern striType programPath; /* defined in s7.c or in the executable of a program */

#ifdef FILE_UNKNOWN
#undef FILE_UNKNOWN
#endif

#define FILE_ABSENT   0 /* A component of path does not exist */
#define FILE_UNKNOWN  1 /* File exists but has an unknown type */
#define FILE_REGULAR  2
#define FILE_DIR      3
#define FILE_CHAR     4
#define FILE_BLOCK    5
#define FILE_FIFO     6
#define FILE_SYMLINK  7
#define FILE_SOCKET   8

#ifdef VERBOSE_EXCEPTIONS
#define logError(logStatements) logStatements
#else
#define logError(logStatements)
#endif



#ifdef USE_CDECL
static int _cdecl cmp_mem (char *strg1, char *strg2)
#else
static int cmp_mem (void const *strg1, void const *strg2)
#endif

  { /* cmp_mem */
    return (int) strCompare(
        ((const_rtlObjectType *) strg1)->value.striValue,
        ((const_rtlObjectType *) strg2)->value.striValue);
  } /* cmp_mem */



static void remove_any_file (const const_os_striType file_name, errInfoType *);
static void copy_any_file (const const_os_striType from_name,
    const const_os_striType to_name, int, errInfoType *err_info);



static void remove_dir (const const_os_striType dir_name, errInfoType *err_info)

  {
    os_DIR *directory;
    os_dirent_struct *current_entry;
    size_t dir_name_size;
    size_t dir_path_capacity = 0;
    os_striType dir_path = NULL;
    size_t new_size;
    os_striType resized_path;
    boolType init_path = TRUE;

  /* remove_dir */
#ifdef TRACE_CMD_RTL
    printf("BEGIN remove_dir(\"" FMT_S_OS "\")\n", dir_name);
#endif
/*  printf("opendir(" FMT_S_OS ");\n", dir_name);
    fflush(stdout); */
    if ((directory = os_opendir(dir_name)) == NULL) {
      *err_info = FILE_ERROR;
    } else {
      do {
        current_entry = os_readdir(directory);
/*      printf("$%ld$\n", (long) current_entry);
        fflush(stdout); */
      } while (current_entry != NULL &&
          (memcmp(current_entry->d_name, dot,    sizeof(os_charType) * 2) == 0 ||
           memcmp(current_entry->d_name, dotdot, sizeof(os_charType) * 3) == 0));
      dir_name_size = os_stri_strlen(dir_name);
      while (*err_info == OKAY_NO_ERROR && current_entry != NULL) {
/*      printf("!" FMT_S_OS "!\n", current_entry->d_name);
        fflush(stdout); */
        new_size = dir_name_size + 1 + os_stri_strlen(current_entry->d_name);
        if (new_size > dir_path_capacity) {
          resized_path = REALLOC_OS_STRI(dir_path, new_size);
          if (resized_path != NULL) {
            dir_path = resized_path;
            dir_path_capacity = new_size;
          } else if (dir_path != NULL) {
            FREE_OS_STRI(dir_path);
            dir_path = NULL;
          } /* if */
        } /* if */
        if (dir_path != NULL) {
          if (init_path) {
            os_stri_strcpy(dir_path, dir_name);
            os_stri_strcpy(&dir_path[dir_name_size], slash);
            init_path = FALSE;
          } /* if */
          os_stri_strcpy(&dir_path[dir_name_size + 1], current_entry->d_name);
          remove_any_file(dir_path, err_info);
        } else {
          *err_info = MEMORY_ERROR;
        } /* if */
        do {
          current_entry = os_readdir(directory);
/*        printf("$%ld$\n", (long) current_entry);
          fflush(stdout); */
        } while (current_entry != NULL &&
            (memcmp(current_entry->d_name, dot,    sizeof(os_charType) * 2) == 0 ||
             memcmp(current_entry->d_name, dotdot, sizeof(os_charType) * 3) == 0));
      } /* while */
      if (dir_path != NULL) {
        FREE_OS_STRI(dir_path);
      } /* if */
      os_closedir(directory);
      if (*err_info == OKAY_NO_ERROR) {
        /* printf("before remove directory <" FMT_S_OS ">\n", dir_name); */
        if (os_rmdir(dir_name) != 0) {
          logError(printf(" *** remove_dir: os_rmdir(\"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          dir_name, errno, strerror(errno)););
          /* printf("EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d  ENOTDIR=%d  EROFS=%d\n",
              EACCES, EBUSY, EEXIST, ENOTEMPTY, ENOENT, ENOTDIR, EROFS);
          printf("EFAULT=%d  EISDIR=%d  ENAMETOOLONG=%d  ENODEV=%d  EINVAL=%d\n",
              EFAULT, EISDIR, ENAMETOOLONG, ENODEV, EINVAL); */
          *err_info = FILE_ERROR;
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END remove_dir(\"" FMT_S_OS "\", %d)\n", dir_name, *err_info);
#endif
  } /* remove_dir */



static void remove_any_file (const const_os_striType file_name, errInfoType *err_info)

  {
    os_stat_struct file_stat;

  /* remove_any_file */
#ifdef TRACE_CMD_RTL
    printf("BEGIN remove_any_file(\"" FMT_S_OS "\")\n", file_name);
#endif
    if (os_lstat(file_name, &file_stat) != 0) {
      logError(printf(" *** remove_any_file: File " FMT_S_OS " does not exist.\n",
                      file_name););
      *err_info = FILE_ERROR;
    } else {
      if (S_ISDIR(file_stat.st_mode)) {
        remove_dir(file_name, err_info);
      } else {
        if (os_remove(file_name) != 0) {
          logError(printf(" *** remove_any_file: os_remove(\"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          file_name, errno, strerror(errno)););
          *err_info = FILE_ERROR;
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END remove_any_file(\"" FMT_S_OS "\", %d)\n", file_name, *err_info);
#endif
  } /* remove_any_file */



static void copy_file (const const_os_striType from_name,
    const const_os_striType to_name, errInfoType *err_info)

  {
    FILE *from_file;
    FILE *to_file;
#ifdef USE_MMAP
    int file_no;
    os_fstat_struct file_stat;
    memSizeType file_length;
    ustriType file_content;
    boolType classic_copy = TRUE;
#endif
    char *normal_buffer;
    char *buffer;
    size_t buffer_size;
    char reserve_buffer[SIZE_RESERVE_BUFFER];
    size_t bytes_read;

  /* copy_file */
#ifdef TRACE_CMD_RTL
    printf("BEGIN copy_file(\"" FMT_S_OS "\", \"" FMT_S_OS "\")\n", from_name, to_name);
#endif
    if ((from_file = os_fopen(from_name, os_mode_rb)) != NULL) {
      if ((to_file = os_fopen(to_name, os_mode_wb)) != NULL) {
#ifdef USE_MMAP
        file_no = fileno(from_file);
        if (file_no != -1 && os_fstat(file_no, &file_stat) == 0) {
          if (file_stat.st_size >= 0 && (unsigned_os_off_t) file_stat.st_size < MAX_MEMSIZETYPE) {
            file_length = (memSizeType) file_stat.st_size;
            if ((file_content = (ustriType) mmap(NULL, file_length,
                PROT_READ, MAP_PRIVATE, fileno(from_file),
                0)) != (ustriType) -1) {
              if (fwrite(file_content, 1, file_length, to_file) != file_length) {
                *err_info = FILE_ERROR;
              } /* if */
              munmap(file_content, file_length);
              classic_copy = FALSE;
            } /* if */
          } /* if */
        } /* if */
        if (classic_copy) {
#endif
          if (ALLOC_BYTES(normal_buffer, SIZE_NORMAL_BUFFER)) {
            buffer = normal_buffer;
            buffer_size = SIZE_NORMAL_BUFFER;
          } else {
            buffer = reserve_buffer;
            buffer_size = SIZE_RESERVE_BUFFER;
          } /* if */
          while (*err_info == OKAY_NO_ERROR && (bytes_read =
              fread(buffer, 1, buffer_size, from_file)) != 0) {
            if (fwrite(buffer, 1, bytes_read, to_file) != bytes_read) {
              *err_info = FILE_ERROR;
            } /* if */
          } /* while */
          if (normal_buffer != NULL) {
            FREE_BYTES(normal_buffer, SIZE_NORMAL_BUFFER);
          } /* if */
#ifdef USE_MMAP
        } /* if */
#endif
        if (fclose(from_file) != 0) {
          *err_info = FILE_ERROR;
        } /* if */
        if (fclose(to_file) != 0) {
          *err_info = FILE_ERROR;
        } /* if */
        if (*err_info != OKAY_NO_ERROR) {
          os_remove(to_name);
        } /* if */
      } else {
        logError(printf(" *** copy_file: os_fopen(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        to_name, os_mode_wb, errno, strerror(errno)););
        /* printf("EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d  ENOTDIR=%d  EROFS=%d\n",
            EACCES, EBUSY, EEXIST, ENOTEMPTY, ENOENT, ENOTDIR, EROFS);
        printf("EFAULT=%d  EISDIR=%d  ENAMETOOLONG=%d  ENODEV=%d  EINVAL=%d\n",
            EFAULT, EISDIR, ENAMETOOLONG, ENODEV, EINVAL); */
        fclose(from_file);
        *err_info = FILE_ERROR;
      } /* if */
    } else {
      logError(printf(" *** copy_file: os_fopen(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                      "errno=%d\nerror: %s\n",
                      from_name, os_mode_rb, errno, strerror(errno)););
      /* printf("EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d  ENOTDIR=%d  EROFS=%d\n",
          EACCES, EBUSY, EEXIST, ENOTEMPTY, ENOENT, ENOTDIR, EROFS);
      printf("EFAULT=%d  EISDIR=%d  ENAMETOOLONG=%d  ENODEV=%d  EINVAL=%d\n",
          EFAULT, EISDIR, ENAMETOOLONG, ENODEV, EINVAL); */
      *err_info = FILE_ERROR;
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END copy_file(\"" FMT_S_OS "\", \"" FMT_S_OS "\", %d)\n",
        from_name, to_name, *err_info);
#endif
  } /* copy_file */



static void copy_dir (const const_os_striType from_name,
    const const_os_striType to_name, int flags, errInfoType *err_info)

  {
    os_DIR *directory;
    os_dirent_struct *current_entry;
    size_t from_name_size;
    size_t to_name_size;
    size_t d_name_size;
    size_t from_path_capacity = 0;
    os_striType from_path = NULL;
    size_t to_path_capacity = 0;
    os_striType to_path = NULL;
    size_t new_size;
    os_striType resized_path;
    boolType init_path = TRUE;

  /* copy_dir */
#ifdef TRACE_CMD_RTL
    printf("BEGIN copy_dir(\"" FMT_S_OS "\", \"" FMT_S_OS "\")\n", from_name, to_name);
#endif
    /* printf("opendir(" FMT_S_OS ");\n", from_name);
       fflush(stdout); */
    if ((directory = os_opendir(from_name)) == NULL) {
      *err_info = FILE_ERROR;
    } else {
      if (os_mkdir(to_name, (S_IRWXU | S_IRWXG | S_IRWXO)) != 0) {
        *err_info = FILE_ERROR;
      } else {
        do {
          current_entry = os_readdir(directory);
          /* printf("$%ld$\n", (long) current_entry);
             fflush(stdout); */
        } while (current_entry != NULL &&
            (memcmp(current_entry->d_name, dot,    sizeof(os_charType) * 2) == 0 ||
             memcmp(current_entry->d_name, dotdot, sizeof(os_charType) * 3) == 0));
        from_name_size = os_stri_strlen(from_name);
        to_name_size = os_stri_strlen(to_name);
        while (*err_info == OKAY_NO_ERROR && current_entry != NULL) {
          /* printf("!" FMT_S_OS "!\n", current_entry->d_name);
             fflush(stdout); */
          d_name_size = os_stri_strlen(current_entry->d_name);
          new_size = from_name_size + 1 + d_name_size;
          if (new_size > from_path_capacity) {
            resized_path = REALLOC_OS_STRI(from_path, new_size);
            if (resized_path != NULL) {
              from_path = resized_path;
              from_path_capacity = new_size;
            } else if (from_path != NULL) {
              FREE_OS_STRI(from_path);
              from_path = NULL;
            } /* if */
          } /* if */
          new_size = to_name_size + 1 + d_name_size;
          if (new_size > to_path_capacity) {
            resized_path = REALLOC_OS_STRI(to_path, new_size);
            if (resized_path != NULL) {
              to_path = resized_path;
              to_path_capacity = new_size;
            } else if (to_path != NULL) {
              FREE_OS_STRI(to_path);
              to_path = NULL;
            } /* if */
          } /* if */
          if (from_path != NULL && to_path != NULL) {
            if (init_path) {
              os_stri_strcpy(from_path, from_name);
              os_stri_strcpy(&from_path[from_name_size], slash);
              os_stri_strcpy(to_path, to_name);
              os_stri_strcpy(&to_path[to_name_size], slash);
              init_path = FALSE;
            } /* if */
            os_stri_strcpy(&from_path[from_name_size + 1], current_entry->d_name);
            os_stri_strcpy(&to_path[to_name_size + 1], current_entry->d_name);
            copy_any_file(from_path, to_path, flags, err_info);
          } else {
            *err_info = MEMORY_ERROR;
          } /* if */
          do {
            current_entry = os_readdir(directory);
            /* printf("$%ld$\n", (long) current_entry);
               fflush(stdout); */
          } while (current_entry != NULL &&
              (memcmp(current_entry->d_name, dot,    sizeof(os_charType) * 2) == 0 ||
               memcmp(current_entry->d_name, dotdot, sizeof(os_charType) * 3) == 0));
        } /* while */
        if (*err_info != OKAY_NO_ERROR) {
          remove_dir(to_name, err_info);
        } /* if */
        if (from_path != NULL) {
          FREE_OS_STRI(from_path);
        } /* if */
        if (to_path != NULL) {
          FREE_OS_STRI(to_path);
        } /* if */
      } /* if */
      os_closedir(directory);
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END copy_dir(\"" FMT_S_OS "\", \"" FMT_S_OS "\", %d)\n",
        from_name, to_name, *err_info);
#endif
  } /* copy_dir */



static void copy_any_file (const const_os_striType from_name,
    const const_os_striType to_name, int flags, errInfoType *err_info)

  {
    os_stat_struct from_stat;
    int from_stat_result;
#ifdef HAS_SYMLINKS
    os_striType link_destination;
    ssize_t readlink_result;
#endif
    os_utimbuf_struct to_utime;

  /* copy_any_file */
#ifdef TRACE_CMD_RTL
    printf("BEGIN copy_any_file(\"" FMT_S_OS "\", \"" FMT_S_OS "\")\n", from_name, to_name);
#endif
    if (flags & PRESERVE_SYMLINKS) {
      from_stat_result = os_lstat(from_name, &from_stat);
    } else {
      from_stat_result = os_stat(from_name, &from_stat);
    } /* if */
    if (from_stat_result != 0) {
      /* Source file does not exist */
      *err_info = FILE_ERROR;
    } else {
      if (S_ISLNK(from_stat.st_mode)) {
#ifdef HAS_SYMLINKS
        /* printf("link size=%lu\n", from_stat.st_size); */
        if (from_stat.st_size < 0 ||
            (unsigned_os_off_t) from_stat.st_size > MAX_OS_STRI_LEN) {
          *err_info = RANGE_ERROR;
        } else {
          if (!os_stri_alloc(link_destination, (memSizeType) from_stat.st_size)) {
            *err_info = MEMORY_ERROR;
          } else {
            readlink_result = readlink(from_name, link_destination,
                                       (size_t) from_stat.st_size);
            if (readlink_result != -1) {
              link_destination[readlink_result] = '\0';
              /* printf("readlink_result=%lu\n", readlink_result);
                 printf("link=" FMT_S_OS "\n", link_destination); */
              if (symlink(link_destination, to_name) != 0) {
                *err_info = FILE_ERROR;
              } /* if */
            } else {
              *err_info = FILE_ERROR;
            } /* if */
            os_stri_free(link_destination);
          } /* if */
        } /* if */
#else
        *err_info = FILE_ERROR;
#endif
      } else if (S_ISREG(from_stat.st_mode)) {
        copy_file(from_name, to_name, err_info);
      } else if (S_ISDIR(from_stat.st_mode)) {
        copy_dir(from_name, to_name, flags, err_info);
      } else if (S_ISFIFO(from_stat.st_mode)) {
#ifdef HAS_FIFO_FILES
        if (mkfifo(to_name, (S_IRWXU | S_IRWXG | S_IRWXO)) != 0) {
          *err_info = FILE_ERROR;
        } /* if */
#else
        *err_info = FILE_ERROR;
#endif
      } else {
        *err_info = FILE_ERROR;
      } /* if */
      if (*err_info == OKAY_NO_ERROR && !S_ISLNK(from_stat.st_mode)) {
        if (flags & PRESERVE_TIMESTAMPS) {
          to_utime.actime = from_stat.st_atime;
          to_utime.modtime = from_stat.st_mtime;
          /* printf("copy_any_file: st_atime=%ld\n", from_stat.st_atime); */
          /* printf("copy_any_file: st_mtime=%ld\n", from_stat.st_mtime); */
          os_utime(to_name, &to_utime);
        } /* if */
        if (flags & PRESERVE_MODE) {
          os_chmod(to_name, from_stat.st_mode);
        } /* if */
        if (flags & PRESERVE_OWNERSHIP) {
          os_chown(to_name, from_stat.st_uid, from_stat.st_gid);
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END copy_any_file(\"" FMT_S_OS "\", \"" FMT_S_OS "\", %d)\n",
        from_name, to_name, *err_info);
#endif
  } /* copy_any_file */



static void move_with_copy (const const_os_striType from_name,
    const const_os_striType to_name, errInfoType *err_info)

  {
    os_striType temp_name;

  /* move_with_copy */
    temp_name = temp_name_in_dir(from_name);
    if (unlikely(temp_name == NULL)) {
      *err_info = MEMORY_ERROR;
    } else {
      if (os_rename(from_name, temp_name) != 0) {
        logError(printf(" *** move_with_copy: os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        from_name, temp_name, errno, strerror(errno)););
        /* printf("EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d  ENOTDIR=%d  EROFS=%d\n",
           EACCES, EBUSY, EEXIST, ENOTEMPTY, ENOENT, ENOTDIR, EROFS); */
        *err_info = FILE_ERROR;
      } else {
        copy_any_file(temp_name, to_name, PRESERVE_ALL, err_info);
        if (*err_info == OKAY_NO_ERROR) {
          remove_any_file(temp_name, err_info);
        } else {
          /* Rename back to the original name. */
          if (os_rename(temp_name, from_name) != 0) {
            logError(printf(" *** move_with_copy: os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                            "errno=%d\nerror: %s\n",
                            temp_name, from_name, errno, strerror(errno)););
            /* printf("EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d  ENOTDIR=%d  EROFS=%d\n",
               EACCES, EBUSY, EEXIST, ENOTEMPTY, ENOENT, ENOTDIR, EROFS); */
          } /* if */
        } /* if */
      } /* if */
      os_stri_free(temp_name);
    } /* if */
  } /* move_with_copy */



#ifdef USE_EACCES_INSTEAD_OF_EXDEV
static boolType devices_differ (const const_os_striType from_name,
    const const_os_striType to_name)

  {
    os_stat_struct from_stat;
    os_stat_struct dir_stat;
    memSizeType to_name_length;
    memSizeType pos;
    memSizeType dir_name_length;
    os_striType dir_name;
    boolType differs = TRUE;

  /* devices_differ */
    if (os_stat(from_name, &from_stat) == 0) {
      to_name_length = os_stri_strlen(to_name);
      pos = to_name_length;
      while (pos > 0 && to_name[pos - 1] != '/' && to_name[pos - 1] != '\\') {
        pos--;
      } /* while */
      if (pos > 0) {
        dir_name_length = pos;
        if (likely(os_stri_alloc(dir_name, dir_name_length))) {
          memcpy(dir_name, to_name, dir_name_length * sizeof(os_charType));
          dir_name[dir_name_length] = '\0';
          /* printf("dir_name: " FMT_S_OS "\n", dir_name); */
          if (os_stat(dir_name, &dir_stat) == 0) {
            /* printf("from device: %ld, to device: %ld\n",
               from_stat.st_dev, dir_stat.st_dev); */
            differs = from_stat.st_dev != dir_stat.st_dev;
          } /* if */
          os_stri_free(dir_name);
        } /* if */
      } /* if */
    } /* if */
    /* printf("devices_differ --> %d\n", differs); */
    return differs;
  } /* devices_differ */
#endif



static void move_any_file (const const_os_striType from_name,
    const const_os_striType to_name, errInfoType *err_info)

  {
    os_stat_struct to_stat;

  /* move_any_file */
#ifdef TRACE_CMD_RTL
    printf("BEGIN move_any_file(\"" FMT_S_OS "\", \"" FMT_S_OS "\")\n", from_name, to_name);
#endif
    if (os_stat(to_name, &to_stat) == 0) {
      logError(printf(" *** move_any_file: Destination " FMT_S_OS
                      " exists already.\n", to_name););
      *err_info = FILE_ERROR;
    } else {
      if (os_rename(from_name, to_name) != 0) {
        switch (errno) {
          case EXDEV:
            /* printf("move_any_file: os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") triggers EXDEV\n",
               from_name, to_name); */
            move_with_copy(from_name, to_name, err_info);
            break;
#ifdef USE_EACCES_INSTEAD_OF_EXDEV
          case EACCES:
            /* printf("move_any_file: os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") triggers EACCES\n",
               from_name, to_name); */
            if (devices_differ(from_name, to_name)) {
              move_with_copy(from_name, to_name, err_info);
            } else {
              logError(printf(" *** move_any_file: os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                              "errno=%d\nerror: %s\n",
                              from_name, to_name, EACCES, strerror(EACCES)););
            } /* if */
            break;
#endif
          default:
            logError(printf(" *** move_any_file: os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                            "errno=%d\nerror: %s\n",
                            from_name, to_name, errno, strerror(errno)););
            /* printf("EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d  ENOTDIR=%d  EROFS=%d\n",
                EACCES, EBUSY, EEXIST, ENOTEMPTY, ENOENT, ENOTDIR, EROFS); */
            *err_info = FILE_ERROR;
            break;
        } /* switch */
      } /* if */
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END move_any_file(\"" FMT_S_OS "\", \"" FMT_S_OS "\", %d)\n",
        from_name, to_name, *err_info);
#endif
  } /* move_any_file */



static rtlArrayType add_stri_to_array (const striType stri,
    rtlArrayType work_array, intType *used_max_position, errInfoType *err_info)

  {
    rtlArrayType resized_work_array;

  /* add_stri_to_array */
    /* printf("add_stri_to_array\n"); */
    if (*used_max_position >= work_array->max_position) {
      if (work_array->max_position >= MAX_MEM_INDEX - ARRAY_SIZE_DELTA) {
        resized_work_array = NULL;
      } else {
        resized_work_array = REALLOC_RTL_ARRAY(work_array,
            (uintType) work_array->max_position,
            (uintType) work_array->max_position + ARRAY_SIZE_DELTA);
      } /* if */
      if (resized_work_array == NULL) {
        *err_info = MEMORY_ERROR;
      } else {
        work_array = resized_work_array;
        COUNT3_RTL_ARRAY((uintType) work_array->max_position,
            (uintType) work_array->max_position + ARRAY_SIZE_DELTA);
        work_array->max_position += ARRAY_SIZE_DELTA;
      } /* if */
    } /* if */
    if (*err_info == OKAY_NO_ERROR) {
      work_array->arr[*used_max_position].value.striValue = stri;
      (*used_max_position)++;
    } /* if */
    return work_array;
  } /* add_stri_to_array */



static rtlArrayType complete_stri_array (rtlArrayType work_array, intType used_max_position,
    errInfoType *err_info)

  {
    rtlArrayType resized_work_array;
    memSizeType position;

  /* complete_stri_array */
    if (*err_info == OKAY_NO_ERROR) {
      resized_work_array = REALLOC_RTL_ARRAY(work_array,
          (uintType) work_array->max_position, (uintType) used_max_position);
      if (resized_work_array == NULL) {
        *err_info = MEMORY_ERROR;
      } else {
        work_array = resized_work_array;
        COUNT3_RTL_ARRAY((uintType) work_array->max_position, (uintType) used_max_position);
        work_array->max_position = used_max_position;
      } /* if */
    } /* if */
    if (*err_info != OKAY_NO_ERROR) {
      for (position = 0; position < (memSizeType) used_max_position; position++) {
        FREE_STRI(work_array->arr[position].value.striValue,
            work_array->arr[position].value.striValue->size);
      } /* for */
      FREE_RTL_ARRAY(work_array, (uintType) work_array->max_position);
      work_array = NULL;
    } /* if */
    return work_array;
  } /* complete_stri_array */



static rtlArrayType read_dir (const const_striType dir_name, errInfoType *err_info)

  {
    intType used_max_position;
    dirType directory;
    striType nameStri;
    rtlArrayType dir_array;

  /* read_dir */
/*  printf("read_dir(");
    prot_stri(dir_name);
    printf(");\n");
    fflush(stdout); */
    if ((directory = dirOpen(dir_name)) != NULL) {
      if (ALLOC_RTL_ARRAY(dir_array, INITAL_ARRAY_SIZE)) {
        dir_array->min_position = 1;
        dir_array->max_position = INITAL_ARRAY_SIZE;
        used_max_position = 0;
        nameStri = dirRead(directory);
        while (*err_info == OKAY_NO_ERROR && nameStri != NULL) {
          dir_array = add_stri_to_array(nameStri, dir_array,
              &used_max_position, err_info);
          nameStri = dirRead(directory);
        } /* while */
        dir_array = complete_stri_array(dir_array, used_max_position, err_info);
      } else {
        *err_info = MEMORY_ERROR;
      } /* if */
      dirClose(directory);
    } else {
      dir_array = NULL;
      *err_info = FILE_ERROR;
    } /* if */
    return dir_array;
  } /* read_dir */



#ifdef os_putenv
static void setEnvironmentVariable (const const_striType name, const const_striType value,
    errInfoType *err_info)

  {
    memSizeType stri_size;
    striType stri;
    os_striType env_stri;
    int putenv_result;

  /* setEnvironmentVariable */
    if (strChPos(name, (charType) '=') != 0) {
      /* Putenv() expects a string of the form "name=value". */
      *err_info = RANGE_ERROR;
    } else if (unlikely(name->size > MAX_STRI_LEN - value->size - 1)) {
      /* Number of bytes does not fit into memSizeType. */
      *err_info = MEMORY_ERROR;
    } else {
      stri_size = name->size + value->size + 1;
      if (unlikely(!ALLOC_STRI_SIZE_OK(stri, stri_size))) {
        *err_info = MEMORY_ERROR;
      } else {
        stri->size = stri_size;
        memcpy(stri->mem, name->mem,
            name->size * sizeof(strElemType));
        stri->mem[name->size] = (strElemType) '=';
        memcpy(&stri->mem[name->size + 1], value->mem,
            value->size * sizeof(strElemType));
        env_stri = stri_to_os_stri(stri, err_info);
        FREE_STRI(stri, stri->size);
        if (likely(env_stri != NULL)) {
          putenv_result = os_putenv(env_stri);
          os_stri_free(env_stri);
          if (unlikely(putenv_result != 0)) {
            logError(printf(" *** setEnvironmentVariable: os_putenv(\"" FMT_S_OS "\") failed:\n"
                            "errno=%d\nerror: %s\n",
                            env_stri, errno, strerror(errno)););
            *err_info = RANGE_ERROR;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
  } /* setEnvironmentVariable */

#else



void setEnvironmentVariable (const const_striType name, const const_striType value,
    errInfoType *err_info)

  {
    os_striType env_name;
    os_striType env_value;
    int setenv_result;
    int saved_errno;

  /* setEnvironmentVariable */
    env_name = stri_to_os_stri(name, err_info);
    if (likely(*err_info == OKAY_NO_ERROR)) {
      env_value = stri_to_os_stri(value, err_info);
      if (likely(*err_info == OKAY_NO_ERROR)) {
        setenv_result = os_setenv(env_name, env_value, 1);
        saved_errno = errno;
        os_stri_free(env_value);
        if (unlikely(setenv_result != 0)) {
          logError(printf(" *** setEnvironmentVariable: os_setenv(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          env_name, env_value, saved_errno, strerror(saved_errno)););
          if (saved_errno == ENOMEM) {
            *err_info = MEMORY_ERROR;
          } else {
            *err_info = RANGE_ERROR;
          } /* if */
        } /* if */
      } /* if */
      os_stri_free(env_name);
    } /* if */
  } /* setEnvironmentVariable */

#endif



static rtlArrayType getSearchPath (errInfoType *err_info)

  {
    static const os_charType path_variable[] = {'P', 'A', 'T', 'H', 0};
    os_striType path_environment_variable;
    memSizeType path_length;
    os_striType path_copy;
    os_striType path_start;
    os_striType path_end;
    striType pathStri;
    intType used_max_position;
    rtlArrayType path_array;

  /* getSearchPath */
    /* printf("getSearchPath\n"); */
    if (ALLOC_RTL_ARRAY(path_array, INITAL_ARRAY_SIZE)) {
      path_array->min_position = 1;
      path_array->max_position = INITAL_ARRAY_SIZE;
      used_max_position = 0;
      path_environment_variable = os_getenv(path_variable);
      if (path_environment_variable != NULL) {
        path_length = os_stri_strlen(path_environment_variable);
        if (unlikely(!os_stri_alloc(path_copy, path_length))) {
          *err_info = MEMORY_ERROR;
        } else {
          os_stri_strcpy(path_copy, path_environment_variable);
          path_start = path_copy;
          do {
            path_end = os_stri_strchr(path_start, SEARCH_PATH_DELIMITER);
            if (path_end != NULL) {
              *path_end = '\0';
            } /* if */
            pathStri = cp_from_os_path(path_start, err_info);
            if (*err_info == OKAY_NO_ERROR) {
              while (pathStri->size > 1 && pathStri->mem[pathStri->size - 1] == (charType) '/') {
                pathStri->size--;
#ifdef WITH_STRI_CAPACITY
                COUNT3_STRI(pathStri->size + 1, pathStri->size);
#endif
              } /* while */
              path_array = add_stri_to_array(pathStri, path_array,
                  &used_max_position, err_info);
            } /* if */
            if (path_end == NULL) {
              path_start = NULL;
            } else {
              path_start = path_end + 1;
            } /* if */
          } while (path_start != NULL && *err_info == OKAY_NO_ERROR);
          os_stri_free(path_copy);
        } /* if */
        os_getenv_string_free(path_environment_variable);
        path_array = complete_stri_array(path_array, used_max_position, err_info);
      } /* if */
    } else {
      *err_info = MEMORY_ERROR;
    } /* if */
    return path_array;
  } /* getSearchPath */



void setSearchPath (rtlArrayType searchPath, errInfoType *err_info)

  {
    memSizeType numElements;
    memSizeType idx;
    memSizeType length = 0;
    memSizeType pos;
    striType pathElement;
    striType pathStri;
    striType pathVariableName;

  /* setSearchPath */
    numElements = arraySize(searchPath);
    if (numElements != 0) {
      for (idx = 0; idx < numElements; idx++) {
        length += searchPath->arr[idx].value.striValue->size;
      } /* for */
      length += numElements - 1;
    } /* if */
    if (!ALLOC_STRI_SIZE_OK(pathStri, length)) {
      *err_info = MEMORY_ERROR;
    } else {
      pathStri->size = length;
      if (numElements != 0) {
        pathElement = searchPath->arr[0].value.striValue;
        memcpy(pathStri->mem, pathElement->mem, pathElement->size * sizeof(strElemType));
        pos = pathElement->size;
        for (idx = 1; idx < numElements; idx++) {
          pathStri->mem[pos] = (charType) SEARCH_PATH_DELIMITER;
          pos++;
          pathElement = searchPath->arr[idx].value.striValue;
          memcpy(&pathStri->mem[pos], pathElement->mem, pathElement->size * sizeof(strElemType));
          pos += pathElement->size;
        } /* for */
      } /* if */
      pathVariableName = cstri_to_stri("PATH");
      if (pathVariableName == NULL) {
        *err_info = MEMORY_ERROR;
      } else {
        setEnvironmentVariable(pathVariableName, pathStri, err_info);
        FREE_STRI(pathVariableName, pathVariableName->size);
      } /* if */
      FREE_STRI(pathStri, pathStri->size);
    } /* if */
  } /* setSearchPath */



#ifdef HAS_SYMLINKS
striType followLink (striType path)

  {
    striType startPath;
    striType helpPath;
    int number_of_links_followed = 5;

  /* followLink */
    if (cmdFileTypeSL(path) == 7) {
      /* printf("symbolic link: ");
         prot_stri(path);
         printf("\n"); */
      startPath = path;
      path = cmdReadlink(startPath);
      while (cmdFileTypeSL(path) == 7 && number_of_links_followed != 0) {
        helpPath = path;
        path = cmdReadlink(helpPath);
        FREE_STRI(helpPath, helpPath->size);
        number_of_links_followed--;
      } /* if */
      if (number_of_links_followed == 0) {
        FREE_STRI(path, path->size);
        path = startPath;
      } else {
        FREE_STRI(startPath, startPath->size);
      } /* if */
    } /* if */
    return path;
  } /* followLink */
#endif



#ifdef EMULATE_ROOT_CWD
void initEmulatedCwd (errInfoType *err_info)

  {
    os_charType buffer[PATH_MAX + 1];
    os_striType cwd;

  /* initEmulatedCwd */
    if ((cwd = os_getcwd(buffer, PATH_MAX)) == NULL) {
      *err_info = FILE_ERROR;
    } else {
      setEmulatedCwd(cwd, err_info);
    } /* if */
  } /* initEmulatedCwd */
#endif



/**
 *  Determine the size of a file.
 *  The file size is measured in bytes.
 *  For directories a size of 0 is returned.
 *  @return the size of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR It was not possible to determine the file size.
 */
bigIntType cmdBigFileSize (const const_striType filePath)

  {
    os_striType os_path;
    os_stat_struct stat_buf;
    int stat_result;
    fileType aFile;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
    bigIntType result;

  /* cmdBigFileSize */
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (path_info == PATH_IS_EMULATED_ROOT) {
        result = bigIConv(0);
      } else {
        raise_error(err_info);
        result = NULL;
      } /* if */
#else
      raise_error(err_info);
      result = NULL;
#endif
    } else {
      stat_result = os_stat(os_path, &stat_buf);
      if (stat_result == 0 && S_ISREG(stat_buf.st_mode)) {
#if OS_OFF_T_SIZE == 32
        result = bigFromUInt32((uint32Type) stat_buf.st_size);
#elif OS_OFF_T_SIZE == 64
        result = bigFromUInt64((uint64Type) stat_buf.st_size);
#else
#error "sizeof(os_off_t) is neither 4 nor 8."
#endif
      } else if (stat_result == 0 && S_ISDIR(stat_buf.st_mode)) {
        result = bigIConv(0);
      } else {
        aFile = os_fopen(os_path, os_mode_rb);
        if (aFile == NULL) {
          /* if (stat_result == 0) {
            printf("stat_buf.st_blksize=%lu\n", stat_buf.st_blksize);
            printf("stat_buf.st_blocks=%llu\n", stat_buf.st_blocks);
            printf("stat_buf.st_size=%llu\n", stat_buf.st_size);
          } */
          err_info = FILE_ERROR;
          result = NULL;
        } else {
          result = getBigFileLengthUsingSeek(aFile);
          fclose(aFile);
        } /* if */
      } /* if */
      os_stri_free(os_path);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
    return result;
  } /* cmdBigFileSize */



/**
 *  Changes the current working directory of the calling process.
 *  @exception MEMORY_ERROR Not enough memory to convert 'dirPath' to
 *             the system path type.
 *  @exception RANGE_ERROR 'dirPath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR A system function returns an error.
 */
void cmdChdir (const const_striType dirPath)

  {
    os_striType os_path;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
    int chdir_result;

  /* cmdChdir */
    os_path = cp_to_os_path(dirPath, &path_info, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (path_info == PATH_IS_EMULATED_ROOT) {
        setEmulatedCwd(emulated_root, &err_info);
      } else {
        raise_error(err_info);
      } /* if */
    } else {
      chdir_result = os_chdir(os_path);
      if (unlikely(chdir_result != 0)) {
        err_info = FILE_ERROR;
      } else {
        if (dirPath->size >= 1 && dirPath->mem[0] == '/') {
          /* Absolute path */
          setEmulatedCwd(os_path, &err_info);
        } else {
          /* Relative path */
          initEmulatedCwd(&err_info);
        } /* if */
      } /* if */
#else
      raise_error(err_info);
    } else {
      chdir_result = os_chdir(os_path);
      if (unlikely(chdir_result != 0)) {
        err_info = FILE_ERROR;
      } /* if */
#endif
      os_stri_free(os_path);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
  } /* cmdChdir */



/**
 *  Clone a file or directory tree.
 *  Permissions/mode, ownership and timestamps of the original are
 *  preserved. Symlinks are not followed. Instead the symlink is
 *  copied. Note that 'cloneFile' does not preserve hard links (they
 *  are resolved to distinct files).
 *  @exception MEMORY_ERROR Not enough memory to convert
 *             'sourcePath' or 'destPath' to the system path type.
 *  @exception RANGE_ERROR 'sourcePath' or 'destPath' does not use
 *             the standard path representation or one of them cannot be
 *             converted to the system path type.
 *  @exception FILE_ERROR Source file does not exist, destination file
 *             already exists or a system function returns an error.
 */
void cmdCloneFile (const const_striType sourcePath, const const_striType destPath)

  {
    os_striType os_sourcePath;
    os_striType os_destPath;
    os_stat_struct to_stat;
    int path_info;
    errInfoType err_info = OKAY_NO_ERROR;

  /* cmdCloneFile */
#ifdef TRACE_CMD_RTL
    printf("cmdCloneFile(");
    prot_stri(sourcePath);
    printf(", ");
    prot_stri(destPath);
    printf(")\n");
#endif
    os_sourcePath = cp_to_os_path(sourcePath, &path_info, &err_info);
    if (likely(err_info == OKAY_NO_ERROR)) {
      os_destPath = cp_to_os_path(destPath, &path_info, &err_info);
      if (likely(err_info == OKAY_NO_ERROR)) {
        if (os_stat(os_destPath, &to_stat) == 0) {
          logError(printf(" *** cmdCloneFile: Destination " FMT_S_OS
                          " exists already.\n", os_destPath););
          err_info = FILE_ERROR;
        } else {
          copy_any_file(os_sourcePath, os_destPath, PRESERVE_ALL, &err_info);
        } /* if */
        os_stri_free(os_destPath);
      } /* if */
      os_stri_free(os_sourcePath);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* cmdCloneFile */



striType cmdConfigValue (const const_striType name)

  {
    char opt_name[MAX_CSTRI_BUFFER_LEN + 1];
    const_cstriType opt;
    char buffer[4];
    errInfoType err_info = OKAY_NO_ERROR;
    striType result;

  /* cmdConfigValue */
    if (name->size > MAX_CSTRI_BUFFER_LEN) {
      opt = "";
    } else {
      conv_to_cstri(opt_name, name, &err_info);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
        return NULL;
      } else if (strcmp(opt_name, "OBJECT_FILE_EXTENSION") == 0) {
        opt = OBJECT_FILE_EXTENSION;
      } else if (strcmp(opt_name, "LIBRARY_FILE_EXTENSION") == 0) {
        opt = LIBRARY_FILE_EXTENSION;
      } else if (strcmp(opt_name, "EXECUTABLE_FILE_EXTENSION") == 0) {
        opt = EXECUTABLE_FILE_EXTENSION;
      } else if (strcmp(opt_name, "C_COMPILER") == 0) {
#ifdef PATHS_RELATIVE_TO_EXECUTABLE
        result = relativeToProgramPath(programPath, "bin/call_gcc");
        opt = NULL;
#else
        opt = C_COMPILER;
#endif
      } else if (strcmp(opt_name, "CPLUSPLUS_COMPILER") == 0) {
#ifdef PATHS_RELATIVE_TO_EXECUTABLE
        result = relativeToProgramPath(programPath, "bin/call_gcc");
        opt = NULL;
#else
        opt = CPLUSPLUS_COMPILER;
#endif
      } else if (strcmp(opt_name, "C_COMPILER_VERSION") == 0) {
        opt = C_COMPILER_VERSION;
      } else if (strcmp(opt_name, "GET_CC_VERSION_INFO") == 0) {
        opt = GET_CC_VERSION_INFO;
      } else if (strcmp(opt_name, "CC_OPT_DEBUG_INFO") == 0) {
        opt = CC_OPT_DEBUG_INFO;
      } else if (strcmp(opt_name, "CC_OPT_NO_WARNINGS") == 0) {
        opt = CC_OPT_NO_WARNINGS;
      } else if (strcmp(opt_name, "CC_FLAGS") == 0) {
        opt = CC_FLAGS;
      } else if (strcmp(opt_name, "REDIRECT_C_ERRORS") == 0) {
        opt = REDIRECT_C_ERRORS;
      } else if (strcmp(opt_name, "LINKER_OPT_DEBUG_INFO") == 0) {
        opt = LINKER_OPT_DEBUG_INFO;
      } else if (strcmp(opt_name, "LINKER_OPT_NO_DEBUG_INFO") == 0) {
        opt = LINKER_OPT_NO_DEBUG_INFO;
      } else if (strcmp(opt_name, "LINKER_OPT_OUTPUT_FILE") == 0) {
        opt = LINKER_OPT_OUTPUT_FILE;
      } else if (strcmp(opt_name, "LINKER_FLAGS") == 0) {
        opt = LINKER_FLAGS;
      } else if (strcmp(opt_name, "SYSTEM_LIBS") == 0) {
        opt = SYSTEM_LIBS;
      } else if (strcmp(opt_name, "SYSTEM_CONSOLE_LIBS") == 0) {
        opt = SYSTEM_CONSOLE_LIBS;
      } else if (strcmp(opt_name, "SYSTEM_DRAW_LIBS") == 0) {
        opt = SYSTEM_DRAW_LIBS;
      } else if (strcmp(opt_name, "SYSTEM_DB_LIBS") == 0) {
        opt = SYSTEM_DB_LIBS;
      } else if (strcmp(opt_name, "SEED7_LIB") == 0) {
        opt = SEED7_LIB;
      } else if (strcmp(opt_name, "CONSOLE_LIB") == 0) {
        opt = CONSOLE_LIB;
      } else if (strcmp(opt_name, "DRAW_LIB") == 0) {
        opt = DRAW_LIB;
      } else if (strcmp(opt_name, "COMP_DATA_LIB") == 0) {
        opt = COMP_DATA_LIB;
      } else if (strcmp(opt_name, "COMPILER_LIB") == 0) {
        opt = COMPILER_LIB;
      } else if (strcmp(opt_name, "S7_LIB_DIR") == 0) {
#ifdef PATHS_RELATIVE_TO_EXECUTABLE
        result = relativeToProgramPath(programPath, "bin");
        opt = NULL;
#else
        opt = S7_LIB_DIR;
#endif
      } else if (strcmp(opt_name, "INT32TYPE") == 0) {
        opt = INT32TYPE_STRI;
      } else if (strcmp(opt_name, "UINT32TYPE") == 0) {
        opt = UINT32TYPE_STRI;
      } else if (strcmp(opt_name, "INT64TYPE") == 0) {
        opt = INT64TYPE_STRI;
      } else if (strcmp(opt_name, "UINT64TYPE") == 0) {
        opt = UINT64TYPE_STRI;
      } else if (strcmp(opt_name, "INT128TYPE") == 0) {
        opt = INT128TYPE_STRI;
      } else if (strcmp(opt_name, "UINT128TYPE") == 0) {
        opt = UINT128TYPE_STRI;
      } else if (strcmp(opt_name, "INT32TYPE_LITERAL_SUFFIX") == 0) {
        opt = INT32TYPE_LITERAL_SUFFIX;
      } else if (strcmp(opt_name, "INT64TYPE_LITERAL_SUFFIX") == 0) {
        opt = INT64TYPE_LITERAL_SUFFIX;
      } else if (strcmp(opt_name, "INTTYPE_SIZE") == 0) {
        sprintf(buffer, "%d", INTTYPE_SIZE);
        opt = buffer;
      } else if (strcmp(opt_name, "FLOATTYPE_SIZE") == 0) {
        sprintf(buffer, "%d", FLOATTYPE_SIZE);
        opt = buffer;
      } else if (strcmp(opt_name, "POINTER_SIZE") == 0) {
        sprintf(buffer, "%d", POINTER_SIZE);
        opt = buffer;
      } else if (strcmp(opt_name, "INT_SIZE") == 0) {
        sprintf(buffer, "%d", INT_SIZE);
        opt = buffer;
      } else if (strcmp(opt_name, "MACRO_DEFS") == 0) {
        opt = MACRO_DEFS;
      } else if (strcmp(opt_name, "OVERFLOW_SIGNAL") == 0) {
        opt = OVERFLOW_SIGNAL;
      } else if (strcmp(opt_name, "FLOATTYPE_DOUBLE") == 0) {
#ifdef FLOATTYPE_DOUBLE
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else if (strcmp(opt_name, "HAS_SIGSETJMP") == 0) {
#ifdef HAS_SIGSETJMP
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else if (strcmp(opt_name, "ISNAN_WITH_UNDERLINE") == 0) {
#ifdef ISNAN_WITH_UNDERLINE
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else if (strcmp(opt_name, "DO_SIGFPE_WITH_DIV_BY_ZERO") == 0) {
#ifdef DO_SIGFPE_WITH_DIV_BY_ZERO
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else if (strcmp(opt_name, "CHECK_INT_DIV_BY_ZERO") == 0) {
#ifdef CHECK_INT_DIV_BY_ZERO
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else if (strcmp(opt_name, "CHECK_INT_REM_BY_ZERO") == 0) {
#ifdef CHECK_INT_REM_BY_ZERO
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else if (strcmp(opt_name, "CHECK_INT_REM_ZERO_BY_ZERO") == 0) {
#ifdef CHECK_INT_REM_ZERO_BY_ZERO
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else if (strcmp(opt_name, "CHECK_FLOAT_DIV_BY_ZERO") == 0) {
#ifdef CHECK_FLOAT_DIV_BY_ZERO
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else if (strcmp(opt_name, "WITH_STRI_CAPACITY") == 0) {
#ifdef WITH_STRI_CAPACITY
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else if (strcmp(opt_name, "ALLOW_STRITYPE_SLICES") == 0) {
#ifdef ALLOW_STRITYPE_SLICES
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else if (strcmp(opt_name, "ALLOW_BSTRITYPE_SLICES") == 0) {
#ifdef ALLOW_BSTRITYPE_SLICES
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else if (strcmp(opt_name, "RSHIFT_DOES_SIGN_EXTEND") == 0) {
#ifdef RSHIFT_DOES_SIGN_EXTEND
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else if (strcmp(opt_name, "TWOS_COMPLEMENT_INTTYPE") == 0) {
#ifdef TWOS_COMPLEMENT_INTTYPE
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else if (strcmp(opt_name, "LITTLE_ENDIAN_INTTYPE") == 0) {
#ifdef LITTLE_ENDIAN_INTTYPE
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else if (strcmp(opt_name, "NAN_COMPARISON_WRONG") == 0) {
#ifdef NAN_COMPARISON_WRONG
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else if (strcmp(opt_name, "POWER_OF_ZERO_WRONG") == 0) {
#ifdef POWER_OF_ZERO_WRONG
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else if (strcmp(opt_name, "FLOAT_ZERO_DIV_ERROR") == 0) {
#ifdef FLOAT_ZERO_DIV_ERROR
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else if (strcmp(opt_name, "LIMITED_CSTRI_LITERAL_LEN") == 0) {
#ifdef LIMITED_CSTRI_LITERAL_LEN
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else if (strcmp(opt_name, "CC_SOURCE_UTF8") == 0) {
#ifdef CC_SOURCE_UTF8
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else if (strcmp(opt_name, "USE_WMAIN") == 0) {
#ifdef USE_WMAIN
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else if (strcmp(opt_name, "USE_WINMAIN") == 0) {
#ifdef USE_WINMAIN
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else {
        opt = "";
      } /* if */
    } /* if */
    if (opt != NULL) {
      result = cstri8_or_cstri_to_stri(opt);
    } /* if */
    if (unlikely(result == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return result;
  } /* cmdConfigValue */



/**
 *  Copy a file or directory tree.
 *  Permissions/mode, ownership and timestamps of the destination file
 *  are determined independent of the corresponding source properties.
 *  The destination file gets the permissions/mode defined by umask.
 *  The user executing the program is the owner of the destination file.
 *  The timestamps of the destination file are set to the current time.
 *  Symbolic links in 'sourcePath' are always followed.
 *  Therefore 'copyFile' will never create a symbolic link.
 *  Note that 'copyFile' does not preserve hard links (they are
 *  resolved to distinct files).
 *  @exception MEMORY_ERROR Not enough memory to convert 'sourcePath'
 *             or 'destPath' to the system path type.
 *  @exception RANGE_ERROR 'sourcePath' or 'destPath' does not use
 *             the standard path representation or one of them cannot be
 *             converted to the system path type.
 *  @exception FILE_ERROR Source file does not exist, destination file
 *             already exists or a system function returns an error.
 */
void cmdCopyFile (const const_striType sourcePath, const const_striType destPath)

  {
    os_striType os_sourcePath;
    os_striType os_destPath;
    os_stat_struct to_stat;
    int path_info;
    errInfoType err_info = OKAY_NO_ERROR;

  /* cmdCopyFile */
#ifdef TRACE_CMD_RTL
    printf("cmdCopyFile(");
    prot_stri(sourcePath);
    printf(", ");
    prot_stri(destPath);
    printf(")\n");
#endif
    os_sourcePath = cp_to_os_path(sourcePath, &path_info, &err_info);
    if (likely(err_info == OKAY_NO_ERROR)) {
      os_destPath = cp_to_os_path(destPath, &path_info, &err_info);
      if (likely(err_info == OKAY_NO_ERROR)) {
        if (os_stat(os_destPath, &to_stat) == 0) {
          logError(printf(" *** cmdCopyFile: Destination " FMT_S_OS
                          " exists already.\n", os_destPath););
          err_info = FILE_ERROR;
        } else {
          copy_any_file(os_sourcePath, os_destPath, PRESERVE_NOTHING, &err_info);
        } /* if */
        os_stri_free(os_destPath);
      } /* if */
      os_stri_free(os_sourcePath);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* cmdCopyFile */



/**
 *  Returns the list of environment variable names as array of strings.
 *  @return the list of environment variable names.
 *  @exception MEMORY_ERROR Not enough memory to create the result.
 */
rtlArrayType cmdEnvironment (void)

  {
#ifdef INITIALIZE_OS_ENVIRON
    static const os_charType empty_os_stri[] = {0};
#endif
    intType used_max_position;
    os_striType *nameStartPos;
    os_striType nameEndPos;
    striType variableName;
    errInfoType err_info = OKAY_NO_ERROR;
    rtlArrayType environment_array;

  /* cmdEnvironment */
#ifdef INITIALIZE_OS_ENVIRON
    if (os_environ == NULL) {
      (void) os_getenv(empty_os_stri);
    } /* if */
#endif
    if (ALLOC_RTL_ARRAY(environment_array, INITAL_ARRAY_SIZE)) {
      environment_array->min_position = 1;
      environment_array->max_position = INITAL_ARRAY_SIZE;
      used_max_position = 0;
      for (nameStartPos = os_environ; *nameStartPos != NULL; ++nameStartPos) {
        if ((*nameStartPos)[0] != '=' && (*nameStartPos)[0] != '\0') {
          nameEndPos = os_stri_strchr(*nameStartPos, '=');
          if (nameEndPos != NULL) {
            variableName = conv_from_os_stri(*nameStartPos, (memSizeType) (nameEndPos - *nameStartPos));
            if (unlikely(variableName == NULL)) {
              err_info = MEMORY_ERROR;
            } /* if */
          } else {
            variableName = os_stri_to_stri(*nameStartPos, &err_info);
          } /* if */
          if (err_info == OKAY_NO_ERROR) {
            environment_array = add_stri_to_array(variableName, environment_array,
                &used_max_position, &err_info);
          } /* if */
        } /* if */
      } /* for */
      environment_array = complete_stri_array(environment_array, used_max_position, &err_info);
      if (err_info != OKAY_NO_ERROR) {
        raise_error(err_info);
        environment_array = NULL;
      } /* if */
    } else {
      raise_error(MEMORY_ERROR);
    } /* if */
    return environment_array;
  } /* cmdEnvironment */



/**
 *  Determine the file mode (permissions) of a file.
 *  @return the file mode.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR A system function returns an error.
 */
setType cmdFileMode (const const_striType filePath)

  {
    os_striType os_path;
    os_stat_struct stat_buf;
    int stat_result;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
    setType result;

  /* cmdFileMode */
#ifdef TRACE_CMD_RTL
    printf("BEGIN cmdFileMode(");
    prot_stri(filePath);
    printf(")\n");
#endif
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (path_info == PATH_IS_EMULATED_ROOT) {
        result = setIConv(0444);
      } else {
        raise_error(err_info);
        result = NULL;
      } /* if */
#else
      raise_error(err_info);
      result = NULL;
#endif
    } else {
      stat_result = os_stat(os_path, &stat_buf);
      os_stri_free(os_path);
      if (unlikely(stat_result != 0)) {
        logError(printf(" *** cmdFileMode: os_stat(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_path, errno, strerror(errno)););
        /* printf("EACCES=%d  EBADF=%d  EFAULT=%d  ELOOP=%d  ENAMETOOLONG=%d  ENOENT=%d\n",
            EACCES, EBADF, EFAULT, ELOOP, ENAMETOOLONG, ENOENT);
        printf("ENOMEM=%d  EOVERFLOW=%d\n",
            ENOMEM, EOVERFLOW); */
        raise_error(FILE_ERROR);
        result = NULL;
      } else {
        /* printf("cmdFileMode: st_mode=0%o\n", stat_buf.st_mode); */
#if MODE_BITS_NORMAL
        result = setIConv(0777 & stat_buf.st_mode);
#else
        /* Force the bits to the standard sequence */
        result = setIConv(        {
            (stat_buf.st_mode & S_IRUSR ? 0400 : 0) |
            (stat_buf.st_mode & S_IWUSR ? 0200 : 0) |
            (stat_buf.st_mode & S_IXUSR ? 0100 : 0) |
            (stat_buf.st_mode & S_IRGRP ? 0040 : 0) |
            (stat_buf.st_mode & S_IWGRP ? 0020 : 0) |
            (stat_buf.st_mode & S_IXGRP ? 0010 : 0) |
            (stat_buf.st_mode & S_IROTH ? 0004 : 0) |
            (stat_buf.st_mode & S_IWOTH ? 0002 : 0) |
            (stat_buf.st_mode & S_IXOTH ? 0001 : 0));
#endif
      } /* if */
    } /* if */
    return result;
  } /* cmdFileMode */



/**
 *  Determine the size of a file.
 *  The file size is measured in bytes.
 *  For directories a size of 0 is returned.
 *  @return the size of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception RANGE_ERROR The file size is not representable as integer.
 *  @exception FILE_ERROR It was not possible to determine the file size.
 */
intType cmdFileSize (const const_striType filePath)

  {
    os_striType os_path;
    os_stat_struct stat_buf;
    int stat_result;
    fileType aFile;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
    intType result;

  /* cmdFileSize */
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (path_info != PATH_IS_EMULATED_ROOT) {
        raise_error(err_info);
      } /* if */
#else
      raise_error(err_info);
#endif
      result = 0;
    } else {
      stat_result = os_stat(os_path, &stat_buf);
      if (stat_result == 0 && S_ISREG(stat_buf.st_mode)) {
        if (stat_buf.st_size > INTTYPE_MAX || stat_buf.st_size < 0) {
          err_info = RANGE_ERROR;
          result = 0;
        } else {
          result = (intType) stat_buf.st_size;
        } /* if */
      } else if (stat_result == 0 && S_ISDIR(stat_buf.st_mode)) {
        result = 0;
      } else {
        aFile = os_fopen(os_path, os_mode_rb);
        if (aFile == NULL) {
          /* if (stat_result == 0) {
            printf("stat_buf.st_blksize=%lu\n", stat_buf.st_blksize);
            printf("stat_buf.st_blocks=%llu\n", stat_buf.st_blocks);
            printf("stat_buf.st_size=%llu\n", stat_buf.st_size);
          } */
          err_info = FILE_ERROR;
          result = 0;
        } else {
          result = getFileLengthUsingSeek(aFile);
          fclose(aFile);
        } /* if */
      } /* if */
      os_stri_free(os_path);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
    return result;
  } /* cmdFileSize */



/**
 *  Determine the type of a file.
 *  The function does follow symbolic links. Therefore it never
 *  returns 'FILE_SYMLINK'. A return value of 'FILE_ABSENT' does
 *  not imply that a file with this name can be created, since missing
 *  directories and illegal file names cause also 'FILE_ABSENT'.
 *  @return the type of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation.
 *  @exception FILE_ERROR The system function returns an error other
 *             than ENOENT, ENOTDIR or ENAMETOOLONG.
 */
intType cmdFileType (const const_striType filePath)

  {
    os_striType os_path;
    os_stat_struct stat_buf;
    int stat_result;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
    int saved_errno;
    intType result;

  /* cmdFileType */
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (path_info == PATH_IS_EMULATED_ROOT) {
        result = FILE_DIR;
      } else if (path_info == PATH_NOT_MAPPED) {
        result = FILE_ABSENT;
      } else {
        raise_error(err_info);
        result = FILE_ABSENT;
      } /* if */
#else
      raise_error(err_info);
      result = FILE_ABSENT;
#endif
    } else {
      stat_result = os_stat(os_path, &stat_buf);
      saved_errno = errno;
      os_stri_free(os_path);
      if (stat_result == 0) {
        if (S_ISREG(stat_buf.st_mode)) {
          result = FILE_REGULAR;
        } else if (S_ISDIR(stat_buf.st_mode)) {
          result = FILE_DIR;
        } else if (S_ISCHR(stat_buf.st_mode)) {
          result = FILE_CHAR;
        } else if (S_ISBLK(stat_buf.st_mode)) {
          result = FILE_BLOCK;
        } else if (S_ISFIFO(stat_buf.st_mode)) {
          result = FILE_FIFO;
        } else if (S_ISLNK(stat_buf.st_mode)) {
          result = FILE_SYMLINK;
          raise_error(FILE_ERROR);
        } else if (S_ISSOCK(stat_buf.st_mode)) {
          result = FILE_SOCKET;
        } else {
          result = FILE_UNKNOWN;
        } /* if */
      } else {
        result = FILE_ABSENT;
        if (unlikely(filePath->size != 0 && saved_errno != ENOENT &&
            saved_errno != ENOTDIR && saved_errno != ENAMETOOLONG)) {
          logError(printf(" *** cmdFileType: os_stat(\"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_path, saved_errno, strerror(saved_errno)););
          /* printf("EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d\n",
              EACCES, EBUSY, EEXIST, ENOTEMPTY, ENOENT);
          printf("ENOTDIR=%d  EROFS=%d  EIO=%d  ELOOP=%d  ENAMETOOLONG=%d\n",
              ENOTDIR, EROFS, EIO, ELOOP, ENAMETOOLONG);
          printf("EOVERFLOW=%d  EBADF=%d  EFAULT=%d  ENOMEM=%d\n",
              EOVERFLOW, EBADF, EFAULT, ENOMEM); */
          /* printf("filePath->size=%lu\n", filePath->size); */
          /* printf("strlen(os_path)=%d\n", os_stri_strlen(os_path)); */
          raise_error(FILE_ERROR);
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* cmdFileType */



/**
 *  Determine the type of a file.
 *  The function does not follow symbolic links. Therefore it may
 *  return 'FILE_SYMLINK'. A return value of 'FILE_ABSENT' does
 *  not imply that a file with this name can be created, since missing
 *  directories and illegal file names cause also 'FILE_ABSENT'.
 *  @return the type of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation.
 *  @exception FILE_ERROR The system function returns an error other
 *             than ENOENT, ENOTDIR or ENAMETOOLONG.
 */
intType cmdFileTypeSL (const const_striType filePath)

  {
    os_striType os_path;
    os_stat_struct stat_buf;
    int stat_result;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
    int saved_errno;
    intType result;

  /* cmdFileTypeSL */
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (path_info == PATH_IS_EMULATED_ROOT) {
        result = FILE_DIR;
      } else if (path_info == PATH_NOT_MAPPED) {
        result = FILE_ABSENT;
      } else {
        raise_error(err_info);
        result = FILE_ABSENT;
      } /* if */
#else
      raise_error(err_info);
      result = FILE_ABSENT;
#endif
    } else {
      stat_result = os_lstat(os_path, &stat_buf);
      saved_errno = errno;
      os_stri_free(os_path);
      if (stat_result == 0) {
        if (S_ISREG(stat_buf.st_mode)) {
          result = FILE_REGULAR;
        } else if (S_ISDIR(stat_buf.st_mode)) {
          result = FILE_DIR;
        } else if (S_ISCHR(stat_buf.st_mode)) {
          result = FILE_CHAR;
        } else if (S_ISBLK(stat_buf.st_mode)) {
          result = FILE_BLOCK;
        } else if (S_ISFIFO(stat_buf.st_mode)) {
          result = FILE_FIFO;
        } else if (S_ISLNK(stat_buf.st_mode)) {
          result = FILE_SYMLINK;
        } else if (S_ISSOCK(stat_buf.st_mode)) {
          result = FILE_SOCKET;
        } else {
          result = FILE_UNKNOWN;
        } /* if */
      } else {
        result = FILE_ABSENT;
        if (unlikely(filePath->size != 0 && saved_errno != ENOENT &&
            saved_errno != ENOTDIR && saved_errno != ENAMETOOLONG)) {
          logError(printf(" *** cmdFileTypeSL: os_stat(\"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_path, saved_errno, strerror(saved_errno)););
          /* printf("EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d\n",
              EACCES, EBUSY, EEXIST, ENOTEMPTY, ENOENT);
          printf("ENOTDIR=%d  EROFS=%d  EIO=%d  ELOOP=%d  ENAMETOOLONG=%d\n",
              ENOTDIR, EROFS, EIO, ELOOP, ENAMETOOLONG);
          printf("EOVERFLOW=%d  EBADF=%d  EFAULT=%d  ENOMEM=%d\n",
              EOVERFLOW, EBADF, EFAULT, ENOMEM); */
          /* printf("filePath->size=%lu\n", filePath->size); */
          /* printf("strlen(os_path)=%d\n", os_stri_strlen(os_path)); */
          raise_error(FILE_ERROR);
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* cmdFileTypeSL */



/**
 *  Determine the current working directory of the calling process.
 *  @return The absolute path of the current working directory.
 *  @exception MEMORY_ERROR Not enough memory to represent the
 *             result string.
 *  @exception FILE_ERROR The system function returns an error.
 */
striType cmdGetcwd (void)

  {
    os_charType buffer[PATH_MAX + 1];
    os_striType cwd;
    errInfoType err_info = OKAY_NO_ERROR;
    striType result;

  /* cmdGetcwd */
#ifdef EMULATE_ROOT_CWD
    if (IS_EMULATED_ROOT(current_emulated_cwd)) {
      result = cp_from_os_path(current_emulated_cwd, &err_info);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } else {
#endif
      if (unlikely((cwd = os_getcwd(buffer, PATH_MAX)) == NULL)) {
        raise_error(FILE_ERROR);
        result = NULL;
      } else {
        result = cp_from_os_path(cwd, &err_info);
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
        } /* if */
      } /* if */
#ifdef EMULATE_ROOT_CWD
    } /* if */
#endif
    return result;
  } /* cmdGetcwd */



/**
 *  Determine the value of an environment variable.
 *  The function getenv searches the environment for an environment variable
 *  with the given 'name'. When such an environment variable exists the
 *  corresponding string value is returned.
 *  @return the value of an environment variable or "",
 *          when the requested environment variable does not exist.
 *  @exception MEMORY_ERROR Not enough memory to convert 'name' to the
 *             system string type or not enough memory to represent the
 *             result string.
 */
striType cmdGetenv (const const_striType name)

  {
    os_striType env_name;
    os_striType env_value;
    errInfoType err_info = OKAY_NO_ERROR;
    striType result;

  /* cmdGetenv */
    env_name = stri_to_os_stri(name, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
      result = NULL;
    } else {
      env_value = os_getenv(env_name);
      os_stri_free(env_name);
      if (env_value == NULL) {
        result = cstri_to_stri("");
        if (unlikely(result == NULL)) {
          err_info = MEMORY_ERROR;
        } /* if */
      } else {
        result = os_stri_to_stri(env_value, &err_info);
        os_getenv_string_free(env_value);
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
    return result;
  } /* cmdGetenv */



/**
 *  Determine the access time of a file.
 *  @return the access time of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR A system function returns an error.
 */
void cmdGetATime (const const_striType filePath,
    intType *year, intType *month, intType *day, intType *hour,
    intType *min, intType *sec, intType *micro_sec, intType *time_zone,
    boolType *is_dst)

  {
    os_striType os_path;
    os_stat_struct stat_buf;
    int stat_result;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;

  /* cmdGetATime */
#ifdef TRACE_CMD_RTL
    printf("BEGIN cmdGetATime(");
    prot_stri(filePath);
    printf(")\n");
#endif
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (path_info == PATH_IS_EMULATED_ROOT) {
        timFromTimestamp(0,
            year, month, day, hour,
            min, sec, micro_sec, time_zone, is_dst);
      } else {
        raise_error(err_info);
      } /*if */
#else
      raise_error(err_info);
#endif
    } else {
      stat_result = os_stat(os_path, &stat_buf);
      os_stri_free(os_path);
      if (unlikely(stat_result != 0)) {
        logError(printf(" *** cmdGetATime: os_stat(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_path, errno, strerror(errno)););
        raise_error(FILE_ERROR);
      } else {
        /* printf("cmdGetATime: st_atime=%ld\n", stat_buf.st_atime); */
        timFromTimestamp(stat_buf.st_atime,
            year, month, day, hour,
            min, sec, micro_sec, time_zone, is_dst);
      } /* if */
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END cmdGetATime(" F_D(04) "-" F_D(02) "-" F_D(02) " "
           F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) " " FMT_D " %d)\n",
           *year, *month, *day, *hour, *min, *sec,
           *micro_sec, *time_zone, *is_dst);
#endif
  } /* cmdGetATime */



/**
 *  Determine the change time of a file.
 *  @return the change time of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR A system function returns an error.
 */
void cmdGetCTime (const const_striType filePath,
    intType *year, intType *month, intType *day, intType *hour,
    intType *min, intType *sec, intType *micro_sec, intType *time_zone,
    boolType *is_dst)

  {
    os_striType os_path;
    os_stat_struct stat_buf;
    int stat_result;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;

  /* cmdGetCTime */
#ifdef TRACE_CMD_RTL
    printf("BEGIN cmdGetCTime(");
    prot_stri(filePath);
    printf(")\n");
#endif
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (path_info == PATH_IS_EMULATED_ROOT) {
        timFromTimestamp(0,
            year, month, day, hour,
            min, sec, micro_sec, time_zone, is_dst);
      } else {
        raise_error(err_info);
      } /*if */
#else
      raise_error(err_info);
#endif
    } else {
      stat_result = os_stat(os_path, &stat_buf);
      os_stri_free(os_path);
      if (unlikely(stat_result != 0)) {
        logError(printf(" *** cmdGetCTime: os_stat(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_path, errno, strerror(errno)););
        raise_error(FILE_ERROR);
      } else {
        /* printf("cmdGetCTime: st_ctime=%ld\n", stat_buf.st_ctime); */
        timFromTimestamp(stat_buf.st_ctime,
            year, month, day, hour,
            min, sec, micro_sec, time_zone, is_dst);
      } /* if */
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END cmdGetCTime(" F_D(04) "-" F_D(02) "-" F_D(02) " "
           F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) " " FMT_D " %d)\n",
           *year, *month, *day, *hour, *min, *sec,
           *micro_sec, *time_zone, *is_dst);
#endif
  } /* cmdGetCTime */



/**
 *  Determine the modification time of a file.
 *  @return the modification time of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR A system function returns an error.
 */
void cmdGetMTime (const const_striType filePath,
    intType *year, intType *month, intType *day, intType *hour,
    intType *min, intType *sec, intType *micro_sec, intType *time_zone,
    boolType *is_dst)

  {
    os_striType os_path;
    os_stat_struct stat_buf;
    int stat_result;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;

  /* cmdGetMTime */
#ifdef TRACE_CMD_RTL
    printf("BEGIN cmdGetMTime(");
    prot_stri(filePath);
    printf(")\n");
#endif
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (path_info == PATH_IS_EMULATED_ROOT) {
        timFromTimestamp(0,
            year, month, day, hour,
            min, sec, micro_sec, time_zone, is_dst);
      } else {
        raise_error(err_info);
      } /*if */
#else
      raise_error(err_info);
#endif
    } else {
      stat_result = os_stat(os_path, &stat_buf);
      os_stri_free(os_path);
      if (unlikely(stat_result != 0)) {
        logError(printf(" *** cmdGetMTime: os_stat(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_path, errno, strerror(errno)););
        raise_error(FILE_ERROR);
      } else {
        /* printf("cmdGetMTime: st_mtime=%ld\n", stat_buf.st_mtime); */
        timFromTimestamp(stat_buf.st_mtime,
            year, month, day, hour,
            min, sec, micro_sec, time_zone, is_dst);
      } /* if */
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END cmdGetMTime(" F_D(04) "-" F_D(02) "-" F_D(02) " "
           F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) " " FMT_D " %d)\n",
           *year, *month, *day, *hour, *min, *sec,
           *micro_sec, *time_zone, *is_dst);
#endif
  } /* cmdGetMTime */



/**
 *  Returns the search path of the system as array of strings.
 *  @return the search path of the system.
 *  @exception MEMORY_ERROR Not enough memory to create the result.
 */
rtlArrayType cmdGetSearchPath (void)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    rtlArrayType result;

  /* cmdGetSearchPath */
    result = getSearchPath(&err_info);
    if (unlikely(result == NULL)) {
      raise_error(err_info);
    } /* if */
    return result;
  } /* cmdGetSearchPath */



/**
 *  Determine the home directory of the user.
 *  This function should be preferred over the use of an environment
 *  variable such as $HOME. $HOME is not supported under all operating
 *  systems and it is not guaranteed, that it uses the standard path
 *  representation.
 *  @return The absolute path of the home directory.
 *  @exception MEMORY_ERROR Not enough memory to represent the
 *             result string.
 *  @exception FILE_ERROR Not able to determine the home directory.
 */
striType cmdHomeDir (void)

  {
    static const os_charType home_dir_env_var[] = HOME_DIR_ENV_VAR;
    os_striType os_home_dir;
#ifdef DEFAULT_HOME_DIR
    static const os_charType default_home_dir[] = DEFAULT_HOME_DIR;
#endif
    errInfoType err_info = OKAY_NO_ERROR;
    striType home_dir;

  /* cmdHomeDir */
    os_home_dir = os_getenv(home_dir_env_var);
    if (os_home_dir == NULL) {
#ifdef DEFAULT_HOME_DIR
      home_dir = cp_from_os_path(default_home_dir, &err_info);
#else
      raise_error(FILE_ERROR);
      home_dir = NULL;
#endif
    } else {
      home_dir = cp_from_os_path(os_home_dir, &err_info);
      os_getenv_string_free(os_home_dir);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    return home_dir;
  } /* cmdHomeDir */



/**
 *  Determine the filenames in a directory.
 *  The files "." and ".." are left out from the result.
 *  Note that the function returns only the filenames.
 *  Additional information must be obtained with other calls.
 *  @return a string-array containing the filenames in the directory.
 *  @exception MEMORY_ERROR Not enough memory to convert 'dirPath'
 *             to the system path type or not enough memory to
 *             represent the result 'string array'.
 *  @exception RANGE_ERROR 'dirPath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR A system function returns an error.
 */
rtlArrayType cmdLs (const const_striType dirPath)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    rtlArrayType result;

  /* cmdLs */
    result = read_dir(dirPath, &err_info);
    if (unlikely(result == NULL)) {
      raise_error(err_info);
    } else {
      qsort((void *) result->arr,
          (size_t) (result->max_position - result->min_position + 1),
          sizeof(rtlObjectType), &cmp_mem);
    } /* if */
    return result;
  } /* cmdLs */



/**
 *  Creates a new directory.
 *  @exception MEMORY_ERROR Not enough memory to convert 'dirPath' to
 *             the system path type.
 *  @exception RANGE_ERROR 'dirPath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR A system function returns an error.
 */
void cmdMkdir (const const_striType dirPath)

  {
    os_striType os_path;
    int mkdir_result;
    int path_info;
    errInfoType err_info = OKAY_NO_ERROR;

  /* cmdMkdir */
#ifdef TRACE_CMD_RTL
    printf("cmdMkdir(");
    prot_stri(dirPath);
    printf(")\n");
#endif
    os_path = cp_to_os_path(dirPath, &path_info, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } else {
      /* printf("mkdir(");
         prot_os_stri(os_path);
         printf(")\n"); */
      mkdir_result = os_mkdir(os_path, 0777);
      os_stri_free(os_path);
      if (unlikely(mkdir_result != 0)) {
        logError(printf(" *** cmdMkdir: os_mkdir(\"" FMT_S_OS "\", 0777) failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_path, errno, strerror(errno)););
        raise_error(FILE_ERROR);
      } /* if */
    } /* if */
  } /* cmdMkdir */



/**
 *  Move and rename a file or directory tree.
 *  The function uses the C 'rename()' function. When 'rename()' fails
 *  the file (or directory tree) is cloned with 'cloneFile' (which
 *  preserves permissions/mode, ownership and timestamps) to the new
 *  place and with the new name. When 'cloneFile' succeeds the original
 *  file is deleted. When 'cloneFile' fails (no space on device or
 *  other reason) all remains of the failed clone are removed. Note
 *  that 'cloneFile' works for symbolic links but does not preserve
 *  hard links (they are resolved to distinct files).
 *  @exception MEMORY_ERROR Not enough memory to convert 'sourcePath'
 *             or 'destPath' to the system path type.
 *  @exception RANGE_ERROR 'sourcePath' or 'destPath' does not use
 *             the standard path representation or one of them cannot be
 *             converted to the system path type.
 *  @exception FILE_ERROR Source file does not exist, destination file
 *             already exists or a system function returns an error.
 */
void cmdMove (const const_striType sourcePath, const const_striType destPath)

  {
    os_striType os_sourcePath;
    os_striType os_destPath;
    int path_info;
    errInfoType err_info = OKAY_NO_ERROR;

  /* cmdMove */
#ifdef TRACE_CMD_RTL
    printf("cmdMove(");
    prot_stri(sourcePath);
    printf(", ");
    prot_stri(destPath);
    printf(")\n");
#endif
    os_sourcePath = cp_to_os_path(sourcePath, &path_info, &err_info);
    if (likely(err_info == OKAY_NO_ERROR)) {
      os_destPath = cp_to_os_path(destPath, &path_info, &err_info);
      if (likely(err_info == OKAY_NO_ERROR)) {
        move_any_file(os_sourcePath, os_destPath, &err_info);
        os_stri_free(os_destPath);
      } /* if */
      os_stri_free(os_sourcePath);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* cmdMove */



/**
 *  Reads the destination of a symbolic link.
 *  @return The destination refered by the symbolic link.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type or not enough memory to
 *             represent the result string.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with the path does not
 *             exist or is not a symbolic link.
 */
striType cmdReadlink (const const_striType filePath)

  {
#ifdef HAS_SYMLINKS
    os_striType os_filePath;
    os_stat_struct link_stat;
    os_striType link_destination;
    ssize_t readlink_result;
    int path_info;
#endif
    errInfoType err_info = OKAY_NO_ERROR;
    striType result = NULL;

  /* cmdReadlink */
#ifdef HAS_SYMLINKS
    os_filePath = cp_to_os_path(filePath, &path_info, &err_info);
    if (likely(err_info == OKAY_NO_ERROR)) {
      if (os_lstat(os_filePath, &link_stat) != 0 || !S_ISLNK(link_stat.st_mode)) {
        err_info = FILE_ERROR;
      } else {
        /* printf("link size=%lu\n", link_stat.st_size); */
        if (link_stat.st_size < 0 ||
            (unsigned_os_off_t) link_stat.st_size > MAX_OS_STRI_LEN) {
          err_info = RANGE_ERROR;
        } else {
          if (!os_stri_alloc(link_destination, (memSizeType) link_stat.st_size)) {
            err_info = MEMORY_ERROR;
          } else {
            readlink_result = readlink(os_filePath, link_destination,
                                       (size_t) link_stat.st_size);
            if (readlink_result != -1) {
              link_destination[readlink_result] = '\0';
              result = cp_from_os_path(link_destination, &err_info);
            } else {
              err_info = FILE_ERROR;
            } /* if */
            os_stri_free(link_destination);
          } /* if */
        } /* if */
      } /* if */
      os_stri_free(os_filePath);
    } /* if */
#else
    err_info = FILE_ERROR;
#endif
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    return result;
  } /* cmdReadlink */



/**
 *  Remove a file of any type unless it is a directory that is not empty.
 *  An attempt to remove a directory that is not empty triggers FILE_ERROR.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath' to
 *             the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file does not exist or it is a directory
 *             that is not empty or a system function returns an error.
 */
void cmdRemoveFile (const const_striType filePath)

  {
    os_striType os_filePath;
    int path_info;
#ifdef REMOVE_FAILS_FOR_EMPTY_DIRS
    os_stat_struct file_stat;
#endif
#ifdef RENAME_BEFORE_REMOVE
    os_striType temp_name;
#endif
    errInfoType err_info = OKAY_NO_ERROR;

  /* cmdRemoveFile */
#ifdef TRACE_CMD_RTL
    printf("cmdRemoveFile(");
    prot_stri(filePath);
    printf(")\n");
#endif
    os_filePath = cp_to_os_path(filePath, &path_info, &err_info);
    if (likely(err_info == OKAY_NO_ERROR)) {
#ifdef REMOVE_FAILS_FOR_EMPTY_DIRS
      if (os_lstat(os_filePath, &file_stat) != 0) {
        logError(printf(" *** cmdRemoveFile: os_lstat(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_filePath, errno, strerror(errno)););
        err_info = FILE_ERROR;
      } else {
#ifdef RENAME_BEFORE_REMOVE
        temp_name = temp_name_in_dir(os_filePath);
        if (unlikely(temp_name == NULL)) {
          err_info = MEMORY_ERROR;
        } else {
          if (os_rename(os_filePath, temp_name) != 0) {
            logError(printf(" *** cmdRemoveFile: os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                            "errno=%d\nerror: %s\n",
                            os_filePath, temp_name, errno, strerror(errno)););
            err_info = FILE_ERROR;
          } else {
            if (S_ISDIR(file_stat.st_mode)) {
              if (os_rmdir(temp_name) != 0) {
                logError(printf(" *** cmdRemoveFile: os_rmdir(\"" FMT_S_OS "\") failed:\n"
                                "errno=%d\nerror: %s\n",
                                temp_name, errno, strerror(errno)););
                err_info = FILE_ERROR;
              } /* if */
            } else {
              if (os_remove(temp_name) != 0) {
                logError(printf(" *** cmdRemoveFile: os_remove(\"" FMT_S_OS "\") failed:\n"
                                "errno=%d\nerror: %s\n",
                                temp_name, errno, strerror(errno)););
                err_info = FILE_ERROR;
              } /* if */
            } /* if */
            if (err_info != OKAY_NO_ERROR) {
              /* Rename back to the original name. */
              if (os_rename(temp_name, os_filePath) != 0) {
                logError(printf(" *** cmdRemoveFile: os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                                "errno=%d\nerror: %s\n",
                                temp_name, os_filePath, errno, strerror(errno)););
              } /* if */
            } /* if */
          } /* if */
          os_stri_free(temp_name);
        } /* if */
#else
        if (S_ISDIR(file_stat.st_mode)) {
          if (os_rmdir(os_filePath) != 0) {
            logError(printf(" *** cmdRemoveFile: os_rmdir(\"" FMT_S_OS "\") failed:\n"
                            "errno=%d\nerror: %s\n",
                            os_filePath, errno, strerror(errno)););
            err_info = FILE_ERROR;
          } /* if */
        } else {
          if (os_remove(os_filePath) != 0) {
            logError(printf(" *** cmdRemoveFile: os_remove(\"" FMT_S_OS "\") failed:\n"
                            "errno=%d\nerror: %s\n",
                            os_filePath, errno, strerror(errno)););
            err_info = FILE_ERROR;
          } /* if */
        } /* if */
#endif
      } /* if */
#else
#ifdef RENAME_BEFORE_REMOVE
      temp_name = temp_name_in_dir(os_filePath);
      if (unlikely(temp_name == NULL)) {
        err_info = MEMORY_ERROR;
      } else {
        if (os_rename(os_filePath, temp_name) != 0) {
          logError(printf(" *** cmdRemoveFile: os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_filePath, temp_name, errno, strerror(errno)););
          err_info = FILE_ERROR;
        } else {
          if (os_remove(temp_name) != 0) {
            logError(printf(" *** cmdRemoveFile: os_remove(\"" FMT_S_OS "\") failed:\n"
                            "errno=%d\nerror: %s\n",
                            temp_name, errno, strerror(errno)););
            if (os_rename(temp_name, os_filePath) != 0) {
              /* Rename back to the original name. */
              logError(printf(" *** cmdRemoveFile: os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                              "errno=%d\nerror: %s\n",
                              temp_name, os_filePath, errno, strerror(errno)););
            } /* if */
            err_info = FILE_ERROR;
          } /* if */
        } /* if */
        os_stri_free(temp_name);
      } /* if */
#else
      if (os_remove(os_filePath) != 0) {
        logError(printf(" *** cmdRemoveFile: os_remove(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_filePath, errno, strerror(errno)););
        err_info = FILE_ERROR;
      } /* if */
#endif
#endif
#ifdef CHECK_IF_FILE_IS_REMOVED
      if (err_info == OKAY_NO_ERROR &&
          os_lstat(os_filePath, &file_stat) == 0) {
        logError(printf(" *** cmdRemoveFile: File \"" FMT_S_OS "\" still present.\n",
                        os_filePath););
        /* err_info = FILE_ERROR; */
      } /* if */
#endif
      os_stri_free(os_filePath);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* cmdRemoveFile */



/**
 *  Remove a file of any type inclusive a directory tree.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath' to
 *             the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file does not exist or a system function
 *             returns an error.
 */
void cmdRemoveTree (const const_striType filePath)

  {
    os_striType os_filePath;
    int path_info;
    os_stat_struct file_stat;
#ifdef RENAME_BEFORE_REMOVE
    os_striType temp_name;
#endif
    errInfoType err_info = OKAY_NO_ERROR;

  /* cmdRemoveTree */
#ifdef TRACE_CMD_RTL
    printf("cmdRemoveTree(");
    prot_stri(filePath);
    printf(")\n");
#endif
    os_filePath = cp_to_os_path(filePath, &path_info, &err_info);
    if (likely(err_info == OKAY_NO_ERROR)) {
      if (os_lstat(os_filePath, &file_stat) != 0) {
        logError(printf(" *** cmdRemoveTree: os_lstat(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_filePath, errno, strerror(errno)););
        err_info = FILE_ERROR;
      } else {
#ifdef RENAME_BEFORE_REMOVE
        temp_name = temp_name_in_dir(os_filePath);
        if (unlikely(temp_name == NULL)) {
          err_info = MEMORY_ERROR;
        } else {
          if (os_rename(os_filePath, temp_name) != 0) {
            logError(printf(" *** cmdRemoveTree: os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                            "errno=%d\nerror: %s\n",
                            os_filePath, temp_name, errno, strerror(errno)););
            err_info = FILE_ERROR;
          } else {
            if (S_ISDIR(file_stat.st_mode)) {
              remove_any_file(temp_name, &err_info);
            } else {
              if (os_remove(temp_name) != 0) {
                logError(printf(" *** cmdRemoveTree: os_remove(\"" FMT_S_OS "\") failed:\n"
                                "errno=%d\nerror: %s\n",
                                os_filePath, errno, strerror(errno)););
                err_info = FILE_ERROR;
              } /* if */
            } /* if */
            if (err_info != OKAY_NO_ERROR) {
              if (os_rename(temp_name, os_filePath) != 0) {
                /* Rename back to the original name. */
                logError(printf(" *** cmdRemoveTree: os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                                "errno=%d\nerror: %s\n",
                                temp_name, os_filePath, errno, strerror(errno)););
              } /* if */
            } /* if */
          } /* if */
          os_stri_free(temp_name);
        } /* if */
#else
        if (S_ISDIR(file_stat.st_mode)) {
          remove_any_file(os_filePath, &err_info);
        } else {
          if (os_remove(os_filePath) != 0) {
            logError(printf(" *** cmdRemoveTree: os_remove(\"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_filePath, errno, strerror(errno)););
            err_info = FILE_ERROR;
          } /* if */
        } /* if */
#endif
      } /* if */
#ifdef CHECK_IF_FILE_IS_REMOVED
      if (err_info == OKAY_NO_ERROR &&
          os_lstat(os_filePath, &file_stat) == 0) {
        logError(printf(" *** cmdRemoveTree: File \"" FMT_S_OS "\" still present.\n",
                        os_filePath););
        /* err_info = FILE_ERROR; */
      } /* if */
#endif
      os_stri_free(os_filePath);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* cmdRemoveTree */



/**
 *  Add or change an environment variable.
 *  The function searches the environment for an environment variable
 *  with the given 'name'. When such an environment variable exists the
 *  corresponding value is changed to 'value'. When no environment variable
 *  with the given 'name' exists a new environment variable 'name' with
 *  the value 'value' is created.
 *  @exception MEMORY_ERROR Not enough memory to convert 'name' or 'value'
 *             to the system string type.
 *  @exception RANGE_ERROR 'name' or 'value' cannot be converted to the
 *             system string type or a system function returns an error.
 */
void cmdSetenv (const const_striType name, const const_striType value)

  {
    errInfoType err_info = OKAY_NO_ERROR;

  /* cmdSetenv */
    setEnvironmentVariable(name, value, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* cmdSetenv */



/**
 *  Set the access time of a file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception RANGE_ERROR 'aTime' is invalid or cannot be
 *             converted to the system file time.
 *  @exception FILE_ERROR A system function returns an error.
 */
void cmdSetATime (const const_striType filePath,
    intType year, intType month, intType day, intType hour,
    intType min, intType sec, intType micro_sec, intType time_zone)

  {
    const_os_striType os_path;
    os_stat_struct stat_buf;
    os_utimbuf_struct utime_buf;
    int path_info;
    errInfoType err_info = OKAY_NO_ERROR;

  /* cmdSetATime */
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (likely(err_info == OKAY_NO_ERROR)) {
      if (os_stat(os_path, &stat_buf) == 0) {
        utime_buf.actime = timToTimestamp(year, month, day, hour,
            min, sec, micro_sec, time_zone);
        /* printf("cmdSetATime: actime=%ld\n", utime_buf.actime); */
        utime_buf.modtime = stat_buf.st_mtime;
        if (utime_buf.actime == (time_t) -1) {
          err_info = RANGE_ERROR;
        } else if (os_utime(os_path, &utime_buf) != 0) {
          logError(printf(" *** cmdSetATime: os_utime(\"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_path, errno, strerror(errno)););
          /* printf("EPERM=%d, EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d  ENOTDIR=%d  EROFS=%d\n",
              EPERM, EACCES, EBUSY, EEXIST, ENOTEMPTY, ENOENT, ENOTDIR, EROFS); */
          err_info = FILE_ERROR;
        } /* if */
      } else {
        err_info = FILE_ERROR;
      } /* if */
      os_stri_free(os_path);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* cmdSetATime */



/**
 *  Change the file mode (permissions) of a file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR A system function returns an error.
 */
void cmdSetFileMode (const const_striType filePath, const const_setType mode)

  {
    os_striType os_path;
    intType intType_mode;
    int int_mode;
    int chmod_result;
    int path_info;
    errInfoType err_info = OKAY_NO_ERROR;

  /* cmdSetFileMode */
#ifdef TRACE_CMD_RTL
    printf("BEGIN cmdSetFileMode(");
    prot_stri(filePath);
    printf(")\n");
#endif
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (likely(err_info == OKAY_NO_ERROR)) {
      intType_mode = setSConv(mode);
      if (intType_mode >= 0 && intType_mode <= 0777) {
        /* Just the read, write and execute permissions are accepted */
        int_mode = (int) intType_mode;
        /* printf("cmdSetFileMode: mode=0%o\n", int_mode); */
#if MODE_BITS_NORMAL
        chmod_result = os_chmod(os_path, int_mode);
#else
        /* Force the bits to the standard sequence */
        chmod_result = os_chmod(os_path,
            (int_mode & 0400 ? S_IRUSR : 0) |
            (int_mode & 0200 ? S_IWUSR : 0) |
            (int_mode & 0100 ? S_IXUSR : 0) |
            (int_mode & 0040 ? S_IRGRP : 0) |
            (int_mode & 0020 ? S_IWGRP : 0) |
            (int_mode & 0010 ? S_IXGRP : 0) |
            (int_mode & 0004 ? S_IROTH : 0) |
            (int_mode & 0002 ? S_IWOTH : 0) |
            (int_mode & 0001 ? S_IXOTH : 0));
#endif
        if (chmod_result != 0) {
          logError(printf(" *** cmdSetFileMode: os_chmod(\"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_path, errno, strerror(errno)););
          err_info = FILE_ERROR;
        } /* if */
      } else {
        err_info = RANGE_ERROR;
      } /* if */
      os_stri_free(os_path);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* cmdSetFileMode */



/**
 *  Set the modification time of a file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception RANGE_ERROR 'aTime' is invalid or cannot be
 *             converted to the system file time.
 *  @exception FILE_ERROR A system function returns an error.
 */
void cmdSetMTime (const const_striType filePath,
    intType year, intType month, intType day, intType hour,
    intType min, intType sec, intType micro_sec, intType time_zone)

  {
    const_os_striType os_path;
    os_stat_struct stat_buf;
    os_utimbuf_struct utime_buf;
    int path_info;
    errInfoType err_info = OKAY_NO_ERROR;

  /* cmdSetMTime */
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (likely(err_info == OKAY_NO_ERROR)) {
      if (os_stat(os_path, &stat_buf) == 0) {
        utime_buf.actime = stat_buf.st_atime;
        utime_buf.modtime = timToTimestamp(year, month, day, hour,
            min, sec, micro_sec, time_zone);
        /* printf("cmdSetMTime: modtime=%ld\n", utime_buf.modtime); */
        if (utime_buf.actime == (time_t) -1) {
          err_info = RANGE_ERROR;
        } else if (os_utime(os_path, &utime_buf) != 0) {
          logError(printf(" *** cmdSetMTime: os_utime(\"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_path, errno, strerror(errno)););
          /* printf("EPERM=%d, EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d  ENOTDIR=%d  EROFS=%d\n",
              EPERM, EACCES, EBUSY, EEXIST, ENOTEMPTY, ENOENT, ENOTDIR, EROFS); */
          err_info = FILE_ERROR;
        } /* if */
      } else {
        err_info = FILE_ERROR;
      } /* if */
      os_stri_free(os_path);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* cmdSetMTime */



/**
 *  Sets the search path from an array of strings.
 *  The search path is used by the current process and its sub processes.
 *  The path of parent processes is not affected by this function.
 *  @exception MEMORY_ERROR Not enough memory to convert the path
 *             to the system string type.
 *  @exception RANGE_ERROR The path cannot be converted to the
 *             system string type or a system function returns an error.
 */
void cmdSetSearchPath (rtlArrayType searchPath)

  {
    errInfoType err_info = OKAY_NO_ERROR;

  /* cmdSetSearchPath */
    setSearchPath(searchPath, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* cmdSetSearchPath */



/**
 *  Use the shell to execute a 'command' with 'parameters'.
 *  Parameters which contain a space must be enclosed in double
 *  quotes (E.g.: shell("aCommand", "\"par 1\" par2"); ). The
 *  commands supported and the format of the 'parameters' are not
 *  covered by the description of the 'shell' function. Due to the
 *  usage of the operating system shell and external programs, it is
 *  hard to write portable programs, which use the 'shell' function.
 *  @param command Name of the command to be executed. A path must
 *         use the standard path representation.
 *  @param parameters Space separated list of parameters for the
 *         'command', or "" when there are no parameters.
 *  @return the return code of the executed command or of the shell.
 */
intType cmdShell (const const_striType command, const const_striType parameters)

  {
    os_striType os_command;
    errInfoType err_info = OKAY_NO_ERROR;
    intType result;

  /* cmdShell */
    os_command = cp_to_command(command, parameters, &err_info);
    if (unlikely(os_command == NULL)) {
      raise_error(err_info);
      result = 0;
    } else {
      /* printf("os_command: \"" FMT_S_OS "\"\n", os_command); */
      result = (intType) os_system(os_command);
      /* if (result != 0) {
        printf("errno=%d\n", errno);
        printf("E2BIG=%d  ENOENT=%d  ENOEXEC=%d  ENOMEM=%d\n",
            E2BIG, ENOENT, ENOEXEC, ENOMEM);
        printf("result=%d\n", result);
      } */
      FREE_OS_STRI(os_command);
    } /* if */
    return result;
  } /* cmdShell */



/**
 *  Convert a string, such that it can be used as shell parameter.
 *  The function adds escape characters or quotations to a string.
 *  The result is useable as parameter for the functions 'cmdShell'
 *  and 'filPopen'. Shell parameters must be escaped individually.
 *  Afterwards escaped parameters are joined to a space separated
 *  list of parameters.
 *  @return a string which can be used as shell parameter.
 *  @exception MEMORY_ERROR Not enough memory to convert 'stri'.
 */
striType cmdShellEscape (const const_striType stri)

  {
    memSizeType inPos;
    memSizeType outPos;
    boolType quote_path;
#ifndef ESCAPE_SHELL_COMMANDS
    memSizeType countBackslash;
#endif
    errInfoType err_info = OKAY_NO_ERROR;
    striType resized_result;
    striType result;

  /* cmdShellEscape */
    if (unlikely(stri->size > (MAX_STRI_LEN - 2) / 3 ||
        !ALLOC_STRI_SIZE_OK(result, 3 * stri->size + 2))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      quote_path = FALSE;
      for (inPos = 0, outPos = 0; inPos < stri->size; inPos++, outPos++) {
        switch (stri->mem[inPos]) {
#ifdef ESCAPE_SHELL_COMMANDS
          case '\t': case ' ':  case '!':  case '\"': case '$':
          case '&':  case '\'': case '(':  case ')':  case '*':
          case ',':  case ':':  case ';':  case '<':  case '=':
          case '>':  case '?':  case '[':  case '\\': case ']':
          case '^':  case '`':  case '{':  case '|':  case '}':
          case '~':
            result->mem[outPos] = '\\';
            outPos++;
            result->mem[outPos] = stri->mem[inPos];
            break;
          case '\0': case '\n':
            err_info = RANGE_ERROR;
            break;
#else
          case ' ':  case ',':  case ';':  case '=':  case '~':
          case 160:
            quote_path = TRUE;
            result->mem[outPos] = stri->mem[inPos];
            break;
          case '&':  case '<':  case '>':  case '^':  case '|':
            quote_path = TRUE;
            result->mem[outPos] = '^';
            outPos++;
            result->mem[outPos] = stri->mem[inPos];
            break;
          case '%':
            quote_path = TRUE;
            result->mem[outPos] = '\"';
            outPos++;
            result->mem[outPos] = stri->mem[inPos];
            outPos++;
            result->mem[outPos] = '\"';
            break;
          case '\"':
            quote_path = TRUE;
            result->mem[outPos] = '\\';
            outPos++;
            result->mem[outPos] = stri->mem[inPos];
            break;
          case '\\':
            quote_path = TRUE;
            inPos++;
            countBackslash = 1;
            while (stri->mem[inPos] == '\\') {
              inPos++;
              countBackslash++;
            } /* while */
            if (inPos == stri->size || stri->mem[inPos] == '"') {
              countBackslash *= 2;
            } /* if */
            inPos--;
            do {
              result->mem[outPos] = '\\';
              outPos++;
              countBackslash--;
            } while (countBackslash != 0);
            outPos--;
            break;
          case '\0': case '\n': case '\r':
            err_info = RANGE_ERROR;
            break;
#endif
          default:
            result->mem[outPos] = stri->mem[inPos];
            break;
        } /* switch */
      } /* for */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        FREE_STRI(result, 3 * stri->size + 2);
        raise_error(err_info);
        result = NULL;
      } else {
        if (quote_path) {
          memmove(&result->mem[1], result->mem, sizeof(strElemType) * outPos);
          result->mem[0] = '\"';
          result->mem[outPos + 1] = '\"';
          outPos += 2;
        } /* if */
        REALLOC_STRI_SIZE_SMALLER(resized_result, result, 3 * stri->size + 2, outPos);
        if (resized_result == NULL) {
          FREE_STRI(result, 3 * stri->size + 2);
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          result = resized_result;
          COUNT3_STRI(3 * stri->size + 2, outPos);
          result->size = outPos;
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* cmdShellEscape */



/**
 *  Create a symbolic link.
 *  The symbolic link 'destPath' will refer to 'sourcePath' afterwards.
 *  @param sourcePath String to be contained in the symbolic link.
 *  @param destPath Name of the symbolic link to be created.
 *  @exception MEMORY_ERROR Not enough memory to convert sourcePath or
 *             destPath to the system path type.
 *  @exception RANGE_ERROR 'sourcePath' or 'destPath' does not use the
 *             standard path representation or one of them cannot be
 *             converted to the system path type.
 *  @exception FILE_ERROR A system function returns an error.
 */
void cmdSymlink (const const_striType sourcePath, const const_striType destPath)

  {
#ifdef HAS_SYMLINKS
    os_striType os_sourcePath;
    os_striType os_destPath;
    int path_info;
#endif
    errInfoType err_info = OKAY_NO_ERROR;

  /* cmdSymlink */
#ifdef HAS_SYMLINKS
    os_sourcePath = cp_to_os_path(sourcePath, &path_info, &err_info);
    if (likely(err_info == OKAY_NO_ERROR)) {
      os_destPath = cp_to_os_path(destPath, &path_info, &err_info);
      if (likely(err_info == OKAY_NO_ERROR)) {
        if (symlink(os_sourcePath, os_destPath) != 0) {
          err_info = FILE_ERROR;
        } /* if */
        os_stri_free(os_destPath);
      } /* if */
      os_stri_free(os_sourcePath);
    } /* if */
#else
    err_info = FILE_ERROR;
#endif
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* cmdSymlink */



/**
 *  Convert a standard path to the path of the operating system.
 *  The result must be escaped with 'cmdShellEscape' to be useable as
 *  parameter for the functions 'cmdShell' and 'filPopen'.
 *  @param standardPath Path in the standard path representation.
 *  @return a string containing an operating system path.
 *  @exception MEMORY_ERROR Not enough memory to convert 'standardPath'.
 *  @exception RANGE_ERROR 'standardPath' is not representable as operating
 *             system path.
 */
striType cmdToOsPath (const const_striType standardPath)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    striType result;

  /* cmdToOsPath */
#ifdef TRACE_CMD_RTL
    printf("cmdToOsPath(");
    prot_stri(standardPath);
    printf(")\n");
#endif
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
#ifdef FORBID_DRIVE_LETTERS
    if (unlikely(standardPath->size >= 2 && (standardPath->mem[standardPath->size - 1] == '/' ||
                 (standardPath->mem[1] == ':' &&
                 ((standardPath->mem[0] >= 'a' && standardPath->mem[0] <= 'z') ||
                  (standardPath->mem[0] >= 'A' && standardPath->mem[0] <= 'Z')))))) {
#else
    if (unlikely(standardPath->size >= 2 && standardPath->mem[standardPath->size - 1] == '/')) {
#endif
#else
    if (unlikely(standardPath->size >= 2 && standardPath->mem[standardPath->size - 1] == '/' &&
                 (standardPath->size != 3 || standardPath->mem[1] != ':' ||
                 ((standardPath->mem[0] < 'a' || standardPath->mem[0] > 'z') &&
                  (standardPath->mem[0] < 'A' || standardPath->mem[0] > 'Z'))))) {
#endif
      err_info = RANGE_ERROR;
    } else {
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (standardPath->size >= 1 && standardPath->mem[0] == '/') {
        /* Absolute path: Try to map the path to a drive letter */
        if (unlikely(standardPath->size == 1)) {
          /* "/"    cannot be mapped to a drive letter */
          err_info = RANGE_ERROR;
        } else if (standardPath->mem[1] >= 'a' && standardPath->mem[1] <= 'z') {
          if (standardPath->size == 2) {
            /* "/c"   is mapped to "c:/"  */
            if (unlikely(!ALLOC_STRI_SIZE_OK(result, 3))) {
              err_info = MEMORY_ERROR;
            } else {
              result->size = 3;
              result->mem[0] = standardPath->mem[1];
              result->mem[1] = ':';
              result->mem[2] = '/';
            } /* if */
          } else if (unlikely(standardPath->mem[2] != '/')) {
            /* "/cd"  cannot be mapped to a drive letter */
            err_info = RANGE_ERROR;
          } else {
            /* "/c/d" is mapped to "c:/d" */
            if (unlikely(!ALLOC_STRI_SIZE_OK(result, standardPath->size))) {
              err_info = MEMORY_ERROR;
            } else {
              result->size = standardPath->size;
              result->mem[0] = standardPath->mem[1];
              result->mem[1] = ':';
              result->mem[2] = '/';
              memcpy(&result->mem[3], &standardPath->mem[3], (standardPath->size - 3) * sizeof(strElemType));
            } /* if */
          } /* if */
        } else {
          /* "/C"  cannot be mapped to a drive letter */
          err_info = RANGE_ERROR;
        } /* if */
      } else {
#endif
        if (unlikely(!ALLOC_STRI_SIZE_OK(result, standardPath->size))) {
          err_info = MEMORY_ERROR;
        } else {
          result->size = standardPath->size;
          memcpy(result->mem, standardPath->mem, standardPath->size * sizeof(strElemType));
        } /* if */
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      } /* if */
#endif
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
      result = NULL;
    } else {
#if PATH_DELIMITER != '/'
      memSizeType position;

      for (position = 0; position < result->size; position++) {
        if (result->mem[position] == '/') {
          result->mem[position] = PATH_DELIMITER;
        } /* if */
      } /* for */
#endif
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("cmdToOsPath(");
    prot_stri(standardPath);
    printf(") --> ");
    prot_stri(result);
    printf("\n");
#endif
    return result;
  } /* cmdToOsPath */

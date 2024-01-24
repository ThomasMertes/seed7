
/********************************************************************/
/*                                                                  */
/*  cmd_rtl.c     Directory, file and other system functions.       */
/*  Copyright (C) 1989 - 2016, 2018 - 2021  Thomas Mertes           */
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
/*  Changes: 1994, 2006, 2009, 2018 - 2021  Thomas Mertes           */
/*  Content: Directory, file and other system functions.            */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

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
#ifdef OS_GETCWD_INCLUDE_DIRECT_H
#include "direct.h"
#endif
#ifdef OS_WIDE_DIR_INCLUDE_DIR_H
#include "dir.h"
#endif
#ifdef OS_CHMOD_INCLUDE_IO_H
#include "io.h"
#endif
#endif
#ifdef DEFINE_SYSTEM_FUNCTION
#include "emscripten.h"
#endif
#include "errno.h"

#if UNISTD_H_PRESENT
#include "unistd.h"
#endif

#if HAS_MMAP
#include "sys/mman.h"
#endif

#include "common.h"
#include "data_rtl.h"
#include "os_decls.h"
#include "heaputl.h"
#include "striutl.h"
#include "level.h"
#include "str_rtl.h"
#include "chr_rtl.h"
#include "int_rtl.h"
#include "fil_rtl.h"
#include "dir_drv.h"
#include "dir_rtl.h"
#include "set_rtl.h"
#include "tim_rtl.h"
#include "arr_rtl.h"
#include "tim_drv.h"
#include "big_drv.h"
#include "cmd_drv.h"
#include "stat_drv.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "cmd_rtl.h"


#define MAX_CSTRI_BUFFER_LEN 40

#ifndef CALL_C_COMPILER_FROM_SHELL
#define CALL_C_COMPILER_FROM_SHELL 0
#endif
#ifndef CPLUSPLUS_COMPILER
#define CPLUSPLUS_COMPILER C_COMPILER
#endif
#ifndef EXECUTABLE_FILE_EXTENSION
#define EXECUTABLE_FILE_EXTENSION ""
#endif
#ifndef CC_OPT_LINK_TIME_OPTIMIZATION
#define CC_OPT_LINK_TIME_OPTIMIZATION ""
#endif
#ifndef LINKER_OPT_LTO_MANDATORY
#define LINKER_OPT_LTO_MANDATORY 0
#endif
#ifndef CC_OPT_TRAP_OVERFLOW
#define CC_OPT_TRAP_OVERFLOW ""
#endif
#ifndef CC_OPT_VERSION_INFO
#define CC_OPT_VERSION_INFO ""
#endif
#ifndef CC_ENVIRONMENT_INI
#define CC_ENVIRONMENT_INI ""
#endif
#ifndef CC_FLAGS
#define CC_FLAGS ""
#endif
#ifndef C_COMPILER_VERSION
#define C_COMPILER_VERSION ""
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
#ifndef LINKER_OPT_SPECIAL_LIB
#define LINKER_OPT_SPECIAL_LIB ""
#endif
#ifndef LINKER_OPT_STACK_SIZE
#define LINKER_OPT_STACK_SIZE ""
#endif
#ifndef LINKER_FLAGS
#define LINKER_FLAGS ""
#endif
#ifndef DEFAULT_STACK_SIZE
#define DEFAULT_STACK_SIZE 0
#endif
#ifndef LINKED_PROGRAM_EXTENSION
#ifdef EXECUTABLE_FILE_EXTENSION
#define LINKED_PROGRAM_EXTENSION EXECUTABLE_FILE_EXTENSION
#else
#define LINKED_PROGRAM_EXTENSION ""
#endif
#endif
#ifndef SPECIAL_LIB
#define SPECIAL_LIB ""
#endif
#ifndef SYSTEM_LIBS
#define SYSTEM_LIBS ""
#endif
#ifndef SYSTEM_MATH_LIBS
#define SYSTEM_MATH_LIBS ""
#endif
#ifndef PIXEL_RED_MASK
#define PIXEL_RED_MASK "0"
#endif
#ifndef PIXEL_GREEN_MASK
#define PIXEL_GREEN_MASK "0"
#endif
#ifndef PIXEL_BLUE_MASK
#define PIXEL_BLUE_MASK "0"
#endif
#ifndef RGB_TO_PIXEL_FLAG_NAME
#define RGB_TO_PIXEL_FLAG_NAME ""
#endif


#if HAS_BUILTIN_OVERFLOW_OPERATIONS
#if INTTYPE_SIZE == INT_SIZE
#define BUILTIN_ADD_OVERFLOW "__builtin_sadd_overflow"
#define BUILTIN_SUB_OVERFLOW "__builtin_ssub_overflow"
#define BUILTIN_MULT_OVERFLOW "__builtin_smul_overflow"
#elif INTTYPE_SIZE == LONG_SIZE
#define BUILTIN_ADD_OVERFLOW "__builtin_saddl_overflow"
#define BUILTIN_SUB_OVERFLOW "__builtin_ssubl_overflow"
#define BUILTIN_MULT_OVERFLOW "__builtin_smull_overflow"
#elif defined(LONG_LONG_SIZE) && INTTYPE_SIZE == LONG_LONG_SIZE
#define BUILTIN_ADD_OVERFLOW "__builtin_saddll_overflow"
#define BUILTIN_SUB_OVERFLOW "__builtin_ssubll_overflow"
#define BUILTIN_MULT_OVERFLOW "__builtin_smulll_overflow"
#endif
#else
#define BUILTIN_ADD_OVERFLOW ""
#define BUILTIN_SUB_OVERFLOW ""
#define BUILTIN_MULT_OVERFLOW ""
#endif


#ifndef INT64TYPE
#define INT64TYPE_STRI ""
#define UINT64TYPE_STRI ""
#undef INT32TYPE_LITERAL_SUFFIX
#define INT32TYPE_LITERAL_SUFFIX ""
#define INT64TYPE_LITERAL_SUFFIX ""
#endif

#ifndef INT128TYPE
#define INT128TYPE_STRI ""
#define UINT128TYPE_STRI ""
#endif

#ifndef PATH_MAX
#define PATH_MAX 2048
#endif

#define CLASSIC_WINDOWS_MAX_PATH 260

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

#define INITIAL_ARRAY_SIZE 256
#define ARRAY_SIZE_DELTA   256

#define CONFIG_VALUE_BUFFER_SIZE 4096

#if DECLARE_OS_ENVIRON
extern os_striType *os_environ;
#endif

static const os_charType path_variable[] = {'P', 'A', 'T', 'H', 0};


#define driveLetterPathWrong(standardPath) \
    ((standardPath)->size >= 2 && \
     ((standardPath)->mem[(standardPath)->size - 1] == '/' || \
      ((standardPath)->mem[1] == ':' && \
      (((standardPath)->mem[0] >= 'a' && (standardPath)->mem[0] <= 'z') || \
       ((standardPath)->mem[0] >= 'A' && (standardPath)->mem[0] <= 'Z')))))

#define standardPathWrong(standardPath) \
    ((standardPath)->size >= 2 && \
     (standardPath)->mem[(standardPath)->size - 1] == '/')


#ifdef DETERMINE_OS_PROPERTIES_AT_RUNTIME

char *nullDevice;
unsigned char shellPathDelimiter;
boolType shellUsesDriveLetters;
#ifdef EMULATE_ENVIRONMENT
int (*environmentStrncmp) (const char *s1, const char *s2, size_t n);
#endif
#define NULL_DEVICE_FOR_SCRIPTS nullDevice
#define SHELL_PATH_DELIMITER shellPathDelimiter
#define if_pathDelimiterNotSlash(thenPart) if (shellPathDelimiter != '/') thenPart
#define pathIsWrong(standardPath) (shellUsesDriveLetters && driveLetterPathWrong(standardPath)) || \
                                  (!shellUsesDriveLetters && standardPathWrong(standardPath))
#define if_mapAbsoluteShellPathToDriveLetters(cond, thenPart, elsePart) \
    if (shellUsesDriveLetters && (cond)) thenPart else elsePart

#else

#define NULL_DEVICE_FOR_SCRIPTS NULL_DEVICE
#define SHELL_PATH_DELIMITER PATH_DELIMITER

#if PATH_DELIMITER != '/'
#define if_pathDelimiterNotSlash(thenPart) thenPart
#else
#define if_pathDelimiterNotSlash(thenPart)
#endif

#if MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
#define pathIsWrong(standardPath) driveLetterPathWrong(standardPath)
#define if_mapAbsoluteShellPathToDriveLetters(cond, thenPart, elsePart) if (cond) thenPart else elsePart
#else
#define pathIsWrong(standardPath) standardPathWrong(standardPath)
#define if_mapAbsoluteShellPathToDriveLetters(cond, thenPart, elsePart) elsePart
#endif

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
    size_t d_name_size;
    size_t dir_path_capacity = 0;
    os_striType dir_path = NULL;
    size_t new_size;
    os_striType resized_path;
    boolType init_path = TRUE;

  /* remove_dir */
    logFunction(printf("remove_dir(\"" FMT_S_OS "\")\n", dir_name););
    if ((directory = os_opendir(dir_name)) == NULL) {
      logError(printf("remove_dir: os_opendir(\"" FMT_S_OS "\") failed:\n"
                      "errno=%d\nerror: %s\n",
                      dir_name, errno, strerror(errno)););
      *err_info = FILE_ERROR;
    } else {
      do {
        current_entry = os_readdir(directory);
/*      printf("$" FMT_U_MEM "$\n", (memSizeType) current_entry);
        fflush(stdout); */
      } while (current_entry != NULL && current_entry->d_name[0] == '.' &&
               (current_entry->d_name[1] == '\0' ||
                (current_entry->d_name[1] == '.' &&
                 current_entry->d_name[2] == '\0')));
      dir_name_size = os_stri_strlen(dir_name);
      while (*err_info == OKAY_NO_ERROR && current_entry != NULL) {
/*      printf("!" FMT_S_OS "!\n", current_entry->d_name);
        fflush(stdout); */
        d_name_size = os_stri_strlen(current_entry->d_name);
        new_size = dir_name_size + 1 + d_name_size;
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
            memcpy(dir_path, dir_name, dir_name_size * sizeof(os_charType));
            memcpy(&dir_path[dir_name_size], pathDelimiter, sizeof(os_charType));
            init_path = FALSE;
          } /* if */
          memcpy(&dir_path[dir_name_size + 1], current_entry->d_name,
                 d_name_size * sizeof(os_charType));
          dir_path[dir_name_size + 1 + d_name_size] = '\0';
          remove_any_file(dir_path, err_info);
        } else {
          *err_info = MEMORY_ERROR;
        } /* if */
        do {
          current_entry = os_readdir(directory);
/*        printf("$" FMT_U_MEM "$\n", (memSizeType) current_entry);
          fflush(stdout); */
        } while (current_entry != NULL && current_entry->d_name[0] == '.' &&
                 (current_entry->d_name[1] == '\0' ||
                  (current_entry->d_name[1] == '.' &&
                   current_entry->d_name[2] == '\0')));
      } /* while */
      if (dir_path != NULL) {
        FREE_OS_STRI(dir_path);
      } /* if */
      os_closedir(directory);
      if (*err_info == OKAY_NO_ERROR) {
        /* printf("before remove directory <" FMT_S_OS ">\n", dir_name); */
        if (os_rmdir(dir_name) != 0) {
          logError(printf("remove_dir: os_rmdir(\"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          dir_name, errno, strerror(errno)););
          *err_info = FILE_ERROR;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("remove_dir(\"" FMT_S_OS "\", %d) -->\n", dir_name, *err_info););
  } /* remove_dir */



static void remove_any_file (const const_os_striType file_name, errInfoType *err_info)

  {
    os_stat_struct file_stat;

  /* remove_any_file */
    logFunction(printf("remove_any_file(\"" FMT_S_OS "\")\n", file_name););
    if (os_lstat(file_name, &file_stat) != 0) {
      logError(printf("remove_any_file: os_lstat(" FMT_S_OS ", *) failed:\n"
                      "errno=%d\nerror: %s\n",
                      file_name, errno, strerror(errno)););
      *err_info = FILE_ERROR;
    } else {
      if (S_ISDIR(file_stat.st_mode)) {
        remove_dir(file_name, err_info);
      } else {
        if (os_remove(file_name) != 0) {
          logError(printf("remove_any_file: os_remove(\"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          file_name, errno, strerror(errno)););
          *err_info = FILE_ERROR;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("remove_any_file(\"" FMT_S_OS "\", %d) -->\n",
                       file_name, *err_info););
  } /* remove_any_file */



static void copy_file (const const_os_striType from_name,
    const const_os_striType to_name, errInfoType *err_info)

  {
    FILE *from_file;
    FILE *to_file;
#if HAS_MMAP
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
    logFunction(printf("copy_file(\"" FMT_S_OS "\", \"" FMT_S_OS "\")\n",
                       from_name, to_name););
    if ((from_file = os_fopen(from_name, os_mode_rb)) != NULL) {
      if ((to_file = os_fopen(to_name, os_mode_wb)) != NULL) {
#if HAS_MMAP
        file_no = os_fileno(from_file);
        if (file_no != -1 && os_fstat(file_no, &file_stat) == 0) {
          if (file_stat.st_size >= 0 &&
              (unsigned_os_off_t) file_stat.st_size < MAX_MEMSIZETYPE) {
            file_length = (memSizeType) file_stat.st_size;
            if ((file_content = (ustriType) mmap(NULL, file_length,
                PROT_READ, MAP_PRIVATE, file_no, 0)) != (ustriType) -1) {
              if (fwrite(file_content, 1, file_length, to_file) != file_length) {
                logError(printf("copy_file(\"" FMT_S_OS "\", \"" FMT_S_OS "\"): "
                                "fwrite(*, 1, " FMT_U_MEM ", %d) failed:\n"
                                "errno=%d\nerror: %s\n",
                                from_name, to_name, file_length, safe_fileno(to_file),
                                errno, strerror(errno)););
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
              logError(printf("copy_file(\"" FMT_S_OS "\", \"" FMT_S_OS "\"): "
                              "fwrite(*, 1, " FMT_U_MEM ", %d) failed:\n"
                              "errno=%d\nerror: %s\n",
                              from_name, to_name, (memSizeType) bytes_read,
                              safe_fileno(to_file), errno, strerror(errno)););
              *err_info = FILE_ERROR;
            } /* if */
          } /* while */
          if (normal_buffer != NULL) {
            FREE_BYTES(normal_buffer, SIZE_NORMAL_BUFFER);
          } /* if */
#if HAS_MMAP
        } /* if */
#endif
        if (fclose(from_file) != 0) {
          logError(printf("copy_file(\"" FMT_S_OS "\", \"" FMT_S_OS "\"): "
                          "fclose(%d) failed:\n"
                          "errno=%d\nerror: %s\n",
                          from_name, to_name, safe_fileno(from_file),
                          errno, strerror(errno)););
          *err_info = FILE_ERROR;
        } /* if */
        if (fclose(to_file) != 0) {
          logError(printf("copy_file(\"" FMT_S_OS "\", \"" FMT_S_OS "\"): "
                          "fclose(%d) failed:\n"
                          "errno=%d\nerror: %s\n",
                          from_name, to_name, safe_fileno(to_file),
                          errno, strerror(errno)););
          *err_info = FILE_ERROR;
        } /* if */
        if (unlikely(*err_info != OKAY_NO_ERROR)) {
          os_remove(to_name);
        } /* if */
      } else {
        logError(printf("copy_file: os_fopen(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        to_name, os_mode_wb, errno, strerror(errno)););
        fclose(from_file);
        *err_info = FILE_ERROR;
      } /* if */
    } else {
      logError(printf("copy_file: os_fopen(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                      "errno=%d\nerror: %s\n",
                      from_name, os_mode_rb, errno, strerror(errno)););
      *err_info = FILE_ERROR;
    } /* if */
    logFunction(printf("copy_file(\"" FMT_S_OS "\", \"" FMT_S_OS "\", %d) -->\n",
                       from_name, to_name, *err_info););
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
    logFunction(printf("copy_dir(\"" FMT_S_OS "\", \"" FMT_S_OS "\")\n",
                       from_name, to_name););
    if ((directory = os_opendir(from_name)) == NULL) {
      logError(printf("copy_dir: os_opendir(\"" FMT_S_OS "\") failed:\n"
                      "errno=%d\nerror: %s\n",
                      from_name, errno, strerror(errno)););
      *err_info = FILE_ERROR;
    } else {
      if (os_mkdir(to_name, (S_IRWXU | S_IRWXG | S_IRWXO)) != 0) {
        logError(printf("copy_dir: os_mkdir(\"" FMT_S_OS "\", rwxrwxrwx) failed:\n"
                        "errno=%d\nerror: %s\n",
                        to_name, errno, strerror(errno)););
        *err_info = FILE_ERROR;
      } else {
        do {
          current_entry = os_readdir(directory);
          /* printf("$" FMT_U_MEM "$\n", (memSizeType) current_entry);
             fflush(stdout); */
        } while (current_entry != NULL && current_entry->d_name[0] == '.' &&
                 (current_entry->d_name[1] == '\0' ||
                  (current_entry->d_name[1] == '.' &&
                   current_entry->d_name[2] == '\0')));
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
              memcpy(from_path, from_name, from_name_size * sizeof(os_charType));
              memcpy(&from_path[from_name_size], pathDelimiter, sizeof(os_charType));
              memcpy(to_path, to_name, to_name_size * sizeof(os_charType));
              memcpy(&to_path[to_name_size], pathDelimiter, sizeof(os_charType));
              init_path = FALSE;
            } /* if */
            memcpy(&from_path[from_name_size + 1], current_entry->d_name,
                   d_name_size * sizeof(os_charType));
            from_path[from_name_size + 1 + d_name_size] = '\0';
            memcpy(&to_path[to_name_size + 1], current_entry->d_name,
                   d_name_size * sizeof(os_charType));
            to_path[to_name_size + 1 + d_name_size] = '\0';
            copy_any_file(from_path, to_path, flags, err_info);
          } else {
            *err_info = MEMORY_ERROR;
          } /* if */
          do {
            current_entry = os_readdir(directory);
            /* printf("$" FMT_U_MEM "$\n", (memSizeType) current_entry);
               fflush(stdout); */
          } while (current_entry != NULL && current_entry->d_name[0] == '.' &&
                   (current_entry->d_name[1] == '\0' ||
                    (current_entry->d_name[1] == '.' &&
                     current_entry->d_name[2] == '\0')));
        } /* while */
        if (unlikely(*err_info != OKAY_NO_ERROR)) {
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
    logFunction(printf("copy_dir(\"" FMT_S_OS "\", \"" FMT_S_OS "\", %d) -->\n",
                       from_name, to_name, *err_info););
  } /* copy_dir */



static void copy_any_file (const const_os_striType from_name,
    const const_os_striType to_name, int flags, errInfoType *err_info)

  {
    os_stat_struct from_stat;
    int from_stat_result;
#if HAS_READLINK && HAS_SYMLINK
    os_striType link_destination;
    ssize_t readlink_result;
#endif
    os_utimbuf_struct to_utime;

  /* copy_any_file */
    logFunction(printf("copy_any_file(\"" FMT_S_OS "\", \"" FMT_S_OS "\")\n",
                       from_name, to_name););
    if (flags & PRESERVE_SYMLINKS) {
      from_stat_result = os_lstat(from_name, &from_stat);
    } else {
      from_stat_result = os_stat(from_name, &from_stat);
    } /* if */
    if (from_stat_result != 0) {
      /* Source file does not exist */
      logError(printf("copy_any_file: os_%sstat(\"" FMT_S_OS "\", *) failed:\n"
                      "errno=%d\nerror: %s\n",
                      flags & PRESERVE_SYMLINKS ? "l" : "",
                      from_name, errno, strerror(errno)););
      *err_info = FILE_ERROR;
    } else {
      if (S_ISLNK(from_stat.st_mode)) {
#if HAS_READLINK && HAS_SYMLINK
        /* printf("link size=%lu\n", from_stat.st_size); */
        if (unlikely(from_stat.st_size < 0 ||
                     (unsigned_os_off_t) from_stat.st_size > MAX_OS_STRI_LEN)) {
          *err_info = RANGE_ERROR;
        } else {
          if (unlikely(!os_stri_alloc(link_destination,
                                      (memSizeType) from_stat.st_size))) {
            *err_info = MEMORY_ERROR;
          } else {
            readlink_result = readlink(from_name, link_destination,
                                       (size_t) from_stat.st_size);
            if (unlikely(readlink_result != -1)) {
              link_destination[readlink_result] = '\0';
              /* printf("readlink_result=%lu\n", readlink_result);
                 printf("link=" FMT_S_OS "\n", link_destination); */
              if (os_symlink(link_destination, to_name) != 0) {
                *err_info = FILE_ERROR;
              } /* if */
            } else {
              *err_info = FILE_ERROR;
            } /* if */
            os_stri_free(link_destination);
          } /* if */
        } /* if */
#elif defined HAS_DEVICE_IO_CONTROL
        winCopySymlink(from_name, to_name, err_info);
#else
        *err_info = FILE_ERROR;
#endif
      } else if (S_ISREG(from_stat.st_mode)) {
        copy_file(from_name, to_name, err_info);
      } else if (S_ISDIR(from_stat.st_mode)) {
        copy_dir(from_name, to_name, flags, err_info);
      } else if (S_ISFIFO(from_stat.st_mode)) {
#if HAS_FIFO_FILES
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
    logFunction(printf("copy_any_file(\"" FMT_S_OS "\", \"" FMT_S_OS "\", %d) -->\n",
                       from_name, to_name, *err_info););
  } /* copy_any_file */



static void move_with_copy (const const_os_striType from_name,
    const const_os_striType to_name, errInfoType *err_info)

  {
    os_striType temp_name;

  /* move_with_copy */
    logFunction(printf("move_with_copy(\"" FMT_S_OS "\", \"" FMT_S_OS "\")\n",
                       from_name, to_name););
    temp_name = temp_name_in_dir(from_name);
    if (unlikely(temp_name == NULL)) {
      *err_info = MEMORY_ERROR;
    } else {
      if (os_rename(from_name, temp_name) != 0) {
        logError(printf("move_with_copy: "
                        "os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        from_name, temp_name, errno, strerror(errno)););
        *err_info = FILE_ERROR;
      } else {
        copy_any_file(temp_name, to_name, PRESERVE_ALL, err_info);
        if (*err_info == OKAY_NO_ERROR) {
          remove_any_file(temp_name, err_info);
        } else {
          /* Rename back to the original name. */
          if (os_rename(temp_name, from_name) != 0) {
            logError(printf("move_with_copy: "
                            "os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                            "errno=%d\nerror: %s\n",
                            temp_name, from_name, errno, strerror(errno)););
          } /* if */
        } /* if */
      } /* if */
      os_stri_free(temp_name);
    } /* if */
    logFunction(printf("move_with_copy(\"" FMT_S_OS "\", \"" FMT_S_OS "\", %d) -->\n",
                       from_name, to_name, *err_info););
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
    logFunction(printf("devices_differ(" FMT_S_OS ", " FMT_S_OS ") --> %d\n",
                       from_name, to_name, differs););
    return differs;
  } /* devices_differ */
#endif



static void move_any_file (const const_os_striType from_name,
    const const_os_striType to_name, errInfoType *err_info)

  {
    os_stat_struct to_stat;

  /* move_any_file */
    logFunction(printf("move_any_file(\"" FMT_S_OS "\", \"" FMT_S_OS "\")\n",
                       from_name, to_name););
    if (os_lstat(to_name, &to_stat) == 0) {
      logError(printf("move_any_file: "
                      "Destination " FMT_S_OS " already exists.\n",
                      to_name););
      *err_info = FILE_ERROR;
    } else {
      if (os_rename(from_name, to_name) != 0) {
        switch (errno) {
#ifdef EXDEV
          case EXDEV:
            /* printf("move_any_file: "
                "os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") triggers EXDEV\n",
                from_name, to_name); */
            move_with_copy(from_name, to_name, err_info);
            break;
#endif
#ifdef USE_EACCES_INSTEAD_OF_EXDEV
          case EACCES:
            /* printf("move_any_file: "
                "os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") triggers EACCES\n",
                from_name, to_name); */
            if (devices_differ(from_name, to_name)) {
              move_with_copy(from_name, to_name, err_info);
            } else {
              logError(printf("move_any_file: "
                              "os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                              "errno=%d\nerror: %s\n",
                              from_name, to_name, EACCES, strerror(EACCES)););
            } /* if */
            break;
#endif
          default:
            logError(printf("move_any_file: "
                            "os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                            "errno=%d\nerror: %s\n",
                            from_name, to_name, errno, strerror(errno)););
            *err_info = FILE_ERROR;
            break;
        } /* switch */
      } /* if */
    } /* if */
    logFunction(printf("move_any_file(\"" FMT_S_OS "\", \"" FMT_S_OS "\", %d) -->\n",
                       from_name, to_name, *err_info););
  } /* move_any_file */



static rtlArrayType addStriToRtlArray (const striType stri,
    rtlArrayType work_array, intType used_max_position)

  {
    rtlArrayType resized_work_array;

  /* addStriToRtlArray */
    logFunction(printf("addStriToRtlArray(\"%s\", *, " FMT_D ")\n",
                       striAsUnquotedCStri(stri), used_max_position););
    if (used_max_position >= work_array->max_position) {
      if (unlikely(work_array->max_position > (intType) (MAX_RTL_ARR_INDEX - ARRAY_SIZE_DELTA) ||
          (resized_work_array = REALLOC_RTL_ARRAY(work_array,
              (uintType) work_array->max_position,
              (uintType) work_array->max_position + ARRAY_SIZE_DELTA)) == NULL)) {
        FREE_STRI(stri, stri->size);
        freeRtlStriArray(work_array, used_max_position);
        work_array = NULL;
      } else {
        work_array = resized_work_array;
        COUNT3_RTL_ARRAY((uintType) work_array->max_position,
            (uintType) work_array->max_position + ARRAY_SIZE_DELTA);
        work_array->max_position += ARRAY_SIZE_DELTA;
        work_array->arr[used_max_position].value.striValue = stri;
      } /* if */
    } else {
      work_array->arr[used_max_position].value.striValue = stri;
    } /* if */
    return work_array;
  } /* addStriToRtlArray */



static rtlArrayType completeRtlStriArray (rtlArrayType work_array,
    intType used_max_position)

  {
    rtlArrayType resized_work_array;

  /* completeRtlStriArray */
    if (likely(work_array != NULL)) {
      resized_work_array = REALLOC_RTL_ARRAY(work_array,
          (uintType) work_array->max_position, (uintType) used_max_position);
      if (unlikely(resized_work_array == NULL)) {
        freeRtlStriArray(work_array, used_max_position);
        work_array = NULL;
      } else {
        work_array = resized_work_array;
        COUNT3_RTL_ARRAY((uintType) work_array->max_position,
                         (uintType) used_max_position);
        work_array->max_position = used_max_position;
      } /* if */
    } /* if */
    return work_array;
  } /* completeRtlStriArray */



static rtlArrayType read_dir (const const_striType dir_name, errInfoType *err_info)

  {
    intType used_max_position;
    dirType directory;
    striType nameStri;
    rtlArrayType dir_array;

  /* read_dir */
    logFunction(printf("read_dir(\"%s\", *)\n",
                       striAsUnquotedCStri(dir_name)););
    if (unlikely((directory = dirOpen(dir_name)) == NULL)) {
      logError(printf("read_dir: dirOpen(\"%s\") failed.\n",
                      striAsUnquotedCStri(dir_name)););
      dir_array = NULL;
      *err_info = FILE_ERROR;
    } else {
      if (likely(ALLOC_RTL_ARRAY(dir_array, INITIAL_ARRAY_SIZE))) {
        dir_array->min_position = 1;
        dir_array->max_position = INITIAL_ARRAY_SIZE;
        used_max_position = 0;
        nameStri = dirRead(directory);
        if (nameStri != NULL) {
          do {
            dir_array = addStriToRtlArray(nameStri, dir_array,
                used_max_position);
            used_max_position++;
            nameStri = dirRead(directory);
          } while (nameStri != NULL && dir_array != NULL);
        } /* if */
        if (unlikely(nameStri != NULL)) {
          FREE_STRI(nameStri, nameStri->size);
        } /* if */
        dir_array = completeRtlStriArray(dir_array, used_max_position);
        if (unlikely(dir_array == NULL)) {
          *err_info = MEMORY_ERROR;
        } /* if */
      } else {
        *err_info = MEMORY_ERROR;
      } /* if */
      dirClose(directory);
    } /* if */
    return dir_array;
  } /* read_dir */



#ifdef os_setenv
static void setEnvironmentVariable (const const_striType name, const const_striType value,
    errInfoType *err_info)

  {
    os_striType env_name;
    os_striType env_value;
    int setenv_result;
    int saved_errno;

  /* setEnvironmentVariable */
    logFunction(printf("setEnvironmentVariable(\"%s\", ",
                       striAsUnquotedCStri(name));
                printf("\"%s\", *)\n",
                       striAsUnquotedCStri(value)););
    env_name = stri_to_os_stri(name, err_info);
    if (likely(env_name != NULL)) {
      env_value = stri_to_os_stri(value, err_info);
      if (likely(env_value != NULL)) {
        setenv_result = os_setenv(env_name, env_value, 1);
        saved_errno = errno;
        os_stri_free(env_value);
        if (unlikely(setenv_result != 0)) {
          logError(printf("setEnvironmentVariable: "
                          "os_setenv(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
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
    logFunction(printf("setEnvironmentVariable -->\n"););
  } /* setEnvironmentVariable */

#else



static void setEnvironmentVariable (const const_striType name, const const_striType value,
    errInfoType *err_info)

  {
    memSizeType stri_size;
    striType stri;
    os_striType env_stri;
    int putenv_result;

  /* setEnvironmentVariable */
    logFunction(printf("setEnvironmentVariable(\"%s\", ",
                       striAsUnquotedCStri(name));
                printf("\"%s\", *)\n",
                       striAsUnquotedCStri(value)););
    if (strChPos(name, (charType) '=') != 0) {
      logError(printf("setEnvironmentVariable(\"%s\", ",
                      striAsUnquotedCStri(name));
               printf("\"%s\", *): "
                      "Name contains '=' (putenv() works with \"name=value\").\n",
                      striAsUnquotedCStri(value)););
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
          if (unlikely(putenv_result != 0)) {
            logError(printf("setEnvironmentVariable: os_putenv(\"" FMT_S_OS "\") failed:\n"
                            "errno=%d\nerror: %s\n",
                            env_stri, errno, strerror(errno)););
            *err_info = RANGE_ERROR;
          } /* if */
#if DELETE_PUTENV_ARGUMENT
          os_stri_free(env_stri);
#endif
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("setEnvironmentVariable -->\n"););
  } /* setEnvironmentVariable */

#endif



#if SEARCH_PATH_DELIMITER != 0
#define getSearchPathDelimiter(path) SEARCH_PATH_DELIMITER
#else



static os_charType getSearchPathDelimiter (os_striType path)

  {
    static os_charType searchPathDelimiter = 0;
    boolType colonFound = FALSE;

  /* getSearchPathDelimiter */
    if (searchPathDelimiter == 0) {
      if (SEARCH_PATH_DELIMITER != 0) {
        searchPathDelimiter = SEARCH_PATH_DELIMITER;
      } else if (path != NULL) {
        /* Try to guess the search path delimiter. */
        if (((path[0] >= 'a' && path[0] <= 'z') ||
             (path[0] >= 'A' && path[0] <= 'Z')) && path[1] == ':') {
          searchPathDelimiter = ';';
        } else {
          while (*path != '\0' && searchPathDelimiter == 0) {
            if (*path == ';' &&
                ((path[1] >= 'a' && path[1] <= 'z') ||
                 (path[1] >= 'A' && path[1] <= 'Z')) && path[2] == ':') {
              searchPathDelimiter = ';';
            } else if (*path == ':') {
              colonFound = TRUE;
            } /* if */
            path++;
          } /* while */
          if (searchPathDelimiter == 0 && colonFound) {
            searchPathDelimiter = ':';
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return searchPathDelimiter;
  } /* getSearchPathDelimiter */

#endif



static rtlArrayType getSearchPath (errInfoType *err_info)

  {
    os_striType search_path_value;
    os_charType searchPathDelimiter;
    memSizeType path_length;
    os_striType path_copy;
    os_striType path_start;
    os_striType path_end;
    striType pathStri;
    intType used_max_position;
    rtlArrayType path_array;

  /* getSearchPath */
    logFunction(printf("getSearchPath(*)\n"););
    search_path_value = os_getenv(path_variable);
    searchPathDelimiter = getSearchPathDelimiter(search_path_value);
    if (unlikely(searchPathDelimiter == 0)) {
      logError(printf("getSearchPath(*): "
                      "Unable to determine the search path delimiter."););
      path_array = NULL;
      *err_info = FILE_ERROR;
    } else if (unlikely(!ALLOC_RTL_ARRAY(path_array, INITIAL_ARRAY_SIZE))) {
      *err_info = MEMORY_ERROR;
    } else {
      path_array->min_position = 1;
      path_array->max_position = INITIAL_ARRAY_SIZE;
      used_max_position = 0;
      if (search_path_value != NULL) {
        /* printf("path: " FMT_S_OS "\n", search_path_value); */
        path_length = os_stri_strlen(search_path_value);
        if (unlikely(!os_stri_alloc(path_copy, path_length))) {
          *err_info = MEMORY_ERROR;
        } else {
          memcpy(path_copy, search_path_value, path_length * sizeof(os_charType));
          path_copy[path_length] = '\0';
          path_start = path_copy;
          do {
            path_end = os_stri_strchr(path_start, searchPathDelimiter);
            if (path_end != NULL) {
              *path_end = '\0';
            } /* if */
            pathStri = cp_from_os_path(path_start, err_info);
            if (likely(pathStri != NULL)) {
              path_array = addStriToRtlArray(pathStri, path_array,
                  used_max_position);
              used_max_position++;
            } /* if */
            if (path_end == NULL) {
              path_start = NULL;
            } else {
              path_start = path_end + 1;
            } /* if */
          } while (path_start != NULL && path_array != NULL);
          os_stri_free(path_copy);
        } /* if */
      } /* if */
      path_array = completeRtlStriArray(path_array, used_max_position);
      if (unlikely(path_array == NULL)) {
        *err_info = MEMORY_ERROR;
      } /* if */
    } /* if */
    if (search_path_value != NULL) {
      os_getenv_string_free(search_path_value);
    } /* if */
    logFunction(printf("getSearchPath --> " FMT_U_MEM
                       " (size=" FMT_U_MEM ", err_info=%d)\n",
                       (memSizeType) path_array,
                       path_array != NULL ? arraySize(path_array) : 0,
                       *err_info););
    return path_array;
  } /* getSearchPath */



static void setSearchPath (rtlArrayType searchPath, errInfoType *err_info)

  {
    os_charType searchPathDelimiter;
    memSizeType numElements;
    memSizeType idx;
    memSizeType length = 0;
    memSizeType pos;
    striType pathElement;
    striType pathStri;
    striType pathVariableName;

  /* setSearchPath */
    logFunction(printf("setSearchPath(" FMT_U_MEM " (size = " FMT_U_MEM "), *)\n",
                       (memSizeType) searchPath,
                       searchPath != NULL ? arraySize(searchPath) : 0););
    searchPathDelimiter = getSearchPathDelimiter(NULL);
    if (searchPathDelimiter == 0) {
      searchPathDelimiter = getSearchPathDelimiter(os_getenv(path_variable));
    } /* if */
    if (unlikely(searchPathDelimiter == 0)) {
      logError(printf("setSearchPath(*): "
                      "Unable to determine the search path delimiter."););
      *err_info = FILE_ERROR;
    } else {
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
            pathStri->mem[pos] = (charType) searchPathDelimiter;
            pos++;
            pathElement = searchPath->arr[idx].value.striValue;
            memcpy(&pathStri->mem[pos], pathElement->mem,
                   pathElement->size * sizeof(strElemType));
            pos += pathElement->size;
          } /* for */
        } /* if */
        pathVariableName = CSTRI_LITERAL_TO_STRI("PATH");
        if (pathVariableName == NULL) {
          *err_info = MEMORY_ERROR;
        } else {
          setEnvironmentVariable(pathVariableName, pathStri, err_info);
          FREE_STRI(pathVariableName, pathVariableName->size);
        } /* if */
        FREE_STRI(pathStri, pathStri->size);
      } /* if */
    } /* if */
    logFunction(printf("setSearchPath --> (err_info=%d)\n",
                       *err_info););
  } /* setSearchPath */



static intType getFileTypeSL (const const_striType filePath, errInfoType *err_info)

  {
    os_striType os_path;
    os_stat_struct stat_buf;
    int stat_result;
    int path_info = PATH_IS_NORMAL;
    int saved_errno;
    intType type_of_file;

  /* getFileTypeSL */
    logFunction(printf("getFileTypeSL(\"%s\")\n",
                       striAsUnquotedCStri(filePath)););
    os_path = cp_to_os_path(filePath, &path_info, err_info);
    if (unlikely(os_path == NULL)) {
#if MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (path_info == PATH_IS_EMULATED_ROOT) {
        /* The emulated root is a directory. Do not raise an exception. */
        *err_info = OKAY_NO_ERROR;
        type_of_file = FILE_DIR;
      } else if (path_info == PATH_NOT_MAPPED) {
        /* The file path cannot be mapped to a system path. */
        /* Do not raise an exception. */
        *err_info = OKAY_NO_ERROR;
        type_of_file = FILE_ABSENT;
      } else
#endif
      {
        logError(printf("getFileTypeSL: cp_to_os_path(\"%s\", *, *) failed:\n"
                        "path_info=%d, err_info=%d\n",
                        striAsUnquotedCStri(filePath), path_info, *err_info););
        type_of_file = FILE_ABSENT;
      }
    } else {
      stat_result = os_lstat(os_path, &stat_buf);
      saved_errno = errno;
      /* printf("lstat(\"" FMT_S_OS "\") returns: %d, errno=%d\n",
          os_path, stat_result, saved_errno); */
      if (stat_result == 0) {
        if (S_ISREG(stat_buf.st_mode)) {
          type_of_file = FILE_REGULAR;
        } else if (S_ISDIR(stat_buf.st_mode)) {
          type_of_file = FILE_DIR;
        } else if (S_ISCHR(stat_buf.st_mode)) {
          type_of_file = FILE_CHAR;
        } else if (S_ISBLK(stat_buf.st_mode)) {
          type_of_file = FILE_BLOCK;
        } else if (S_ISFIFO(stat_buf.st_mode)) {
          type_of_file = FILE_FIFO;
        } else if (S_ISLNK(stat_buf.st_mode)) {
          type_of_file = FILE_SYMLINK;
        } else if (S_ISSOCK(stat_buf.st_mode)) {
          type_of_file = FILE_SOCKET;
        } else {
          type_of_file = FILE_UNKNOWN;
        } /* if */
      } else {
        type_of_file = FILE_ABSENT;
        if (unlikely(filePath->size != 0 && saved_errno != ENOENT &&
                     saved_errno != ENOTDIR && saved_errno != ENAMETOOLONG &&
                     saved_errno != EACCES)) {
          logError(printf("getFileTypeSL: os_lstat(\"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_path, saved_errno, strerror(saved_errno)););
          /* printf("filePath->size=%lu\n", filePath->size); */
          /* printf("strlen(os_path)=%d\n", os_stri_strlen(os_path)); */
          *err_info = FILE_ERROR;
        } /* if */
      } /* if */
      os_stri_free(os_path);
    } /* if */
    logFunction(printf("getFileTypeSL(\"%s\") --> " FMT_D " (err_info=%d)\n",
                       striAsUnquotedCStri(filePath),
                       type_of_file, *err_info););
    return type_of_file;
  } /* getFileTypeSL */



#if HAS_READLINK
/**
 *  Reads the destination of a symbolic link.
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if a memory allocation failed, and
 *                  RANGE_ERROR if the conversion to the system path failed, and
 *                  FILE_ERROR if the file does not exist or is not a symbolic link.
 *  @return The destination referred by the symbolic link, or
 *          NULL if an error occurred.
 */
striType doReadLink (const const_striType filePath, errInfoType *err_info)

  {
    os_striType os_filePath;
    os_stat_struct link_stat;
    memSizeType link_size;
    memSizeType dest_buffer_size;
    os_striType link_destination;
    os_charType buffer[PATH_MAX];
    ssize_t readlink_result;
    int path_info;
    striType destination = NULL;

  /* doReadLink */
    logFunction(printf("doReadLink(\"%s\", %d)\n",
                       striAsUnquotedCStri(filePath), *err_info););
    os_filePath = cp_to_os_path(filePath, &path_info, err_info);
    if (unlikely(os_filePath == NULL)) {
      logError(printf("doReadLink: cp_to_os_path(\"%s\", *, *) failed:\n"
                      "path_info=%d, err_info=%d\n",
                      striAsUnquotedCStri(filePath), path_info, *err_info););
    } else {
      if (unlikely(os_lstat(os_filePath, &link_stat) != 0)) {
        logError(printf("doReadLink: os_lstat(\"" FMT_S_OS "\", *) failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_filePath, errno, strerror(errno)););
        *err_info = FILE_ERROR;
      } else if (unlikely(!S_ISLNK(link_stat.st_mode))) {
        logError(printf("doReadLink(\"%s\", *): "
                        "The file \"" FMT_S_OS "\" is not a symbolic link.\n",
                        striAsUnquotedCStri(filePath), os_filePath););
        *err_info = FILE_ERROR;
      } else if (unlikely(link_stat.st_size < 0 ||
                          (unsigned_os_off_t) link_stat.st_size > MAX_OS_STRI_LEN)) {
        *err_info = FILE_ERROR;
      } else {
        link_size = (memSizeType) link_stat.st_size;
        /* printf("link size=" FMT_U_MEM "\n", link_size); */
        if (link_size < sizeof(buffer)) {
          dest_buffer_size = sizeof(buffer);
          link_destination = buffer;
        } else {
          dest_buffer_size = link_size + NULL_TERMINATION_LEN;
          if (unlikely(!os_stri_alloc(link_destination, link_size))) {
            *err_info = MEMORY_ERROR;
          } /* if */
        } /* if */
        /* The link_destination is oversized by at least one char. */
        /* This simplifies the check for a truncation below.       */
        if (likely(link_destination != NULL)) {
          readlink_result = readlink(os_filePath, link_destination,
                                     dest_buffer_size);
          if (unlikely(readlink_result < 0)) {
            logError(printf("doReadLink: "
                            "readlink(\"" FMT_S_OS "\", *, " FMT_U_MEM ") failed:\n"
                            "errno=%d\nerror: %s\n",
                            os_filePath, dest_buffer_size,
                            errno, strerror(errno)););
            *err_info = FILE_ERROR;
          } else if (unlikely((memSizeType) readlink_result >= dest_buffer_size)) {
            /* If the additional character in the link_destination */
            /* has been used we assume that a truncation occurred. */
            logError(printf("doReadLink: "
                            "readlink(\"" FMT_S_OS "\", *, " FMT_U_MEM ") "
                            "failed with " FMT_U_MEM ":\n"
                            "Link destination possibly truncated "
                            "(link_size=" FMT_U_MEM ").\n",
                            os_filePath, dest_buffer_size,
                            (memSizeType) readlink_result, link_size););
            *err_info = FILE_ERROR;
          } else {
            destination = cp_from_os_path_buffer(link_destination,
                (memSizeType) readlink_result, err_info);
            if (unlikely(destination == NULL)) {
              logError(printf("doReadLink: "
                              "cp_from_os_path(\"" FMT_S_OS "\", *) failed:\n"
                              "err_info=%d\n",
                              link_destination, *err_info););
            } /* if */
          } /* if */
          if (link_destination != buffer) {
            os_stri_free(link_destination);
          } /* if */
        } /* if */
      } /* if */
      os_stri_free(os_filePath);
    } /* if */
    logFunction(printf("doReadLink(\"%s\", %d) --> ",
                       striAsUnquotedCStri(filePath), *err_info);
                printf("\"%s\"\n", striAsUnquotedCStri(destination)););
    return destination;
  } /* doReadLink */
#endif



#ifdef HAS_DO_READ_LINK
static striType readLinkAbsolute (const const_striType filePath, errInfoType *err_info)

  {
    striType destination;
    striType absolutePath;
    striType absoluteDestination;
    intType pathLength;

  /* readLinkAbsolute */
    logFunction(printf("readLinkAbsolute(\"%s\", %d)\n",
                       striAsUnquotedCStri(filePath), *err_info););
    destination = doReadLink(filePath, err_info);
    if (destination != NULL) {
      if (destination->size >= 1 && destination->mem[0] == '/') {
        absoluteDestination = straightenAbsolutePath(destination);
        if (unlikely(absoluteDestination == NULL)) {
          *err_info = MEMORY_ERROR;
          FREE_STRI(destination, destination->size);
          destination = NULL;
        } else {
          FREE_STRI(destination, destination->size);
          destination = absoluteDestination;
        } /* if */
      } else {
        absolutePath = getAbsolutePath(filePath);
        if (unlikely(absolutePath == NULL)) {
          *err_info = MEMORY_ERROR;
          FREE_STRI(destination, destination->size);
          destination = NULL;
        } else {
          pathLength = strRChPos(absolutePath, '/');
          if (pathLength > 1) {
            pathLength--;
          } /* if */
          absolutePath = strHeadAssign(absolutePath, pathLength);
          absoluteDestination = concatPath(absolutePath, destination);
          FREE_STRI(absolutePath, pathLength);
          FREE_STRI(destination, destination->size);
          destination = absoluteDestination;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("readLinkAbsolute(\"%s\", %d) --> ",
                       striAsUnquotedCStri(filePath), *err_info);
                printf("\"%s\"\n", striAsUnquotedCStri(destination)););
    return destination;
  } /* readLinkAbsolute */



striType followLink (striType startPath, errInfoType *err_info)

  {
    striType helpPath;
    int number_of_links_followed = 5;
    striType path;

  /* followLink */
    logFunction(printf("followLink(\"%s\")\n", striAsUnquotedCStri(startPath)););
    if (getFileTypeSL(startPath, err_info) == FILE_SYMLINK) {
      path = readLinkAbsolute(startPath, err_info);
      while (path != NULL &&
             getFileTypeSL(path, err_info) == FILE_SYMLINK &&
             number_of_links_followed != 0) {
        helpPath = path;
        path = readLinkAbsolute(helpPath, err_info);
        FREE_STRI(helpPath, helpPath->size);
        number_of_links_followed--;
      } /* while */
      if (path == NULL || number_of_links_followed != 0) {
        FREE_STRI(startPath, startPath->size);
      } else {
        FREE_STRI(path, path->size);
        path = startPath;
      } /* if */
    } else {
      path = startPath;
    } /* if */
    logFunction(printf("followLink --> \"%s\" (err_info=%d)\n",
                       striAsUnquotedCStri(path), *err_info););
    return path;
  } /* followLink */
#endif



/**
 *  Return the absolute path of the current working directory.
 *  If the size of the provided 'buffer' is not sufficient a buffer
 *  with sufficient size is allocated. The allocated buffer must be
 *  freed afterwards with FREE_OS_STRI().
 *  @param buffer Buffer to be used if its size is sufficient.
 *  @param buffer_size Size of 'buffer'.
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if a memory allocation failed, and
 *                  FILE_ERROR if the operating system function failed.
 *  @return the 'buffer' or an allocated buffer with a null terminated
 *          operating system string.
 */
static os_striType getOsCwd (const os_striType buffer, memSizeType buffer_size,
    errInfoType *err_info)

  {
    os_striType large_buffer = NULL;
    os_striType os_cwd;

  /* getOsCwd */
    logFunction(printf("getOsCwd(*, " FMT_U_MEM ", *)", buffer_size);
                fflush(stdout););
    if (unlikely((os_cwd = os_getcwd(buffer, buffer_size)) == NULL)) {
      if (errno == ERANGE || errno == ENAMETOOLONG) {
        do {
          buffer_size *= 2;
          FREE_OS_STRI(large_buffer);
#ifdef OS_GETCWD_MAX_BUFFER_SIZE
          if (unlikely(buffer_size > OS_GETCWD_MAX_BUFFER_SIZE)) {
            large_buffer = NULL;
            *err_info = MEMORY_ERROR;
          } else
#endif
          if (unlikely(!ALLOC_OS_STRI(large_buffer, buffer_size))) {
            *err_info = MEMORY_ERROR;
          } else {
            /* printf("getOsCwd: os_getcwd(*, " FMT_U_MEM ")\n", buffer_size); */
            if (unlikely((os_cwd = os_getcwd(large_buffer, buffer_size)) == NULL)) {
              if (errno != ERANGE && errno != ENAMETOOLONG) {
                logError(printf("getOsCwd: os_getcwd(*, " FMT_U_MEM ") failed:\n"
                                "errno=%d\nerror: %s\n",
                                buffer_size, errno, strerror(errno)););
                FREE_OS_STRI(large_buffer);
                large_buffer = NULL;
                *err_info = FILE_ERROR;
              } /* if */
            } /* if */
          } /* if */
        } while (os_cwd == NULL && large_buffer != NULL);
      } else {
        logError(printf("getOsCwd: os_getcwd(*, " FMT_U_MEM ") failed:\n"
                        "errno=%d\nerror: %s\n",
                        buffer_size, errno, strerror(errno)););
        *err_info = FILE_ERROR;
      } /* if */
    } /* if */
#ifdef OS_GETCWD_RETURNS_SLASH
    {
      os_charType *ch;

      for (ch = os_cwd; *ch != 0; ch++) {
        if (*ch == '/') {
          *ch = PATH_DELIMITER;
        } /* if */
      } /* for */
    }
#endif
    logFunctionResult(printf("\"" FMT_S_OS "\"\n", os_cwd););
    return os_cwd;
  } /* getOsCwd */



#if defined USE_EXTENDED_LENGTH_PATH && USE_EXTENDED_LENGTH_PATH
void adjustCwdForShell (errInfoType *err_info)

  {
    struct striStruct stri1_buffer;
    striType dotStri;
    os_striType os_cwd;
    int path_info = PATH_IS_NORMAL;
    int chdir_result;

  /* adjustCwdForShell */
    logFunction(printf("adjustCwdForShell(%d)\n", *err_info););
    dotStri = chrStrMacro('.', stri1_buffer);
    os_cwd = cp_to_os_path(dotStri, &path_info, err_info);
    if (unlikely(os_cwd == NULL)) {
      if (likely(path_info == PATH_IS_EMULATED_ROOT)) {
        logError(printf("adjustCwdForShell: Current directory is emulated root.\n"););
      } else {
        logError(printf("adjustCwdForShell: Cannot get current_emulated_cwd.\n"););
      } /* if */
    } else {
      logMessage(printf("adjustCwdForShell: os_chdir(\"" FMT_S_OS "\")\n",
                        &os_cwd[PREFIX_LEN]););
      chdir_result = os_chdir(&os_cwd[PREFIX_LEN]);
      if (unlikely(chdir_result != 0)) {
        logError(printf("adjustCwdForShell: os_chdir(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        &os_cwd[PREFIX_LEN], errno, strerror(errno)););
        *err_info = FILE_ERROR;
      } /* if */
      os_stri_free(os_cwd);
    } /* if */
    logFunction(printf("adjustCwdForShell(%d) -->\n", *err_info););
  } /* adjustCwdForShell */
#endif



#if EMULATE_ROOT_CWD
void initEmulatedCwd (errInfoType *err_info)

  {
    os_charType buffer[PATH_MAX];
    os_striType os_cwd;

  /* initEmulatedCwd */
    logFunction(printf("initEmulatedCwd(*)\n"););
    if ((os_cwd = getOsCwd(buffer, PATH_MAX, err_info)) != NULL) {
      setEmulatedCwd(os_cwd, err_info);
      if (os_cwd != buffer) {
        FREE_OS_STRI(os_cwd);
      } /* if */
    } /* if */
    logFunction(printf("initEmulatedCwd(%d) -->\n", *err_info););
  } /* initEmulatedCwd */
#endif



#if defined USE_EXTENDED_LENGTH_PATH && USE_EXTENDED_LENGTH_PATH
/**
 *  Determine the current working directory of the calling process.
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if a memory allocation failed, and
 *                  FILE_ERROR if the system function returns an error.
 *  @return The absolute path of the current working directory, or
 *          NULL if an error occurred.
 */
striType doGetCwd (errInfoType *err_info)

  {
    struct striStruct stri1_buffer;
    striType dotStri;
    os_striType os_cwd;
    int path_info = PATH_IS_NORMAL;
    striType cwd;

  /* doGetCwd */
    logFunction(printf("doGetCwd\n"););
    dotStri = chrStrMacro('.', stri1_buffer);
    os_cwd = cp_to_os_path(dotStri, &path_info, err_info);
    if (unlikely(os_cwd == NULL)) {
#if EMULATE_ROOT_CWD
      if (likely(path_info == PATH_IS_EMULATED_ROOT)) {
        cwd = cp_from_os_path(current_emulated_cwd, err_info);
        if (unlikely(cwd == NULL)) {
          logError(printf("doGetCwd: "
                          "cp_from_os_path(\"" FMT_S_OS "\", *) failed:\n"
                          "err_info=%d\n",
                          current_emulated_cwd, *err_info););
        } /* if */
      } else
#endif
      {
        logError(printf("doGetCwd: Cannot get current_emulated_cwd.\n"););
        cwd = NULL;
      }
    } else {
      cwd = cp_from_os_path(os_cwd, err_info);
      if (unlikely(cwd == NULL)) {
        logError(printf("doGetCwd: "
                        "cp_from_os_path(\"" FMT_S_OS "\", *) failed:\n"
                        "err_info=%d\n",
                        os_cwd, *err_info););
      } /* if */
      os_stri_free(os_cwd);
    } /* if */
    logFunction(printf("doGetCwd --> \"%s\" (err_info=%d)\n",
                       striAsUnquotedCStri(cwd), *err_info););
    return cwd;
  } /* doGetCwd */

#else



/**
 *  Determine the current working directory of the calling process.
 *  @param err_info Unchanged if the function succeeds, and
 *                  MEMORY_ERROR if a memory allocation failed, and
 *                  FILE_ERROR if the system function returns an error.
 *  @return The absolute path of the current working directory, or
 *          NULL if an error occurred.
 */
striType doGetCwd (errInfoType *err_info)

  {
    os_charType buffer[PATH_MAX];
    os_striType os_cwd;
    striType cwd;

  /* doGetCwd */
    logFunction(printf("doGetCwd\n"););
#if EMULATE_ROOT_CWD
    if (IS_EMULATED_ROOT(current_emulated_cwd)) {
      cwd = cp_from_os_path(current_emulated_cwd, err_info);
      if (unlikely(cwd == NULL)) {
        logError(printf("doGetCwd: "
                        "cp_from_os_path(\"" FMT_S_OS "\", *) failed:\n"
                        "err_info=%d\n",
                        current_emulated_cwd, *err_info););
      } /* if */
    } else
#endif
    {
      if (unlikely((os_cwd = getOsCwd(buffer, PATH_MAX, err_info)) == NULL)) {
        cwd = NULL;
      } else {
        cwd = cp_from_os_path(os_cwd, err_info);
        if (unlikely(cwd == NULL)) {
          logError(printf("doGetCwd: "
                          "cp_from_os_path(\"" FMT_S_OS "\", *) failed:\n"
                          "err_info=%d\n",
                          os_cwd, *err_info););
        } /* if */
        if (os_cwd != buffer) {
          FREE_OS_STRI(os_cwd);
        } /* if */
      } /* if */
    }
    logFunction(printf("doGetCwd --> \"%s\" (err_info=%d)\n",
                       striAsUnquotedCStri(cwd), *err_info););
    return cwd;
  } /* doGetCwd */

#endif



/**
 *  Get the absolute file path in the standard path representation.
 *  @return the absolute file path in the standard path
 *          representation, or NULL if the memory allocation failed.
 */
striType getAbsolutePath (const const_striType aPath)

  {
    striType cwd;
    errInfoType err_info = OKAY_NO_ERROR;
    striType absolutePath;

  /* getAbsolutePath */
    logFunction(printf("getAbsolutePath(\"%s\")\n",
                       striAsUnquotedCStri(aPath)););
    if (aPath->size >= 1 &&
        aPath->mem[0] == (charType) '/') {
      absolutePath = straightenAbsolutePath(aPath);
    } else {
      cwd = doGetCwd(&err_info);
      if (unlikely(cwd == NULL)) {
        absolutePath = NULL;
      } else {
        absolutePath = concatPath(cwd, aPath);
        FREE_STRI(cwd, cwd->size);
      } /* if */
    } /* if */
    logFunction(printf("getAbsolutePath --> \"%s\"\n",
                       striAsUnquotedCStri(absolutePath)););
    return absolutePath;
  } /* getAbsolutePath */



#ifdef DEFINE_SYSTEM_FUNCTION
static int systemForNodeJs (const char *command)

  {
    int result;

  /* systemForNodeJs */
    logFunction(printf("systemForNodeJs(\"%s\")\n", command););
    result = EM_ASM_INT({
      let cmd = Module.UTF8ToString($0);
      // console.log("I received: " + cmd);
      if (typeof require === "function") {
        var child_process;
        try {
          child_process = require("child_process");
        } catch (e) {
          child_process = null;
        }
        if (child_process !== null) {
          let bslash = String.fromCharCode(92);
          let currDir = process.cwd().replace(new RegExp(bslash + bslash, "g"), "/");
          let newDir = FS.cwd();
          // console.log("emcc cwd: " + newDir);
          // console.log("node cwd: " + currDir);
          if (currDir.charAt(1) === ":" && currDir.charAt(2) === "/") {
            if (newDir !== "/") {
              if (newDir.charAt(0) === "/" && newDir.charAt(1).match(/[a-z]/i) && newDir.charAt(2) === "/") {
                newDir = newDir.charAt(1) + ":" + newDir.substring(2);
              }
            }
          }
          try {
            process.chdir(newDir);
          } catch (e) {
            // console.log("chdir: " + e);
          }
          try {
            child_process.execSync(cmd);
            return 0;
          } catch (e) {
            return -1;
          }
        }
      }
      return -1;
    }, command);
    return result;
  } /* systemForNodeJs */

#ifdef os_system
#undef os_system
#endif
#define os_system(cmd) systemForNodeJs(cmd)
#endif



/**
 *  Determine the size of a file.
 *  The function follows symbolic links. The file size is measured in bytes.
 *  For directories, fifos and sockets a size of 0 is returned.
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
    cFileType aFile;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
    bigIntType size_of_file;

  /* cmdBigFileSize */
    logFunction(printf("cmdBigFileSize(\"%s\")", striAsUnquotedCStri(filePath));
                fflush(stdout););
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(os_path == NULL)) {
#if MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (likely(path_info == PATH_IS_EMULATED_ROOT)) {
        size_of_file = bigIConv(0);
      } else
#endif
      {
        logError(printf("cmdBigFileSize: cp_to_os_path(\"%s\", *, *) failed:\n"
                        "path_info=%d, err_info=%d\n",
                        striAsUnquotedCStri(filePath), path_info, err_info););
        raise_error(err_info);
        size_of_file = NULL;
      }
    } else {
      stat_result = os_stat(os_path, &stat_buf);
      if (stat_result == 0 && S_ISREG(stat_buf.st_mode)) {
#if OS_OFF_T_SIZE == 32
        size_of_file = bigFromUInt32((uint32Type) stat_buf.st_size);
#elif OS_OFF_T_SIZE == 64
        size_of_file = bigFromUInt64((uint64Type) stat_buf.st_size);
#else
#error "sizeof(os_off_t) is neither 4 nor 8."
#endif
      } else if (stat_result == 0 && (S_ISDIR(stat_buf.st_mode) ||
                                      S_ISFIFO(stat_buf.st_mode) ||
                                      S_ISSOCK(stat_buf.st_mode))) {
        size_of_file = bigIConv(0);
      } else {
        aFile = os_fopen(os_path, os_mode_rb);
        if (aFile == NULL) {
          logError(printf("cmdBigFileSize: "
                          "os_fopen(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_path, os_mode_rb, errno, strerror(errno)););
          /* if (stat_result == 0) {
            printf("stat_buf.st_blksize=%lu\n", stat_buf.st_blksize);
            printf("stat_buf.st_blocks=%llu\n", stat_buf.st_blocks);
            printf("stat_buf.st_size=%llu\n", stat_buf.st_size);
          } */
          err_info = FILE_ERROR;
          size_of_file = NULL;
        } else {
          size_of_file = getBigFileLengthUsingSeek(aFile);
          fclose(aFile);
        } /* if */
      } /* if */
      os_stri_free(os_path);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
    return size_of_file;
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
    logFunction(printf("cmdChdir(\"%s\")\n", striAsUnquotedCStri(dirPath)););
    os_path = cp_to_os_path(dirPath, &path_info, &err_info);
    if (unlikely(os_path == NULL)) {
#if MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (likely(path_info == PATH_IS_EMULATED_ROOT)) {
        setEmulatedCwdToRoot();
      } else {
        logError(printf("cmdChdir: cp_to_os_path(\"%s\", *, *) failed:\n"
                        "path_info=%d, err_info=%d\n",
                        striAsUnquotedCStri(dirPath), path_info, err_info););
        raise_error(err_info);
      } /* if */
    } else {
#if defined USE_EXTENDED_LENGTH_PATH && USE_EXTENDED_LENGTH_PATH
      /* The extended length path is only used, if it */
      /* is absolutely necessary. Many subprocesses   */
      /* cannot handle a current working directory    */
      /* with an extended length path.                */
      if (os_stri_strlen(&os_path[PREFIX_LEN]) < CLASSIC_WINDOWS_MAX_PATH) {
        logMessage(printf("cmdChdir: os_chdir(\"" FMT_S_OS "\")\n",
                          &os_path[PREFIX_LEN]););
        chdir_result = os_chdir(&os_path[PREFIX_LEN]);
      } else {
        logMessage(printf("cmdChdir: os_chdir(\"" FMT_S_OS "\")\n",
                          os_path););
        chdir_result = os_chdir(os_path);
      } /* if */
      if (unlikely(chdir_result != 0)) {
        logMessage(printf("cmdChdir: os_chdir(\"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_path, errno, strerror(errno)););
        setEmulatedCwd(os_path, &err_info);
#else
      logMessage(printf("cmdChdir: os_chdir(\"" FMT_S_OS "\")\n",
                        os_path););
      chdir_result = os_chdir(os_path);
      if (unlikely(chdir_result != 0)) {
        logError(printf("cmdChdir: os_chdir(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_path, errno, strerror(errno)););
        err_info = FILE_ERROR;
#endif
      } else {
        setEmulatedCwd(os_path, &err_info);
      } /* if */
#else
      logError(printf("cmdChdir: cp_to_os_path(\"%s\", *, *) failed:\n"
                      "path_info=%d, err_info=%d\n",
                      striAsUnquotedCStri(dirPath), path_info, err_info););
      raise_error(err_info);
    } else {
      logMessage(printf("cmdChdir: os_chdir(\"" FMT_S_OS "\")\n",
                        os_path););
      chdir_result = os_chdir(os_path);
      if (unlikely(chdir_result != 0)) {
        logError(printf("cmdChdir: os_chdir(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_path, errno, strerror(errno)););
        err_info = FILE_ERROR;
      } /* if */
#endif
      os_stri_free(os_path);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
    logFunction(printf("cmdChdir(\"%s\") -->\n",
                       striAsUnquotedCStri(dirPath)););
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
    logFunction(printf("cmdCloneFile(\"%s\", ", striAsUnquotedCStri(sourcePath));
                printf("\"%s\")\n", striAsUnquotedCStri(destPath)););
    os_sourcePath = cp_to_os_path(sourcePath, &path_info, &err_info);
    if (unlikely(os_sourcePath == NULL)) {
      logError(printf("cmdCloneFile: cp_to_os_path(\"%s\", *, *) failed:\n"
                      "path_info=%d, err_info=%d\n",
                      striAsUnquotedCStri(sourcePath), path_info, err_info););
    } else {
      os_destPath = cp_to_os_path(destPath, &path_info, &err_info);
      if (unlikely(os_destPath == NULL)) {
        logError(printf("cmdCloneFile: cp_to_os_path(\"%s\", *, *) failed:\n"
                        "path_info=%d, err_info=%d\n",
                        striAsUnquotedCStri(destPath), path_info, err_info););
      } else {
        if (os_lstat(os_destPath, &to_stat) == 0) {
          logError(printf("cmdCloneFile: "
                          "Destination " FMT_S_OS " already exists.\n",
                          os_destPath););
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



/**
 *  Get a built-in C compiler/runtime configuration value.
 *  The makefile (used to compile Seed7) and the program chkccomp.c
 *  write the configuration values to version.h. The configuration
 *  values are hard-coded in the Seed7 runtime library.
 *  @param name Name of the configuration value to be retrieved.
 *  @return the requested configuration value, and
 *          "" if a value with the name does not exist.
 *  @exception MEMORY_ERROR Not enough memory to convert the
 *             configuration value to a string.
 */
striType cmdConfigValue (const const_striType name)

  {
    char opt_name[MAX_CSTRI_BUFFER_LEN + NULL_TERMINATION_LEN];
    const_cstriType opt;
    char buffer[CONFIG_VALUE_BUFFER_SIZE];
    striType configValue = NULL;

  /* cmdConfigValue */
    if (unlikely(name->size > MAX_CSTRI_BUFFER_LEN)) {
      opt = "";
    } else if (unlikely(conv_to_cstri(opt_name, name) == NULL)) {
      opt = "";
    } else if (strcmp(opt_name, "OBJECT_FILE_EXTENSION") == 0) {
      opt = OBJECT_FILE_EXTENSION;
    } else if (strcmp(opt_name, "LIBRARY_FILE_EXTENSION") == 0) {
      opt = LIBRARY_FILE_EXTENSION;
    } else if (strcmp(opt_name, "EXECUTABLE_FILE_EXTENSION") == 0) {
      opt = EXECUTABLE_FILE_EXTENSION;
    } else if (strcmp(opt_name, "LINKED_PROGRAM_EXTENSION") == 0) {
      opt = LINKED_PROGRAM_EXTENSION;
    } else if (strcmp(opt_name, "C_COMPILER") == 0) {
      opt = C_COMPILER;
    } else if (strcmp(opt_name, "CPLUSPLUS_COMPILER") == 0) {
      opt = CPLUSPLUS_COMPILER;
    } else if (strcmp(opt_name, "CALL_C_COMPILER_FROM_SHELL") == 0) {
      opt = CALL_C_COMPILER_FROM_SHELL ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "C_COMPILER_VERSION") == 0) {
      opt = C_COMPILER_VERSION;
    } else if (strcmp(opt_name, "CC_OPT_DEBUG_INFO") == 0) {
      opt = CC_OPT_DEBUG_INFO;
    } else if (strcmp(opt_name, "CC_OPT_LINK_TIME_OPTIMIZATION") == 0) {
      opt = CC_OPT_LINK_TIME_OPTIMIZATION;
    } else if (strcmp(opt_name, "LINKER_OPT_LTO_MANDATORY") == 0) {
      opt = LINKER_OPT_LTO_MANDATORY ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "CC_OPT_NO_WARNINGS") == 0) {
      opt = CC_OPT_NO_WARNINGS;
    } else if (strcmp(opt_name, "CC_OPT_OPTIMIZE_1") == 0) {
      opt = CC_OPT_OPTIMIZE_1;
    } else if (strcmp(opt_name, "CC_OPT_OPTIMIZE_2") == 0) {
      opt = CC_OPT_OPTIMIZE_2;
    } else if (strcmp(opt_name, "CC_OPT_OPTIMIZE_3") == 0) {
      opt = CC_OPT_OPTIMIZE_3;
    } else if (strcmp(opt_name, "CC_OPT_TRAP_OVERFLOW") == 0) {
      opt = CC_OPT_TRAP_OVERFLOW;
    } else if (strcmp(opt_name, "CC_OPT_VERSION_INFO") == 0) {
      opt = CC_OPT_VERSION_INFO;
    } else if (strcmp(opt_name, "CC_ENVIRONMENT_INI") == 0) {
      opt = CC_ENVIRONMENT_INI;
    } else if (strcmp(opt_name, "CC_FLAGS") == 0) {
      opt = CC_FLAGS;
    } else if (strcmp(opt_name, "CC_ERROR_FILEDES") == 0) {
      opt = STRINGIFY(CC_ERROR_FILEDES);
    } else if (strcmp(opt_name, "CC_VERSION_INFO_FILEDES") == 0) {
      opt = STRINGIFY(CC_VERSION_INFO_FILEDES);
    } else if (strcmp(opt_name, "LINKER_OPT_DEBUG_INFO") == 0) {
      opt = LINKER_OPT_DEBUG_INFO;
    } else if (strcmp(opt_name, "LINKER_OPT_NO_DEBUG_INFO") == 0) {
      opt = LINKER_OPT_NO_DEBUG_INFO;
    } else if (strcmp(opt_name, "LINKER_OPT_OUTPUT_FILE") == 0) {
      opt = LINKER_OPT_OUTPUT_FILE;
    } else if (strcmp(opt_name, "LINKER_OPT_SPECIAL_LIB") == 0) {
      opt = LINKER_OPT_SPECIAL_LIB;
    } else if (strcmp(opt_name, "LINKER_OPT_STACK_SIZE") == 0) {
      opt = LINKER_OPT_STACK_SIZE;
    } else if (strcmp(opt_name, "LINKER_FLAGS") == 0) {
      opt = LINKER_FLAGS;
    } else if (strcmp(opt_name, "DEFAULT_STACK_SIZE") == 0) {
      opt = STRINGIFY(DEFAULT_STACK_SIZE);
    } else if (strcmp(opt_name, "SYSTEM_LIBS") == 0) {
      opt = SYSTEM_LIBS;
    } else if (strcmp(opt_name, "SYSTEM_BIGINT_LIBS") == 0) {
      opt = SYSTEM_BIGINT_LIBS;
    } else if (strcmp(opt_name, "SYSTEM_CONSOLE_LIBS") == 0) {
      opt = SYSTEM_CONSOLE_LIBS;
    } else if (strcmp(opt_name, "SYSTEM_DATABASE_LIBS") == 0) {
      opt = SYSTEM_DATABASE_LIBS;
    } else if (strcmp(opt_name, "SYSTEM_DRAW_LIBS") == 0) {
      opt = SYSTEM_DRAW_LIBS;
    } else if (strcmp(opt_name, "SYSTEM_MATH_LIBS") == 0) {
      opt = SYSTEM_MATH_LIBS;
    } else if (strcmp(opt_name, "SEED7_LIB") == 0) {
      opt = SEED7_LIB;
    } else if (strcmp(opt_name, "DRAW_LIB") == 0) {
      opt = DRAW_LIB;
    } else if (strcmp(opt_name, "CONSOLE_LIB") == 0) {
      opt = CONSOLE_LIB;
    } else if (strcmp(opt_name, "DATABASE_LIB") == 0) {
      opt = DATABASE_LIB;
    } else if (strcmp(opt_name, "COMP_DATA_LIB") == 0) {
      opt = COMP_DATA_LIB;
    } else if (strcmp(opt_name, "COMPILER_LIB") == 0) {
      opt = COMPILER_LIB;
    } else if (strcmp(opt_name, "SPECIAL_LIB") == 0) {
      opt = SPECIAL_LIB;
    } else if (strcmp(opt_name, "S7_LIB_DIR") == 0) {
      opt = S7_LIB_DIR;
    } else if (strcmp(opt_name, "VERSION_REVISION_LEVEL") == 0) {
      opt = STRINGIFY(LEVEL);
    } else if (strcmp(opt_name, "REDIRECT_FILEDES_1") == 0) {
      opt = REDIRECT_FILEDES_1;
    } else if (strcmp(opt_name, "REDIRECT_FILEDES_2") == 0) {
      opt = REDIRECT_FILEDES_2;
    } else if (strcmp(opt_name, "NULL_DEVICE") == 0) {
      opt = NULL_DEVICE_FOR_SCRIPTS;
    } else if (strcmp(opt_name, "BOOLTYPE") == 0) {
      opt = BOOLTYPE_STRI;
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
      opt = STRINGIFY(INT_SIZE);
    } else if (strcmp(opt_name, "LONG_SIZE") == 0) {
      opt = STRINGIFY(LONG_SIZE);
    } else if (strcmp(opt_name, "FLOATTYPE_MANTISSA_BITS") == 0) {
      sprintf(buffer, "%d", FLOATTYPE_MANTISSA_BITS);
      opt = buffer;
    } else if (strcmp(opt_name, "FLOATTYPE_EXPONENT_OFFSET") == 0) {
      sprintf(buffer, "%d", FLOATTYPE_EXPONENT_OFFSET);
      opt = buffer;
    } else if (strcmp(opt_name, "INT_RANGE_IN_FLOATTYPE_MAX") == 0) {
      sprintf(buffer, FMT_D, INT_RANGE_IN_FLOATTYPE_MAX);
      opt = buffer;
    } else if (strcmp(opt_name, "MINIMUM_TRUNC_ARGUMENT") == 0) {
#if FLOATTYPE_DOUBLE && INTTYPE_SIZE == 64
      opt = STRINGIFY(MINIMUM_TRUNC_ARGUMENT);
#else
      opt = "";
#endif
    } else if (strcmp(opt_name, "MAXIMUM_TRUNC_ARGUMENT") == 0) {
#if FLOATTYPE_DOUBLE && INTTYPE_SIZE == 64
      opt = STRINGIFY(MAXIMUM_TRUNC_ARGUMENT);
#else
      opt = "";
#endif
    } else if (strcmp(opt_name, "PIXEL_RED_MASK") == 0) {
      opt = PIXEL_RED_MASK;
    } else if (strcmp(opt_name, "PIXEL_GREEN_MASK") == 0) {
      opt = PIXEL_GREEN_MASK;
    } else if (strcmp(opt_name, "PIXEL_BLUE_MASK") == 0) {
      opt = PIXEL_BLUE_MASK;
    } else if (strcmp(opt_name, "RGB_TO_PIXEL_FLAG_NAME") == 0) {
      opt = RGB_TO_PIXEL_FLAG_NAME;
    } else if (strcmp(opt_name, "RAND_MULTIPLIER") == 0) {
      opt = STRINGIFY(RAND_MULTIPLIER);
    } else if (strcmp(opt_name, "RAND_INCREMENT") == 0) {
      opt = STRINGIFY(RAND_INCREMENT);
    } else if (strcmp(opt_name, "MACRO_DEFS") == 0) {
      /* Use string literal concatenation: */
      opt = MACRO_DEFS  OS_ISNAN_DEFINITION;
    } else if (strcmp(opt_name, "OVERFLOW_SIGNAL") == 0) {
      opt = OVERFLOW_SIGNAL_STR;
    } else if (strcmp(opt_name, "BUILTIN_ADD_OVERFLOW") == 0) {
      opt = BUILTIN_ADD_OVERFLOW;
    } else if (strcmp(opt_name, "BUILTIN_SUB_OVERFLOW") == 0) {
      opt = BUILTIN_SUB_OVERFLOW;
    } else if (strcmp(opt_name, "BUILTIN_MULT_OVERFLOW") == 0) {
      opt = BUILTIN_MULT_OVERFLOW;
    } else if (strcmp(opt_name, "INT_DIV_OVERFLOW") == 0) {
      opt = STRINGIFY(INT_DIV_OVERFLOW);
    } else if (strcmp(opt_name, "INT_REM_OVERFLOW") == 0) {
      opt = STRINGIFY(INT_REM_OVERFLOW);
    } else if (strcmp(opt_name, "FLOATTYPE_DOUBLE") == 0) {
      opt = FLOATTYPE_DOUBLE ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "HAS_SIGSETJMP") == 0) {
      opt = HAS_SIGSETJMP ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "CHECK_INT_DIV_BY_ZERO") == 0) {
      opt = CHECK_INT_DIV_BY_ZERO ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "CHECK_INT_DIV_ZERO_BY_ZERO") == 0) {
      opt = CHECK_INT_DIV_ZERO_BY_ZERO ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "CHECK_INT_REM_BY_ZERO") == 0) {
      opt = CHECK_INT_REM_BY_ZERO ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "CHECK_INT_REM_ZERO_BY_ZERO") == 0) {
      opt = CHECK_INT_REM_ZERO_BY_ZERO ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "HAS_EXP2") == 0) {
      opt = HAS_EXP2 ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "HAS_EXP10") == 0) {
      opt = HAS_EXP10 ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "HAS_EXPM1") == 0) {
      opt = HAS_EXPM1 ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "HAS_CBRT") == 0) {
      opt = HAS_CBRT ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "CHECK_FLOAT_DIV_BY_ZERO") == 0) {
      opt = CHECK_FLOAT_DIV_BY_ZERO ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "FLOAT_COMPARISON_OKAY") == 0) {
      opt = FLOAT_COMPARISON_OKAY ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "WITH_STRI_CAPACITY") == 0) {
      opt = WITH_STRI_CAPACITY ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "ALLOW_STRITYPE_SLICES") == 0) {
      opt = ALLOW_STRITYPE_SLICES ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "ALLOW_BSTRITYPE_SLICES") == 0) {
      opt = ALLOW_BSTRITYPE_SLICES ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "RSHIFT_DOES_SIGN_EXTEND") == 0) {
      opt = RSHIFT_DOES_SIGN_EXTEND ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "TWOS_COMPLEMENT_INTTYPE") == 0) {
      opt = TWOS_COMPLEMENT_INTTYPE ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "LITTLE_ENDIAN_INTTYPE") == 0) {
      opt = LITTLE_ENDIAN_INTTYPE ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "POW_FUNCTION_OKAY") == 0) {
      opt = POW_FUNCTION_OKAY ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "SQRT_FUNCTION_OKAY") == 0) {
      opt = SQRT_FUNCTION_OKAY ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "EXP_FUNCTION_OKAY") == 0) {
      opt = EXP_FUNCTION_OKAY ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "LOG_FUNCTION_OKAY") == 0) {
      opt = LOG_FUNCTION_OKAY ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "LOG10_FUNCTION_OKAY") == 0) {
      opt = LOG10_FUNCTION_OKAY ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "LOG1P_FUNCTION_OKAY") == 0) {
      opt = LOG1P_FUNCTION_OKAY ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "LOG2_FUNCTION_OKAY") == 0) {
      opt = LOG2_FUNCTION_OKAY ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "FMOD_FUNCTION_OKAY") == 0) {
      opt = FMOD_FUNCTION_OKAY ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "LDEXP_FUNCTION_OKAY") == 0) {
      opt = LDEXP_FUNCTION_OKAY ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "FREXP_FUNCTION_OKAY") == 0) {
      opt = FREXP_FUNCTION_OKAY ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "FLOAT_ZERO_DIV_ERROR") == 0) {
      opt = FLOAT_ZERO_DIV_ERROR ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "LIMITED_CSTRI_LITERAL_LEN") == 0) {
      opt = LIMITED_CSTRI_LITERAL_LEN ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "SWITCH_WORKS_FOR_INT64TYPE") == 0) {
      opt = SWITCH_WORKS_FOR_INT64TYPE ? "TRUE" : "FALSE";
    } else if (strcmp(opt_name, "STMT_BLOCK_IN_PARENTHESES_OK") == 0) {
      opt = STMT_BLOCK_IN_PARENTHESES_OK ? "TRUE" : "FALSE";
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
    } else if (strcmp(opt_name, "USE_DO_EXIT") == 0) {
#ifdef USE_DO_EXIT
      opt = "TRUE";
#else
      opt = "FALSE";
#endif
    } else {
      opt = "";
    } /* if */
    if (configValue == NULL) {
      configValue = cstri8_or_cstri_to_stri(opt);
    } /* if */
    if (unlikely(configValue == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return configValue;
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
    logFunction(printf("cmdCopyFile(\"%s\", ", striAsUnquotedCStri(sourcePath));
                printf("\"%s\")\n", striAsUnquotedCStri(destPath)););
    os_sourcePath = cp_to_os_path(sourcePath, &path_info, &err_info);
    if (unlikely(os_sourcePath == NULL)) {
      logError(printf("cmdCopyFile: cp_to_os_path(\"%s\", *, *) failed:\n"
                      "path_info=%d, err_info=%d\n",
                      striAsUnquotedCStri(sourcePath), path_info, err_info););
    } else {
      os_destPath = cp_to_os_path(destPath, &path_info, &err_info);
      if (unlikely(os_destPath == NULL)) {
        logError(printf("cmdCopyFile: cp_to_os_path(\"%s\", *, *) failed:\n"
                        "path_info=%d, err_info=%d\n",
                        striAsUnquotedCStri(destPath), path_info, err_info););
      } else {
        if (os_lstat(os_destPath, &to_stat) == 0) {
          logError(printf("cmdCopyFile: "
                          "Destination " FMT_S_OS " already exists.\n",
                          os_destPath););
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
#if USE_GET_ENVIRONMENT
    os_striType *os_environ;
#elif INITIALIZE_OS_ENVIRON
    static const os_charType empty_os_stri[] = {0};
#endif
    intType used_max_position;
    os_striType *nameStartPos;
    os_striType nameEndPos;
    striType variableName;
    errInfoType err_info = OKAY_NO_ERROR;
    rtlArrayType environment_array;

  /* cmdEnvironment */
    logFunction(printf("cmdEnvironment()\n"););
#if USE_GET_ENVIRONMENT
    os_environ = getEnvironment();
#elif INITIALIZE_OS_ENVIRON
    if (os_environ == NULL) {
      (void) os_getenv(empty_os_stri);
    } /* if */
#endif
    if (unlikely(!ALLOC_RTL_ARRAY(environment_array, INITIAL_ARRAY_SIZE))) {
      raise_error(MEMORY_ERROR);
    } else {
      environment_array->min_position = 1;
      environment_array->max_position = INITIAL_ARRAY_SIZE;
      used_max_position = 0;
      if (os_environ != NULL) {
        for (nameStartPos = os_environ; *nameStartPos != NULL && environment_array != NULL;
             ++nameStartPos) {
          /* printf("nameStartPos: \"" FMT_S_OS "\"\n", *nameStartPos); */
          if ((*nameStartPos)[0] != '=' && (*nameStartPos)[0] != '\0') {
            nameEndPos = os_stri_strchr(*nameStartPos, '=');
            if (unlikely(nameEndPos == NULL)) {
              nameEndPos = *nameStartPos + os_stri_strlen(*nameStartPos);
            } /* if */
            variableName = conv_from_os_stri(*nameStartPos,
                                             (memSizeType) (nameEndPos - *nameStartPos));
            if (unlikely(variableName == NULL)) {
              err_info = MEMORY_ERROR;
            } else {
              environment_array = addStriToRtlArray(variableName, environment_array,
                  used_max_position);
              used_max_position++;
            } /* if */
          } /* if */
        } /* for */
      } /* if */
      environment_array = completeRtlStriArray(environment_array, used_max_position);
      if (unlikely(environment_array == NULL)) {
        err_info = MEMORY_ERROR;
      } /* if */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
        environment_array = NULL;
      } /* if */
    } /* if */
#if USE_GET_ENVIRONMENT
    freeEnvironment(os_environ);
#endif
    logFunction(printf("cmdEnvironment --> " FMT_U_MEM
                       " (size=" FMT_U_MEM ")\n",
                       (memSizeType) environment_array,
                       environment_array != NULL ?
                       arraySize(environment_array) : 0););
    return environment_array;
  } /* cmdEnvironment */



/**
 *  Determine the size of a file.
 *  The function follows symbolic links. The file size is measured in bytes.
 *  For directories, fifos and sockets a size of 0 is returned.
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
    cFileType aFile;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
    intType size_of_file;

  /* cmdFileSize */
    logFunction(printf("cmdFileSize(\"%s\")", striAsUnquotedCStri(filePath));
                fflush(stdout););
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(os_path == NULL)) {
#if MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (unlikely(path_info != PATH_IS_EMULATED_ROOT))
#endif
      {
        logError(printf("cmdFileSize: cp_to_os_path(\"%s\", *, *) failed:\n"
                        "path_info=%d, err_info=%d\n",
                        striAsUnquotedCStri(filePath), path_info, err_info););
        raise_error(err_info);
      }
      size_of_file = 0;
    } else {
      stat_result = os_stat(os_path, &stat_buf);
      if (stat_result == 0 && S_ISREG(stat_buf.st_mode)) {
        if (stat_buf.st_size > INTTYPE_MAX || stat_buf.st_size < 0) {
          logError(printf("cmdFileSize(\"%s\"): "
                          "File size not representable as integer.\n",
                          striAsUnquotedCStri(filePath)););
          err_info = RANGE_ERROR;
          size_of_file = 0;
        } else {
          size_of_file = (intType) stat_buf.st_size;
        } /* if */
      } else if (stat_result == 0 && (S_ISDIR(stat_buf.st_mode) ||
                                      S_ISFIFO(stat_buf.st_mode) ||
                                      S_ISSOCK(stat_buf.st_mode))) {
        size_of_file = 0;
      } else {
        /* printf("stat_result=%d\nerrno=%d\n", stat_result, errno); */
        aFile = os_fopen(os_path, os_mode_rb);
        if (aFile == NULL) {
          logError(printf("cmdFileSize: "
                          "os_fopen(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_path, os_mode_rb, errno, strerror(errno)););
          /* if (stat_result == 0) {
            printf("stat_buf.st_blksize=%lu\n", stat_buf.st_blksize);
            printf("stat_buf.st_blocks=" FMT_U64 "\n", (uint64Type) stat_buf.st_blocks);
            printf("stat_buf.st_size=" FMT_U64 "\n", (uint64Type) stat_buf.st_size);
          } */
          err_info = FILE_ERROR;
          size_of_file = 0;
        } else {
          size_of_file = getFileLengthUsingSeek(aFile);
          fclose(aFile);
        } /* if */
      } /* if */
      os_stri_free(os_path);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
    logFunctionResult(printf("" FMT_D "\n", size_of_file););
    return size_of_file;
  } /* cmdFileSize */



/**
 *  Determine the type of a file.
 *  The function does follow symbolic links. If the chain of
 *  symbolic links is too long the function returns 'FILE_SYMLINK'.
 *  If a symbolic link refers to a place where the permission
 *  is denied the function returns 'FILE_SYMLINK'.
 *  A return value of 'FILE_ABSENT' does not imply that a file
 *  with this name can be created, since missing directories and
 *  illegal file names cause also 'FILE_ABSENT'.
 *  @return the type of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation.
 *  @exception FILE_ERROR The system function returns an error other
 *             than ENOENT, ENOTDIR, ENAMETOOLONG or EACCES.
 */
intType cmdFileType (const const_striType filePath)

  {
    os_striType os_path;
    os_stat_struct stat_buf;
    int stat_result;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
    int saved_errno;
    intType type_of_file;

  /* cmdFileType */
    logFunction(printf("cmdFileType(\"%s\")", striAsUnquotedCStri(filePath));
                fflush(stdout););
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(os_path == NULL)) {
#if MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (path_info == PATH_IS_EMULATED_ROOT) {
        type_of_file = FILE_DIR;
      } else if (path_info == PATH_NOT_MAPPED) {
        type_of_file = FILE_ABSENT;
      } else
#endif
      {
        logError(printf("cmdFileType: cp_to_os_path(\"%s\", *, *) failed:\n"
                        "path_info=%d, err_info=%d\n",
                        striAsUnquotedCStri(filePath), path_info, err_info););
        raise_error(err_info);
        type_of_file = FILE_ABSENT;
      }
    } else {
      stat_result = os_stat(os_path, &stat_buf);
      saved_errno = errno;
      /* printf("stat(\"" FMT_S_OS "\") returns: %d, errno=%d\n",
         os_path, stat_result, saved_errno); */
      if (stat_result == 0) {
        if (S_ISREG(stat_buf.st_mode)) {
          type_of_file = FILE_REGULAR;
        } else if (S_ISDIR(stat_buf.st_mode)) {
          type_of_file = FILE_DIR;
        } else if (S_ISCHR(stat_buf.st_mode)) {
          type_of_file = FILE_CHAR;
        } else if (S_ISBLK(stat_buf.st_mode)) {
          type_of_file = FILE_BLOCK;
        } else if (S_ISFIFO(stat_buf.st_mode)) {
          type_of_file = FILE_FIFO;
        } else if (S_ISLNK(stat_buf.st_mode)) {
          type_of_file = FILE_SYMLINK;
          logError(printf("cmdFileType: os_stat(\"" FMT_S_OS "\", *) "
                          "returns a mode of S_IFLNK.\n",
                          os_path););
          err_info = FILE_ERROR;
        } else if (S_ISSOCK(stat_buf.st_mode)) {
          type_of_file = FILE_SOCKET;
        } else {
          type_of_file = FILE_UNKNOWN;
        } /* if */
#ifdef ELOOP
      } else if (unlikely(saved_errno == ELOOP)) {
        type_of_file = FILE_SYMLINK;
#endif
      } else if (unlikely(saved_errno == EACCES) &&
                          os_lstat(os_path, &stat_buf) == 0 &&
                          S_ISLNK(stat_buf.st_mode)) {
        /* Symbolic link that refers to a place where */
        /* the permission is denied. */
        type_of_file = FILE_SYMLINK;
      } else {
        type_of_file = FILE_ABSENT;
        if (unlikely(filePath->size != 0 && saved_errno != ENOENT &&
                     saved_errno != ENOTDIR && saved_errno != ENAMETOOLONG &&
                     saved_errno != EACCES)) {
          logError(printf("cmdFileType: os_stat(\"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_path, saved_errno, strerror(saved_errno)););
          /* printf("filePath->size=%lu\n", filePath->size); */
          /* printf("strlen(os_path)=%d\n", os_stri_strlen(os_path)); */
          err_info = FILE_ERROR;
        } /* if */
      } /* if */
      os_stri_free(os_path);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
    logFunctionResult(printf("" FMT_D "\n", type_of_file););
    return type_of_file;
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
 *             than ENOENT, ENOTDIR, ENAMETOOLONG or EACCES.
 */
intType cmdFileTypeSL (const const_striType filePath)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    intType type_of_file;

  /* cmdFileTypeSL */
    logFunction(printf("cmdFileTypeSL(\"%s\")", striAsUnquotedCStri(filePath));
                fflush(stdout););
    type_of_file = getFileTypeSL(filePath, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    logFunctionResult(printf("" FMT_D "\n", type_of_file););
    return type_of_file;
  } /* cmdFileTypeSL */



/**
 *  Get the final path that functions like getMTime() and open() would use.
 *  If 'filePath' is not a symbolic link it is returned. For a symbolic link
 *  the function follows the symbolic link chain until the path is not a
 *  symbolic link again. The final path may refer to a non-existing file.
 *  @param filePath Relative or absolute path.
 *  @return The final path after possibly following a symbolic link chain.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type or not enough memory to
 *             represent the result string.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist or a system function returns an error.
 */
striType cmdFinalPath (const const_striType filePath)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    striType destination;

  /* cmdFinalPath */
    logFunction(printf("cmdFinalPath(\"%s\")\n",
                       striAsUnquotedCStri(filePath)););
#ifdef HAS_DO_READ_LINK
    if (unlikely(!ALLOC_STRI_SIZE_OK(destination, filePath->size))) {
      raise_error(MEMORY_ERROR);
    } else {
      destination->size = filePath->size;
      memcpy(destination->mem, filePath->mem, filePath->size * sizeof(strElemType));
      destination = followLink(destination, &err_info);
      if (unlikely(destination == NULL)) {
        raise_error(err_info);
      } /* if */
    } /* if */
#else
    logError(printf("cmdFinalPath: "
                    "Reading symbolic links is not supported.\n"););
    raise_error(FILE_ERROR);
    destination = NULL;
#endif
    logFunction(printf("cmdFinalPath --> \"%s\"\n",
                       striAsUnquotedCStri(destination)););
    return destination;
  } /* cmdFinalPath */



/**
 *  Determine the current working directory of the calling process.
 *  @return The absolute path of the current working directory.
 *  @exception MEMORY_ERROR Not enough memory to represent the
 *             result string.
 *  @exception FILE_ERROR The system function returns an error.
 */
striType cmdGetcwd (void)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    striType cwd;

  /* cmdGetcwd */
    logFunction(printf("cmdGetcwd\n"););
    cwd = doGetCwd(&err_info);
    if (unlikely(cwd == NULL)) {
      logError(printf("cmdGetcwd: doGetCwd() failed:\n"
                      "err_info=%d\n", err_info););
      raise_error(err_info);
    } /* if */
    logFunction(printf("cmdGetcwd --> \"%s\"\n", striAsUnquotedCStri(cwd)););
    return cwd;
  } /* cmdGetcwd */



/**
 *  Determine the value of an environment variable.
 *  The function getenv searches the environment for an environment variable
 *  with the given 'name'. If such an environment variable exists the
 *  corresponding string value is returned.
 *  @return the value of an environment variable, and
 *          "" if the requested environment variable does not exist.
 *  @exception MEMORY_ERROR Not enough memory to convert 'name' to the
 *             system string type or not enough memory to represent the
 *             result string.
 *  @exception RANGE_ERROR 'name' cannot be converted to the system string
 *             type or a system function returns an error.
 */
striType cmdGetenv (const const_striType name)

  {
    os_striType env_name;
    os_striType env_value;
    errInfoType err_info = OKAY_NO_ERROR;
    striType value;

  /* cmdGetenv */
    logFunction(printf("cmdGetenv(\"%s\")", striAsUnquotedCStri(name));
                fflush(stdout););
    env_name = stri_to_os_stri(name, &err_info);
    if (unlikely(env_name == NULL)) {
      logError(printf("cmdGetenv: stri_to_os_stri(\"%s\", *, *) failed:\n"
                      "err_info=%d\n",
                      striAsUnquotedCStri(name), err_info););
      raise_error(err_info);
      value = NULL;
    } else {
      env_value = os_getenv(env_name);
      os_stri_free(env_name);
      if (env_value == NULL) {
        if (unlikely(!ALLOC_STRI_SIZE_OK(value, 0))) {
          err_info = MEMORY_ERROR;
        } else {
          value->size = 0;
        } /* if */
      } else {
        value = os_stri_to_stri(env_value, &err_info);
        os_getenv_string_free(env_value);
      } /* if */
      if (unlikely(value == NULL)) {
        raise_error(err_info);
      } /* if */
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(value)););
    return value;
  } /* cmdGetenv */



/**
 *  Determine the access time of a file.
 *  The function follows symbolic links.
 *  @return the access time of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or a system function returns an error.
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
    logFunction(printf("cmdGetATime(\"%s\")\n", striAsUnquotedCStri(filePath)););
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(os_path == NULL)) {
#if MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (likely(path_info == PATH_IS_EMULATED_ROOT)) {
        timFromTimestamp(0,
            year, month, day, hour,
            min, sec, micro_sec, time_zone, is_dst);
      } else
#endif
      {
        logError(printf("cmdGetATime: cp_to_os_path(\"%s\", *, *) failed:\n"
                        "path_info=%d, err_info=%d\n",
                        striAsUnquotedCStri(filePath), path_info, err_info););
        raise_error(err_info);
      }
    } else {
      stat_result = os_stat(os_path, &stat_buf);
      if (unlikely(stat_result != 0)) {
        logError(printf("cmdGetATime: os_stat(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_path, errno, strerror(errno)););
        err_info = FILE_ERROR;
      } else {
        logMessage(printf("cmdGetATime: st_atime=" FMT_T "\n",
                          stat_buf.st_atime););
        timFromTimestamp(stat_buf.st_atime,
            year, month, day, hour,
            min, sec, micro_sec, time_zone, is_dst);
      } /* if */
      os_stri_free(os_path);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
    logFunction(printf("cmdGetATime(" F_D(04) "-" F_D(02) "-" F_D(02) " "
                       F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) " " FMT_D " %d) -->\n",
                       *year, *month, *day, *hour, *min, *sec,
                       *micro_sec, *time_zone, *is_dst););
  } /* cmdGetATime */



/**
 *  Determine the access time of a symbolic link.
 *  The function only works for symbolic links and does not follow the
 *  symbolic link.
 *  @return the access time of the symbolic link.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or it is not a symbolic link, or a system function
 *             returns an error.
 */
void cmdGetATimeOfSymlink (const const_striType filePath,
    intType *year, intType *month, intType *day, intType *hour,
    intType *min, intType *sec, intType *micro_sec, intType *time_zone,
    boolType *is_dst)

  {
    os_striType os_path;
    os_stat_struct stat_buf;
    int stat_result;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;

  /* cmdGetATimeOfSymlink */
    logFunction(printf("cmdGetATimeOfSymlink(\"%s\")\n", striAsUnquotedCStri(filePath)););
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(os_path == NULL)) {
#if MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (path_info == PATH_IS_EMULATED_ROOT) {
        /* The emulated root is not a symbolic link. */
        logError(printf("cmdGetATimeOfSymlink(\"%s\"): "
                        "The emulated root is not a symbolic link.\n",
                        striAsUnquotedCStri(filePath)););
        err_info = FILE_ERROR;
      } else
#endif
      {
        logError(printf("cmdGetATimeOfSymlink: cp_to_os_path(\"%s\", *, *) failed:\n"
                        "path_info=%d, err_info=%d\n",
                        striAsUnquotedCStri(filePath), path_info, err_info););
      }
    } else {
      stat_result = os_lstat(os_path, &stat_buf);
      if (unlikely(stat_result != 0)) {
        logError(printf("cmdGetATimeOfSymlink(\"%s\"): "
                        "os_lstat(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        striAsUnquotedCStri(filePath), os_path,
                        errno, strerror(errno)););
        err_info = FILE_ERROR;
      } else if (unlikely(!S_ISLNK(stat_buf.st_mode))) {
        logError(printf("cmdGetATimeOfSymlink(\"%s\"): "
                        "The file \"" FMT_S_OS "\" is not a symbolic link.\n",
                        striAsUnquotedCStri(filePath), os_path););
        err_info = FILE_ERROR;
      } else {
        logMessage(printf("cmdGetATimeOfSymlink: st_atime=" FMT_T "\n",
                          stat_buf.st_atime););
        timFromTimestamp(stat_buf.st_atime,
            year, month, day, hour,
            min, sec, micro_sec, time_zone, is_dst);
      } /* if */
      os_stri_free(os_path);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    logFunction(printf("cmdGetATimeOfSymlink(" F_D(04) "-" F_D(02) "-" F_D(02) " "
                       F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) " " FMT_D " %d) -->\n",
                       *year, *month, *day, *hour, *min, *sec,
                       *micro_sec, *time_zone, *is_dst););
  } /* cmdGetATimeOfSymlink */



/**
 *  Determine the change time of a file.
 *  The function follows symbolic links.
 *  @return the change time of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or a system function returns an error.
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
    logFunction(printf("cmdGetCTime(\"%s\")\n", striAsUnquotedCStri(filePath)););
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(os_path == NULL)) {
#if MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (likely(path_info == PATH_IS_EMULATED_ROOT)) {
        timFromTimestamp(0,
            year, month, day, hour,
            min, sec, micro_sec, time_zone, is_dst);
      } else
#endif
      {
        logError(printf("cmdGetCTime: cp_to_os_path(\"%s\", *, *) failed:\n"
                        "path_info=%d, err_info=%d\n",
                        striAsUnquotedCStri(filePath), path_info, err_info););
        raise_error(err_info);
      }
    } else {
#ifdef HAS_WSTAT_CHANGE_TIME
      stat_result = wstatChangeTime(os_path, &stat_buf);
#else
      stat_result = os_stat(os_path, &stat_buf);
#endif
      if (unlikely(stat_result != 0)) {
        logError(printf("cmdGetCTime: os_stat(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_path, errno, strerror(errno)););
        err_info = FILE_ERROR;
      } else {
        logMessage(printf("cmdGetCTime: st_ctime=" FMT_T "\n",
                          stat_buf.st_ctime););
        timFromTimestamp(stat_buf.st_ctime,
            year, month, day, hour,
            min, sec, micro_sec, time_zone, is_dst);
      } /* if */
      os_stri_free(os_path);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
    logFunction(printf("cmdGetCTime(" F_D(04) "-" F_D(02) "-" F_D(02) " "
                       F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) " " FMT_D " %d) -->\n",
                       *year, *month, *day, *hour, *min, *sec,
                       *micro_sec, *time_zone, *is_dst););
  } /* cmdGetCTime */



/**
 *  Determine the file mode (permissions) of a file.
 *  The function follows symbolic links.
 *  @return the file mode.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or a system function returns an error.
 */
setType cmdGetFileMode (const const_striType filePath)

  {
    os_striType os_path;
    os_stat_struct stat_buf;
    int stat_result;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
    setType file_mode;

  /* cmdGetFileMode */
    logFunction(printf("cmdGetFileMode(\"%s\")\n", striAsUnquotedCStri(filePath)););
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(os_path == NULL)) {
#if MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (likely(path_info == PATH_IS_EMULATED_ROOT)) {
        file_mode = setIConv(0555);
      } else
#endif
      {
        logError(printf("cmdGetFileMode: cp_to_os_path(\"%s\", *, *) failed:\n"
                        "path_info=%d, err_info=%d\n",
                        striAsUnquotedCStri(filePath), path_info, err_info););
        raise_error(err_info);
        file_mode = NULL;
      }
    } else {
      stat_result = os_stat(os_path, &stat_buf);
      if (unlikely(stat_result != 0)) {
        logError(printf("cmdGetFileMode: os_stat(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_path, errno, strerror(errno)););
        os_stri_free(os_path);
        raise_error(FILE_ERROR);
        file_mode = NULL;
      } else {
        os_stri_free(os_path);
        logMessage(printf("cmdGetFileMode: st_mode=0%o\n",
                          stat_buf.st_mode););
#if MODE_BITS_NORMAL
        file_mode = setIConv(0777 & stat_buf.st_mode);
#else
        /* Force the bits to the standard sequence */
        file_mode = setIConv(
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
    return file_mode;
  } /* cmdGetFileMode */



/**
 *  Determine the file mode (permissions) of a symbolic link.
 *  The function only works for symbolic links and does not follow the
 *  symbolic link.
 *  @return the file mode of the symbolic link.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or it is not a symbolic link, or a system function
 *             returns an error.
 */
setType cmdGetFileModeOfSymlink (const const_striType filePath)

  {
    os_striType os_path;
    os_stat_struct stat_buf;
    int stat_result;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
    setType file_mode;

  /* cmdGetFileModeOfSymlink */
    logFunction(printf("cmdGetFileModeOfSymlink(\"%s\")\n", striAsUnquotedCStri(filePath)););
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(os_path == NULL)) {
#if MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (path_info == PATH_IS_EMULATED_ROOT) {
        /* The emulated root is not a symbolic link. */
        logError(printf("cmdGetFileModeOfSymlink(\"%s\"): "
                        "The emulated root is not a symbolic link.\n",
                        striAsUnquotedCStri(filePath)););
        err_info = FILE_ERROR;
        file_mode = NULL;
      } else
#endif
      {
        logError(printf("cmdGetFileModeOfSymlink: cp_to_os_path(\"%s\", *, *) failed:\n"
                        "path_info=%d, err_info=%d\n",
                        striAsUnquotedCStri(filePath), path_info, err_info););
        file_mode = NULL;
      }
    } else {
      stat_result = os_lstat(os_path, &stat_buf);
      if (unlikely(stat_result != 0)) {
        logError(printf("cmdGetFileModeOfSymlink(\"%s\"): "
                        "os_lstat(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        striAsUnquotedCStri(filePath), os_path,
                        errno, strerror(errno)););
        err_info = FILE_ERROR;
        file_mode = NULL;
      } else if (unlikely(!S_ISLNK(stat_buf.st_mode))) {
        logError(printf("cmdGetFileModeOfSymlink(\"%s\"): "
                        "The file \"" FMT_S_OS "\" is not a symbolic link.\n",
                        striAsUnquotedCStri(filePath), os_path););
        err_info = FILE_ERROR;
        file_mode = NULL;
      } else {
        logMessage(printf("cmdGetFileModeOfSymlink: st_mode=0%o\n",
                          stat_buf.st_mode););
#if MODE_BITS_NORMAL
        file_mode = setIConv(0777 & stat_buf.st_mode);
#else
        /* Force the bits to the standard sequence */
        file_mode = setIConv(
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
      os_stri_free(os_path);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    return file_mode;
  } /* cmdGetFileModeOfSymlink */



/**
 *  Determine the modification time of a file.
 *  The function follows symbolic links.
 *  @return the modification time of the file.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or a system function returns an error.
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
    logFunction(printf("cmdGetMTime(\"%s\")\n", striAsUnquotedCStri(filePath)););
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(os_path == NULL)) {
#if MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (likely(path_info == PATH_IS_EMULATED_ROOT)) {
        timFromTimestamp(0,
            year, month, day, hour,
            min, sec, micro_sec, time_zone, is_dst);
      } else
#endif
      {
        logError(printf("cmdGetMTime: cp_to_os_path(\"%s\", *, *) failed:\n"
                        "path_info=%d, err_info=%d\n",
                        striAsUnquotedCStri(filePath), path_info, err_info););
        raise_error(err_info);
      }
    } else {
      stat_result = os_stat(os_path, &stat_buf);
      if (unlikely(stat_result != 0)) {
        logError(printf("cmdGetMTime(\"%s\"): "
                        "os_stat(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        striAsUnquotedCStri(filePath), os_path,
                        errno, strerror(errno)););
        err_info = FILE_ERROR;
      } else {
        logMessage(printf("cmdGetMTime: st_mtime=" FMT_T "\n",
                          stat_buf.st_mtime););
        timFromTimestamp(stat_buf.st_mtime,
            year, month, day, hour,
            min, sec, micro_sec, time_zone, is_dst);
      } /* if */
      os_stri_free(os_path);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
    logFunction(printf("cmdGetMTime(" F_D(04) "-" F_D(02) "-" F_D(02) " "
                       F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) " " FMT_D " %d) -->\n",
                       *year, *month, *day, *hour, *min, *sec,
                       *micro_sec, *time_zone, *is_dst););
  } /* cmdGetMTime */



/**
 *  Determine the modification time of a symbolic link.
 *  The function only works for symbolic links and does not follow the
 *  symbolic link.
 *  @return the modification time of the symbolic link.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or it is not a symbolic link, or a system function
 *             returns an error.
 */
void cmdGetMTimeOfSymlink (const const_striType filePath,
    intType *year, intType *month, intType *day, intType *hour,
    intType *min, intType *sec, intType *micro_sec, intType *time_zone,
    boolType *is_dst)

  {
    os_striType os_path;
    os_stat_struct stat_buf;
    int stat_result;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;

  /* cmdGetMTimeOfSymlink */
    logFunction(printf("cmdGetMTimeOfSymlink(\"%s\")\n", striAsUnquotedCStri(filePath)););
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(os_path == NULL)) {
#if MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (path_info == PATH_IS_EMULATED_ROOT) {
        /* The emulated root is not a symbolic link. */
        logError(printf("cmdGetMTimeOfSymlink(\"%s\"): "
                        "The emulated root is not a symbolic link.\n",
                        striAsUnquotedCStri(filePath)););
        err_info = FILE_ERROR;
      } else
#endif
      {
        logError(printf("cmdGetMTimeOfSymlink: cp_to_os_path(\"%s\", *, *) failed:\n"
                        "path_info=%d, err_info=%d\n",
                        striAsUnquotedCStri(filePath), path_info, err_info););
      }
    } else {
      stat_result = os_lstat(os_path, &stat_buf);
      if (unlikely(stat_result != 0)) {
        logError(printf("cmdGetMTimeOfSymlink(\"%s\"): "
                        "os_lstat(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        striAsUnquotedCStri(filePath), os_path,
                        errno, strerror(errno)););
        err_info = FILE_ERROR;
      } else if (unlikely(!S_ISLNK(stat_buf.st_mode))) {
        logError(printf("cmdGetMTimeOfSymlink(\"%s\"): "
                        "The file \"" FMT_S_OS "\" is not a symbolic link.\n",
                        striAsUnquotedCStri(filePath), os_path););
        err_info = FILE_ERROR;
      } else {
        logMessage(printf("cmdGetMTimeOfSymlink: st_mtime=" FMT_T "\n",
                          stat_buf.st_mtime););
        timFromTimestamp(stat_buf.st_mtime,
            year, month, day, hour,
            min, sec, micro_sec, time_zone, is_dst);
      } /* if */
      os_stri_free(os_path);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    logFunction(printf("cmdGetMTimeOfSymlink(" F_D(04) "-" F_D(02) "-" F_D(02) " "
                       F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) " " FMT_D " %d) -->\n",
                       *year, *month, *day, *hour, *min, *sec,
                       *micro_sec, *time_zone, *is_dst););
  } /* cmdGetMTimeOfSymlink */



/**
 *  Returns the search path of the system as array of strings.
 *  @return the search path of the system.
 *  @exception MEMORY_ERROR Not enough memory to create the result.
 */
rtlArrayType cmdGetSearchPath (void)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    rtlArrayType searchPath;

  /* cmdGetSearchPath */
    logFunction(printf("cmdGetSearchPath()\n"););
    searchPath = getSearchPath(&err_info);
    if (unlikely(searchPath == NULL)) {
      logError(printf("cmdGetSearchPath: getSearchPath(*) failed:\n"
                      "err_info=%d\n",
                      err_info););
      raise_error(err_info);
    } /* if */
    return searchPath;
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
#ifdef DETERMINE_OS_PROPERTIES_AT_RUNTIME
    static const os_charType home_dir_env_var_unx[] = {'H', 'O', 'M', 'E', 0};
    static const os_charType home_dir_env_var_win[] = {'U', 'S', 'E', 'R',
                                                       'P', 'R', 'O', 'F', 'I', 'L', 'E', 0};
    const os_charType *home_dir_env_var;
#else
    static const os_charType home_dir_env_var[] = HOME_DIR_ENV_VAR;
#endif
    os_striType os_home_dir;
#ifdef DEFAULT_HOME_DIR
    static const os_charType default_home_dir[] = DEFAULT_HOME_DIR;
#endif
    errInfoType err_info = OKAY_NO_ERROR;
    striType home_dir;

  /* cmdHomeDir */
    logFunction(printf("cmdHomeDir\n"););
#ifdef DETERMINE_OS_PROPERTIES_AT_RUNTIME
    if (shellUsesDriveLetters) {
      home_dir_env_var = home_dir_env_var_win;
    } else {
      home_dir_env_var = home_dir_env_var_unx;
    } /* if */
#endif
    os_home_dir = os_getenv(home_dir_env_var);
    /* printf("os_getenv(\"" FMT_S_OS "\") returns: " FMT_S_OS "\n",
        home_dir_env_var, os_home_dir); */
    if (unlikely(os_home_dir == NULL)) {
#ifdef DEFAULT_HOME_DIR
      home_dir = cp_from_os_path(default_home_dir, &err_info);
#else
      err_info = FILE_ERROR;
      home_dir = NULL;
#endif
    } else {
      home_dir = cp_from_os_path(os_home_dir, &err_info);
      os_getenv_string_free(os_home_dir);
    } /* if */
    if (unlikely(home_dir == NULL)) {
      raise_error(err_info);
    } /* if */
    logFunction(printf("cmdHomeDir --> \"%s\"\n", striAsUnquotedCStri(home_dir)););
    return home_dir;
  } /* cmdHomeDir */



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
    logFunction(printf("cmdMkdir(\"%s\")\n", striAsUnquotedCStri(dirPath)););
    os_path = cp_to_os_path(dirPath, &path_info, &err_info);
    if (unlikely(os_path == NULL)) {
      logError(printf("cmdMkdir: cp_to_os_path(\"%s\", *, *) failed:\n"
                      "path_info=%d, err_info=%d\n",
                      striAsUnquotedCStri(dirPath), path_info, err_info););
      raise_error(err_info);
    } else {
      /* printf("mkdir(\"" FMT_S_OS "\")\n", os_path); */
      mkdir_result = os_mkdir(os_path, 0777);
      if (unlikely(mkdir_result != 0)) {
        logError(printf("cmdMkdir: os_mkdir(\"" FMT_S_OS "\", 0777) failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_path, errno, strerror(errno)););
        os_stri_free(os_path);
        raise_error(FILE_ERROR);
      } else {
        os_stri_free(os_path);
      } /* if */
    } /* if */
  } /* cmdMkdir */



/**
 *  Move and rename a file or directory tree.
 *  The function uses the C 'rename()' function. If 'rename()' fails
 *  the file (or directory tree) is cloned with 'cloneFile' (which
 *  preserves permissions/mode, ownership and timestamps) to the new
 *  place and with the new name. If 'cloneFile' succeeds the original
 *  file is deleted. If 'cloneFile' fails (no space on device or
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
    logFunction(printf("cmdMove(\"%s\", ", striAsUnquotedCStri(sourcePath));
                printf("\"%s\")\n", striAsUnquotedCStri(destPath)););
    os_sourcePath = cp_to_os_path(sourcePath, &path_info, &err_info);
    if (unlikely(os_sourcePath == NULL)) {
      logError(printf("cmdMove: cp_to_os_path(\"%s\", *, *) failed:\n"
                      "path_info=%d, err_info=%d\n",
                      striAsUnquotedCStri(sourcePath), path_info, err_info););
    } else {
      os_destPath = cp_to_os_path(destPath, &path_info, &err_info);
      if (unlikely(os_destPath == NULL)) {
        logError(printf("cmdMove: cp_to_os_path(\"%s\", *, *) failed:\n"
                        "path_info=%d, err_info=%d\n",
                        striAsUnquotedCStri(destPath), path_info, err_info););
      } else {
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
 *  Determine the filenames in a directory.
 *  The function does follow symbolic links.
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
 *  @exception FILE_ERROR The file described with 'dirPath' does not
 *             exist, or it is not a directory, or a system function
 *             returns an error.
 */
rtlArrayType cmdReadDir (const const_striType dirPath)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    rtlArrayType result;

  /* cmdReadDir */
    logFunction(printf("cmdReadDir(\"%s\")\n", striAsUnquotedCStri(dirPath)););
    result = read_dir(dirPath, &err_info);
    if (unlikely(result == NULL)) {
      logError(printf("cmdReadDir: read_dir(\"%s\", *) failed:\n"
                      "err_info=%d\n",
                      striAsUnquotedCStri(dirPath), err_info););
      raise_error(err_info);
    } else {
      qsort((void *) result->arr,
          (size_t) (result->max_position - result->min_position + 1),
          sizeof(rtlObjectType), &cmp_mem);
    } /* if */
    logFunction(if (result == NULL) {
                  printf("cmdReadDir --> NULL\n");
                } else {
                  printf("cmdReadDir --> array[size = " FMT_U_MEM "]\n",
                         arraySize(result));
                });
    return result;
  } /* cmdReadDir */



/**
 *  Reads the destination of a symbolic link.
 *  This function reads the link destination from the file system
 *  without any change. Symbolic links can be relative or absolute.
 *  Relative symbolic links are relative to their place in the
 *  file system and not relative to the current working directory.
 *  @param filePath Relative or absolute path of a symbolic link.
 *  @return The destination referred by the symbolic link.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type or not enough memory to
 *             represent the result string.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist or is not a symbolic link.
 */
striType cmdReadLink (const const_striType filePath)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    striType destination;

  /* cmdReadLink */
    logFunction(printf("cmdReadLink(\"%s\")\n",
                       striAsUnquotedCStri(filePath)););
#ifdef HAS_DO_READ_LINK
    destination = doReadLink(filePath, &err_info);
    if (unlikely(destination == NULL)) {
      raise_error(err_info);
    } /* if */
#else
    logError(printf("cmdReadLink: "
                    "Reading symbolic links is not supported.\n"););
    raise_error(FILE_ERROR);
    destination = NULL;
#endif
    logFunction(printf("cmdReadLink --> \"%s\"\n",
                       striAsUnquotedCStri(destination)););
    return destination;
  } /* cmdReadLink */



/**
 *  Reads the absolute destination path of a symbolic link.
 *  Symbolic links can be relative or absolute. Relative symbolic links
 *  are relative to their place in the file system and not relative to
 *  the current working directory. For a relative symbolic link
 *  'filePath' is converted to an absolute path and the symbolic
 *  link is concatenated to that path.
 *  @param filePath Relative or absolute path of a symbolic link.
 *  @return The absolute destination path referred by the symbolic link.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type or not enough memory to
 *             represent the result string.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist or is not a symbolic link.
 */
striType cmdReadLinkAbsolute (const const_striType filePath)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    striType destination;

  /* cmdReadLinkAbsolute */
    logFunction(printf("cmdReadLinkAbsolute(\"%s\")\n",
                       striAsUnquotedCStri(filePath)););
#ifdef HAS_DO_READ_LINK
    destination = readLinkAbsolute(filePath, &err_info);
    if (unlikely(destination == NULL)) {
      raise_error(err_info);
    } /* if */
#else
    logError(printf("cmdReadLinkAbsolute: "
                    "Reading symbolic links is not supported.\n"););
    raise_error(FILE_ERROR);
    destination = NULL;
#endif
    logFunction(printf("cmdReadLinkAbsolute --> \"%s\"\n",
                       striAsUnquotedCStri(destination)););
    return destination;
  } /* cmdReadLinkAbsolute */



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
    os_striType os_path;
    int path_info;
#if REMOVE_FAILS_FOR_EMPTY_DIRS
    os_stat_struct file_stat;
#endif
#ifdef RENAME_BEFORE_REMOVE
    os_striType temp_name;
#endif
    errInfoType err_info = OKAY_NO_ERROR;

  /* cmdRemoveFile */
    logFunction(printf("cmdRemoveFile(\"%s\")\n", striAsUnquotedCStri(filePath)););
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(os_path == NULL)) {
      logError(printf("cmdRemoveFile: cp_to_os_path(\"%s\", *, *) failed:\n"
                      "path_info=%d, err_info=%d\n",
                      striAsUnquotedCStri(filePath), path_info, err_info););
    } else {
#if REMOVE_FAILS_FOR_EMPTY_DIRS
      if (os_lstat(os_path, &file_stat) != 0) {
        logError(printf("cmdRemoveFile: os_lstat(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_path, errno, strerror(errno)););
        err_info = FILE_ERROR;
      } else {
#ifdef RENAME_BEFORE_REMOVE
        temp_name = temp_name_in_dir(os_path);
        if (unlikely(temp_name == NULL)) {
          err_info = MEMORY_ERROR;
        } else {
          if (os_rename(os_path, temp_name) != 0) {
            logError(printf("cmdRemoveFile: "
                            "os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                            "errno=%d\nerror: %s\n",
                            os_path, temp_name, errno, strerror(errno)););
            err_info = FILE_ERROR;
          } else {
            if (S_ISDIR(file_stat.st_mode)) {
              if (os_rmdir(temp_name) != 0) {
                logError(printf("cmdRemoveFile: os_rmdir(\"" FMT_S_OS "\") failed:\n"
                                "errno=%d\nerror: %s\n",
                                temp_name, errno, strerror(errno)););
                err_info = FILE_ERROR;
              } /* if */
            } else {
              if (os_remove(temp_name) != 0) {
                logError(printf("cmdRemoveFile: os_remove(\"" FMT_S_OS "\") failed:\n"
                                "errno=%d\nerror: %s\n",
                                temp_name, errno, strerror(errno)););
                err_info = FILE_ERROR;
              } /* if */
            } /* if */
            if (unlikely(err_info != OKAY_NO_ERROR)) {
              /* Rename back to the original name. */
              if (os_rename(temp_name, os_path) != 0) {
                logError(printf("cmdRemoveFile: "
                                "os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                                "errno=%d\nerror: %s\n",
                                temp_name, os_path, errno, strerror(errno)););
              } /* if */
            } /* if */
          } /* if */
          os_stri_free(temp_name);
        } /* if */
#else
        if (S_ISDIR(file_stat.st_mode)) {
          if (os_rmdir(os_path) != 0) {
            logError(printf("cmdRemoveFile: os_rmdir(\"" FMT_S_OS "\") failed:\n"
                            "errno=%d\nerror: %s\n",
                            os_path, errno, strerror(errno)););
            err_info = FILE_ERROR;
          } /* if */
        } else {
          if (os_remove(os_path) != 0) {
            logError(printf("cmdRemoveFile: os_remove(\"" FMT_S_OS "\") failed:\n"
                            "errno=%d\nerror: %s\n",
                            os_path, errno, strerror(errno)););
            err_info = FILE_ERROR;
          } /* if */
        } /* if */
#endif
      } /* if */
#else
#ifdef RENAME_BEFORE_REMOVE
      temp_name = temp_name_in_dir(os_path);
      if (unlikely(temp_name == NULL)) {
        err_info = MEMORY_ERROR;
      } else {
        if (os_rename(os_path, temp_name) != 0) {
          logError(printf("cmdRemoveFile: "
                          "os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_path, temp_name, errno, strerror(errno)););
          err_info = FILE_ERROR;
        } else {
          if (os_remove(temp_name) != 0) {
            logError(printf("cmdRemoveFile: os_remove(\"" FMT_S_OS "\") failed:\n"
                            "errno=%d\nerror: %s\n",
                            temp_name, errno, strerror(errno)););
            if (os_rename(temp_name, os_path) != 0) {
              /* Rename back to the original name. */
              logError(printf("cmdRemoveFile: "
                              "os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                              "errno=%d\nerror: %s\n",
                              temp_name, os_path, errno, strerror(errno)););
            } /* if */
            err_info = FILE_ERROR;
          } /* if */
        } /* if */
        os_stri_free(temp_name);
      } /* if */
#else
      if (os_remove(os_path) != 0) {
        logError(printf("cmdRemoveFile: os_remove(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_path, errno, strerror(errno)););
        err_info = FILE_ERROR;
      } /* if */
#endif
#endif
#ifdef CHECK_IF_FILE_IS_REMOVED
      if (err_info == OKAY_NO_ERROR &&
          os_lstat(os_path, &file_stat) == 0) {
        logError(printf("cmdRemoveFile: File \"" FMT_S_OS "\" still present.\n",
                        os_path););
        /* err_info = FILE_ERROR; */
      } /* if */
#endif
      os_stri_free(os_path);
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
    os_striType os_path;
    int path_info;
    os_stat_struct file_stat;
#ifdef RENAME_BEFORE_REMOVE
    os_striType temp_name;
#endif
    errInfoType err_info = OKAY_NO_ERROR;

  /* cmdRemoveTree */
    logFunction(printf("cmdRemoveTree(\"%s\")\n", striAsUnquotedCStri(filePath)););
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(os_path == NULL)) {
      logError(printf("cmdRemoveTree: cp_to_os_path(\"%s\", *, *) failed:\n"
                      "path_info=%d, err_info=%d\n",
                      striAsUnquotedCStri(filePath), path_info, err_info););
    } else {
      if (os_lstat(os_path, &file_stat) != 0) {
        logError(printf("cmdRemoveTree: os_lstat(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_path, errno, strerror(errno)););
        err_info = FILE_ERROR;
      } else {
#ifdef RENAME_BEFORE_REMOVE
        temp_name = temp_name_in_dir(os_path);
        if (unlikely(temp_name == NULL)) {
          err_info = MEMORY_ERROR;
        } else {
          if (os_rename(os_path, temp_name) != 0) {
            logError(printf("cmdRemoveTree: "
                            "os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                            "errno=%d\nerror: %s\n",
                            os_path, temp_name, errno, strerror(errno)););
            err_info = FILE_ERROR;
          } else {
            if (S_ISDIR(file_stat.st_mode)) {
              remove_dir(temp_name, &err_info);
            } else {
              if (os_remove(temp_name) != 0) {
                logError(printf("cmdRemoveTree: os_remove(\"" FMT_S_OS "\") failed:\n"
                                "errno=%d\nerror: %s\n",
                                os_path, errno, strerror(errno)););
                err_info = FILE_ERROR;
              } /* if */
            } /* if */
            if (unlikely(err_info != OKAY_NO_ERROR)) {
              if (os_rename(temp_name, os_path) != 0) {
                /* Rename back to the original name. */
                logError(printf("cmdRemoveTree: "
                                "os_rename(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                                "errno=%d\nerror: %s\n",
                                temp_name, os_path, errno, strerror(errno)););
              } /* if */
            } /* if */
          } /* if */
          os_stri_free(temp_name);
        } /* if */
#else
        if (S_ISDIR(file_stat.st_mode)) {
          remove_dir(os_path, &err_info);
        } else {
          if (os_remove(os_path) != 0) {
            logError(printf("cmdRemoveTree: os_remove(\"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_path, errno, strerror(errno)););
            err_info = FILE_ERROR;
          } /* if */
        } /* if */
#endif
      } /* if */
#ifdef CHECK_IF_FILE_IS_REMOVED
      if (err_info == OKAY_NO_ERROR &&
          os_lstat(os_path, &file_stat) == 0) {
        logError(printf("cmdRemoveTree: File \"" FMT_S_OS "\" still present.\n",
                        os_path););
        /* err_info = FILE_ERROR; */
      } /* if */
#endif
      os_stri_free(os_path);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* cmdRemoveTree */



/**
 *  Add or change an environment variable.
 *  The function searches the environment for an environment variable
 *  with the given 'name'. If such an environment variable exists the
 *  corresponding value is changed to 'value'. If no environment variable
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
 *  The function follows symbolic links.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception RANGE_ERROR 'aTime' is invalid or cannot be
 *             converted to the system file time.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or a system function returns an error.
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
    logFunction(printf("cmdSetATime(\"%s\", " F_D(04) "-" F_D(02) "-" F_D(02) " "
                       F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) " " FMT_D ")\n",
                       striAsUnquotedCStri(filePath), year, month, day,
                       hour, min, sec, micro_sec, time_zone););
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(os_path == NULL)) {
      logError(printf("cmdSetATime: cp_to_os_path(\"%s\", *, *) failed:\n"
                      "path_info=%d, err_info=%d\n",
                      striAsUnquotedCStri(filePath), path_info, err_info););
    } else {
      if (unlikely(os_stat(os_path, &stat_buf) != 0)) {
        logError(printf("cmdSetATime: os_stat(\"" FMT_S_OS "\", *) failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_path, errno, strerror(errno)););
        err_info = FILE_ERROR;
      } else {
        utime_buf.actime = timToOsTimestamp(year, month, day, hour,
            min, sec, time_zone);
        logMessage(printf("cmdSetATime: actime=" FMT_T "\n",
                          utime_buf.actime););
        utime_buf.modtime = stat_buf.st_mtime;
        if (unlikely(utime_buf.actime == (time_t) TIME_T_ERROR)) {
          logError(printf("cmdSetATime: timToOsTimestamp("
                          F_D(04) "-" F_D(02) "-" F_D(02) " " F_D(02) ":"
                          F_D(02) ":" F_D(02) "." F_D(06) " " FMT_D ") failed.\n",
                          year, month, day, hour, min, sec,
                          micro_sec, time_zone););
          err_info = RANGE_ERROR;
        } else if (unlikely(os_utime(os_path, &utime_buf) != 0)) {
          logError(printf("cmdSetATime: os_utime(\"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_path, errno, strerror(errno)););
          err_info = FILE_ERROR;
        } /* if */
      } /* if */
      os_stri_free(os_path);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    logFunction(printf("cmdSetATime -->\n"););
  } /* cmdSetATime */



/**
 *  Change the file mode (permissions) of a file.
 *  The function follows symbolic links.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or a system function returns an error.
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
    logFunction(printf("cmdSetFileMode(\"%s\", 0x" F_X(3) ")\n",
                       striAsUnquotedCStri(filePath), setSConv(mode)););
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(os_path == NULL)) {
      logError(printf("cmdSetFileMode: cp_to_os_path(\"%s\", *, *) failed:\n"
                      "path_info=%d, err_info=%d\n",
                      striAsUnquotedCStri(filePath), path_info, err_info););
    } else {
      intType_mode = setSConv(mode);
      if (intType_mode >= 0 && intType_mode <= 0777) {
        /* Just the read, write and execute permissions are accepted */
        int_mode = (int) intType_mode;
        logMessage(printf("cmdSetFileMode: mode=0%o\n", int_mode););
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
          logError(printf("cmdSetFileMode: os_chmod(\"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_path, errno, strerror(errno)););
          err_info = FILE_ERROR;
        } /* if */
      } else {
        logError(printf("cmdSetFileMode(\"%s\", 0x" F_X(3) ") failed:\n"
                        "Mode outside allowed range.\n",
                        striAsUnquotedCStri(filePath), setSConv(mode)););
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
 *  The function follows symbolic links.
 *  @exception MEMORY_ERROR Not enough memory to convert 'filePath'
 *             to the system path type.
 *  @exception RANGE_ERROR 'filePath' does not use the standard path
 *             representation or it cannot be converted to the system
 *             path type.
 *  @exception RANGE_ERROR 'aTime' is invalid or cannot be
 *             converted to the system file time.
 *  @exception FILE_ERROR The file described with 'filePath' does not
 *             exist, or a system function returns an error.
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
    logFunction(printf("cmdSetMTime(\"%s\", " F_D(04) "-" F_D(02) "-" F_D(02) " "
                       F_D(02) ":" F_D(02) ":" F_D(02) "." F_D(06) " " FMT_D ")\n",
                       striAsUnquotedCStri(filePath), year, month,
                       day, hour, min, sec, micro_sec, time_zone););
    os_path = cp_to_os_path(filePath, &path_info, &err_info);
    if (unlikely(os_path == NULL)) {
      logError(printf("cmdSetMTime: cp_to_os_path(\"%s\", *, *) failed:\n"
                      "path_info=%d, err_info=%d\n",
                      striAsUnquotedCStri(filePath), path_info, err_info););
    } else {
      if (unlikely(os_stat(os_path, &stat_buf) != 0)) {
        logError(printf("cmdSetMTime: os_stat(\"" FMT_S_OS "\", *) failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_path, errno, strerror(errno)););
        err_info = FILE_ERROR;
      } else {
        utime_buf.actime = stat_buf.st_atime;
        utime_buf.modtime = timToOsTimestamp(year, month, day, hour,
            min, sec, time_zone);
        logMessage(printf("cmdSetMTime: modtime=" FMT_T "\n",
                          utime_buf.modtime););
        if (unlikely(utime_buf.modtime == (time_t) TIME_T_ERROR)) {
          logError(printf("cmdSetMTime: timToOsTimestamp("
                          F_D(04) "-" F_D(02) "-" F_D(02) " " F_D(02) ":"
                          F_D(02) ":" F_D(02) "." F_D(06) " " FMT_D ") failed.\n",
                          year, month, day, hour, min, sec,
                          micro_sec, time_zone););
          err_info = RANGE_ERROR;
        } else if (unlikely(os_utime(os_path, &utime_buf) != 0)) {
          logError(printf("cmdSetMTime: os_utime(\"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_path, errno, strerror(errno)););
          err_info = FILE_ERROR;
        } /* if */
      } /* if */
      os_stri_free(os_path);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    logFunction(printf("cmdSetMTime -->\n"););
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
 *         'command', or "" if there are no parameters.
 *  @return the return code of the executed command or of the shell.
 */
intType cmdShell (const const_striType command, const const_striType parameters)

  {
    os_striType os_command;
    errInfoType err_info = OKAY_NO_ERROR;
    intType result;

  /* cmdShell */
    logFunction(printf("cmdShell(\"%s\", ", striAsUnquotedCStri(command));
                printf("\"%s\")\n", striAsUnquotedCStri(parameters)););
#if defined USE_EXTENDED_LENGTH_PATH && USE_EXTENDED_LENGTH_PATH
    adjustCwdForShell(&err_info);
#endif
    os_command = cp_to_command(command, parameters, &err_info);
    if (unlikely(os_command == NULL)) {
      logError(printf("cmdShell: cp_to_command(\"%s\", ",
                      striAsUnquotedCStri(command));
               printf("\"%s\", *) failed:\n"
                      "err_info=%d\n",
                      striAsUnquotedCStri(parameters), err_info););
      raise_error(err_info);
      result = 0;
    } else {
      logMessage(printf("cmdShell: os_command: \"" FMT_S_OS "\"\n", os_command););
      result = (intType) os_system(os_command);
      /* if (result != 0) {
        printf("errno=%d\nerror: %s\n", errno, strerror(errno));
        printf("result=" FMT_D "\n", result);
      } */
      FREE_OS_STRI(os_command);
    } /* if */
    logFunction(printf("cmdShell --> " FMT_D "\n", result););
    return result;
  } /* cmdShell */



#ifdef ESCAPE_SHELL_COMMANDS
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
    /* A shell parameter might start and end with quote ("): */
    const memSizeType numOfQuotes = 0;
    /* Maximum escape sequence length in shell parameter: */
    const memSizeType escSequenceMax = STRLEN("\\=");
    memSizeType inPos;
    memSizeType outPos;
    errInfoType err_info = OKAY_NO_ERROR;
    striType resized_result;
    striType result;

  /* cmdShellEscape */
    logFunction(printf("cmdShellEscape(\"%s\")", striAsUnquotedCStri(stri));
                fflush(stdout););
    if (unlikely(stri->size > (MAX_STRI_LEN - numOfQuotes) / escSequenceMax ||
                 !ALLOC_STRI_SIZE_OK(result, escSequenceMax * stri->size + numOfQuotes))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      for (inPos = 0, outPos = 0; inPos < stri->size; inPos++, outPos++) {
        switch (stri->mem[inPos]) {
          case '\t': case ' ':  case '!':  case '\"': case '#':
          case '$':  case '&':  case '\'': case '(':  case ')':
          case '*':  case ',':  case ':':  case ';':  case '<':
          case '=':  case '>':  case '?':  case '[':  case '\\':
          case ']':  case '^':  case '`':  case '{':  case '|':
          case '}':  case '~':
            result->mem[outPos] = '\\';
            outPos++;
            result->mem[outPos] = stri->mem[inPos];
            break;
          case '\0': case '\n':
            logError(printf("cmdShellEscape: "
                            "Illegal character in string ('\\" FMT_U32 ";').\n",
                            stri->mem[inPos]););
            err_info = RANGE_ERROR;
            break;
          default:
            result->mem[outPos] = stri->mem[inPos];
            break;
        } /* switch */
      } /* for */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        FREE_STRI(result, escSequenceMax * stri->size + numOfQuotes);
        raise_error(err_info);
        result = NULL;
      } else {
        REALLOC_STRI_SIZE_SMALLER(resized_result, result,
            escSequenceMax * stri->size + numOfQuotes, outPos);
        if (unlikely(resized_result == NULL)) {
          FREE_STRI(result, escSequenceMax * stri->size + numOfQuotes);
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          result = resized_result;
          COUNT3_STRI(escSequenceMax * stri->size + numOfQuotes, outPos);
          result->size = outPos;
        } /* if */
      } /* if */
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(result)););
    return result;
  } /* cmdShellEscape */

#else



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
    /* A shell parameter might start and end with quote ("): */
    const memSizeType numOfQuotes = 2;
    /* Maximum escape sequence length in shell parameter: */
    const memSizeType escSequenceMax = 4;
    memSizeType inPos;
    memSizeType outPos;
    boolType quotation_mode = FALSE;
    boolType in_escaped_quotation = FALSE;
    memSizeType countBackslash;
    errInfoType err_info = OKAY_NO_ERROR;
    striType resized_result;
    striType result;

  /* cmdShellEscape */
    logFunction(printf("cmdShellEscape(\"%s\")", striAsUnquotedCStri(stri));
                fflush(stdout););
    if (unlikely(stri->size > (MAX_STRI_LEN - numOfQuotes) / escSequenceMax ||
                 !ALLOC_STRI_SIZE_OK(result, escSequenceMax * stri->size + numOfQuotes))) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      for (inPos = 0, outPos = 0; inPos < stri->size; inPos++, outPos++) {
        switch (stri->mem[inPos]) {
          case '\t': case '\f': case ' ':  case '%':  case '*':
          case ',':  case ';':  case '=':  case '~':  case 160:
            if (!quotation_mode) {
              quotation_mode = TRUE;
              result->mem[outPos] = '"';
              outPos++;
            } /* if */
            result->mem[outPos] = stri->mem[inPos];
            break;
          case '&':  case '<':  case '>':  case '^':  case '|':
            if (!quotation_mode) {
              quotation_mode = TRUE;
              result->mem[outPos] = '"';
              outPos++;
            } /* if */
            if (in_escaped_quotation) {
              result->mem[outPos] = '^';
              outPos++;
            } /* if */
            result->mem[outPos] = stri->mem[inPos];
            break;
          case '\"':
            if (!quotation_mode) {
              quotation_mode = TRUE;
              result->mem[outPos] = '"';
              outPos++;
            } /* if */
            result->mem[outPos] = '\\';
            outPos++;
            result->mem[outPos] = stri->mem[inPos];
            in_escaped_quotation = !in_escaped_quotation;
            break;
          case '\\':
            result->mem[outPos] = '\\';
            outPos++;
            result->mem[outPos] = stri->mem[inPos];
            break;
          case '\0': case '\n': case '\r':
            logError(printf("cmdShellEscape: "
                            "Illegal character in string ('\\" FMT_U32 ";').\n",
                            stri->mem[inPos]););
            err_info = RANGE_ERROR;
            break;
          default:
            if (quotation_mode) {
              quotation_mode = FALSE;
              result->mem[outPos] = '"';
              outPos++;
            } /* if */
            result->mem[outPos] = stri->mem[inPos];
            break;
        } /* switch */
      } /* for */
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        FREE_STRI(result, escSequenceMax * stri->size + numOfQuotes);
        raise_error(err_info);
        result = NULL;
      } else {
        if (quotation_mode) {
          result->mem[outPos] = '"';
          outPos++;
        } /* if */
        for (inPos = 0; inPos < outPos; inPos++) {
          if (result->mem[inPos] == '\\') {
            inPos++;
            countBackslash = 1;
            while (inPos < outPos && result->mem[inPos] == '\\') {
              inPos++;
              countBackslash++;
            } /* while */
            if (inPos == outPos || result->mem[inPos] != '"') {
              countBackslash /= 2;
              memcpy(&result->mem[inPos - countBackslash], &result->mem[inPos],
                     (outPos - inPos) * sizeof(strElemType));
              inPos -= countBackslash;
              outPos -= countBackslash;
            } /* if */
            inPos--;
          } /* if */
        } /* for */
        REALLOC_STRI_SIZE_SMALLER(resized_result, result,
            escSequenceMax * stri->size + numOfQuotes, outPos);
        if (unlikely(resized_result == NULL)) {
          FREE_STRI(result, escSequenceMax * stri->size + numOfQuotes);
          raise_error(MEMORY_ERROR);
          result = NULL;
        } else {
          result = resized_result;
          COUNT3_STRI(escSequenceMax * stri->size + numOfQuotes, outPos);
          result->size = outPos;
        } /* if */
      } /* if */
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(result)););
    return result;
  } /* cmdShellEscape */

#endif



/**
 *  Create a symbolic link.
 *  The symbolic link 'symlinkPath' will refer to 'targetPath' afterwards.
 *  @param targetPath String to be contained in the symbolic link.
 *  @param symlinkPath Name of the symbolic link to be created.
 *  @exception MEMORY_ERROR Not enough memory to convert targetPath or
 *             symlinkPath to the system path type.
 *  @exception RANGE_ERROR 'targetPath' or 'symlinkPath' does not use the
 *             standard path representation or one of them cannot be
 *             converted to the system path type.
 *  @exception FILE_ERROR A system function returns an error.
 */
void cmdSymlink (const const_striType targetPath, const const_striType symlinkPath)

  {
#if HAS_SYMLINK
    os_striType os_targetPath;
    os_striType os_symlinkPath;
    int path_info;
#endif
    errInfoType err_info = OKAY_NO_ERROR;

  /* cmdSymlink */
    logFunction(printf("cmdSymlink(\"%s\", ", striAsUnquotedCStri(targetPath));
                printf("\"%s\")\n", striAsUnquotedCStri(symlinkPath)););
#if HAS_SYMLINK
    os_targetPath = cp_to_os_path(targetPath, &path_info, &err_info);
    if (likely(os_targetPath != NULL)) {
      os_symlinkPath = cp_to_os_path(symlinkPath, &path_info, &err_info);
      if (likely(os_symlinkPath != NULL)) {
        logMessage(printf("os_symlink(\"" FMT_S_OS "\", \"" FMT_S_OS "\")\n",
                          os_targetPath, os_symlinkPath););
        if (unlikely(os_symlink(os_targetPath, os_symlinkPath) != 0)) {
          logError(printf("cmdSymlink: "
                          "os_symlink(\"" FMT_S_OS "\", \"" FMT_S_OS "\") failed:\n"
                          "errno=%d\nerror: %s\n",
                          os_targetPath, os_symlinkPath,
                          errno, strerror(errno)););
          err_info = FILE_ERROR;
        } /* if */
        os_stri_free(os_symlinkPath);
      } /* if */
      os_stri_free(os_targetPath);
    } /* if */
#elif defined DEFINE_WIN_SYMLINK
    winSymlink(targetPath, symlinkPath, &err_info);
#else
    logError(printf("cmdSymlink(\"%s\", ", striAsUnquotedCStri(targetPath));
             printf("\"%s\"): Creating symbolic links is not supported.\n",
                    striAsUnquotedCStri(symlinkPath)););
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
    logFunction(printf("cmdToOsPath(\"%s\")", striAsUnquotedCStri(standardPath));
                fflush(stdout););
    if (unlikely(pathIsWrong(standardPath))) {
      logError(printf("cmdToOsPath: "
                      "\"%s\" uses a drive letter or ends with slash.\n",
                      striAsUnquotedCStri(standardPath)););
      err_info = RANGE_ERROR;
    } else {
      if_mapAbsoluteShellPathToDriveLetters(
          standardPath->size >= 1 && standardPath->mem[0] == '/', {
        /* Absolute path: Try to map the path to a drive letter */
        if (unlikely(standardPath->size == 1)) {
          /* "/"    cannot be mapped to a drive letter */
          logError(printf("cmdToOsPath: "
                          "\"%s\" cannot be mapped to a drive letter.\n",
                          striAsUnquotedCStri(standardPath)););
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
            logError(printf("cmdToOsPath: "
                            "\"%s\" cannot be mapped to a drive letter.\n",
                            striAsUnquotedCStri(standardPath)););
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
              memcpy(&result->mem[3], &standardPath->mem[3],
                     (standardPath->size - 3) * sizeof(strElemType));
            } /* if */
          } /* if */
        } else {
          /* "/C"  cannot be mapped to a drive letter */
          logError(printf("cmdToOsPath: "
                          "\"%s\" cannot be mapped to a drive letter.\n",
                          striAsUnquotedCStri(standardPath)););
          err_info = RANGE_ERROR;
        } /* if */
      }, /* else */ {
        if (unlikely(!ALLOC_STRI_SIZE_OK(result, standardPath->size))) {
          err_info = MEMORY_ERROR;
        } else {
          result->size = standardPath->size;
          memcpy(result->mem, standardPath->mem, standardPath->size * sizeof(strElemType));
        } /* if */
      });
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
      result = NULL;
    } else {
      if_pathDelimiterNotSlash({
        memSizeType position;

        for (position = 0; position < result->size; position++) {
          if (result->mem[position] == '/') {
            result->mem[position] = SHELL_PATH_DELIMITER;
          } /* if */
        } /* for */
      });
    } /* if */
    logFunctionResult(printf("\"%s\"\n", striAsUnquotedCStri(result)););
    return result;
  } /* cmdToOsPath */



#ifdef os_unsetenv
/**
 *  Deletes the variable 'name' from the environment.
 *  If 'name' does not exist in the environment,
 *  then the function succeeds, and the environment is unchanged.
 *  @exception MEMORY_ERROR Not enough memory to convert 'name' to the
 *             system string type.
 *  @exception RANGE_ERROR 'name' cannot be converted to the system string
 *             type or a system function returns an error.
 */
void cmdUnsetenv (const const_striType name)

  {
    os_striType env_name;
    int unsetenv_result;
    errInfoType err_info = OKAY_NO_ERROR;

  /* cmdUnsetenv */
    logFunction(printf("cmdUnsetenv(\"%s\")", striAsUnquotedCStri(name));
                fflush(stdout););
    env_name = stri_to_os_stri(name, &err_info);
    if (unlikely(env_name == NULL)) {
      logError(printf("cmdUnsetenv: stri_to_os_stri(\"%s\", *, *) failed:\n"
                      "err_info=%d\n",
                      striAsUnquotedCStri(name), err_info););
      raise_error(err_info);
    } else {
      unsetenv_result = os_unsetenv(env_name);
      if (unlikely(unsetenv_result != 0)) {
        logError(printf("cmdUnsetenv: os_unsetenv(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        env_name, errno, strerror(errno)););
        err_info = RANGE_ERROR;
      } /* if */
      os_stri_free(env_name);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(err_info);
      } /* if */
    } /* if */
    logFunction(printf("cmdUnsetenv -->\n"););
  } /* cmdUnsetenv */

#else



/**
 *  Deletes the variable 'name' from the environment.
 *  If 'name' does not exist in the environment,
 *  then the function succeeds, and the environment is unchanged.
 *  @exception MEMORY_ERROR Not enough memory to convert 'name' to the
 *             system string type.
 *  @exception RANGE_ERROR 'name' cannot be converted to the system string
 *             type or a system function returns an error.
 */
void cmdUnsetenv (const const_striType name)

  {
    memSizeType stri_size;
    striType stri;
    os_striType env_stri;
    int putenv_result;
    errInfoType err_info = OKAY_NO_ERROR;

  /* cmdUnsetenv */
    logFunction(printf("cmdUnsetenv(\"%s\")", striAsUnquotedCStri(name));
                fflush(stdout););
    if (strChPos(name, (charType) '=') != 0) {
      logError(printf("cmdUnsetenv(\"%s\"): "
                      "Name contains '=' (putenv() works with \"name=value\").\n",
                      striAsUnquotedCStri(name)););
      err_info = RANGE_ERROR;
    } else if (unlikely(name->size > MAX_STRI_LEN - 1)) {
      /* Number of bytes does not fit into memSizeType. */
      err_info = MEMORY_ERROR;
    } else {
      stri_size = name->size + 1;
      if (unlikely(!ALLOC_STRI_SIZE_OK(stri, stri_size))) {
        err_info = MEMORY_ERROR;
      } else {
        stri->size = stri_size;
        memcpy(stri->mem, name->mem,
            name->size * sizeof(strElemType));
        stri->mem[name->size] = (strElemType) '=';
        env_stri = stri_to_os_stri(stri, &err_info);
        FREE_STRI(stri, stri->size);
        if (likely(env_stri != NULL)) {
          /* printf("os_putenv(\"" FMT_S_OS "\")\n", env_stri); */
          putenv_result = os_putenv(env_stri);
          if (unlikely(putenv_result != 0)) {
            logError(printf("cmdUnsetenv: os_putenv(\"" FMT_S_OS "\") failed:\n"
                            "errno=%d\nerror: %s\n",
                            env_stri, errno, strerror(errno)););
            err_info = RANGE_ERROR;
          } /* if */
#if DELETE_PUTENV_ARGUMENT
          os_stri_free(env_stri);
#endif
        } /* if */
      } /* if */
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    logFunction(printf("cmdUnsetenv -->\n"););
  } /* cmdUnsetenv */

#endif

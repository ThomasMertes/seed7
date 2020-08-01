/********************************************************************/
/*                                                                  */
/*  cmd_rtl.c     Primitive actions for various commands.           */
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
/*  File: seed7/src/cmd_rtl.c                                       */
/*  Changes: 1994, 2006, 2009  Thomas Mertes                        */
/*  Content: Primitive actions for various commands.                */
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
#ifdef OS_PATH_WCHAR
#include "wchar.h"
#ifdef OS_WIDE_DIR_INCLUDE_DIR_H
#include "dir.h"
#endif
#ifdef OS_CHMOD_INCLUDE_IO_H
#include "io.h"
#endif
#endif
#include "errno.h"

#ifdef USE_MYUNISTD_H
#include "myunistd.h"
#else
#include "unistd.h"
#endif

#include "dir_drv.h"

#include "common.h"
#include "data_rtl.h"
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


#ifndef EXECUTABLE_FILE_EXTENSION
#define EXECUTABLE_FILE_EXTENSION ""
#endif
#ifndef CC_FLAGS
#define CC_FLAGS ""
#endif
#ifndef LINKER_OPT_DEBUG_INFO
#define LINKER_OPT_DEBUG_INFO ""
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

#define MAX_SYMLINK_PATH_LENGTH 0x1FFFFFFF

#define USR_BITS_NORMAL (S_IRUSR == 0400 && S_IWUSR == 0200 && S_IXUSR == 0100)
#define GRP_BITS_NORMAL (S_IRGRP == 0040 && S_IWGRP == 0020 && S_IXGRP == 0010)
#define OTH_BITS_NORMAL (S_IROTH == 0004 && S_IWOTH == 0002 && S_IXOTH == 0001)
#define MODE_BITS_NORMAL (USR_BITS_NORMAL && GRP_BITS_NORMAL && OTH_BITS_NORMAL)



#ifdef ANSI_C

#ifdef USE_CDECL
static int _cdecl cmp_mem (char *strg1, char *strg2)
#else
static int cmp_mem (void const *strg1, void const *strg2)
#endif
#else

static int cmp_mem (strg1, strg2)
char *strg1;
char *strg2;
#endif

  { /* cmp_mem */
    return (int) strCompare(
        ((const_rtlObjecttype *) strg1)->value.strivalue,
        ((const_rtlObjecttype *) strg2)->value.strivalue);
  } /* cmp_mem */



#ifdef ANSI_C
static void remove_any_file (os_stritype, errinfotype *);
static void copy_any_file (os_stritype, os_stritype, int, errinfotype *);
#else
static void remove_any_file ();
static void copy_any_file ();
#endif



#ifdef ANSI_C

static void remove_dir (os_stritype dir_name, errinfotype *err_info)
#else

static void remove_dir (dir_name, err_info)
os_stritype dir_name;
errinfotype *err_info;
#endif

  {
    os_DIR *directory;
    os_dirent_struct *current_entry;
    size_t dir_name_size;
    size_t dir_path_capacity = 0;
    os_stritype dir_path = NULL;
    size_t new_size;
    os_stritype resized_path;
    booltype init_path = TRUE;

  /* remove_dir */
#ifdef TRACE_CMD_RTL
    printf("BEGIN remove_dir(\"%s\")\n", dir_name);
#endif
/*  printf("opendir(%s);\n", dir_name);
    fflush(stdout); */
    if ((directory = os_opendir(dir_name)) == NULL) {
      *err_info = FILE_ERROR;
    } else {
      do {
        current_entry = os_readdir(directory);
/*      printf("$%ld$\n", (long) current_entry);
        fflush(stdout); */
      } while (current_entry != NULL &&
          (memcmp(current_entry->d_name, dot,    sizeof(os_chartype) * 2) == 0 ||
           memcmp(current_entry->d_name, dotdot, sizeof(os_chartype) * 3) == 0));
      dir_name_size = os_stri_strlen(dir_name);
      while (*err_info == OKAY_NO_ERROR && current_entry != NULL) {
/*      printf("!%s!\n", current_entry->d_name);
        fflush(stdout); */
        new_size = dir_name_size + 1 + os_stri_strlen(current_entry->d_name);
        if (new_size > dir_path_capacity) {
          resized_path = os_stri_realloc(dir_path, new_size);
          if (resized_path != NULL) {
            dir_path = resized_path;
            dir_path_capacity = new_size;
          } else if (dir_path != NULL) {
            os_stri_free(dir_path);
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
            (memcmp(current_entry->d_name, dot,    sizeof(os_chartype) * 2) == 0 ||
             memcmp(current_entry->d_name, dotdot, sizeof(os_chartype) * 3) == 0));
      } /* while */
      if (dir_path != NULL) {
        os_stri_free(dir_path);
      } /* if */
      os_closedir(directory);
      if (*err_info == OKAY_NO_ERROR) {
        /* printf("before remove directory <%s>\n", dir_name); */
        if (os_rmdir(dir_name) != 0) {
          *err_info = FILE_ERROR;
/*        printf("errno=%d\n", errno);
          printf("EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d  ENOTDIR=%d  EROFS=%d\n",
              EACCES, EBUSY, EEXIST, ENOTEMPTY, ENOENT, ENOTDIR, EROFS);
          printf("dir_name=\"%ls\"\n", dir_name); */
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END remove_dir(\"%s\", %d)\n", dir_name, *err_info);
#endif
  } /* remove_dir */



#ifdef ANSI_C

static void remove_any_file (os_stritype file_name, errinfotype *err_info)
#else

static void remove_any_file (file_name, err_info)
os_stritype file_name;
errinfotype *err_info;
#endif

  {
    os_stat_struct file_stat;

  /* remove_any_file */
#ifdef TRACE_CMD_RTL
    printf("BEGIN remove_any_file(\"%s\")\n", file_name);
#endif
    if (os_lstat(file_name, &file_stat) != 0) {
      /* File does not exist */
      *err_info = FILE_ERROR;
    } else {
      if (S_ISDIR(file_stat.st_mode)) {
        remove_dir(file_name, err_info);
      } else {
        if (os_remove(file_name) != 0) {
          *err_info = FILE_ERROR;
        } /* if */
      } /* if */
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END remove_any_file(\"%s\", %d)\n", file_name, *err_info);
#endif
  } /* remove_any_file */



#ifdef ANSI_C

static void copy_file (os_stritype from_name, os_stritype to_name, errinfotype *err_info)
#else

static void copy_file (from_name, to_name, err_info)
os_stritype from_name;
os_stritype to_name;
errinfotype *err_info;
#endif

  {
    FILE *from_file;
    FILE *to_file;
#ifdef USE_MMAP
    int file_no;
    os_fstat_struct file_stat;
    memsizetype file_length;
    ustritype file_content;
    booltype classic_copy = TRUE;
#endif
    char *buffer;
    size_t buffer_size;
    char reserve_buffer[SIZE_RESERVE_BUFFER];
    size_t bytes_read;

  /* copy_file */
#ifdef TRACE_CMD_RTL
    printf("BEGIN copy_file(\"%s\", \"%s\")\n", from_name, to_name);
#endif
#ifdef OS_PATH_WCHAR
    if ((from_file = wide_fopen(from_name, L"rb")) != NULL) {
      if ((to_file = wide_fopen(to_name, L"wb")) != NULL) {
#else
    if ((from_file = fopen(from_name, "rb")) != NULL) {
      if ((to_file = fopen(to_name, "wb")) != NULL) {
#endif
#ifdef USE_MMAP
        file_no = fileno(from_file);
        if (file_no != -1 && os_fstat(file_no, &file_stat) == 0) {
          if (file_stat.st_size < MAX_MEMSIZETYPE && file_stat.st_size >= 0) {
            file_length = (memsizetype) file_stat.st_size;
            if ((file_content = (ustritype) mmap(NULL, file_length,
                PROT_READ, MAP_PRIVATE, fileno(from_file),
                0)) != (ustritype) -1) {
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
          if (ALLOC_BYTES(buffer, SIZE_NORMAL_BUFFER)) {
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
          if (buffer != reserve_buffer) {
            FREE_BYTES(buffer, SIZE_NORMAL_BUFFER);
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
        fclose(from_file);
        *err_info = FILE_ERROR;
      } /* if */
    } else {
      *err_info = FILE_ERROR;
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END copy_file(\"%s\", \"%s\", %d)\n",
        from_name, to_name, *err_info);
#endif
  } /* copy_file */



#ifdef ANSI_C

static void copy_dir (os_stritype from_name, os_stritype to_name,
    int flags, errinfotype *err_info)
#else

static void copy_dir (from_name, to_name, flags, err_info)
os_stritype from_name;
os_stritype to_name;
int flags;
errinfotype *err_info;
#endif

  {
    os_DIR *directory;
    os_dirent_struct *current_entry;
    size_t from_name_size;
    size_t to_name_size;
    size_t d_name_size;
    size_t from_path_capacity = 0;
    os_stritype from_path = NULL;
    size_t to_path_capacity = 0;
    os_stritype to_path = NULL;
    size_t new_size;
    os_stritype resized_path;
    booltype init_path = TRUE;

  /* copy_dir */
#ifdef TRACE_CMD_RTL
    printf("BEGIN copy_dir(\"%s\", \"%s\")\n", from_name, to_name);
#endif
    /* printf("opendir(%s);\n", from_name);
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
            (memcmp(current_entry->d_name, dot,    sizeof(os_chartype) * 2) == 0 ||
             memcmp(current_entry->d_name, dotdot, sizeof(os_chartype) * 3) == 0));
        from_name_size = os_stri_strlen(from_name);
        to_name_size = os_stri_strlen(to_name);
        while (*err_info == OKAY_NO_ERROR && current_entry != NULL) {
          /* printf("!%s!\n", current_entry->d_name);
             fflush(stdout); */
          d_name_size = os_stri_strlen(current_entry->d_name);
          new_size = from_name_size + 1 + d_name_size;
          if (new_size > from_path_capacity) {
            resized_path = os_stri_realloc(from_path, new_size);
            if (resized_path != NULL) {
              from_path = resized_path;
              from_path_capacity = new_size;
            } else if (from_path != NULL) {
              os_stri_free(from_path);
              from_path = NULL;
            } /* if */
          } /* if */
          new_size = to_name_size + 1 + d_name_size;
          if (new_size > to_path_capacity) {
            resized_path = os_stri_realloc(to_path, new_size);
            if (resized_path != NULL) {
              to_path = resized_path;
              to_path_capacity = new_size;
            } else if (to_path != NULL) {
              os_stri_free(to_path);
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
              (memcmp(current_entry->d_name, dot,    sizeof(os_chartype) * 2) == 0 ||
               memcmp(current_entry->d_name, dotdot, sizeof(os_chartype) * 3) == 0));
        } /* while */
        if (*err_info != OKAY_NO_ERROR) {
          remove_dir(to_name, err_info);
        } /* if */
        if (from_path != NULL) {
          os_stri_free(from_path);
        } /* if */
        if (to_path != NULL) {
          os_stri_free(to_path);
        } /* if */
      } /* if */
      os_closedir(directory);
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END copy_dir(\"%s\", \"%s\", %d)\n",
        from_name, to_name, *err_info);
#endif
  } /* copy_dir */



#ifdef ANSI_C

static void copy_any_file (os_stritype from_name, os_stritype to_name,
    int flags, errinfotype *err_info)
#else

static void copy_any_file (from_name, to_name, flags, err_info)
os_stritype from_name;
os_stritype to_name;
int flags;
errinfotype *err_info;
#endif

  {
    os_stat_struct from_stat;
    int from_stat_result;
#ifdef HAS_SYMLINKS
    os_stritype link_destination;
    ssize_t readlink_result;
#endif
    os_utimbuf_struct to_utime;

  /* copy_any_file */
#ifdef TRACE_CMD_RTL
    printf("BEGIN copy_any_file(\"%s\", \"%s\")\n", from_name, to_name);
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
        if (from_stat.st_size > MAX_SYMLINK_PATH_LENGTH || from_stat.st_size < 0) {
          *err_info = RANGE_ERROR;
        } else {
          if (!os_stri_alloc(link_destination, from_stat.st_size)) {
            *err_info = MEMORY_ERROR;
          } else {
            readlink_result = readlink(from_name, link_destination,
                                       (size_t) from_stat.st_size);
            if (readlink_result != -1) {
              link_destination[readlink_result] = '\0';
              /* printf("readlink_result=%lu\n", readlink_result);
                 printf("link=%s\n", link_destination); */
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
    printf("END copy_any_file(\"%s\", \"%s\", %d)\n",
        from_name, to_name, *err_info);
#endif
  } /* copy_any_file */



#ifdef ANSI_C

static void move_any_file (os_stritype from_name, os_stritype to_name, errinfotype *err_info)
#else

static void move_any_file (from_name, to_name, err_info)
os_stritype from_name;
os_stritype to_name;
errinfotype *err_info;
#endif

  {
    os_stat_struct to_stat;

  /* move_any_file */
#ifdef TRACE_CMD_RTL
    printf("BEGIN move_any_file(\"%s\", \"%s\")\n", from_name, to_name);
#endif
    if (os_stat(to_name, &to_stat) == 0) {
      /* Destination file exists already */
      *err_info = FILE_ERROR;
    } else {
      if (os_rename(from_name, to_name) != 0) {
        switch (errno) {
          case EXDEV:
            copy_any_file(from_name, to_name, PRESERVE_ALL, err_info);
            if (*err_info == OKAY_NO_ERROR) {
              remove_any_file(from_name, err_info);
            } /* if */
            break;
          default:
            /* printf("errno=%d\n", errno);
            printf("EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d  ENOTDIR=%d  EROFS=%d\n",
                EACCES, EBUSY, EEXIST, ENOTEMPTY, ENOENT, ENOTDIR, EROFS); */
            *err_info = FILE_ERROR;
            break;
        } /* switch */
      } /* if */
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END move_any_file(\"%s\", \"%s\", %d)\n",
        from_name, to_name, *err_info);
#endif
  } /* move_any_file */



#ifdef ANSI_C

static rtlArraytype read_dir (const const_stritype dir_name, errinfotype *err_info)
#else

static rtlArraytype read_dir (dir_name, err_info)
stritype dir_name;
errinfotype *err_info;
#endif

  {
    rtlArraytype dir_array;
    rtlArraytype resized_dir_array;
    memsizetype max_array_size;
    inttype used_array_size;
    memsizetype position;
    dirtype directory;
    stritype stri1;

  /* read_dir */
/*  printf("opendir(");
    prot_stri(dir_name);
    printf(");\n");
    fflush(stdout); */
    if ((directory = dirOpen(dir_name)) != NULL) {
      max_array_size = 256;
      if (ALLOC_RTL_ARRAY(dir_array, max_array_size)) {
        used_array_size = 0;
        stri1 = dirRead(directory);
        while (*err_info == OKAY_NO_ERROR && stri1 != NULL) {
          if ((memsizetype) used_array_size >= max_array_size) {
            if (max_array_size >= MAX_MEM_INDEX - 256) {
              resized_dir_array = NULL;
            } else {
              resized_dir_array = REALLOC_RTL_ARRAY(dir_array,
                  max_array_size, max_array_size + 256);
            } /* if */
            if (resized_dir_array == NULL) {
              *err_info = MEMORY_ERROR;
            } else {
              dir_array = resized_dir_array;
              COUNT3_RTL_ARRAY(max_array_size, max_array_size + 256);
              max_array_size += 256;
            } /* if */
          } /* if */
          if (*err_info == OKAY_NO_ERROR) {
            dir_array->arr[used_array_size].value.strivalue = stri1;
            used_array_size++;
            stri1 = dirRead(directory);
          } /* if */
        } /* while */
        if (*err_info == OKAY_NO_ERROR) {
          resized_dir_array = REALLOC_RTL_ARRAY(dir_array,
              max_array_size, (memsizetype) used_array_size);
          if (resized_dir_array == NULL) {
            *err_info = MEMORY_ERROR;
          } else {
            dir_array = resized_dir_array;
            COUNT3_RTL_ARRAY(max_array_size, (memsizetype) used_array_size);
            dir_array->min_position = 1;
            dir_array->max_position = used_array_size;
          } /* if */
        } /* if */
        if (*err_info != OKAY_NO_ERROR) {
          for (position = 0; position < (memsizetype) used_array_size; position++) {
            FREE_STRI(dir_array->arr[position].value.strivalue,
                dir_array->arr[position].value.strivalue->size);
          } /* for */
          FREE_RTL_ARRAY(dir_array, max_array_size);
          dir_array = NULL;
        } /* if */
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



#ifdef ANSI_C

stritype getProgramPath (const const_stritype source_file_name)
#else

stritype getProgramPath (source_file_name)
stritype source_file_name;
#endif

  {
    stritype cwd;
    stritype result;

  /* getProgramPath */
    if (source_file_name->size >= 1 &&
        source_file_name->mem[0] == (chartype) '/') {
      result = strCreate(source_file_name);
    } else {
      cwd = cmdGetcwd();
      result = concat_path(cwd, source_file_name);
      FREE_STRI(cwd, cwd->size);
    } /* if */
    if (result->size <= 4 ||
        result->mem[result->size - 4] != '.' ||
        result->mem[result->size - 3] != 's' ||
        result->mem[result->size - 2] != 'd' ||
        result->mem[result->size - 1] != '7') {
      strPush(&result, (chartype) '.');
      strPush(&result, (chartype) 's');
      strPush(&result, (chartype) 'd');
      strPush(&result, (chartype) '7');
    } /* if */
    return result;
  } /* getProgramPath */



#ifdef ANSI_C

biginttype cmdBigFileSize (const const_stritype file_name)
#else

biginttype cmdBigFileSize (file_name)
stritype file_name;
#endif

  {
    os_stritype os_path;
    os_stat_struct stat_buf;
    int stat_result;
    filetype aFile;
    errinfotype err_info = OKAY_NO_ERROR;
    biginttype result;

  /* cmdBigFileSize */
    os_path = cp_to_os_path(file_name, &err_info);
    if (unlikely(os_path == NULL)) {
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (file_name->size != 1 || file_name->mem[0] != '/') {
#endif
        raise_error(err_info);
        result = NULL;
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      } else {
        result = bigIConv(0);
      } /* if */
#endif
    } else {
      stat_result = os_stat(os_path, &stat_buf);
      if (stat_result == 0 && S_ISREG(stat_buf.st_mode)) {
        if (sizeof(stat_buf.st_size) == 8) {
          result = bigFromUInt64((uint64type) stat_buf.st_size);
        } else {
          result = bigFromUInt32((uint32type) stat_buf.st_size);
        } /* if */
      } else if (stat_result == 0 && S_ISDIR(stat_buf.st_mode)) {
        result = bigIConv(0);
      } else {
#ifdef OS_PATH_WCHAR
        aFile = wide_fopen(os_path, L"r");
#else
        aFile = fopen(os_path, "r");
#endif
        if (aFile == NULL) {
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



#ifdef ANSI_C

void cmdChdir (const const_stritype dir_name)
#else

void cmdChdir (dir_name)
stritype dir_name;
#endif

  {
    os_stritype os_path;
    errinfotype err_info = OKAY_NO_ERROR;
    int chdir_result;

  /* cmdChdir */
    os_path = cp_to_os_path(dir_name, &err_info);
    if (unlikely(os_path == NULL)) {
      raise_error(err_info);
    } else {
      chdir_result = os_chdir(os_path);
      os_stri_free(os_path);
      if (unlikely(chdir_result != 0)) {
        raise_error(FILE_ERROR);
      } /* if */
    } /* if */
  } /* cmdChdir */



#ifdef ANSI_C

void cmdCloneFile (const const_stritype source_name, const const_stritype dest_name)
#else

void cmdCloneFile (source_name, dest_name)
stritype source_name;
stritype dest_name;
#endif

  {
    os_stritype os_source_name;
    os_stritype os_dest_name;
    os_stat_struct to_stat;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdCloneFile */
    os_source_name = cp_to_os_path(source_name, &err_info);
    if (os_source_name != NULL) {
      os_dest_name = cp_to_os_path(dest_name, &err_info);
      if (os_dest_name != NULL) {
        if (os_stat(os_dest_name, &to_stat) == 0) {
          /* Destination file exists already */
          err_info = FILE_ERROR;
        } else {
          copy_any_file(os_source_name, os_dest_name, PRESERVE_ALL, &err_info);
        } /* if */
        os_stri_free(os_dest_name);
      } /* if */
      os_stri_free(os_source_name);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* cmdCloneFile */



#ifdef ANSI_C

stritype cmdConfigValue (const const_stritype name)
#else

stritype cmdConfigValue (name)
stritype name;
#endif

  {
    char opt_name[250];
    const_cstritype opt;
    char buffer[4];
    stritype result;

  /* cmdConfigValue */
    if (name->size >= 250 / MAX_UTF8_EXPANSION_FACTOR) {
      opt = "";
    } else {
      stri_export((ustritype) opt_name, name);
      if (strcmp(opt_name, "OBJECT_FILE_EXTENSION") == 0) {
        opt = OBJECT_FILE_EXTENSION;
      } else if (strcmp(opt_name, "EXECUTABLE_FILE_EXTENSION") == 0) {
        opt = EXECUTABLE_FILE_EXTENSION;
      } else if (strcmp(opt_name, "C_COMPILER") == 0) {
        opt = C_COMPILER;
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
      } else if (strcmp(opt_name, "LINKER_OPT_OUTPUT_FILE") == 0) {
        opt = LINKER_OPT_OUTPUT_FILE;
      } else if (strcmp(opt_name, "LINKER_FLAGS") == 0) {
        opt = LINKER_FLAGS;
      } else if (strcmp(opt_name, "SYSTEM_LIBS") == 0) {
        opt = SYSTEM_LIBS;
      } else if (strcmp(opt_name, "SEED7_LIB") == 0) {
        opt = SEED7_LIB;
      } else if (strcmp(opt_name, "COMP_DATA_LIB") == 0) {
        opt = COMP_DATA_LIB;
      } else if (strcmp(opt_name, "COMPILER_LIB") == 0) {
        opt = COMPILER_LIB;
      } else if (strcmp(opt_name, "INT32TYPE") == 0) {
        opt = INT32TYPE_STRI;
      } else if (strcmp(opt_name, "UINT32TYPE") == 0) {
        opt = UINT32TYPE_STRI;
      } else if (strcmp(opt_name, "INT64TYPE") == 0) {
        opt = INT64TYPE_STRI;
      } else if (strcmp(opt_name, "UINT64TYPE") == 0) {
        opt = UINT64TYPE_STRI;
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
      } else if (strcmp(opt_name, "FLOATTYPE_DOUBLE") == 0) {
#ifdef FLOATTYPE_DOUBLE
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else if (strcmp(opt_name, "USE_SIGSETJMP") == 0) {
#ifdef USE_SIGSETJMP
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
      } else if (strcmp(opt_name, "CHECK_INT_DIV_BY_ZERO") == 0) {
#ifdef CHECK_INT_DIV_BY_ZERO
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
      } else if (strcmp(opt_name, "FLOAT_ZERO_DIV_ERROR") == 0) {
#ifdef FLOAT_ZERO_DIV_ERROR
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
      } else {
        opt = "";
      } /* if */
    } /* if */
    result = cstri_to_stri(opt);
    if (unlikely(result == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    return result;
  } /* cmdConfigValue */



#ifdef ANSI_C

void cmdCopyFile (const const_stritype source_name, const const_stritype dest_name)
#else

void cmdCopyFile (source_name, dest_name)
stritype source_name;
stritype dest_name;
#endif

  {
    os_stritype os_source_name;
    os_stritype os_dest_name;
    os_stat_struct to_stat;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdCopyFile */
    os_source_name = cp_to_os_path(source_name, &err_info);
    if (os_source_name != NULL) {
      os_dest_name = cp_to_os_path(dest_name, &err_info);
      if (os_dest_name != NULL) {
        if (os_stat(os_dest_name, &to_stat) == 0) {
          /* Destination file exists already */
          err_info = FILE_ERROR;
        } else {
          copy_any_file(os_source_name, os_dest_name, PRESERVE_NOTHING, &err_info);
        } /* if */
        os_stri_free(os_dest_name);
      } /* if */
      os_stri_free(os_source_name);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* cmdCopyFile */



#ifdef ANSI_C

settype cmdFileMode (const const_stritype file_name)
#else

settype cmdFileMode (file_name)
stritype file_name;
#endif

  {
    os_stritype os_path;
    os_stat_struct stat_buf;
    int stat_result;
    settype result;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdFileMode */
#ifdef TRACE_CMD_RTL
    printf("BEGIN cmdFileMode(");
    prot_stri(file_name);
    printf(")\n");
#endif
    os_path = cp_to_os_path(file_name, &err_info);
    if (unlikely(os_path == NULL)) {
      raise_error(err_info);
      result = NULL;
    } else {
      stat_result = os_stat(os_path, &stat_buf);
      os_stri_free(os_path);
      if (unlikely(stat_result != 0)) {
        /* printf("errno=%d\n", errno); */
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



#ifdef ANSI_C

inttype cmdFileSize (const const_stritype file_name)
#else

inttype cmdFileSize (file_name)
stritype file_name;
#endif

  {
    os_stritype os_path;
    os_stat_struct stat_buf;
    int stat_result;
    filetype aFile;
    errinfotype err_info = OKAY_NO_ERROR;
    inttype result;

  /* cmdFileSize */
    os_path = cp_to_os_path(file_name, &err_info);
    if (unlikely(os_path == NULL)) {
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (file_name->size != 1 || file_name->mem[0] != '/') {
#endif
        raise_error(err_info);
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      } /* if */
#endif
      result = 0;
    } else {
      stat_result = os_stat(os_path, &stat_buf);
      if (stat_result == 0 && S_ISREG(stat_buf.st_mode)) {
        if (stat_buf.st_size > INTTYPE_MAX || stat_buf.st_size < 0) {
          err_info = RANGE_ERROR;
          result = 0;
        } else {
          result = (inttype) stat_buf.st_size;
        } /* if */
      } else if (stat_result == 0 && S_ISDIR(stat_buf.st_mode)) {
        result = 0;
      } else {
#ifdef OS_PATH_WCHAR
        aFile = wide_fopen(os_path, L"r");
#else
        aFile = fopen(os_path, "r");
#endif
        if (aFile == NULL) {
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



#ifdef ANSI_C

inttype cmdFileType (const const_stritype file_name)
#else

inttype cmdFileType (file_name)
stritype file_name;
#endif

  {
    os_stritype os_path;
    os_stat_struct stat_buf;
    int stat_result;
    errinfotype err_info = OKAY_NO_ERROR;
    inttype result;

  /* cmdFileType */
    os_path = cp_to_os_path(file_name, &err_info);
    if (unlikely(os_path == NULL)) {
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (file_name->size == 1 && file_name->mem[0] == '/') {
        result = 3;
      } else {
#endif
        raise_error(err_info);
        result = 0;
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      } /* if */
#endif
    } else {
      stat_result = os_stat(os_path, &stat_buf);
      os_stri_free(os_path);
      if (stat_result == 0) {
        if (S_ISREG(stat_buf.st_mode)) {
          result = 2;
        } else if (S_ISDIR(stat_buf.st_mode)) {
          result = 3;
        } else if (S_ISCHR(stat_buf.st_mode)) {
          result = 4;
        } else if (S_ISBLK(stat_buf.st_mode)) {
          result = 5;
        } else if (S_ISFIFO(stat_buf.st_mode)) {
          result = 6;
        } else if (S_ISLNK(stat_buf.st_mode)) {
          result = 7;
          raise_error(FILE_ERROR);
        } else if (S_ISSOCK(stat_buf.st_mode)) {
          result = 8;
        } else {
          result = 1;
        } /* if */
      } else {
        result = 0;
        if (unlikely(file_name->size != 0 && errno != ENOENT && errno != ENOTDIR)) {
          /* printf("errno=%d\n", errno);
          printf("EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d  ENOTDIR=%d  EROFS=%d\n",
              EACCES, EBUSY, EEXIST, ENOTEMPTY, ENOENT, ENOTDIR, EROFS);
          printf("EIO=%d  ELOOP=%d  ENAMETOOLONG=%d  EOVERFLOW=%d\n",
              EIO, ELOOP, ENAMETOOLONG, EOVERFLOW);
          printf("EBADF=%d  EFAULT=%d  ENOMEM=%d\n",
              EBADF, EFAULT, ENOMEM); */
          raise_error(FILE_ERROR);
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* cmdFileType */



#ifdef ANSI_C

inttype cmdFileTypeSL (const const_stritype file_name)
#else

inttype cmdFileTypeSL (file_name)
stritype file_name;
#endif

  {
    os_stritype os_path;
    os_stat_struct stat_buf;
    int stat_result;
    errinfotype err_info = OKAY_NO_ERROR;
    inttype result;

  /* cmdFileTypeSL */
    os_path = cp_to_os_path(file_name, &err_info);
    if (unlikely(os_path == NULL)) {
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (file_name->size == 1 && file_name->mem[0] == '/') {
        result = 3;
      } else {
#endif
        raise_error(err_info);
        result = 0;
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      } /* if */
#endif
    } else {
      stat_result = os_lstat(os_path, &stat_buf);
      os_stri_free(os_path);
      if (stat_result == 0) {
        if (S_ISREG(stat_buf.st_mode)) {
          result = 2;
        } else if (S_ISDIR(stat_buf.st_mode)) {
          result = 3;
        } else if (S_ISCHR(stat_buf.st_mode)) {
          result = 4;
        } else if (S_ISBLK(stat_buf.st_mode)) {
          result = 5;
        } else if (S_ISFIFO(stat_buf.st_mode)) {
          result = 6;
        } else if (S_ISLNK(stat_buf.st_mode)) {
          result = 7;
        } else if (S_ISSOCK(stat_buf.st_mode)) {
          result = 8;
        } else {
          result = 1;
        } /* if */
      } else {
        result = 0;
        if (unlikely(file_name->size != 0 && errno != ENOENT && errno != ENOTDIR)) {
          /* printf("errno=%d\n", errno);
          printf("EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d  ENOTDIR=%d  EROFS=%d\n",
              EACCES, EBUSY, EEXIST, ENOTEMPTY, ENOENT, ENOTDIR, EROFS); */
          raise_error(FILE_ERROR);
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* cmdFileTypeSL */



#ifdef ANSI_C

stritype cmdGetcwd (void)
#else

stritype cmdGetcwd ()
#endif

  {
    os_chartype buffer[PATH_MAX + 1];
    os_stritype cwd;
    stritype result;

  /* cmdGetcwd */
    if (unlikely((cwd = os_getcwd(buffer, PATH_MAX)) == NULL)) {
      raise_error(FILE_ERROR);
      result = NULL;
    } else {
      result = cp_from_os_path(cwd);
      if (unlikely(result == NULL)) {
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
    return result;
  } /* cmdGetcwd */



#ifdef ANSI_C

stritype cmdGetenv (const const_stritype name)
#else

stritype cmdGetenv (name)
stritype name;
#endif

  {
    os_stritype env_name;
    os_stritype env_value;
    errinfotype err_info = OKAY_NO_ERROR;
    static os_chartype null_char = (os_chartype) '\0';
    stritype result;

  /* cmdGetenv */
    env_name = stri_to_os_stri(name, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
      result = NULL;
    } else {
      env_value = os_getenv(env_name);
      os_stri_free(env_name);
      if (env_value == NULL) {
        result = os_stri_to_stri(&null_char);
      } else {
        result = os_stri_to_stri(env_value);
#ifdef USE_WGETENV_WSTRI
        os_stri_free(env_value);
#endif
      } /* if */
      if (unlikely(result == NULL)) {
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
    return result;
  } /* cmdGetenv */



#ifdef ANSI_C

void cmdGetATime (const const_stritype file_name,
    inttype *year, inttype *month, inttype *day, inttype *hour,
    inttype *min, inttype *sec, inttype *micro_sec, inttype *time_zone,
    booltype *is_dst)
#else

void cmdGetATime (file_name,
    year, month, day, hour, min, sec, micro_sec, time_zone, is_dst)
stritype file_name;
inttype *year;
inttype *month;
inttype *day;
inttype *hour;
inttype *min;
inttype *sec;
inttype *micro_sec;
inttype *time_zone;
booltype *is_dst;
#endif

  {
    os_stritype os_path;
    os_stat_struct stat_buf;
    int stat_result;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdGetATime */
#ifdef TRACE_CMD_RTL
    printf("BEGIN cmdGetATime\n");
#endif
    os_path = cp_to_os_path(file_name, &err_info);
    if (unlikely(os_path == NULL)) {
      raise_error(err_info);
    } else {
      stat_result = os_stat(os_path, &stat_buf);
      os_stri_free(os_path);
      if (unlikely(stat_result != 0)) {
        /* printf("errno=%d\n", errno); */
        raise_error(FILE_ERROR);
      } else {
        /* printf("cmdGetATime: st_atime=%ld\n", stat_buf.st_atime); */
        timFromTimestamp(stat_buf.st_atime,
            year, month, day, hour,
            min, sec, micro_sec, time_zone, is_dst);
      } /* if */
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END cmdGetATime(%04ld-%02ld-%02ld %02ld:%02ld:%02ld.%06ld %ld %d)\n",
        *year, *month, *day, *hour, *min, *sec,
        *micro_sec, *time_zone, *is_dst);
#endif
  } /* cmdGetATime */



#ifdef ANSI_C

void cmdGetCTime (const const_stritype file_name,
    inttype *year, inttype *month, inttype *day, inttype *hour,
    inttype *min, inttype *sec, inttype *micro_sec, inttype *time_zone,
    booltype *is_dst)
#else

void cmdGetCTime (file_name,
    year, month, day, hour, min, sec, micro_sec, time_zone, is_dst)
stritype file_name;
inttype *year;
inttype *month;
inttype *day;
inttype *hour;
inttype *min;
inttype *sec;
inttype *micro_sec;
inttype *time_zone;
booltype *is_dst;
#endif

  {
    os_stritype os_path;
    os_stat_struct stat_buf;
    int stat_result;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdGetCTime */
#ifdef TRACE_CMD_RTL
    printf("BEGIN cmdGetCTime\n");
#endif
    os_path = cp_to_os_path(file_name, &err_info);
    if (unlikely(os_path == NULL)) {
      raise_error(err_info);
    } else {
      stat_result = os_stat(os_path, &stat_buf);
      os_stri_free(os_path);
      if (unlikely(stat_result != 0)) {
        /* printf("errno=%d\n", errno); */
        raise_error(FILE_ERROR);
      } else {
        /* printf("cmdGetCTime: st_ctime=%ld\n", stat_buf.st_ctime); */
        timFromTimestamp(stat_buf.st_ctime,
            year, month, day, hour,
            min, sec, micro_sec, time_zone, is_dst);
      } /* if */
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END cmdGetCTime(%04ld-%02ld-%02ld %02ld:%02ld:%02ld.%06ld %ld %d)\n",
        *year, *month, *day, *hour, *min, *sec,
        *micro_sec, *time_zone, *is_dst);
#endif
  } /* cmdGetCTime */



#ifdef ANSI_C

void cmdGetMTime (const const_stritype file_name,
    inttype *year, inttype *month, inttype *day, inttype *hour,
    inttype *min, inttype *sec, inttype *micro_sec, inttype *time_zone,
    booltype *is_dst)
#else

void cmdGetMTime (file_name,
    year, month, day, hour, min, sec, micro_sec, time_zone, is_dst)
stritype file_name;
inttype *year;
inttype *month;
inttype *day;
inttype *hour;
inttype *min;
inttype *sec;
inttype *micro_sec;
inttype *time_zone;
booltype *is_dst;
#endif

  {
    os_stritype os_path;
    os_stat_struct stat_buf;
    int stat_result;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdGetMTime */
#ifdef TRACE_CMD_RTL
    printf("BEGIN cmdGetMTime(");
    prot_stri(file_name);
    printf(")\n");
#endif
    os_path = cp_to_os_path(file_name, &err_info);
    if (unlikely(os_path == NULL)) {
      raise_error(err_info);
    } else {
      stat_result = os_stat(os_path, &stat_buf);
      os_stri_free(os_path);
      if (unlikely(stat_result != 0)) {
        /* printf("errno=%d\n", errno); */
        raise_error(FILE_ERROR);
      } else {
        /* printf("cmdGetMTime: st_mtime=%ld\n", stat_buf.st_mtime); */
        timFromTimestamp(stat_buf.st_mtime,
            year, month, day, hour,
            min, sec, micro_sec, time_zone, is_dst);
      } /* if */
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END cmdGetMTime(%04ld-%02ld-%02ld %02ld:%02ld:%02ld.%06ld %ld %d)\n",
        *year, *month, *day, *hour, *min, *sec,
        *micro_sec, *time_zone, *is_dst);
#endif
  } /* cmdGetMTime */



#ifdef ANSI_C

rtlArraytype cmdLs (const const_stritype dir_name)
#else

rtlArraytype cmdLs (dir_name)
stritype dir_name;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;
    rtlArraytype result;

  /* cmdLs */
    result = read_dir(dir_name, &err_info);
    if (unlikely(result == NULL)) {
      raise_error(err_info);
    } else {
      qsort((void *) result->arr,
          (size_t) (result->max_position - result->min_position + 1),
          sizeof(rtlObjecttype), &cmp_mem);
    } /* if */
    return result;
  } /* cmdLs */



#ifdef ANSI_C

void cmdMkdir (const const_stritype dir_name)
#else

void cmdMkdir (dir_name)
stritype dir_name;
#endif

  {
    os_stritype os_path;
    int mkdir_result;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdMkdir */
    os_path = cp_to_os_path(dir_name, &err_info);
    if (unlikely(os_path == NULL)) {
      raise_error(err_info);
    } else {
      mkdir_result = os_mkdir(os_path, 0777);
      os_stri_free(os_path);
      if (unlikely(mkdir_result != 0)) {
        raise_error(FILE_ERROR);
      } /* if */
    } /* if */
  } /* cmdMkdir */



#ifdef ANSI_C

void cmdMove (const const_stritype source_name, const const_stritype dest_name)
#else

void cmdMove (source_name, dest_name)
stritype source_name;
stritype dest_name;
#endif

  {
    os_stritype os_source_name;
    os_stritype os_dest_name;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdMove */
    os_source_name = cp_to_os_path(source_name, &err_info);
    if (os_source_name != NULL) {
      os_dest_name = cp_to_os_path(dest_name, &err_info);
      if (os_dest_name != NULL) {
        move_any_file(os_source_name, os_dest_name, &err_info);
        os_stri_free(os_dest_name);
      } /* if */
      os_stri_free(os_source_name);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* cmdMove */



#ifdef ANSI_C

stritype cmdReadlink (const const_stritype link_name)
#else

stritype cmdReadlink (link_name)
stritype link_name;
#endif

  {
#ifdef HAS_SYMLINKS
    os_stritype os_link_name;
    os_stat_struct link_stat;
    os_stritype link_destination;
    ssize_t readlink_result;
#endif
    errinfotype err_info = OKAY_NO_ERROR;
    stritype result = NULL;

  /* cmdReadlink */
#ifdef HAS_SYMLINKS
    os_link_name = cp_to_os_path(link_name, &err_info);
    if (os_link_name != NULL) {
      if (os_lstat(os_link_name, &link_stat) != 0 || !S_ISLNK(link_stat.st_mode)) {
        err_info = FILE_ERROR;
      } else {
        /* printf("link size=%lu\n", link_stat.st_size); */
        if (link_stat.st_size > MAX_SYMLINK_PATH_LENGTH || link_stat.st_size < 0) {
          err_info = RANGE_ERROR;
        } else {
          if (!os_stri_alloc(link_destination, link_stat.st_size)) {
            err_info = MEMORY_ERROR;
          } else {
            readlink_result = readlink(os_link_name, link_destination,
                                       (size_t) link_stat.st_size);
            if (readlink_result != -1) {
              link_destination[readlink_result] = '\0';
              result = cp_from_os_path(link_destination);
              if (result == NULL) {
                err_info = MEMORY_ERROR;
              } /* if */
            } else {
              err_info = FILE_ERROR;
            } /* if */
            os_stri_free(link_destination);
          } /* if */
        } /* if */
      } /* if */
      os_stri_free(os_link_name);
    } /* if */
#else
    err_info = FILE_ERROR;
#endif
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
    return result;
  } /* cmdReadlink */



#ifdef ANSI_C

void cmdRemove (const const_stritype file_name)
#else

void cmdRemove (file_name)
stritype file_name;
#endif

  {
#ifdef REMOVE_FAILS_FOR_EMPTY_DIRS
    os_stat_struct file_stat;
#endif
    os_stritype os_file_name;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdRemove */
#ifdef TRACE_CMD_RTL
    printf("BEGIN cmdRemove(");
    prot_stri(file_name);
    printf(")\n");
#endif
    os_file_name = cp_to_os_path(file_name, &err_info);
    if (os_file_name != NULL) {
#ifdef REMOVE_FAILS_FOR_EMPTY_DIRS
      if (os_lstat(os_file_name, &file_stat) != 0) {
        /* File does not exist */
        err_info = FILE_ERROR;
      } else {
        if (S_ISDIR(file_stat.st_mode)) {
          if (os_rmdir(os_file_name) != 0) {
            err_info = FILE_ERROR;
          } /* if */
        } else {
          if (os_remove(os_file_name) != 0) {
            err_info = FILE_ERROR;
          } /* if */
        } /* if */
      } /* if */
#else
      if (os_remove(os_file_name) != 0) {
        err_info = FILE_ERROR;
      } /* if */
#endif
      os_stri_free(os_file_name);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END cmdRemove(");
    prot_stri(file_name);
    printf(")\n");
#endif
  } /* cmdRemove */



#ifdef ANSI_C

void cmdRemoveAnyFile (const const_stritype file_name)
#else

void cmdRemoveAnyFile (file_name)
stritype file_name;
#endif

  {
    os_stritype os_file_name;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdRemoveAnyFile */
#ifdef TRACE_CMD_RTL
    printf("BEGIN cmdRemoveAnyFile\n");
#endif
    os_file_name = cp_to_os_path(file_name, &err_info);
    if (os_file_name != NULL) {
      remove_any_file(os_file_name, &err_info);
      os_stri_free(os_file_name);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END cmdRemoveAnyFile\n");
#endif
  } /* cmdRemoveAnyFile */



#ifdef os_putenv
#ifdef ANSI_C

void cmdSetenv (const const_stritype name, const const_stritype value)
#else

void cmdSetenv (name, value)
stritype name;
stritype value;
#endif

  {
    memsizetype stri_size;
    stritype stri;
    os_stritype env_stri;
    int putenv_result;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdSetenv */
    if (unlikely(name->size > MAX_STRI_LEN - value->size - 1)) {
      /* number of bytes does not fit into memsizetype */
      raise_error(MEMORY_ERROR);
    } else {
      stri_size = name->size + value->size + 1;
      if (unlikely(!ALLOC_STRI_SIZE_OK(stri, stri_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        stri->size = stri_size;
        memcpy(stri->mem, name->mem,
            name->size * sizeof(strelemtype));
        stri->mem[name->size] = (strelemtype) '=';
        memcpy(&stri->mem[name->size + 1], value->mem,
            value->size * sizeof(strelemtype));
        env_stri = stri_to_os_stri(stri, &err_info);
        FREE_STRI(stri, stri->size);;
        if (unlikely(env_stri == NULL)) {
          raise_error(MEMORY_ERROR);
        } else {
          putenv_result = os_putenv(env_stri);
          os_stri_free(env_stri);
          if (unlikely(putenv_result != 0)) {
            /* printf("errno=%d\n", errno); */
            raise_error(RANGE_ERROR);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
  } /* cmdSetenv */

#else



#ifdef ANSI_C

void cmdSetenv (const const_stritype name, const const_stritype value)
#else

void cmdSetenv (name, value)
stritype name;
stritype value;
#endif

  {
    os_stritype env_name;
    os_stritype env_value;
    int setenv_result;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdSetenv */
    env_name = stri_to_os_stri(name, &err_info);
    if (unlikely(env_name == NULL)) {
      raise_error(err_info);
    } else {
      env_value = stri_to_os_stri(value, &err_info);
      if (unlikely(env_value == NULL)) {
        os_stri_free(env_name);
        raise_error(err_info);
      } else {
        setenv_result = os_setenv(env_name, env_value, 1);
        os_stri_free(env_name);
        os_stri_free(env_value);
        if (unlikely(setenv_result != 0)) {
          /* printf("errno=%d\n", errno); */
          if (errno == ENOMEM) {
            raise_error(MEMORY_ERROR);
          } else {
            raise_error(RANGE_ERROR);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
  } /* cmdSetenv */

#endif



#ifdef ANSI_C

void cmdSetATime (const const_stritype file_name,
    inttype year, inttype month, inttype day, inttype hour,
    inttype min, inttype sec, inttype micro_sec, inttype time_zone)
#else

void cmdSetATime (file_name,
    year, month, day, hour, min, sec, micro_sec, time_zone)
 stritype file_name;
inttype year;
inttype month;
inttype day;
inttype hour;
inttype min;
inttype sec;
inttype micro_sec;
inttype time_zone;
#endif

  {
    os_stritype os_path;
    os_stat_struct stat_buf;
    os_utimbuf_struct utime_buf;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdSetATime */
    os_path = cp_to_os_path(file_name, &err_info);
    if (os_path != NULL) {
      if (os_stat(os_path, &stat_buf) == 0) {
        utime_buf.actime = timToTimestamp(year, month, day, hour,
            min, sec, micro_sec, time_zone);
        /* printf("cmdSetATime: actime=%ld\n", utime_buf.actime); */
        utime_buf.modtime = stat_buf.st_mtime;
        if (utime_buf.actime == (time_t) -1) {
          err_info = RANGE_ERROR;
        } else if (os_utime(os_path, &utime_buf) != 0) {
          /* printf("errno=%d\n", errno);
          printf("EPERM=%d, EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d  ENOTDIR=%d  EROFS=%d\n",
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



#ifdef ANSI_C

void cmdSetFileMode (const const_stritype file_name, const const_settype mode)
#else

void cmdSetFileMode (file_name, mode)
stritype file_name;
settype mode;
#endif

  {
    os_stritype os_path;
    int int_mode;
    int chmod_result;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdSetFileMode */
#ifdef TRACE_CMD_RTL
    printf("BEGIN cmdSetFileMode(");
    prot_stri(file_name);
    printf(")\n");
#endif
    os_path = cp_to_os_path(file_name, &err_info);
    if (os_path != NULL) {
      int_mode = setSConv(mode);
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
        /* printf("\nerrno=%d %s\n", errno, strerror(errno)); */
        err_info = FILE_ERROR;
      } /* if */
      os_stri_free(os_path);
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* cmdSetFileMode */



#ifdef ANSI_C

void cmdSetMTime (const const_stritype file_name,
    inttype year, inttype month, inttype day, inttype hour,
    inttype min, inttype sec, inttype micro_sec, inttype time_zone)
#else

void cmdSetMTime (file_name,
    year, month, day, hour, min, sec, micro_sec, time_zone)
stritype file_name;
inttype year;
inttype month;
inttype day;
inttype hour;
inttype min;
inttype sec;
inttype micro_sec;
inttype time_zone;
#endif

  {
    os_stritype os_path;
    os_stat_struct stat_buf;
    os_utimbuf_struct utime_buf;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdSetMTime */
    os_path = cp_to_os_path(file_name, &err_info);
    if (os_path != NULL) {
      if (os_stat(os_path, &stat_buf) == 0) {
        utime_buf.actime = stat_buf.st_atime;
        utime_buf.modtime = timToTimestamp(year, month, day, hour,
            min, sec, micro_sec, time_zone);
        /* printf("cmdSetMTime: modtime=%ld\n", utime_buf.modtime); */
        if (utime_buf.actime == (time_t) -1) {
          err_info = RANGE_ERROR;
        } else if (os_utime(os_path, &utime_buf) != 0) {
          /* printf("errno=%d\n", errno);
          printf("EPERM=%d, EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d  ENOTDIR=%d  EROFS=%d\n",
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



#ifdef ANSI_C

inttype cmdShell (const const_stritype command_stri)
#else

inttype cmdShell (command_stri)
stritype command_stri;
#endif

  {
    os_stritype os_command_stri;
    errinfotype err_info = OKAY_NO_ERROR;
    inttype result;

  /* cmdShell */
    os_command_stri = cp_to_command(command_stri, &err_info);
    if (unlikely(os_command_stri == NULL)) {
      raise_error(err_info);
      result = 0;
    } else {
      result = (inttype) os_system(os_command_stri);
      os_stri_free(os_command_stri);
    } /* if */
    return result;
  } /* cmdShell */



#ifdef ANSI_C

void cmdSymlink (const const_stritype source_name, const const_stritype dest_name)
#else

void cmdSymlink (source_name, dest_name)
stritype source_name;
stritype dest_name;
#endif

  {
#ifdef HAS_SYMLINKS
    os_stritype os_source_name;
    os_stritype os_dest_name;
#endif
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdSymlink */
#ifdef HAS_SYMLINKS
    os_source_name = cp_to_os_path(source_name, &err_info);
    if (os_source_name != NULL) {
      os_dest_name = cp_to_os_path(dest_name, &err_info);
      if (os_dest_name != NULL) {
        if (symlink(os_source_name, os_dest_name) != 0) {
          err_info = FILE_ERROR;
        } /* if */
        os_stri_free(os_dest_name);
      } /* if */
      os_stri_free(os_source_name);
    } /* if */
#else
    err_info = FILE_ERROR;
#endif
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      raise_error(err_info);
    } /* if */
  } /* cmdSymlink */

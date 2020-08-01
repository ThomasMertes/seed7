/********************************************************************/
/*                                                                  */
/*  cmd_rtl.c     Primitive actions for various commands.           */
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
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/cmd_rtl.c                                       */
/*  Changes: 1994, 2006  Thomas Mertes                              */
/*  Content: Primitive actions for various commands.                */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "limits.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "errno.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "str_rtl.h"
#include "dir_rtl.h"
#include "rtl_err.h"

#ifdef USE_MYUNISTD_H
#include "myunistd.h"
#else
#include "unistd.h"
#endif

#ifdef USE_MMAP
#include "sys/mman.h"
#endif

#undef EXTERN
#define EXTERN
#include "cmd_rtl.h"


#undef TRACE_CMD_RTL


#ifndef PATH_MAX
#define PATH_MAX 2048
#endif


#ifndef S_ISLNK
#ifdef S_IFLNK
#define S_ISLNK(mode) (((mode) & S_IFMT) == S_IFLNK)
#else
#define S_ISLNK(mode) FALSE
#endif
#endif

#ifndef S_ISSOCK
#ifdef S_IFSOCK
#define S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)
#else
#define S_ISSOCK(mode) FALSE
#endif
#endif

#ifndef S_ISCHR
#ifdef S_IFCHR
#define S_ISCHR(mode) (((mode) & S_IFMT) == S_IFCHR)
#else
#define S_ISCHR(mode) FALSE
#endif
#endif

#ifndef S_ISBLK
#ifdef S_IFBLK
#define S_ISBLK(mode) (((mode) & S_IFMT) == S_IFBLK)
#else
#define S_ISBLK(mode) FALSE
#endif
#endif

#ifndef S_ISFIFO
#ifdef S_IFFIFO
#define S_ISFIFO(mode) (((mode) & S_IFMT) == S_IFFIFO)
#else
#define S_ISFIFO(mode) FALSE
#endif
#endif

#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
#endif

#ifndef S_IRUSR
#ifdef S_IREAD
#define S_IRUSR S_IREAD
#else
#define S_IRUSR 0000400
#endif
#endif

#ifndef S_IWUSR
#ifdef S_IREAD
#define S_IWUSR S_IWRITE
#else
#define S_IWUSR 0000200
#endif
#endif

#ifndef S_IXUSR
#ifdef S_IREAD
#define S_IXUSR S_IEXEC
#else
#define S_IXUSR 0000100
#endif
#endif

#ifndef S_IRGRP
#define S_IRGRP 0000040
#endif

#ifndef S_IWGRP
#define S_IWGRP 0000020
#endif

#ifndef S_IXGRP
#define S_IXGRP 0000010
#endif

#ifndef S_IROTH
#define S_IROTH 0000004
#endif

#ifndef S_IWOTH
#define S_IWOTH 0000002
#endif

#ifndef S_IXOTH
#define S_IXOTH 0000001
#endif

#ifndef S_IRWXU
#define S_IRWXU (S_IRUSR | S_IWUSR | S_IXUSR)
#endif

#ifndef S_IRWXG
#define S_IRWXG (S_IRGRP | S_IWGRP | S_IXGRP)
#endif

#ifndef S_IRWXO
#define S_IRWXO (S_IROTH | S_IWOTH | S_IXOTH)
#endif


#define SIZE_NORMAL_BUFFER   32768
#define SIZE_RESERVE_BUFFER   2048



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
    return(strCompare(
        ((rtlObjecttype *) strg1)->value.strivalue,
        ((rtlObjecttype *) strg2)->value.strivalue));
  } /* cmp_mem */



#ifdef ANSI_C
static void remove_any_file (char *, errinfotype *);
static void copy_any_file (char *, char *, errinfotype *);
#else
static void remove_any_file ();
static void copy_any_file ();
#endif



#ifdef ANSI_C

static void remove_dir (char *file_name, errinfotype *err_info)
#else

static void remove_dir (file_name, err_info)
char *file_name;
errinfotype *err_info;
#endif

  {
    DIR *directory;
    struct dirent *current_entry;
    SIZE_TYPE rem_file_name_size;
    cstritype rem_file_name;

  /* remove_dir */
#ifdef TRACE_CMD_RTL
    printf("BEGIN remove_dir (\"%s\")\n", file_name);
#endif
/*  printf("opendir(%s);\n", file_name);
    fflush(stdout); */
    if ((directory = opendir(file_name)) == NULL) {
      *err_info = FILE_ERROR;
    } else {
      do {
        current_entry = readdir(directory);
/*      printf("$%ld$\n", (long) current_entry);
        fflush(stdout); */
      } while (current_entry != NULL &&
          (strcmp(current_entry->d_name, ".") == 0 ||
          strcmp(current_entry->d_name, "..") == 0));
      while (*err_info == OKAY_NO_ERROR && current_entry != NULL) {
/*      printf("!%s!\n", current_entry->d_name);
        fflush(stdout); */
        rem_file_name_size = strlen(file_name) + 1 + strlen(current_entry->d_name);
        if (ALLOC_CSTRI(rem_file_name, rem_file_name_size)) {
          strcpy(rem_file_name, file_name);
          strcat(rem_file_name, "/");
          strcat(rem_file_name, current_entry->d_name);
          remove_any_file(rem_file_name, err_info);
          UNALLOC_CSTRI(rem_file_name, rem_file_name_size);
        } else {
          *err_info = MEMORY_ERROR;
        } /* if */
        do {
          current_entry = readdir(directory);
/*        printf("$%ld$\n", (long) current_entry);
          fflush(stdout); */
        } while (current_entry != NULL &&
            (strcmp(current_entry->d_name, ".") == 0 ||
            strcmp(current_entry->d_name, "..") == 0));
      } /* while */
      closedir(directory);
      if (*err_info == OKAY_NO_ERROR) {
        /* printf("before remove directory <%s>\n", file_name); */
        if (rmdir(file_name) != 0) {
          *err_info = FILE_ERROR;
        } /* if */
/*      okay = errno;
        printf("errno=%d\n", okay);
        printf("EACCES=%d  EBUSY=%d  EEXIST=%d  ENOTEMPTY=%d  ENOENT=%d  ENOTDIR=%d  EROFS=%d\n",
            EACCES, EBUSY, EEXIST, ENOTEMPTY, ENOENT, ENOTDIR, EROFS); */
        /* printf("remove ==> %d\n", remove(file_name)); */
      } /* if */
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END remove_dir (\"%s\")\n", file_name);
#endif
  } /* remove_dir */



#ifdef ANSI_C

static void remove_any_file (char *file_name, errinfotype *err_info)
#else

static void remove_any_file (file_name, err_info)
char *file_name;
errinfotype *err_info;
#endif

  {
    struct stat file_stat;

  /* remove_any_file */
#ifdef TRACE_CMD_RTL
    printf("BEGIN remove_any_file (\"%s\")\n", file_name);
#endif
    if (stat(file_name, &file_stat) == 0) {
      *err_info = FILE_ERROR;
    } else {
      if (S_ISLNK(file_stat.st_mode)) {
      } else if (S_ISREG(file_stat.st_mode)) {
        if (remove(file_name) != 0) {
          *err_info = FILE_ERROR;
        } /* if */
      } else if (S_ISDIR(file_stat.st_mode)) {
        remove_dir(file_name, err_info);
      } /* if */
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END remove_any_file (\"%s\")\n", file_name);
#endif
  } /* remove_any_file */



#ifdef ANSI_C

static void copy_file (char *from_name, char *to_name, errinfotype *err_info)
#else

static void copy_file (from_name, to_name, err_info)
char *from_name;
char *to_name;
errinfotype *err_info;
#endif

  {
    FILE *from_file;
    FILE *to_file;
#ifdef USE_MMAP
    struct stat file_stat;
    memsizetype file_length;
    ustritype file_content;
#else
    char *buffer;
    SIZE_TYPE buffer_size;
    char reserve_buffer[SIZE_RESERVE_BUFFER];
    SIZE_TYPE bytes_read;
#endif

  /* copy_file */
#ifdef TRACE_CMD_RTL
    printf("BEGIN copy_file (\"%s\", \"%s\")\n", from_name, to_name);
#endif
    if ((from_file = fopen(from_name, "rb")) != NULL) {
      if ((to_file = fopen(to_name, "wb")) != NULL) {
#ifdef USE_MMAP
        if (fstat(fileno(from_file), &file_stat) == 0) {
          file_length = file_stat.st_size;
          if ((file_content = (ustritype) mmap(NULL, file_length,
              PROT_READ, MAP_PRIVATE, fileno(from_file),
              0)) != (ustritype) -1) {
            if (fwrite(file_content, 1, file_length, to_file) != file_length) {
              *err_info = FILE_ERROR;
            } /* if */
          } /* if */
        } /* if */
#else
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
#endif
        if (fclose(from_file) != 0) {
          *err_info = FILE_ERROR;
        } /* if */
        if (fclose(to_file) != 0) {
          *err_info = FILE_ERROR;
        } /* if */
        if (*err_info != OKAY_NO_ERROR) {
          remove(to_name);
        } /* if */
      } else {
        fclose(from_file);
        *err_info = FILE_ERROR;
      } /* if */
    } else {
      *err_info = FILE_ERROR;
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END copy_file (\"%s\", \"%s\")\n", from_name, to_name);
#endif
  } /* copy_file */



#ifdef ANSI_C

static void copy_dir (char *from_name, char *to_name, errinfotype *err_info)
#else

static void copy_dir (from_name, to_name, err_info)
char *from_name;
char *to_name;
errinfotype *err_info;
#endif

  {
    DIR *directory;
    struct dirent *current_entry;
    char from_file_name[2000];
    char to_file_name[2000];
    struct stat to_stat;

  /* copy_dir */
#ifdef TRACE_CMD_RTL
    printf("BEGIN copy_dir (\"%s\", \"%s\")\n", from_name, to_name);
#endif
/*  printf("opendir(%s);\n", from_name);
    fflush(stdout); */
    if ((directory = opendir(from_name)) == NULL) {
      *err_info = FILE_ERROR;
    } else {
      if (stat(to_name, &to_stat) == 0) {
        /* Directore exists already */
        *err_info = FILE_ERROR;
      } else {
        if (mkdir(to_name, (S_IRWXU | S_IRWXG | S_IRWXO)) != 0) {
          *err_info = FILE_ERROR;
        } else {
          do {
            current_entry = readdir(directory);
  /*        printf("$%ld$\n", (long) current_entry);
            fflush(stdout); */
          } while (current_entry != NULL &&
              (strcmp(current_entry->d_name, ".") == 0 ||
              strcmp(current_entry->d_name, "..") == 0));
          while (*err_info == OKAY_NO_ERROR && current_entry != NULL) {
  /*        printf("!%s!\n", current_entry->d_name);
            fflush(stdout); */
            strcpy(from_file_name, from_name);
            strcat(from_file_name, "/");
            strcat(from_file_name, current_entry->d_name);
            strcpy(to_file_name, to_name);
            strcat(to_file_name, "/");
            strcat(to_file_name, current_entry->d_name);
            copy_any_file(from_file_name, to_file_name, err_info);
            do {
              current_entry = readdir(directory);
  /*          printf("$%ld$\n", (long) current_entry);
              fflush(stdout); */
            } while (current_entry != NULL &&
                (strcmp(current_entry->d_name, ".") == 0 ||
                strcmp(current_entry->d_name, "..") == 0));
          } /* while */
          if (*err_info != OKAY_NO_ERROR) {
            remove_dir(to_name, err_info);
          } /* if */
        } /* if */
      } /* if */
      closedir(directory);
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END copy_dir (\"%s\", \"%s\")\n", from_name, to_name);
#endif
  } /* copy_dir */



#ifdef ANSI_C

static void copy_any_file (char *from_name, char *to_name, errinfotype *err_info)
#else

static void copy_any_file (from_name, to_name, err_info)
char *from_name;
char *to_name;
errinfotype *err_info;
#endif

  {
    struct stat from_stat;
    struct stat to_stat;

  /* copy_any_file */
#ifdef TRACE_CMD_RTL
    printf("BEGIN copy_any_file (\"%s\", \"%s\")\n", from_name, to_name);
#endif
    if (stat(from_name, &from_stat) != 0) {
      *err_info = FILE_ERROR;
    } else {
      if (stat(to_name, &to_stat) == 0) {
        *err_info = FILE_ERROR;
      } else {
        if (S_ISLNK(from_stat.st_mode)) {
        } else if (S_ISREG(from_stat.st_mode)) {
          copy_file(from_name, to_name, err_info);
        } else if (S_ISDIR(from_stat.st_mode)) {
          copy_dir(from_name, to_name, err_info);
        } /* if */
        chmod(to_name, from_stat.st_mode);
        chown(to_name, from_stat.st_uid, from_stat.st_gid);
      } /* if */
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END copy_any_file (\"%s\", \"%s\")\n", from_name, to_name);
#endif
  } /* copy_any_file */



#ifdef ANSI_C

static void move_any_file (char *from_name, char *to_name, errinfotype *err_info)
#else

static void move_any_file (from_name, to_name, err_info)
char *from_name;
char *to_name;
errinfotype *err_info;
#endif

  { /* move_any_file */
#ifdef TRACE_CMD_RTL
    printf("BEGIN move_any_file (\"%s\", \"%s\")\n", from_name, to_name);
#endif
    if (rename(from_name, to_name) != 0) {
      switch (errno) {
        case EXDEV:
          copy_any_file(from_name, to_name, err_info);
          if (*err_info == OKAY_NO_ERROR) {
            remove_any_file(from_name, err_info);
          } else {
            remove_any_file(to_name, err_info);
          } /* if */
          break;
      } /* switch */
    } /* if */
#ifdef TRACE_CMD_RTL
    printf("END move_any_file (\"%s\", \"%s\", %d)\n", from_name, to_name, *err_info);
#endif
  } /* move_any_file */



#ifdef ANSI_C

static rtlArraytype read_dir (stritype dir_name, errinfotype *err_info)
#else

static rtlArraytype read_dir (dir_name, err_info)
stritype dir_name;
errinfotype *err_info;
#endif

  {
    rtlArraytype dir_array;
    rtlArraytype resized_dir_array;
    memsizetype max_array_size;
    memsizetype used_array_size;
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
          if (used_array_size >= max_array_size) {
            resized_dir_array = REALLOC_RTL_ARRAY(dir_array,
                max_array_size, max_array_size + 256);
            if (resized_dir_array == NULL) {
              *err_info = MEMORY_ERROR;
            } else {
              dir_array = resized_dir_array;
              COUNT3_RTL_ARRAY(max_array_size, max_array_size + 256);
              max_array_size += 256;
            } /* if */
          } /* if */
          if (*err_info == OKAY_NO_ERROR) {
            dir_array->arr[(int) used_array_size].value.strivalue = stri1;
            used_array_size++;
            stri1 = dirRead(directory);
          } /* if */
        } /* while */
        if (*err_info == OKAY_NO_ERROR) {
          resized_dir_array = REALLOC_RTL_ARRAY(dir_array,
              max_array_size, used_array_size);
          if (resized_dir_array == NULL) {
            *err_info = MEMORY_ERROR;
          } else {
            dir_array = resized_dir_array;
            COUNT3_RTL_ARRAY(max_array_size, used_array_size);
            dir_array->min_position = 1;
            dir_array->max_position = used_array_size;
          } /* if */
        } /* if */
        if (*err_info != OKAY_NO_ERROR) {
          for (position = 0; position < used_array_size; position++) {
            FREE_STRI(dir_array->arr[(int) position].value.strivalue,
                dir_array->arr[(int) position].value.strivalue->size);
          } /* for */
          FREE_RTL_ARRAY(dir_array, max_array_size);
          dir_array = NULL;
        } /* if */
      } else {
        *err_info = MEMORY_ERROR;
      } /* if */
      closedir(directory);
    } else {
      dir_array = NULL;
      *err_info = FILE_ERROR;
    } /* if */
    return(dir_array);
  } /* read_dir */



#ifdef ANSI_C

void cmdChdir (stritype dir_name)
#else

void cmdChdir (dir_name)
stritype dir_name;
#endif

  {
    cstritype os_dir_name;

  /* cmdChdir */
    os_dir_name = cp_to_cstri(dir_name);
    if (os_dir_name == NULL) {
      raise_error(MEMORY_ERROR);
    } else {
      if (chdir(os_dir_name) != 0) {
        free_cstri(os_dir_name, dir_name);
        raise_error(FILE_ERROR);
      } else {
        free_cstri(os_dir_name, dir_name);
      } /* if */
    } /* if */
  } /* cmdChdir */



#ifdef ANSI_C

stritype cmdConfigValue (stritype name)
#else

stritype cmdConfigValue (name)
stritype name;
#endif

  {
    char opt_name[250];
    cstritype opt;
    stritype result;

  /* cmdConfigValue */
    if (compr_size(name) + 1 > 250) {
      opt = "";
    } else {
      stri_export(opt_name, name);
      if (strcmp(opt_name, "OBJECT_FILE_EXTENSION") == 0) {
        opt = OBJECT_FILE_EXTENSION;
      } else if (strcmp(opt_name, "EXECUTABLE_FILE_EXTENSION") == 0) {
        opt = EXECUTABLE_FILE_EXTENSION;
      } else if (strcmp(opt_name, "C_COMPILER") == 0) {
        opt = C_COMPILER;
      } else if (strcmp(opt_name, "INHIBIT_C_WARNINGS") == 0) {
        opt = INHIBIT_C_WARNINGS;
      } else if (strcmp(opt_name, "REDIRECT_C_ERRORS") == 0) {
        opt = REDIRECT_C_ERRORS;
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
      } else if (strcmp(opt_name, "INTTYPE_LITERAL_SUFFIX") == 0) {
        opt = INTTYPE_LITERAL_SUFFIX;
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
      } else if (strcmp(opt_name, "FLOAT_ZERO_DIV_ERROR") == 0) {
#ifdef FLOAT_ZERO_DIV_ERROR
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else if (strcmp(opt_name, "INTTYPE_64BIT") == 0) {
#ifdef INTTYPE_64BIT
        opt = "TRUE";
#else
        opt = "FALSE";
#endif
      } else {
        opt = "";
      } /* if */
    } /* if */
    result = cstri_to_stri(opt);
    if (result == NULL) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      return(result);
    } /* if */
  } /* cmdConfigValue */



#ifdef ANSI_C

void cmdCopy (stritype source_name, stritype dest_name)
#else

void cmdCopy (source_name, dest_name)
stritype source_name;
stritype dest_name;
#endif

  {
    cstritype os_source_name;
    cstritype os_dest_name;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdCopy */
    os_source_name = cp_to_cstri(source_name);
    if (os_source_name == NULL) {
      err_info = MEMORY_ERROR;
    } else {
      os_dest_name = cp_to_cstri(dest_name);
      if (os_dest_name == NULL) {
        err_info = MEMORY_ERROR;
      } else {
        copy_any_file(os_source_name, os_dest_name, &err_info); /* SYS_FILE_EXCEPTION */
        free_cstri(os_dest_name, dest_name);
      } /* if */
      free_cstri(os_source_name, source_name);
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      raise_error(err_info);
    } /* if */
  } /* cmdCopy */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

inttype cmdFileCTime (stritype file_name,
    inttype *year, inttype *month, inttype *day, inttype *hour,
    inttype *min, inttype *sec, inttype *mycro_sec, inttype *time_zone)
#else

inttype cmdFileCTime (file_name)
stritype file_name;
#endif

  {
    cstritype os_file_name;
    struct stat stat_buf;
    inttype result;

  /* cmdFileCTime */
    os_file_name = cp_to_cstri(file_name);
    if (os_file_name == NULL) {
      raise_error(MEMORY_ERROR);
      return(0);
    } else {
      if (stat(os_file_name, &stat_buf) == 0) {
        stat_buf.st_ctime;
      } else {
        result = 0;
      } /* if */
      free_cstri(os_file_name, file_name);
    } /* if */
    return(result);
  } /* cmdFileCTime */
#endif



#ifdef ANSI_C

inttype cmdFileType (stritype file_name)
#else

inttype cmdFileType (file_name)
stritype file_name;
#endif

  {
    cstritype os_file_name;
    struct stat stat_buf;
    inttype result;

  /* cmdFileType */
    os_file_name = cp_to_cstri(file_name);
    if (os_file_name == NULL) {
      raise_error(MEMORY_ERROR);
      return(0);
    } else {
      if (stat(os_file_name, &stat_buf) == 0) {
        if (S_ISREG(stat_buf.st_mode)) {
          result = 1;
        } else if (S_ISDIR(stat_buf.st_mode)) {
          result = 2;
        } else if (S_ISCHR(stat_buf.st_mode)) {
          result = 3;
        } else if (S_ISBLK(stat_buf.st_mode)) {
          result = 4;
        } else if (S_ISFIFO(stat_buf.st_mode)) {
          result = 5;
        } else if (S_ISLNK(stat_buf.st_mode)) {
          result = 6;
        } else if (S_ISSOCK(stat_buf.st_mode)) {
          result = 7;
        } else {
          result = 0;
        } /* if */
      } else {
        result = 0;
      } /* if */
      free_cstri(os_file_name, file_name);
    } /* if */
    return(result);
  } /* cmdFileType */



#ifdef ANSI_C

stritype cmdGetcwd (void)
#else

stritype cmdGetcwd ()
#endif

  {
    char buffer[PATH_MAX + 1];
    char *cwd;
    stritype result;

  /* cmdGetcwd */
    if ((cwd = getcwd(buffer, PATH_MAX)) == NULL) {
      raise_error(MEMORY_ERROR);
      return(NULL);
    } else {
      result = cstri_to_stri(cwd);
      if (result == NULL) {
        raise_error(MEMORY_ERROR);
        return(NULL);
      } else {
        return(result);
      } /* if */
    } /* if */
  } /* cmdGetcwd */



#ifdef ANSI_C

inttype cmdLng (stritype file_name)
#else

inttype cmdLng (file_name)
stritype file_name;
#endif

  {
    cstritype os_file_name;
    struct stat stat_buf;
    inttype result;

  /* cmdLng */
    os_file_name = cp_to_cstri(file_name);
    if (os_file_name == NULL) {
      raise_error(MEMORY_ERROR);
      return(0);
    } else {
      if (stat(os_file_name, &stat_buf) == 0) {
        result = (inttype) stat_buf.st_size;
      } else {
        result = 0;
      } /* if */
      free_cstri(os_file_name, file_name);
    } /* if */
    return(result);
  } /* cmdLng */



#ifdef ANSI_C

rtlArraytype cmdLs (stritype dir_name)
#else

rtlArraytype cmdLs (dir_name)
stritype dir_name;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;
    rtlArraytype result;

  /* cmdLs */
    result = read_dir(dir_name, &err_info);
    if (result == NULL) {
      raise_error(err_info);
    } else {
      qsort((void *) result->arr,
          (size_t) (result->max_position - result->min_position + 1),
          sizeof(rtlObjecttype), &cmp_mem);
    } /* if */
    return(result);
  } /* cmdLs */



#ifdef ANSI_C

void cmdMkdir (stritype dir_name)
#else

void cmdMkdir (dir_name)
stritype dir_name;
#endif

  {
    cstritype os_dir_name;

  /* cmdMkdir */
    os_dir_name = cp_to_cstri(dir_name);
    if (os_dir_name == NULL) {
      raise_error(MEMORY_ERROR);
    } else {
      if (mkdir(os_dir_name, 0777) != 0) {
        free_cstri(os_dir_name, dir_name);
        raise_error(FILE_ERROR);
      } else {
        free_cstri(os_dir_name, dir_name);
      } /* if */
    } /* if */
  } /* cmdMkdir */



#ifdef ANSI_C

void cmdMove (stritype source_name, stritype dest_name)
#else

void cmdMove (source_name, dest_name)
stritype source_name;
stritype dest_name;
#endif

  {
    cstritype os_source_name;
    cstritype os_dest_name;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdMove */
    os_source_name = cp_to_cstri(source_name);
    if (os_source_name == NULL) {
      err_info = MEMORY_ERROR;
    } else {
      os_dest_name = cp_to_cstri(dest_name);
      if (os_dest_name == NULL) {
        err_info = MEMORY_ERROR;
      } else {
        move_any_file(os_source_name, os_dest_name, &err_info); /* SYS_FILE_EXCEPTION */
        free_cstri(os_dest_name, dest_name);
      } /* if */
      free_cstri(os_source_name, source_name);
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      raise_error(err_info);
    } /* if */
  } /* cmdMove */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

stritype cmdReadlink (stritype link_name)
#else

stritype cmdReadlink (link_name)
stritype link_name;
#endif

  {
    cstritype os_link_name;
    char link_dest[PATH_MAX + 1];
    int readlink_value;
    errinfotype err_info = OKAY_NO_ERROR;
    stritype result;

  /* cmdReadlink */
    os_link_name = cp_to_cstri(link_name);
    if (os_link_name == NULL) {
      err_info = MEMORY_ERROR;
      result = NULL;
    } else {
      readlink_value = readlink(os_link_name, link_dest, PATH_MAX);
      if (readlink_value != -1 && readlink_value <= PATH_MAX) {
        link_dest[readlink_value] = '\0';
        result = cstri_to_stri(link_dest);
        if (result == NULL) {
          err_info = MEMORY_ERROR;
        } /* if */
      } else {
        err_info = FILE_ERROR;
        result = NULL;
      } /* if */
      free_cstri(os_link_name, link_name);
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      raise_error(err_info);
    } /* if */
    return(result);
  } /* cmdReadlink */
#endif



#ifdef ANSI_C

void cmdRemove (stritype file_name)
#else

void cmdRemove (file_name)
stritype file_name;
#endif

  {
    cstritype os_file_name;

  /* cmdRemove */
    os_file_name = cp_to_cstri(file_name);
    if (os_file_name == NULL) {
      raise_error(MEMORY_ERROR);
    } else {
      remove(os_file_name);
      free_cstri(os_file_name, file_name);
    } /* if */
  } /* cmdRemove */



#ifdef ANSI_C

void cmdSh (stritype command_stri)
#else

void cmdSh (command_stri)
stritype command_stri;
#endif

  {
    cstritype os_command_stri;

  /* cmdSh */
    os_command_stri = cp_to_command(command_stri);
    if (os_command_stri == NULL) {
      raise_error(MEMORY_ERROR);
    } else {
      system(os_command_stri);
      free_cstri(os_command_stri, command_stri);
    } /* if */
  } /* cmdSh */



#ifdef ANSI_C

void cmdSymlink (stritype source_name, stritype dest_name)
#else

void cmdSymlink (source_name, dest_name)
stritype source_name;
stritype dest_name;
#endif

  {
    cstritype os_source_name;
    cstritype os_dest_name;
    errinfotype err_info = OKAY_NO_ERROR;

  /* cmdSymlink */
    os_source_name = cp_to_cstri(source_name);
    if (os_source_name == NULL) {
      err_info = MEMORY_ERROR;
    } else {
      os_dest_name = cp_to_cstri(dest_name);
      if (os_dest_name == NULL) {
        err_info = MEMORY_ERROR;
      } else {
#ifdef OUT_OF_ORDER
        if (symlink(os_source_name, os_dest_name) != 0) {
          err_info = FILE_ERROR;
        } /* if */
#endif
        free_cstri(os_dest_name, dest_name);
      } /* if */
      free_cstri(os_source_name, source_name);
    } /* if */
    if (err_info != OKAY_NO_ERROR) {
      raise_error(err_info);
    } /* if */
  } /* cmdSymlink */



#ifdef FTELL_WRONG_FOR_PIPE
#undef ftell



#ifdef ANSI_C

long improved_ftell (FILE *stream)
#else

long improved_ftell (stream)
FILE *stream;
#endif

  {
    int file_no;
    struct stat stat_buf;
    int result;

  /* improved_ftell */
    file_no = fileno(stream);
    if (file_no != -1 && fstat(file_no, &stat_buf) == 0 &&
        S_ISREG(stat_buf.st_mode)) {
      result = ftell(stream);
    } else {
      result = -1;
    } /* if */
    return(result);
  } /* improved_ftell */
#endif

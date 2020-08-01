/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2000  Thomas Mertes                        */
/*                                                                  */
/*  This program is free software; you can redistribute it and/or   */
/*  modify it under the terms of the GNU General Public License as  */
/*  published by the Free Software Foundation; either version 2 of  */
/*  the License, or (at your option) any later version.             */
/*                                                                  */
/*  This program is distributed in the hope that it will be useful, */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   */
/*  GNU General Public License for more details.                    */
/*                                                                  */
/*  You should have received a copy of the GNU General Public       */
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/unxlib.c                                        */
/*  Changes: 1994  Thomas Mertes                                    */
/*  Content: All primitive actions for files and directorys.        */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "version.h"
#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "striutl.h"
#include "runfile.h"
#include "runerr.h"
#include "memory.h"
#include "dir_drv.h"

#undef EXTERN
#define EXTERN
#include "unxlib.h"



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

  {
    register memsizetype size1;
    register memsizetype size2;
    int result;

  /* cmp_mem */
    size1 = ((objecttype) strg1)->value.strivalue->size;
    size2 = ((objecttype) strg2)->value.strivalue->size;
/*
    printf("MEM_CMP(");
    fwrite(((objecttype) strg1)->value.strivalue->mem, 1, size1, stdout);
    printf(",");
    fwrite(((objecttype) strg2)->value.strivalue->mem, 1, size2, stdout);
    printf(");\n");
*/
    if (size1 == size2) {
      return(memcmp(
          ((objecttype) strg1)->value.strivalue->mem,
          ((objecttype) strg2)->value.strivalue->mem,
          (SIZE_TYPE) size1));
    } else if (size1 > size2) {
      if ((result = memcmp(
          ((objecttype) strg1)->value.strivalue->mem,
          ((objecttype) strg2)->value.strivalue->mem,
          (SIZE_TYPE) size2)) == 0) {
        return(1);
      } /* if */
    } else {
      if ((result = memcmp(
          ((objecttype) strg1)->value.strivalue->mem,
          ((objecttype) strg2)->value.strivalue->mem,
          (SIZE_TYPE) size1)) == 0) {
        return(-1);
      } /* if */
    } /* if */
    return(result);
  } /* cmp_mem */



#ifdef ANSI_C

static arraytype read_dir (char *dir_name)
#else

static arraytype read_dir (dir_name)
char *dir_name;
#endif

  {
    arraytype dir_array;
    memsizetype max_array_size;
    memsizetype used_array_size;
    memsizetype position;
    DIR *directory;
    struct dirent *current_entry;
    stritype str1;
    booltype okay;

  /* read_dir */
/*  printf("opendir(%s);\n", dir_name);
    fflush(stdout); */
    if ((directory = opendir(dir_name)) != NULL) {
      max_array_size = 256;
      if (ALLOC_ARRAY(dir_array, max_array_size)) {
        COUNT_ARRAY(max_array_size);
        used_array_size = 0;
        do {
          current_entry = readdir(directory);
/*        printf("$%ld$\n", (long) current_entry);
          fflush(stdout); */
        } while (current_entry != NULL &&
          (strcmp(current_entry->d_name, ".") == 0 ||
          strcmp(current_entry->d_name, "..") == 0));
        okay = TRUE;
        while (okay && current_entry != NULL) {
          if (used_array_size >= max_array_size) {
            if (!RESIZE_ARRAY(dir_array, max_array_size,
                max_array_size + 256)) {
              okay = FALSE;
            } else {
              COUNT3_ARRAY(max_array_size, max_array_size + 256);
              max_array_size = max_array_size + 256;
            } /* if */
          } /* if */
          if (okay) {
            str1 = cp_to_stri(current_entry->d_name);
            if (str1 == NULL) {
              okay = FALSE;
            } else {
              dir_array->arr[(int) used_array_size].type_of = take_type(SYS_STRI_TYPE);
              dir_array->arr[(int) used_array_size].entity = NULL;
              dir_array->arr[(int) used_array_size].value.strivalue = str1;
              INIT_CLASS_OF_VAR(&dir_array->arr[(int) used_array_size],
                  STRIOBJECT);
              used_array_size++;
              do {
                current_entry = readdir(directory);
              } while (current_entry != NULL &&
                (strcmp(current_entry->d_name, ".") == 0 ||
                strcmp(current_entry->d_name, "..") == 0));
            } /* if */
          } /* if */
        } /* while */
        if (okay) {
          if (!RESIZE_ARRAY(dir_array, max_array_size,
              used_array_size)) {
            okay = FALSE;
          } else {
            COUNT3_ARRAY(max_array_size, used_array_size);
            dir_array->min_position = 1;
            dir_array->max_position = used_array_size;
          } /* if */
        } /* if */
        if (!okay) {
          for (position = 0; position < used_array_size; position++) {
            FREE_STRI(dir_array->arr[(int) position].value.strivalue,
                dir_array->arr[(int) position].value.strivalue->size);
          } /* for */
          FREE_ARRAY(dir_array, max_array_size);
          dir_array = NULL;
        } /* if */
      } /* if */
      closedir(directory);
    } else {
      if (ALLOC_ARRAY(dir_array, (memsizetype) 0)) {
        COUNT_ARRAY(0);
        dir_array->min_position = 1;
        dir_array->max_position = 0;
      } /* if */
    } /* if */
    return(dir_array);
  } /* read_dir */



#ifdef ANSI_C

objecttype unx_cd (listtype arguments)
#else

objecttype unx_cd (arguments)
listtype arguments;
#endif

  {
    stritype str1;
    cstritype dir_name;

  /* unx_cd */
    isit_stri(arg_1(arguments));
    str1 = take_stri(arg_1(arguments));
    dir_name = cp_to_cstri(str1);
    if (dir_name == NULL) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      chdir(dir_name);
      free_cstri(dir_name, str1);
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* unx_cd */



#ifdef ANSI_C

objecttype unx_cp (listtype arguments)
#else

objecttype unx_cp (arguments)
listtype arguments;
#endif

  {
    stritype old_str;
    stritype new_str;
    cstritype old_name;
    cstritype new_name;
    errinfotype err_info = NO_ERROR;

  /* unx_cp */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    old_str = take_stri(arg_1(arguments));
    new_str = take_stri(arg_2(arguments));
    old_name = cp_to_cstri(old_str);
    if (old_name == NULL) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      new_name = cp_to_cstri(new_str);
      if (new_name == NULL) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } else {
        copy_any_file(old_name, new_name, &err_info); /* SYS_FILE_EXCEPTION */
        free_cstri(new_name, new_str);
      } /* if */
      free_cstri(old_name, old_str);
    } /* if */
    if (err_info == FILE_ERROR) {
      return(raise_exception(SYS_IO_EXCEPTION));
    } else if (err_info != NO_ERROR) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      return(SYS_EMPTY_OBJECT);
    } /* if */
  } /* unx_cp */



#ifdef ANSI_C

objecttype unx_lng (listtype arguments)
#else

objecttype unx_lng (arguments)
listtype arguments;
#endif

  {
    stritype str1;
    cstritype file_name;
    struct stat stat_buf;
    inttype result;

  /* unx_lng */
    isit_stri(arg_1(arguments));
    str1 = take_stri(arg_1(arguments));
    file_name = cp_to_cstri(str1);
    if (file_name == NULL) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      if (stat(file_name, &stat_buf) == 0) {
        result = (inttype) stat_buf.st_size;
      } else {
        result = 0;
      } /* if */
      free_cstri(file_name, str1);
    } /* if */
    return(bld_int_temp(result));
  } /* unx_lng */



#ifdef ANSI_C

objecttype unx_ls (listtype arguments)
#else

objecttype unx_ls (arguments)
listtype arguments;
#endif

  {
    stritype str1;
    cstritype dir_name;
    arraytype result;

  /* unx_ls */
    isit_stri(arg_1(arguments));
    str1 = take_stri(arg_1(arguments));
    dir_name = cp_to_cstri(str1);
    if (dir_name == NULL) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      if ((result = read_dir(dir_name)) == NULL) {
        return(raise_with_arguments(SYS_MEM_EXCEPTION, arguments));
      } else {
        free_cstri(dir_name, str1);
        qsort((void *) result->arr,
            (size_t) (result->max_position - result->min_position + 1),
            sizeof(objectrecord), &cmp_mem);
        return(bld_array_temp(result));
      } /* if */
    } /* if */
  } /* unx_ls */



#ifdef ANSI_C

objecttype unx_mkdir (listtype arguments)
#else

objecttype unx_mkdir (arguments)
listtype arguments;
#endif

  {
    stritype str1;
    cstritype dir_name;

  /* unx_mkdir */
    isit_stri(arg_1(arguments));
    str1 = take_stri(arg_1(arguments));
    dir_name = cp_to_cstri(str1);
    if (dir_name == NULL) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      mkdir(dir_name, 0777);
      free_cstri(dir_name, str1);
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* unx_mkdir */



#ifdef ANSI_C

objecttype unx_mv (listtype arguments)
#else

objecttype unx_mv (arguments)
listtype arguments;
#endif

  {
    stritype old_str;
    stritype new_str;
    cstritype old_name;
    cstritype new_name;
    errinfotype err_info = NO_ERROR;

  /* unx_mv */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    old_str = take_stri(arg_1(arguments));
    new_str = take_stri(arg_2(arguments));
    old_name = cp_to_cstri(old_str);
    if (old_name == NULL) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      new_name = cp_to_cstri(new_str);
      if (new_name == NULL) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } else {
        move_any_file(old_name, new_name, &err_info); /* SYS_FILE_EXCEPTION */
        free_cstri(new_name, new_str);
      } /* if */
      free_cstri(old_name, old_str);
    } /* if */
    if (err_info != NO_ERROR) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      return(SYS_EMPTY_OBJECT);
    } /* if */
  } /* unx_mv */



#ifdef ANSI_C

objecttype unx_pwd (listtype arguments)
#else

objecttype unx_pwd (arguments)
listtype arguments;
#endif

  {
    char buffer[2000];
    char *cwd;
    stritype result;

  /* unx_pwd */
    if ((cwd = getcwd(buffer, 2000)) == NULL) {
      return(raise_exception(SYS_RNG_EXCEPTION));
    } else {
      result = cp_to_stri(cwd);
      if (result == NULL) {
        return(raise_exception(SYS_MEM_EXCEPTION));
      } else {
        return(bld_stri_temp(result));
      } /* if */
    } /* if */
  } /* unx_pwd */



#ifdef ANSI_C

objecttype unx_rm (listtype arguments)
#else

objecttype unx_rm (arguments)
listtype arguments;
#endif

  {
    stritype str1;
    cstritype file_name;

  /* unx_rm */
    isit_stri(arg_1(arguments));
    str1 = take_stri(arg_1(arguments));
    file_name = cp_to_cstri(str1);
    if (file_name == NULL) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      remove(file_name);
      free_cstri(file_name, str1);
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* unx_rm */



#ifdef ANSI_C

objecttype unx_sh (listtype arguments)
#else

objecttype unx_sh (arguments)
listtype arguments;
#endif

  {
    stritype str1;
    cstritype command_name;

  /* unx_sh */
    isit_stri(arg_1(arguments));
    str1 = take_stri(arg_1(arguments));
    command_name = cp_to_cstri(str1);
    if (command_name == NULL) {
      return(raise_exception(SYS_MEM_EXCEPTION));
    } else {
      system(command_name);
      free_cstri(command_name, str1);
    } /* if */
    return(SYS_EMPTY_OBJECT);
  } /* unx_sh */



#ifdef ANSI_C

objecttype unx_sleep (listtype arguments)
#else

objecttype unx_sleep (arguments)
listtype arguments;
#endif

  {
    inttype seconds;

  /* unx_sleep */
    isit_int(arg_1(arguments));
    seconds = take_int(arg_1(arguments));
    sleep(seconds);
    return(SYS_EMPTY_OBJECT);
  } /* unx_sleep */

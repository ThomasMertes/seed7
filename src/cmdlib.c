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
/*  File: seed7/src/cmdlib.c                                        */
/*  Changes: 1994  Thomas Mertes                                    */
/*  Content: Primitive actions for various commands.                */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "syvarutl.h"
#include "striutl.h"
#include "runerr.h"
#include "memory.h"
#include "dir_drv.h"
#include "str_rtl.h"
#include "cmd_rtl.h"

#undef EXTERN
#define EXTERN
#include "cmdlib.h"



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
        ((objecttype) strg1)->value.strivalue,
        ((objecttype) strg2)->value.strivalue));
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
            str1 = cstri_to_stri(current_entry->d_name);
            if (str1 == NULL) {
              okay = FALSE;
            } else {
              dir_array->arr[(int) used_array_size].type_of = take_type(SYS_STRI_TYPE);
              dir_array->arr[(int) used_array_size].descriptor.entity = NULL;
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

objecttype cmd_chdir (listtype arguments)
#else

objecttype cmd_chdir (arguments)
listtype arguments;
#endif

  { /* cmd_chdir */
    isit_stri(arg_1(arguments));
    cmdChdir(take_stri(arg_1(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* cmd_chdir */



#ifdef ANSI_C

objecttype cmd_copy (listtype arguments)
#else

objecttype cmd_copy (arguments)
listtype arguments;
#endif

  { /* cmd_copy */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    cmdCopy(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* cmd_copy */



#ifdef ANSI_C

objecttype cmd_filetype (listtype arguments)
#else

objecttype cmd_filetype (arguments)
listtype arguments;
#endif

  { /* cmd_filetype */
    isit_stri(arg_1(arguments));
    return(bld_int_temp(
        cmdFileType(take_stri(arg_1(arguments)))));
  } /* cmd_filetype */



#ifdef ANSI_C

objecttype cmd_getcwd (listtype arguments)
#else

objecttype cmd_getcwd (arguments)
listtype arguments;
#endif

  { /* cmd_getcwd */
    return(bld_stri_temp(cmdGetcwd()));
  } /* cmd_getcwd */



#ifdef ANSI_C

objecttype cmd_lng (listtype arguments)
#else

objecttype cmd_lng (arguments)
listtype arguments;
#endif

  { /* cmd_lng */
    isit_stri(arg_1(arguments));
    return(bld_int_temp(
        cmdLng(take_stri(arg_1(arguments)))));
  } /* cmd_lng */



#ifdef ANSI_C

objecttype cmd_ls (listtype arguments)
#else

objecttype cmd_ls (arguments)
listtype arguments;
#endif

  {
    stritype str1;
    cstritype dir_name;
    arraytype result;

  /* cmd_ls */
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
  } /* cmd_ls */



#ifdef ANSI_C

objecttype cmd_mkdir (listtype arguments)
#else

objecttype cmd_mkdir (arguments)
listtype arguments;
#endif

  {
    stritype str1;
    cstritype dir_name;

  /* cmd_mkdir */
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
  } /* cmd_mkdir */



#ifdef ANSI_C

objecttype cmd_move (listtype arguments)
#else

objecttype cmd_move (arguments)
listtype arguments;
#endif

  { /* cmd_move */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    cmdMove(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* cmd_move */



#ifdef ANSI_C

objecttype cmd_remove (listtype arguments)
#else

objecttype cmd_remove (arguments)
listtype arguments;
#endif

  { /* cmd_remove */
    isit_stri(arg_1(arguments));
    cmdRemove(take_stri(arg_1(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* cmd_remove */



#ifdef ANSI_C

objecttype cmd_sh (listtype arguments)
#else

objecttype cmd_sh (arguments)
listtype arguments;
#endif

  { /* cmd_sh */
    isit_stri(arg_1(arguments));
    cmdSh(take_stri(arg_1(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* cmd_sh */



#ifdef ANSI_C

objecttype cmd_symlink (listtype arguments)
#else

objecttype cmd_symlink (arguments)
listtype arguments;
#endif

  { /* cmd_symlink */
    isit_stri(arg_1(arguments));
    isit_stri(arg_2(arguments));
    cmdSymlink(take_stri(arg_1(arguments)), take_stri(arg_2(arguments)));
    return(SYS_EMPTY_OBJECT);
  } /* cmd_symlink */

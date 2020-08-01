/********************************************************************/
/*                                                                  */
/*  dir_rtl.c     Primitive actions for the directory type.         */
/*  Copyright (C) 1989 - 2008  Thomas Mertes                        */
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
/*  File: seed7/src/dir_rtl.c                                       */
/*  Changes: 2008  Thomas Mertes                                    */
/*  Content: Primitive actions for the directory type.              */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "striutl.h"
#include "heaputl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "dir_rtl.h"



#ifdef USE_WOPENDIR



#ifdef ANSI_C

void dirClose (dirtype directory)
#else

void dirClose (directory)
dirtype directory;
#endif

  { /* dirClose */
    wclosedir(directory);
  } /* dirClose */



#ifdef ANSI_C

dirtype dirOpen (stritype file_name)
#else

dirtype dirOpen (file_name)
stritype file_name;
#endif

  {
    wchar_t *name;
    dirtype result;

  /* dirOpen */
    name = cp_to_wstri(file_name);
    if (name == NULL) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result = wopendir(name);
      free_wstri(name, file_name);
    } /* if */
    return(result);
  } /* dirOpen */



#ifdef ANSI_C

stritype dirRead (dirtype directory)
#else

stritype dirRead (directory)
dirtype directory;
#endif

  {
    struct wdirent *current_entry;
    wchar_t dot[]    = {'.', 0};
    wchar_t dotdot[] = {'.', '.', 0};
    stritype result;

  /* dirRead */
    do {
      current_entry = wreaddir(directory);
    } while (current_entry != NULL &&
        (memcmp(current_entry->d_name, dot,    sizeof(dot))    == 0 ||
         memcmp(current_entry->d_name, dotdot, sizeof(dotdot)) == 0));
    if (current_entry == NULL) {
      result = NULL;
    } else {
      result = wstri_to_stri(current_entry->d_name);
      if (result == NULL) {
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
    return(result);
  } /* dirRead */



#else



#ifdef ANSI_C

void dirClose (dirtype directory)
#else

void dirClose (directory)
dirtype directory;
#endif

  { /* dirClose */
    closedir(directory);
  } /* dirClose */



#ifdef ANSI_C

dirtype dirOpen (stritype file_name)
#else

dirtype dirOpen (file_name)
stritype file_name;
#endif

  {
    cstritype name;
    dirtype result;

  /* dirOpen */
    name = cp_to_cstri(file_name);
    if (name == NULL) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      result = opendir(name);
      free_cstri(name, file_name);
    } /* if */
    return(result);
  } /* dirOpen */



#ifdef ANSI_C

stritype dirRead (dirtype directory)
#else

stritype dirRead (directory)
dirtype directory;
#endif

  {
    struct dirent *current_entry;
    stritype result;

  /* dirRead */
    do {
      current_entry = readdir(directory);
    } while (current_entry != NULL &&
        (strcmp(current_entry->d_name, ".")  == 0 ||
         strcmp(current_entry->d_name, "..") == 0));
    if (current_entry == NULL) {
      result = NULL;
    } else {
#ifdef READDIR_UTF8
      result = cstri8_to_stri(current_entry->d_name);
      if (result == NULL) {
        result = cstri_to_stri(current_entry->d_name);
      } /* if */
#else
      result = cstri_to_stri(current_entry->d_name);
#endif
      if (result == NULL) {
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
    return(result);
  } /* dirRead */



#endif

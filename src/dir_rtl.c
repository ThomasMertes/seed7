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

#include "dir_drv.h"

#include "common.h"
#include "striutl.h"
#include "heaputl.h"
#include "cmd_drv.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "dir_rtl.h"


#ifdef OS_PATH_WCHAR
wchar_t dot[]    = {'.', 0};
wchar_t dotdot[] = {'.', '.', 0};
wchar_t slash[]  = {'/', 0};
#else
char dot[]    = ".";
char dotdot[] = "..";
char slash[]  = "/";
#endif



#ifdef ANSI_C

void dirClose (dirtype directory)
#else

void dirClose (directory)
dirtype directory;
#endif

  { /* dirClose */
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
    if (IS_VOLUME_LIST(directory)) {
      closeVolumeList((volumeListType *) directory);
    } else {
#endif
      os_closedir(directory);
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
    } /* if */
#endif
  } /* dirClose */



#ifdef ANSI_C

dirtype dirOpen (const const_stritype file_name)
#else

dirtype dirOpen (file_name)
stritype file_name;
#endif

  {
    os_stritype name;
    errinfotype err_info = OKAY_NO_ERROR;
    dirtype result;

  /* dirOpen */
    name = cp_to_os_path(file_name, &err_info);
    if (name == NULL) {
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (file_name->size == 1 && file_name->mem[0] == '/') {
        result = (dirtype) openVolumeList();
      } else {
#endif
        raise_error(err_info);
        result = NULL;
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      } /* if */
#endif
    } else {
      result = os_opendir(name);
      os_stri_free(name);
    } /* if */
    return result;
  } /* dirOpen */



#ifdef ANSI_C

stritype dirRead (dirtype directory)
#else

stritype dirRead (directory)
dirtype directory;
#endif

  {
    os_dirent_struct *current_entry;
    stritype result;

  /* dirRead */
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
    if (IS_VOLUME_LIST(directory)) {
      result = readVolumeName((volumeListType *) directory);
    } else {
#endif
      do {
        current_entry = os_readdir(directory);
      } while (current_entry != NULL &&
          (memcmp(current_entry->d_name, dot,    sizeof(os_chartype) * 2) == 0 ||
           memcmp(current_entry->d_name, dotdot, sizeof(os_chartype) * 3) == 0));
      if (current_entry == NULL) {
        result = NULL;
      } else {
        result = os_stri_to_stri(current_entry->d_name);
        if (result == NULL) {
          raise_error(MEMORY_ERROR);
        } /* if */
      } /* if */
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
    } /* if */
#endif
    return result;
  } /* dirRead */

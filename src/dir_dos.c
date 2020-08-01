/********************************************************************/
/*                                                                  */
/*  dir_dos.c     Directory access using the dos capabilitys.       */
/*  Copyright (C) 1989 - 2007  Thomas Mertes                        */
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
/*  File: seed7/src/dir_dos.c                                       */
/*  Changes: 1993, 1994, 2008  Thomas Mertes                        */
/*  Content: Directory access using DOS findfirst and findnext.     */
/*                                                                  */
/*  Implements opendir, readdir and closedir in the way it is       */
/*  defined in unix.                                                */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"

#undef EXTERN
#define EXTERN
#include "dir_dos.h"



#ifdef ANSI_C

DIR *opendir (char *name)
#else

DIR *opendir (name)
char *name;
#endif

  {
    unsigned int name_len;
    char dir_name[250];
    DIR *result;

  /* opendir */
    if ((result = (DIR *) malloc(sizeof(DIR))) != NULL) {
/*    printf("opendir(%s);\n", name); */
      name_len = strlen(name);
      if (name_len >= sizeof(dir_name) - 4) {
        name_len = sizeof(dir_name) - 5;
      } /* if */
      memcpy(dir_name, name, name_len);
      dir_name[name_len] = '\0';
      if (name_len > 0) {
        if (name[name_len - 1] != '/' &&
            name[name_len - 1] != '\\') {
          strcat(dir_name, "\\");
        } /* if */
        strcat(dir_name, "*.*");
      } /* if */
#ifdef DIR_WIN_DOS
      result->dir_handle = _findfirst(dir_name, &result->find_record);
      if (result->dir_handle != -1) {
#else
      if ( _dos_findfirst(dir_name, _A_ARCH | _A_SUBDIR,
          &result->find_record) == 0) {
#endif
/*      printf("==> OK\n");
        printf(">%s<\n", result->find_record.name); */
        result->first_element = TRUE;
      } else {
/*      printf("==> ERROR\n"); */
        free(result);
        result = NULL;
      } /* if */
    } /* if */
    return(result);
  } /* opendir */



#ifdef ANSI_C

struct dirent *readdir (DIR *curr_dir)
#else

struct dirent *readdir (curr_dir)
DIR *curr_dir;
#endif

  {
    unsigned int name_len;
    struct dirent *result;

  /* readdir */
/*  printf("readdir();\n"); */
    result = &curr_dir->dir_entry;
    if (curr_dir->first_element) {
/*    printf("first\n"); */
      curr_dir->first_element = FALSE;
      name_len = strlen(curr_dir->find_record.name);
      if (name_len >= sizeof(result->d_name)) {
        name_len = sizeof(result->d_name) - 1;
      } /* if */
      memcpy(result->d_name, curr_dir->find_record.name, name_len);
      result->d_name[name_len] = '\0';
/*    printf(">%s<\n", result->d_name); */
    } else {
#ifdef DIR_WIN_DOS
      if (_findnext(curr_dir->dir_handle, &curr_dir->find_record) == 0) {
#else
      if ( _dos_findnext(&curr_dir->find_record) == 0) {
#endif
        name_len = strlen(curr_dir->find_record.name);
        if (name_len >= sizeof(result->d_name)) {
          name_len = sizeof(result->d_name) - 1;
        } /* if */
        memcpy(result->d_name, curr_dir->find_record.name, name_len);
        result->d_name[name_len] = '\0';
/*      printf(">%s<\n", result->d_name); */
      } else {
/*      printf("end\n"); */
        result = NULL;
      } /* if */
    } /* if */
    return(result);
  } /* readdir */



#ifdef ANSI_C

int closedir (DIR *curr_dir)
#else

int closedir (curr_dir)
DIR *curr_dir;
#endif

  { /* closedir */
    free(curr_dir);
    return(0);
  } /* closedir */

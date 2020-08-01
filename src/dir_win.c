/********************************************************************/
/*                                                                  */
/*  dir_win.c     Directory access using the windows capabilitys.   */
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
/*  File: seed7/src/dir_win.c                                       */
/*  Changes: 1993, 1994, 2007, 2008  Thomas Mertes                  */
/*  Content: Directory access using FindFirstFile and FindNextFile. */
/*                                                                  */
/*  Implements opendir, readdir and closedir in the way it is       */
/*  defined in unix. Additionally wopendir, wreaddir and wclosedir  */
/*  are defined which use wchar_t instead of char.                  */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"

#undef EXTERN
#define EXTERN
#include "dir_win.h"



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
      if (name_len > sizeof(dir_name) - 5) {
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
      result->dir_handle = FindFirstFileA(dir_name, &result->find_record);
      if (result->dir_handle != INVALID_HANDLE_VALUE) {
/*      printf("==> OK\n");
        printf(">%s<\n", result->find_record.name); */
        result->first_element = 1;
      } else {
/*      printf("==> ERROR\n"); */
        free(result);
        result = NULL;
      } /* if */
    } /* if */
    return result;
  } /* opendir */



#ifdef ANSI_C

struct dirent *readdir (DIR *curr_dir)
#else

struct dirent *readdir (curr_dir)
DIR *curr_dir;
#endif

  {
    struct dirent *result;

  /* readdir */
/*  printf("readdir();\n"); */
    if (curr_dir->first_element) {
/*    printf("first\n"); */
      curr_dir->first_element = 0;
      curr_dir->dir_entry.d_name = (char *) curr_dir->find_record.cFileName;
      result = &curr_dir->dir_entry;
/*    printf(">%s<\n", result->d_name); */
    } else if (FindNextFileA(curr_dir->dir_handle, &curr_dir->find_record) != 0) {
      curr_dir->dir_entry.d_name = (char *) curr_dir->find_record.cFileName;
      result = &curr_dir->dir_entry;
/*    printf(">%s<\n", result->d_name); */
    } else {
/*    printf("end\n"); */
      result = NULL;
    } /* if */
    return result;
  } /* readdir */



#ifdef ANSI_C

int closedir (DIR *curr_dir)
#else

int closedir (curr_dir)
DIR *curr_dir;
#endif

  { /* closedir */
    FindClose(curr_dir->dir_handle);
    free(curr_dir);
    return 0;
  } /* closedir */



#ifdef ANSI_C

WDIR *wopendir (wchar_t *name)
#else

WDIR *wopendir (name)
wchar_t *name;
#endif

  {
    unsigned int name_len;
    wchar_t dir_name[250];
    WDIR *result;

  /* wopendir */
    if ((result = (WDIR *) malloc(sizeof(WDIR))) != NULL) {
      name_len = 0;
      while (name[name_len] != 0) {
        name_len++;
      } /* while */
      if (name_len > sizeof(dir_name) / sizeof(wchar_t) - 5) {
        name_len = sizeof(dir_name) / sizeof(wchar_t) - 5;
      } /* if */
      memcpy(dir_name, name, name_len * sizeof(wchar_t));
      if (name_len > 0) {
        if (name[name_len - 1] != '/' &&
            name[name_len - 1] != '\\') {
          dir_name[name_len++] = '\\';
        } /* if */
        dir_name[name_len++] = '*';
        dir_name[name_len++] = '.';
        dir_name[name_len++] = '*';
      } /* if */
      dir_name[name_len] = '\0';
      result->dir_handle = FindFirstFileW(dir_name, &result->find_record);
      if (result->dir_handle != INVALID_HANDLE_VALUE) {
/*      printf("==> OK\n");
        printf(">%s<\n", result->find_record.name); */
        result->first_element = 1;
      } else {
/*      printf("==> ERROR\n"); */
        free(result);
        result = NULL;
      } /* if */
    } /* if */
    return result;
  } /* wopendir */



#ifdef ANSI_C

struct wdirent *wreaddir (WDIR *curr_dir)
#else

struct wdirent *wreaddir (curr_dir)
WDIR *curr_dir;
#endif

  {
    struct wdirent *result;

  /* wreaddir */
/*  printf("wreaddir();\n"); */
    if (curr_dir->first_element) {
/*    printf("first\n"); */
      curr_dir->first_element = 0;
      curr_dir->dir_entry.d_name = (wchar_t *) curr_dir->find_record.cFileName;
      result = &curr_dir->dir_entry;
/*    printf(">%s<\n", result->d_name); */
    } else if (FindNextFileW(curr_dir->dir_handle, &curr_dir->find_record) != 0) {
      curr_dir->dir_entry.d_name = (wchar_t *) curr_dir->find_record.cFileName;
      result = &curr_dir->dir_entry;
/*    printf(">%s<\n", result->d_name); */
    } else {
/*    printf("end\n"); */
      result = NULL;
    } /* if */
    return result;
  } /* wreaddir */



#ifdef ANSI_C

int wclosedir (WDIR *curr_dir)
#else

int wclosedir (curr_dir)
WDIR *curr_dir;
#endif

  { /* wclosedir */
    FindClose(curr_dir->dir_handle);
    free(curr_dir);
    return 0;
  } /* wclosedir */

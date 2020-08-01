/********************************************************************/
/*                                                                  */
/*  dir_win.c     Directory functions which call the Windows API.   */
/*  Copyright (C) 1989 - 2014  Thomas Mertes                        */
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
/*  Changes: 1993, 1994, 2007, 2008, 2014  Thomas Mertes            */
/*  Content: Directory access using FindFirstFile and FindNextFile. */
/*                                                                  */
/*  Implements opendir, readdir and closedir in the way it is       */
/*  defined in Unix. Additionally wopendir, wreaddir and wclosedir  */
/*  are defined which use wchar_t instead of char.                  */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "wchar.h"

#include "common.h"

#undef EXTERN
#define EXTERN
#include "dir_win.h"



/**
 *  Opens a directory stream corresponding to the directory name.
 *  The stream is positioned at the first entry in the directory.
 *  @param name Name of the directory to be opened.
 *  @return a pointer to the directory stream or NULL if the
 *          directory stream could not be opened.
 */
DIR *opendir (const char *name)

  {
    unsigned int name_len;
    char dir_name[260];
    DIR *result;

  /* opendir */
    logFunction(printf("opendir(%s);\n", name););
    name_len = strlen(name);
    if (name_len == 0 || name_len > sizeof(dir_name) - 5) {
      logError(printf("opendir(\"%s\"): Name too long "
                      "(length: " FMT_U_MEM ", max length: " FMT_U_MEM ")\n",
                      name, name_len, sizeof(dir_name) - 5););
      result = NULL;
    } else if ((result = (DIR *) malloc(sizeof(DIR))) != NULL) {
      memcpy(dir_name, name, name_len);
      if (name[name_len - 1] != '/' &&
          name[name_len - 1] != '\\') {
        dir_name[name_len++] = '\\';
      } /* if */
      strcpy(&dir_name[name_len], "*.*");
      result->dir_handle = FindFirstFileA(dir_name, &result->find_record);
      if (result->dir_handle != INVALID_HANDLE_VALUE) {
        /* printf("--> OK\n");
        printf(">%s<\n", result->find_record.cFileName); */
        result->first_element = 1;
      } else {
        logError(printf("opendir(\"%s\"): FindFirstFileA() failed.\n",
                        name););
        free(result);
        result = NULL;
      } /* if */
    } /* if */
    return result;
  } /* opendir */



/**
 *  Determine dirent structure representing the next directory entry.
 *  The element d_name in the dirent structure gets the file name of
 *  the next directory entry. Besides d_name the dirent structure
 *  has no other elements.
 *  @param curr_dir Directory stream from which the directory entry
 *         is read.
 *  @return a pointer to a dirent structure representing the next
 *          directory entry. It returns NULL on reaching the end of
 *          the directory stream or if an error occurred.
 */
struct dirent *readdir (DIR *curr_dir)

  {
    struct dirent *result;

  /* readdir */
    logFunction(printf("readdir\n"););
    if (curr_dir->first_element) {
      /* printf("first\n"); */
      curr_dir->first_element = 0;
      curr_dir->dir_entry.d_name = (char *) curr_dir->find_record.cFileName;
      result = &curr_dir->dir_entry;
      /* printf(">%s<\n", result->d_name); */
    } else if (FindNextFileA(curr_dir->dir_handle, &curr_dir->find_record) != 0) {
      curr_dir->dir_entry.d_name = (char *) curr_dir->find_record.cFileName;
      result = &curr_dir->dir_entry;
      /* printf(">%s<\n", result->d_name); */
    } else {
      /* printf("end\n"); */
      result = NULL;
    } /* if */
    logFunction(printf("readdir --> %s\n",
                       result == NULL ? "NULL" : result->d_name););
    return result;
  } /* readdir */



/**
 *  Closes the given directory stream.
 *  @param curr_dir The directory stream to be closed.
 *  @return 0 on success.
 */
int closedir (DIR *curr_dir)

  { /* closedir */
    logFunction(printf("closedir\n"););
    FindClose(curr_dir->dir_handle);
    free(curr_dir);
    return 0;
  } /* closedir */



/**
 *  Opens a directory stream corresponding to the directory name.
 *  The stream is positioned at the first entry in the directory.
 *  @param name Name of the directory to be opened.
 *  @return a pointer to the directory stream or NULL if the
 *          directory stream could not be opened.
 */
WDIR *wopendir (const wchar_t *name)

  {
    unsigned int name_len;
    wchar_t dir_name[260];
    WDIR *result;

  /* wopendir */
    logFunction(printf("wopendir(%ls);\n", name););
    name_len = wcslen(name);
    if (name_len == 0 ||
        name_len > sizeof(dir_name) / sizeof(wchar_t) - 5) {
      logError(printf("wopendir(\"%ls\"): Name too long "
                      "(length: " FMT_U_MEM ", max length: " FMT_U_MEM ")\n",
                      name, name_len, sizeof(dir_name) / sizeof(wchar_t) - 5););
      result = NULL;
    } else if ((result = (WDIR *) malloc(sizeof(WDIR))) != NULL) {
      memcpy(dir_name, name, name_len * sizeof(wchar_t));
      if (name[name_len - 1] != '/' &&
          name[name_len - 1] != '\\') {
        dir_name[name_len++] = '\\';
      } /* if */
      dir_name[name_len++] = '*';
      dir_name[name_len++] = '.';
      dir_name[name_len++] = '*';
      dir_name[name_len] = '\0';
      result->dir_handle = FindFirstFileW(dir_name, &result->find_record);
      if (result->dir_handle != INVALID_HANDLE_VALUE) {
        /* printf("--> OK\n");
        printf(">%ls<\n", result->find_record.cFileName); */
        result->first_element = 1;
      } else {
        logError(printf("wopendir(\"%ls\"): FindFirstFileW() failed.\n",
                        name););
        free(result);
        result = NULL;
      } /* if */
    } /* if */
    return result;
  } /* wopendir */



/**
 *  Determine wdirent structure representing the next directory entry.
 *  The element d_name in the wdirent structure gets the file name of
 *  the next directory entry. Besides d_name the wdirent structure
 *  has no other elements.
 *  @param curr_dir Directory stream from which the directory entry
 *         is read.
 *  @return a pointer to a wdirent structure representing the next
 *          directory entry. It returns NULL on reaching the end of
 *          the directory stream or if an error occurred.
 */
struct wdirent *wreaddir (WDIR *curr_dir)

  {
    struct wdirent *result;

  /* wreaddir */
    logFunction(printf("wreaddir\n"););
    if (curr_dir->first_element) {
      /* printf("first\n"); */
      curr_dir->first_element = 0;
      curr_dir->dir_entry.d_name = (wchar_t *) curr_dir->find_record.cFileName;
      result = &curr_dir->dir_entry;
      /* printf(">%ls<\n", result->d_name); */
    } else if (FindNextFileW(curr_dir->dir_handle, &curr_dir->find_record) != 0) {
      curr_dir->dir_entry.d_name = (wchar_t *) curr_dir->find_record.cFileName;
      result = &curr_dir->dir_entry;
      /* printf(">%ls<\n", result->d_name); */
    } else {
      /* printf("end\n"); */
      result = NULL;
    } /* if */
    logFunction(printf("wreaddir --> %ls\n",
                       result == NULL ? "NULL" : result->d_name););
    return result;
  } /* wreaddir */



/**
 *  Closes the given directory stream.
 *  @param curr_dir The directory stream to be closed.
 *  @return 0 on success.
 */
int wclosedir (WDIR *curr_dir)

  { /* wclosedir */
    logFunction(printf("wclosedir\n"););
    FindClose(curr_dir->dir_handle);
    free(curr_dir);
    return 0;
  } /* wclosedir */

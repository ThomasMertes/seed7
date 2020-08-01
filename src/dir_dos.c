/********************************************************************/
/*                                                                  */
/*  dir_dos.c     Directory functions which call the Dos API.       */
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
/*  File: seed7/src/dir_dos.c                                       */
/*  Changes: 1993, 1994, 2008, 2014  Thomas Mertes                  */
/*  Content: Directory access using DOS findfirst and findnext.     */
/*                                                                  */
/*  Implements opendir, readdir and closedir in the way it is       */
/*  defined in Unix.                                                */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"

#undef EXTERN
#define EXTERN
#include "dir_dos.h"



/**
 *  Opens a directory stream corresponding to the directory name.
 *  The stream is positioned at the first entry in the directory.
 *  @param name Name of the directory to be opened.
 *  @return a pointer to the directory stream or NULL if the
 *          directory stream could not be opened.
 */
DIR *opendir (char *name)

  {
    unsigned int name_len;
    char dir_name[260];
    DIR *result;

  /* opendir */
    name_len = strlen(name);
    if (name_len == 0 || name_len > sizeof(dir_name) - 5) {
      logError(printf("opendir(\"%s\"): Name too long "
                      "(length: " FMT_U_MEM ", max length: " FMT_U_MEM ")\n",
                      name, name_len, sizeof(dir_name) - 5););
      result = NULL;
    } else if ((result = (DIR *) malloc(sizeof(DIR))) != NULL) {
      /* printf("opendir(%s);\n", name); */
      memcpy(dir_name, name, name_len);
      if (name[name_len - 1] != '/' &&
          name[name_len - 1] != '\\') {
        dir_name[name_len++] = '\\';
      } /* if */
      strcpy(&dir_name[name_len], "*.*");
#ifdef DIR_WIN_DOS
      result->dir_handle = _findfirst(dir_name, &result->find_record);
      if (result->dir_handle != -1) {
#else
      if ( _dos_findfirst(dir_name, _A_ARCH | _A_SUBDIR,
          &result->find_record) == 0) {
#endif
        /* printf("--> OK\n");
        printf(">%s<\n", result->find_record.name); */
        result->first_element = 1;
      } else {
        logError(printf("opendir(\"%s\"): findfirst() failed.\n",
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
    unsigned int name_len;
    struct dirent *result;

  /* readdir */
    /* printf("readdir();\n"); */
    result = &curr_dir->dir_entry;
    if (curr_dir->first_element) {
      /* printf("first\n"); */
      curr_dir->first_element = 0;
      name_len = strlen(curr_dir->find_record.name);
      if (name_len >= sizeof(result->d_name)) {
        name_len = sizeof(result->d_name) - 1;
      } /* if */
      memcpy(result->d_name, curr_dir->find_record.name, name_len);
      result->d_name[name_len] = '\0';
      /* printf(">%s<\n", result->d_name); */
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
        /* printf(">%s<\n", result->d_name); */
      } else {
        /* printf("end\n"); */
        result = NULL;
      } /* if */
    } /* if */
    return result;
  } /* readdir */



/**
 *  Closes the given directory stream.
 *  @param curr_dir The directory stream to be closed.
 *  @return 0 on success.
 */
int closedir (DIR *curr_dir)

  { /* closedir */
    free(curr_dir);
    return 0;
  } /* closedir */

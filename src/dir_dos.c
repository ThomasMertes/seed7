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
 *  Opens a directory stream corresponding to the directory dirName.
 *  The stream is positioned at the first entry in the directory.
 *  @param dirName Name of the directory to be opened.
 *  @return a pointer to the directory stream or NULL if the
 *          directory stream could not be opened.
 */
DIR *opendir (char *dirName)

  {
    memSizeType nameLen;
    char fileNamePattern[260];
    DIR *directory;

  /* opendir */
    logFunction(printf("opendir(%s);\n", dirName););
    nameLen = strlen(dirName);
    if (nameLen == 0 || nameLen > sizeof(fileNamePattern) - 5) {
      logError(printf("opendir(\"%s\"): Name too long "
                      "(length: " FMT_U_MEM ", max length: " FMT_U_MEM ")\n",
                      dirName, nameLen, sizeof(fileNamePattern) - 5););
      directory = NULL;
    } else if ((directory = (DIR *) malloc(sizeof(DIR))) != NULL) {
      memcpy(fileNamePattern, dirName, nameLen);
      if (dirName[nameLen - 1] != '/' &&
          dirName[nameLen - 1] != '\\') {
        fileNamePattern[nameLen++] = '\\';
      } /* if */
      strcpy(&fileNamePattern[nameLen], "*.*");
#ifdef DIR_WIN_DOS
      directory->dirHandle = _findfirst(fileNamePattern, &directory->findData);
      if (directory->dirHandle != -1) {
#else
      if ( _dos_findfirst(fileNamePattern, _A_ARCH | _A_SUBDIR,
          &directory->findData) == 0) {
#endif
        /* printf("--> OK\n");
        printf(">%s<\n", directory->findData.name); */
        directory->firstElement = 1;
      } else {
        logError(printf("opendir(\"%s\"): findfirst() failed.\n",
                        dirName););
        free(directory);
        directory = NULL;
      } /* if */
    } /* if */
    return directory;
  } /* opendir */



/**
 *  Determine dirent structure representing the next directory entry.
 *  The element d_name in the dirent structure gets the file name of
 *  the next directory entry. Besides d_name the dirent structure
 *  has no other elements.
 *  @param directory Directory stream from which the directory entry
 *         is read.
 *  @return a pointer to a dirent structure representing the next
 *          directory entry. It returns NULL on reaching the end of
 *          the directory stream or if an error occurred.
 */
struct dirent *readdir (DIR *directory)

  {
    unsigned int nameLen;
    struct dirent *dirEntry;

  /* readdir */
    logFunction(printf("readdir\n"););
    dirEntry = &directory->dirEntry;
    if (directory->firstElement) {
      /* printf("first\n"); */
      directory->firstElement = 0;
      nameLen = strlen(directory->findData.name);
      if (nameLen >= sizeof(dirEntry->d_name)) {
        nameLen = sizeof(dirEntry->d_name) - 1;
      } /* if */
      memcpy(dirEntry->d_name, directory->findData.name, nameLen);
      dirEntry->d_name[nameLen] = '\0';
      /* printf(">%s<\n", dirEntry->d_name); */
    } else {
#ifdef DIR_WIN_DOS
      if (_findnext(directory->dirHandle, &directory->findData) == 0) {
#else
      if ( _dos_findnext(&directory->findData) == 0) {
#endif
        nameLen = strlen(directory->findData.name);
        if (nameLen >= sizeof(dirEntry->d_name)) {
          nameLen = sizeof(dirEntry->d_name) - 1;
        } /* if */
        memcpy(dirEntry->d_name, directory->findData.name, nameLen);
        dirEntry->d_name[nameLen] = '\0';
        /* printf(">%s<\n", dirEntry->d_name); */
      } else {
        /* printf("end\n"); */
        dirEntry = NULL;
      } /* if */
    } /* if */
    logFunction(printf("readdir --> %s\n",
                       dirEntry == NULL ? "NULL" : dirEntry->d_name););
    return dirEntry;
  } /* readdir */



/**
 *  Closes the given directory stream.
 *  @param directory The directory stream to be closed.
 *  @return 0 on success.
 */
int closedir (DIR *directory)

  { /* closedir */
    logFunction(printf("closedir\n"););
    free(directory);
    return 0;
  } /* closedir */

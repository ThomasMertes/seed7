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
 *  Opens a directory stream corresponding to the directory dirName.
 *  The stream is positioned at the first entry in the directory.
 *  @param dirName Name of the directory to be opened.
 *  @return a pointer to the directory stream or NULL if the
 *          directory stream could not be opened.
 */
DIR *opendir (const char *dirName)

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
      fileNamePattern[nameLen++] = '*';
      fileNamePattern[nameLen] = '\0';
      directory->dirHandle = FindFirstFileA(fileNamePattern, &directory->findData);
      if (directory->dirHandle != INVALID_HANDLE_VALUE) {
        /* printf("--> OK\n");
        printf(">%s<\n", directory->findData.cFileName); */
        directory->firstElement = 1;
      } else {
        logError(printf("opendir(\"%s\"): FindFirstFileA() failed.\n",
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
    struct dirent *dirEntry;

  /* readdir */
    logFunction(printf("readdir\n"););
    if (directory->firstElement) {
      /* printf("first\n"); */
      directory->firstElement = 0;
      directory->dirEntry.d_name = (char *) directory->findData.cFileName;
      dirEntry = &directory->dirEntry;
      /* printf(">%s<\n", dirEntry->d_name); */
    } else if (FindNextFileA(directory->dirHandle, &directory->findData) != 0) {
      directory->dirEntry.d_name = (char *) directory->findData.cFileName;
      dirEntry = &directory->dirEntry;
      /* printf(">%s<\n", dirEntry->d_name); */
    } else {
      /* printf("end\n"); */
      dirEntry = NULL;
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
    FindClose(directory->dirHandle);
    free(directory);
    return 0;
  } /* closedir */



/**
 *  Opens a directory stream corresponding to the directory dirName.
 *  The stream is positioned at the first entry in the directory.
 *  @param dirName Name of the directory to be opened.
 *  @return a pointer to the directory stream or NULL if the
 *          directory stream could not be opened.
 */
WDIR *wopendir (const wchar_t *dirName)

  {
    memSizeType nameLen;
    wchar_t fileNamePattern[260];
    WDIR *directory;

  /* wopendir */
    logFunction(printf("wopendir(%ls);\n", dirName););
    nameLen = wcslen(dirName);
    if (nameLen == 0 ||
        nameLen > sizeof(fileNamePattern) / sizeof(wchar_t) - 5) {
      logError(printf("wopendir(\"%ls\"): Name too long "
                      "(length: " FMT_U_MEM ", max length: " FMT_U_MEM ")\n",
                      dirName, nameLen, sizeof(fileNamePattern) / sizeof(wchar_t) - 5););
      directory = NULL;
    } else if ((directory = (WDIR *) malloc(sizeof(WDIR))) != NULL) {
      memcpy(fileNamePattern, dirName, nameLen * sizeof(wchar_t));
      if (dirName[nameLen - 1] != '/' &&
          dirName[nameLen - 1] != '\\') {
        fileNamePattern[nameLen++] = '\\';
      } /* if */
      fileNamePattern[nameLen++] = '*';
      fileNamePattern[nameLen] = '\0';
      directory->dirHandle = FindFirstFileW(fileNamePattern, &directory->findData);
      if (directory->dirHandle != INVALID_HANDLE_VALUE) {
        /* printf("--> OK\n");
        printf(">%ls<\n", directory->findData.cFileName); */
        directory->firstElement = 1;
      } else {
        logError(printf("wopendir(\"%ls\"): FindFirstFileW() failed.\n",
                        dirName););
        free(directory);
        directory = NULL;
      } /* if */
    } /* if */
    return directory;
  } /* wopendir */



/**
 *  Determine wdirent structure representing the next directory entry.
 *  The element d_name in the wdirent structure gets the file name of
 *  the next directory entry. Besides d_name the wdirent structure
 *  has no other elements.
 *  @param directory Directory stream from which the directory entry
 *         is read.
 *  @return a pointer to a wdirent structure representing the next
 *          directory entry. It returns NULL on reaching the end of
 *          the directory stream or if an error occurred.
 */
struct wdirent *wreaddir (WDIR *directory)

  {
    struct wdirent *dirEntry;

  /* wreaddir */
    logFunction(printf("wreaddir\n"););
    if (directory->firstElement) {
      /* printf("first\n"); */
      directory->firstElement = 0;
      directory->dirEntry.d_name = (wchar_t *) directory->findData.cFileName;
      dirEntry = &directory->dirEntry;
      /* printf(">%ls<\n", dirEntry->d_name); */
    } else if (FindNextFileW(directory->dirHandle, &directory->findData) != 0) {
      directory->dirEntry.d_name = (wchar_t *) directory->findData.cFileName;
      dirEntry = &directory->dirEntry;
      /* printf(">%ls<\n", dirEntry->d_name); */
    } else {
      /* printf("end\n"); */
      dirEntry = NULL;
    } /* if */
    logFunction(printf("wreaddir --> %ls\n",
                       dirEntry == NULL ? "NULL" : dirEntry->d_name););
    return dirEntry;
  } /* wreaddir */



/**
 *  Closes the given directory stream.
 *  @param directory The directory stream to be closed.
 *  @return 0 on success.
 */
int wclosedir (WDIR *directory)

  { /* wclosedir */
    logFunction(printf("wclosedir\n"););
    FindClose(directory->dirHandle);
    free(directory);
    return 0;
  } /* wclosedir */

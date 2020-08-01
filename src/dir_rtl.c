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

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "dir_drv.h"

#include "common.h"
#include "data_rtl.h"
#include "os_decls.h"
#include "striutl.h"
#include "heaputl.h"
#include "cmd_drv.h"
#include "rtl_err.h"
#include "errno.h"

#undef EXTERN
#define EXTERN
#include "dir_rtl.h"


#ifdef OS_STRI_WCHAR
wchar_t dot[]    = {'.', 0};
wchar_t dotdot[] = {'.', '.', 0};
wchar_t slash[]  = {'/', 0};
#else
char dot[]    = ".";
char dotdot[] = "..";
char slash[]  = "/";
#endif



#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
static striType readVolumeName (volumeListType *volumeList)

  {
    os_charType os_path[4];
    os_stat_struct stat_buf;
    int stat_result;
    os_DIR *directory;
    char buffer[2];
    striType volumeName;

  /* readVolumeName */
    os_path[1] = (os_charType) ':';
    os_path[2] = (os_charType) '\\';
    os_path[3] = (os_charType) '\0';
    volumeName = NULL;
    do {
      while (volumeList->currentDrive < 26 &&
             (volumeList->driveBitmask & (uint32Type) 1 << volumeList->currentDrive) == 0) {
        volumeList->currentDrive++;
      } /* while */
      if (volumeList->currentDrive < 26) {
        /* printf("%c:\\\n", (char) ((int) 'a' + volumeList->currentDrive)); */
        os_path[0] = (os_charType) ((int) 'a' + volumeList->currentDrive);
        stat_result = os_stat(os_path, &stat_buf);
        if (stat_result == 0) {
          /* printf("%c:\\ st_mode=%u\n",
              (char) ((int) 'a' + volumeList->currentDrive),
              S_ISDIR(stat_buf.st_mode)); */
          directory = os_opendir(os_path);
          if (directory != NULL) {
            os_closedir(directory);
            buffer[0] = (char) ((int) 'a' + volumeList->currentDrive);
            buffer[1] = '\0';
            volumeName = cstri_to_stri(buffer);
          } /* if */
        } /* if */
        volumeList->currentDrive++;
      } /* if */
    } while (volumeName == NULL && volumeList->currentDrive < 26);
    return volumeName;
  } /* readVolumeName */



static void closeVolumeList (volumeListType *volumeList)

  { /* closeVolumeList */
    free(volumeList);
  } /* closeVolumeList */
#endif



/**
 *  Close the given directory stream.
 *  @param directory The directory stream to be closed.
 */
void dirClose (dirType directory)

  { /* dirClose */
    logFunction(printf("dirClose(" FMT_U_MEM ")\n", (memSizeType) directory););
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
    if (IS_VOLUME_LIST(directory)) {
      closeVolumeList((volumeListType *) directory);
    } else {
#endif
      os_closedir(directory);
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
    } /* if */
#endif
    logFunction(printf("dirClose -->\n"););
  } /* dirClose */



/**
 *  Open a directory stream corresponding to the directory path.
 *  The directory stream is positioned at the first entry in the
 *  directory.
 *  @param path Path of the directory to be opened. The path must
 *         use the standard path representation.
 *  @return the directory stream opened, or NULL if it could not
 *          be opened.
 *  @exception MEMORY_ERROR Not enough memory to convert the path
 *             to the system path type.
 *  @exception RANGE_ERROR The 'path' does not use the standard
 *             path representation or 'path' cannot be converted
 *             to the system path type.
 */
dirType dirOpen (const const_striType path)

  {
    os_striType os_path;
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;
    dirType directory;

  /* dirOpen */
    logFunction(printf("dirOpen(\"%s\")\n", striAsUnquotedCStri(path)););
    os_path = cp_to_os_path(path, &path_info, &err_info);
    if (unlikely(err_info != OKAY_NO_ERROR)) {
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
      if (path_info == PATH_IS_EMULATED_ROOT) {
        directory = (dirType) openVolumeList();
      } else
#endif
      {
        logError(printf("dirOpen: cp_to_os_path(\"%s\", *, *) failed:\n"
                        "path_info=%d, err_info=%d\n",
                        striAsUnquotedCStri(path), path_info, err_info););
        raise_error(err_info);
        directory = NULL;
      }
    } else {
      directory = os_opendir(os_path);
      if (unlikely(directory == NULL)) {
        logError(printf("dirOpen: os_opendir(\"" FMT_S_OS "\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        os_path, errno, strerror(errno)););
      } /* if */
      os_stri_free(os_path);
    } /* if */
    logFunction(printf("dirOpen --> " FMT_U_MEM "\n",
                       (memSizeType) directory););
    return directory;
  } /* dirOpen */



/**
 *  Read the next file name from the given directory stream.
 *  Consecutive calls return all file names in the directory.
 *  The file names . and .. are skipped. Only filenames can be
 *  read from the directory strem. Additional information must
 *  be obtained with other calls.
 *  @param directory The directory stream from which file names
 *         are read.
 *  @return the file name read or NULL on reaching the end of the
 *          directory stream or if an error occurred.
 *  @exception MEMORY_ERROR Not enough memory to convert the file
 *             name from the system path type.
 */
striType dirRead (dirType directory)

  {
    os_dirent_struct *current_entry;
    errInfoType err_info = OKAY_NO_ERROR;
    striType fileName;

  /* dirRead */
    logFunction(printf("dirRead(" FMT_U_MEM ")\n", (memSizeType) directory););
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
    if (IS_VOLUME_LIST(directory)) {
      fileName = readVolumeName((volumeListType *) directory);
    } else {
#endif
      do {
        current_entry = os_readdir(directory);
      } while (current_entry != NULL &&
          (memcmp(current_entry->d_name, dot,    sizeof(os_charType) * 2) == 0 ||
           memcmp(current_entry->d_name, dotdot, sizeof(os_charType) * 3) == 0));
      if (current_entry == NULL) {
        fileName = NULL;
      } else {
        /* printf("os_readdir() -> \"" FMT_S_OS "\"\n", current_entry->d_name); */
        fileName = os_stri_to_stri(current_entry->d_name, &err_info);
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(err_info);
        } /* if */
      } /* if */
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
    } /* if */
#endif
    logFunction(printf("dirRead --> \"%s\"\n",
                striAsUnquotedCStri(fileName)););
    return fileName;
  } /* dirRead */

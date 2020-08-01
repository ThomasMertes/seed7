/********************************************************************/
/*                                                                  */
/*  fil_win.c     File functions which call the Windows API.        */
/*  Copyright (C) 1989 - 2012  Thomas Mertes                        */
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
/*  File: seed7/src/fil_win.c                                       */
/*  Changes: 2011, 2012  Thomas Mertes                              */
/*  Content: File functions which call the Windows API.             */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "windows.h"
#include "conio.h"
#include "io.h"
#include "fcntl.h"
#include "sys/types.h"
#include "sys/stat.h"
#ifdef UNISTD_H_PRESENT
#include "unistd.h"
#endif

#include "common.h"
#include "os_decls.h"
#include "fil_rtl.h"
#include "rtl_err.h"



boolType filInputReady (fileType aFile)

  {
    int file_no;
    HANDLE fileHandle;
    os_fstat_struct stat_buf;
    DWORD totalBytesAvail;
    boolType result;

  /* filInputReady */
    file_no = fileno(aFile);
    if (file_no != -1 && os_fstat(file_no, &stat_buf) == 0) {
      if (S_ISREG(stat_buf.st_mode)) {
        result = TRUE;
      } else if (S_ISCHR(stat_buf.st_mode)) {
        /* printf("read_buffer_empty(aFile)=%d\n", read_buffer_empty(aFile)); */
        if (!read_buffer_empty(aFile)) {
          result = TRUE;
        } else if (file_no == 0) {
          /* printf("kbhit()=%d\n", kbhit()); */
          result = kbhit() != 0;
        } else {
          fileHandle = (HANDLE) _get_osfhandle(file_no);
          if (fileHandle != (HANDLE) -1) {
            result = WaitForSingleObject(fileHandle, 0) == WAIT_OBJECT_0;
          } else {
            raise_error(FILE_ERROR);
            result = FALSE;
          } /* if */
        } /* if */
      } else if (S_ISFIFO(stat_buf.st_mode)) {
        /* printf("read_buffer_empty(aFile)=%d\n", read_buffer_empty(aFile)); */
        if (!read_buffer_empty(aFile)) {
          result = TRUE;
        } else {
          fileHandle = (HANDLE) _get_osfhandle(file_no);
          if (fileHandle != (HANDLE) -1) {
            if (PeekNamedPipe(fileHandle, NULL, 0, NULL, &totalBytesAvail, NULL) != 0) {
              result = totalBytesAvail >= 1;
            } else if (GetLastError() == ERROR_BROKEN_PIPE || feof(aFile)) {
              result = TRUE;
            } else {
              raise_error(FILE_ERROR);
              result = FALSE;
            } /* if */
          } else {
            raise_error(FILE_ERROR);
            result = FALSE;
          } /* if */
        } /* if */
      } else {
        raise_error(FILE_ERROR);
        result = FALSE;
      } /* if */
    } else {
      raise_error(FILE_ERROR);
      result = FALSE;
    } /* if */
    return result;
  } /* filInputReady */



void setupFiles (void)

  { /* setupFiles */
    setmode(fileno(stdin), _O_BINARY);
    setmode(fileno(stdout), _O_BINARY);
    setmode(fileno(stderr), _O_BINARY);
  } /* setupFiles */

/********************************************************************/
/*                                                                  */
/*  fil_win.c     File functions which call the Windows API.        */
/*  Copyright (C) 1989 - 2011  Thomas Mertes                        */
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
/*  Changes: 2011  Thomas Mertes                                    */
/*  Content: File functions which call the Windows API.             */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "windows.h"
#include "sys/types.h"
#include "sys/stat.h"

#ifdef USE_MYUNISTD_H
#include "myunistd.h"
#else
#include "unistd.h"
#endif

#include "common.h"
#include "os_decls.h"
#include "fil_rtl.h"
#include "rtl_err.h"



#ifdef ANSI_C

booltype filInputReady (filetype aFile)
#else

booltype filInputReady (aFile)
filetype aFile;
#endif

  {
    int file_no;
    long pipeHandle;
    os_fstat_struct stat_buf;
    DWORD totalBytesAvail;
    booltype result;

  /* filInputReady */
    file_no = fileno(aFile);
    if (file_no != -1 && os_fstat(file_no, &stat_buf) == 0) {
      if (S_ISREG(stat_buf.st_mode)) {
        result = TRUE;
      } else if (S_ISFIFO(stat_buf.st_mode)) {
        pipeHandle = _get_osfhandle(file_no);
        if (pipeHandle != -1) {
          if (PeekNamedPipe((HANDLE) pipeHandle, NULL, 0, NULL, &totalBytesAvail, NULL) != 0) {
            result = totalBytesAvail >= 1;
          } else if (feof(aFile)) {
            result = TRUE;
          } else {
            raise_error(FILE_ERROR);
            result = FALSE;
          } /* if */
        } else {
          raise_error(FILE_ERROR);
          result = FALSE;
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

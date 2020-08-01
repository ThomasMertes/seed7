/********************************************************************/
/*                                                                  */
/*  fil_unx.c     File functions which call the Unix API.           */
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
/*  File: seed7/src/fil_unx.c                                       */
/*  Changes: 2011, 2012  Thomas Mertes                              */
/*  Content: File functions which call the Unix API.                */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#ifdef OUT_OF_ORDER
#include "sys/select.h"
#endif
#include "poll.h"

#include "common.h"
#include "os_decls.h"
#include "rtl_err.h"



#ifdef ANSI_C

booltype filInputReady (filetype aFile)
#else

booltype filInputReady (aFile)
filetype aFile;
#endif

  {
    int file_no;
    struct pollfd pollFd[1];
    int poll_result;
    booltype result;

  /* filInputReady */
    /* printf("filInputReady(%lx)\n", aFile); */
    if (!read_buffer_empty(aFile)) {
      result = TRUE;
    } else {
      file_no = fileno(aFile);
      if (file_no != -1) {
        /* printf("file_no=%d\n", file_no); */
        pollFd[0].fd = file_no;
        pollFd[0].events = POLLIN;
        poll_result = os_poll(pollFd, 1, 0);
        if (unlikely(poll_result < 0)) {
          raise_error(FILE_ERROR);
          result = FALSE;
        } else {
          /* printf("poll_result=%d, pollFd[0].revents=%08x\n", poll_result, pollFd[0].revents); */
          result = poll_result == 1 && (pollFd[0].revents & POLLIN);
        } /* if */
      } else {
        raise_error(FILE_ERROR);
        result = FALSE;
      } /* if */
    } /* if */
    return result;
  } /* filInputReady */

	
	
#ifdef OUT_OF_ORDER
#ifdef ANSI_C

booltype filInputReady (filetype aFile)
#else

booltype filInputReady (aFile)
filetype aFile;
#endif

  {
    int file_no;
    int nfds;
    fd_set readfds;
    struct timeval timeout;
    int select_result;
    booltype result;

  /* filInputReady */
    file_no = fileno(aFile);
    if (file_no != -1) {
      FD_ZERO(&readfds);
      FD_SET(file_no, &readfds);
      nfds = (int) file_no + 1;
      timeout.tv_sec = 0;
      timeout.tv_usec = 0;
      /* printf("select(%d, %d)\n", nfds, file_no); */
      select_result = select(nfds, &readfds, NULL, NULL, &timeout);
      /* printf("select_result: %d\n", select_result); */
      if (unlikely(select_result < 0)) {
        raise_error(FILE_ERROR);
        result = FALSE;
      } else {
        result = FD_ISSET(file_no, &readfds);
      } /* if */
    } else {
      raise_error(FILE_ERROR);
      result = FALSE;
    } /* if */
    /* printf("filInputReady --> %d\n", result); */
    return result;
  } /* filInputReady */
#endif

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

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "signal.h"
#include "setjmp.h"
#if defined read_buffer_empty && HAS_POLL
#include "poll.h"
#else
#include "fcntl.h"
#endif
#ifdef MOUNT_NODEFS
#include "emscripten.h"
#endif
#include "errno.h"

#if UNISTD_H_PRESENT
#include "unistd.h"
#endif

#include "common.h"
#include "os_decls.h"
#include "rtl_err.h"

#if HAS_SIGACTION || HAS_SIGNAL
static longjmpPosition sigintOccurred;
#endif



#if HAS_SIGACTION || HAS_SIGNAL
static void handleIntSignal (int sig_num)

  {
#if SIGNAL_RESETS_HANDLER
    signal(SIGINT, handleIntSignal);
#endif
    do_longjmp(sigintOccurred, 1);
  }



int readCharChkCtrlC (fileType inFile, boolType *sigintReceived)

  {
#if HAS_SIGACTION
    struct sigaction sigAct;
    struct sigaction oldSigAct;
#elif HAS_SIGNAL
    void (*oldSigHandler) (int);
#endif
    int ch = ' ';

  /* readCharChkCtrlC */
    logFunction(printf("readCharChkCtrlC(%d, %d)\n",
                       safe_fileno(inFile), *sigintReceived););
#if HAS_SIGACTION
    sigAct.sa_handler = handleIntSignal;
    sigemptyset(&sigAct.sa_mask);
    sigAct.sa_flags = SA_RESTART;
    if (unlikely(sigaction(SIGINT, &sigAct, &oldSigAct) != 0)) {
#elif HAS_SIGNAL
    oldSigHandler = signal(SIGINT, handleIntSignal);
    if (unlikely(oldSigHandler == SIG_ERR)) {
#endif
      logError(printf("readCharChkCtrlC(%d, *): "
                      "signal(SIGINT, handleIntSignal) failed:\n"
                      "errno=%d\nerror: %s\n",
                      safe_fileno(inFile), errno, strerror(errno)););
      raise_error(FILE_ERROR);
    } else {
      if (do_setjmp(sigintOccurred) == 0) {
        ch = getc(inFile);
        *sigintReceived = FALSE;
      } else {
        *sigintReceived = TRUE;
      } /* if */
#if HAS_SIGACTION
      if (unlikely(sigaction(SIGINT, &oldSigAct, NULL) != 0)) {
#elif HAS_SIGNAL
      if (unlikely(signal(SIGINT, oldSigHandler) == SIG_ERR)) {
#endif
        logError(printf("readCharChkCtrlC(%d, *): "
                        "signal(SIGINT, oldSigHandler) failed:\n"
                        "errno=%d\nerror: %s\n",
                        safe_fileno(inFile), errno, strerror(errno)););
        raise_error(FILE_ERROR);
      } /* if */
    } /* if */
    logFunction(printf("readCharChkCtrlC(%d, %d) --> %d\n",
                       safe_fileno(inFile), *sigintReceived, ch););
    return ch;
  } /* readCharChkCtrlC */
#endif



#if defined read_buffer_empty && HAS_POLL
boolType filInputReady (fileType aFile)

  {
    int file_no;
    struct pollfd pollFd[1];
    int poll_result;
    boolType inputReady;

  /* filInputReady */
    logFunction(printf("filInputReady(%d)\n", safe_fileno(aFile)););
    if (!read_buffer_empty(aFile)) {
      inputReady = TRUE;
    } else {
      file_no = fileno(aFile);
      if (unlikely(file_no == -1)) {
        logError(printf("filInputReady(%d): fileno(%d) failed:\n"
                        "errno=%d\nerror: %s\n",
                        safe_fileno(aFile), safe_fileno(aFile),
                        errno, strerror(errno)););
        raise_error(FILE_ERROR);
        inputReady = FALSE;
      } else {
        /* printf("file_no=%d\n", file_no); */
        pollFd[0].fd = file_no;
        pollFd[0].events = POLLIN;
        poll_result = os_poll(pollFd, 1, 0);
        if (unlikely(poll_result < 0)) {
          logError(printf("filInputReady(%d): poll([%d, POLLIN], 1, 0) failed:\n"
                          "errno=%d\nerror: %s\n",
                          safe_fileno(aFile), safe_fileno(aFile),
                          errno, strerror(errno)););
          raise_error(FILE_ERROR);
          inputReady = FALSE;
        } else {
          /* printf("poll_result=%d, pollFd[0].revents=%08x\n", poll_result, pollFd[0].revents);
          printf("POLLIN=%08x, POLLPRI=%08x, POLLERR=%08x, POLLHUP=%08x\n",
              POLLIN, POLLPRI, POLLERR, POLLHUP); */
          inputReady = poll_result == 1 && (pollFd[0].revents & (POLLIN | POLLHUP));
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("filInputReady(%d) --> %d\n",
                       safe_fileno(aFile), inputReady););
    return inputReady;
  } /* filInputReady */

#else



boolType filInputReady (fileType aFile)

  {
    int file_no;
    int flags;
    int ch;
    int saved_errno;
    boolType inputReady;

  /* filInputReady */
    logFunction(printf("filInputReady(%d)\n", safe_fileno(aFile)););
    file_no = fileno(aFile);
    if (unlikely(file_no == -1)) {
      logError(printf("filInputReady(%d): fileno(%d) failed:\n"
                      "errno=%d\nerror: %s\n",
                      safe_fileno(aFile), safe_fileno(aFile),
                      errno, strerror(errno)););
      raise_error(FILE_ERROR);
      inputReady = FALSE;
    } else {
      /* printf("file_no=%d\n", file_no); */
      flags = fcntl(file_no, F_GETFL);
      fcntl(file_no, F_SETFL, flags|O_NONBLOCK);
      ch = getc(aFile);
      saved_errno = errno;
      /* printf("errno=%d ", saved_errno); */
      if (ch == EOF) {
        if (feof(aFile)) {
          clearerr(aFile);
          inputReady = TRUE;
        } else if (saved_errno == EAGAIN || saved_errno == EIO) {
          inputReady = FALSE;
        } else {
          inputReady = TRUE;
        } /* if */
      } else {
        ungetc(ch, aFile);
        inputReady = TRUE;
      } /* if */
      fcntl(file_no, F_SETFL, flags);
    } /* if */
    logFunction(printf("filInputReady(%d) --> %d\n",
                       safe_fileno(aFile), inputReady););
    return inputReady;
  } /* filInputReady */
#endif



#ifdef OUT_OF_ORDER
boolType filInputReady (fileType aFile)

  {
    int file_no;
    int nfds;
    fd_set readfds;
    struct timeval timeout;
    int select_result;
    boolType inputReady;

  /* filInputReady */
    logFunction(printf("filInputReady(%d)\n", safe_fileno(aFile)););
    file_no = fileno(aFile);
    if (unlikely(file_no == -1)) {
      logError(printf("filInputReady(%d): fileno(%d) failed:\n"
                      "errno=%d\nerror: %s\n",
                      safe_fileno(aFile), safe_fileno(aFile),
                      errno, strerror(errno)););
      raise_error(FILE_ERROR);
      inputReady = FALSE;
    } else {
      FD_ZERO(&readfds);
      FD_SET(file_no, &readfds);
      nfds = (int) file_no + 1;
      timeout.tv_sec = 0;
      timeout.tv_usec = 0;
      /* printf("select(%d, %d)\n", nfds, file_no); */
      select_result = select(nfds, &readfds, NULL, NULL, &timeout);
      /* printf("select_result: %d\n", select_result); */
      if (unlikely(select_result < 0)) {
        logError(printf("filInputReady(%d): "
                        "select(%d, [%d], NULL, NULL, [0, 0]) failed:\n"
                        "errno=%d\nerror: %s\n",
                        safe_fileno(aFile), nfds, sock,
                        errno, strerror(errno)););
        raise_error(FILE_ERROR);
        inputReady = FALSE;
      } else {
        inputReady = FD_ISSET(file_no, &readfds);
      } /* if */
    } /* if */
    logFunction(printf("filInputReady(%d) --> %d\n",
                       safe_fileno(aFile), inputReady););
    return inputReady;
  } /* filInputReady */
#endif



void filPipe (fileType *inFile, fileType *outFile)

  {
    int aPipe[2];

  /* filPipe */
    if (unlikely(pipe(aPipe) != 0)) {
      logError(printf("filPipe: pipe(aPipe) failed:\n"
                      "errno=%d\nerror: %s\n",
                      errno, strerror(errno)););
      *inFile = NULL;
      *outFile = NULL;
      raise_error(FILE_ERROR);
    } else {
      if (unlikely((*inFile = fdopen(aPipe[0], "rb")) == NULL)) {
        logError(printf("filPipe: fdopen(%d, \"rb\") failed:\n"
                      "errno=%d\nerror: %s\n",
                      aPipe[0], errno, strerror(errno)););
        close(aPipe[0]);
        close(aPipe[1]);
        *outFile = NULL;
        raise_error(FILE_ERROR);
      } else if (unlikely((*outFile = fdopen(aPipe[1], "wb")) == NULL)) {
        logError(printf("filPipe: fdopen(%d, \"wb\") failed:\n"
                      "errno=%d\nerror: %s\n",
                      aPipe[1], errno, strerror(errno)););
        fclose(*inFile);
        *inFile = NULL;
        close(aPipe[1]);
        raise_error(FILE_ERROR);
      } /* if */
    } /* if */
    logFunction(printf("filPipe --> {%d, %d}\n",
                       safe_fileno(*inFile), safe_fileno(*outFile)));
  } /* filPipe */



void setupFiles (void)

  { /* setupFiles */
    logFunction(printf("setupFiles\n"););
#ifdef MOUNT_NODEFS
#ifdef _WIN32
    EM_ASM(
      var fs = require('fs');
      /* FS.unmount('/'); */
      /* FS.mount(NODEFS, { root: 'c:/' }, '/root'); */
      var workDir = process.cwd().replace(/\\\\/g, '/');
      var mountPoint = '.';
      if (workDir.charAt(1) == ':' && workDir.charAt(2) == '/') {
        workDir = '/' + workDir.charAt(0).toLowerCase() + workDir.substring(2);
        count = (workDir.match(new RegExp("/", "g")) || []).length;
        mountPoint = new Array(count).join('/..').substring(1);
      }
      process.stdout.write(workDir);
      process.stdout.write(count.toString());
      process.stdout.write(mountPoint);
      FS.mkdir('/root');
      FS.mount(NODEFS, { root: '..' }, '/root');
      FS.chdir('/root/prg');
    );
#else
    EM_ASM(
      var fs = require('fs');
      var mountPoint = '.';
      var rootDir = '/';
      var workDir = process.cwd();
      // process.stdout.write(workDir);
      // process.stdout.write('\n');
      workDir = workDir.replace(/\\\\/g, '/');
      // process.stdout.write(workDir);
      // process.stdout.write('\n');
      if (workDir.charAt(1) == ':' && workDir.charAt(2) == '/') {
        mountPoint = workDir.substring(0, 3);
        rootDir = '/' + workDir.charAt(0).toLowerCase();
        workDir = rootDir + workDir.substring(2);
      }
      // process.stdout.write('workDir: ' + workDir);
      // process.stdout.write('\n');
      // process.stdout.write('mountPoint: ' + mountPoint);
      // process.stdout.write('\n');
      // process.stdout.write('rootDir: ' + rootDir);
      // process.stdout.write('\n');
      FS.mkdir(rootDir);
      FS.mount(NODEFS, { root: mountPoint }, rootDir);
      // Note: Chdir fails for unsufficient permission. E.g. with /c/Users
      // process.stdout.write('before chdir');
      FS.chdir(workDir);
      // process.stdout.write('after chdir');
    );
#endif
#endif
    logFunction(printf("setupFiles -->\n"););
  } /* setupFiles */

/********************************************************************/
/*                                                                  */
/*  fil_dos.c     File functions which call the Dos API.            */
/*  Copyright (C) 1989 - 2018  Thomas Mertes                        */
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
/*  Changes: 2011, 2018  Thomas Mertes                              */
/*  Content: File functions which call the Dos API.                 */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdio.h"
#include "string.h"
#include "signal.h"
#include "setjmp.h"
#include "termios.h"
#include "io.h"
#include "fcntl.h"
#if UNISTD_H_PRESENT
#include "unistd.h"
#endif
#include "errno.h"

#include "common.h"
#include "os_decls.h"
#include "fil_rtl.h"
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



int readCharChkCtrlC (cFileType inFile, boolType *sigintReceived)

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



/**
 *  Determine if at least one character can be read without blocking.
 *  Blocking means that 'getc' would wait until a character is
 *  received. Blocking can last for a period of unspecified length.
 *  Regular files do not block.
 *  @return TRUE if 'getc' would not block, FALSE otherwise.
 */
boolType filInputReady (fileType inFile)

  { /* filInputReady */
    return TRUE;
  } /* filInputReady */



void filPipe (fileType *inFile, fileType *outFile)

  { /* filPipe */
    *inFile = NULL;
    *outFile = NULL;
    raise_error(FILE_ERROR);
  } /* filPipe */



void setupFiles (void)

  {
    struct termios term_descr;

  /* setupFiles */
    stdinFileRecord.cFile = stdin;
    stdoutFileRecord.cFile = stdout;
    stderrFileRecord.cFile = stderr;
    if (os_isatty(STDIN_FILENO)) {
      if (tcgetattr(STDIN_FILENO, &term_descr) != 0) {
        printf("setupFiles: tcgetattr(STDIN_FILENO, ...) failed:\n"
               "errno=%d\nerror: %s\n",
               errno, strerror(errno));
      } else {
        term_descr.c_cc[VEOL] = (cc_t) 3;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &term_descr) != 0) {
          printf("setupFiles: tcsetattr(STDIN_FILENO, TCSANOW, VEOL=3) failed:\n"
                 "errno=%d\nerror: %s\n",
                 errno, strerror(errno));
        } /* if */
      } /* if */
    } else {
      setmode(STDIN_FILENO, O_BINARY);
    } /* if */
    if (!os_isatty(STDOUT_FILENO)) {
      setmode(STDOUT_FILENO, O_BINARY);
    } /* if */
    if (!os_isatty(STDERR_FILENO)) {
      setmode(STDERR_FILENO, O_BINARY);
    } /* if */
  } /* setupFiles */

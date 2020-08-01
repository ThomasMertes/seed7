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

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "windows.h"
#include "io.h"
#include "fcntl.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "signal.h"
#if UNISTD_H_PRESENT
#include "unistd.h"
#endif
#include "errno.h"

#include "common.h"
#include "os_decls.h"
#include "stat_drv.h"
#include "fil_rtl.h"
#include "rtl_err.h"


#if !INTPTR_T_DEFINED
#if POINTER_SIZE == 32
typedef int32Type  intptr_t;
#elif POINTER_SIZE == 64
typedef int64Type  intptr_t;
#endif
#endif

#if HAS_SIGACTION || HAS_SIGNAL
static volatile int waitForHandler;
#endif



#if HAS_SIGACTION || HAS_SIGNAL
static void handleIntSignal (int sig_num)

  {
#if SIGNAL_RESETS_HANDLER
    signal(SIGINT, handleIntSignal);
#endif
    waitForHandler = 0;
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
      waitForHandler = 1;
      ch = getc(inFile);
      *sigintReceived = feof(inFile);
      if (*sigintReceived) {
        clearerr(inFile);
        fflush(inFile);
        /* The interrupt thread runs in parallel. */
        /* Wait until handleIntSignal() has finished. */
        while (waitForHandler) ;
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



static boolType stdinReady (void)

  {
    HANDLE hKeyboard;
    DWORD numEvents;
    INPUT_RECORD *events;
    DWORD count;
    DWORD idx;
    boolType result = FALSE;

  /* stdinReady */
    logFunction(printf("stdinReady\n"););
    hKeyboard = GetStdHandle(STD_INPUT_HANDLE);
    if (hKeyboard != INVALID_HANDLE_VALUE &&
        GetNumberOfConsoleInputEvents(hKeyboard, &numEvents) != 0) {
      /* printf("numEvents: %lu\n", (unsigned long) numEvents); */
      events = malloc(sizeof(INPUT_RECORD) * numEvents);
      if (events != NULL) {
        if (PeekConsoleInputW(hKeyboard, events, numEvents, &count) != 0) {
          for (idx = 0; idx < count; idx++) {
            /* printf("EventType: %d\n", events[idx].EventType); */
            if (events[idx].EventType == KEY_EVENT &&
                events[idx].Event.KeyEvent.bKeyDown &&
                events[idx].Event.KeyEvent.wVirtualKeyCode == VK_RETURN) {
              result = TRUE;
            } /* if */
          } /* for */
        } /* if */
        free(events);
      } /* if */
    } /* if */
    logFunction(printf("stdinReady --> %d\n", result););
    return result;
  } /* stdinReady */



boolType filInputReady (fileType aFile)

  {
    int file_no;
    HANDLE fileHandle;
    os_fstat_struct stat_buf;
    DWORD totalBytesAvail;
    boolType result;

  /* filInputReady */
    logFunction(printf("filInputReady(%d)\n", safe_fileno(aFile)););
    file_no = fileno(aFile);
    if (unlikely(file_no == -1)) {
      logError(printf("filInputReady(%d): fileno(%d) failed:\n"
                      "errno=%d\nerror: %s\n",
                      safe_fileno(aFile), safe_fileno(aFile),
                      errno, strerror(errno)););
      raise_error(FILE_ERROR);
      result = FALSE;
    } else if (unlikely(os_fstat(file_no, &stat_buf) != 0)) {
      logError(printf("filInputReady(%d): fstat(%d, *) failed:\n"
                      "errno=%d\nerror: %s\n",
                      safe_fileno(aFile), safe_fileno(aFile),
                      errno, strerror(errno)););
      raise_error(FILE_ERROR);
      result = FALSE;
    } else {
      if (S_ISREG(stat_buf.st_mode)) {
        result = TRUE;
      } else if (S_ISCHR(stat_buf.st_mode)) {
        /* printf("read_buffer_empty(aFile)=%d\n", read_buffer_empty(aFile)); */
        if (!read_buffer_empty(aFile)) {
          result = TRUE;
        } else if (file_no == 0) {
          result = stdinReady();
          /* printf("stdinReady()=%d\n", result); */
        } else {
          fileHandle = (HANDLE) _get_osfhandle(file_no);
          if (unlikely(fileHandle == (HANDLE) -1)) {
            logError(printf("filInputReady(%d): _get_osfhandle(%d) failed:\n"
                            "errno=%d\nerror: %s\n",
                            safe_fileno(aFile), safe_fileno(aFile),
                            errno, strerror(errno)););
            raise_error(FILE_ERROR);
            result = FALSE;
          } else {
            result = WaitForSingleObject(fileHandle, 0) == WAIT_OBJECT_0;
          } /* if */
        } /* if */
      } else if (S_ISFIFO(stat_buf.st_mode)) {
        /* printf("read_buffer_empty(aFile)=%d\n", read_buffer_empty(aFile)); */
        if (!read_buffer_empty(aFile)) {
          result = TRUE;
        } else {
          fileHandle = (HANDLE) _get_osfhandle(file_no);
          if (unlikely(fileHandle == (HANDLE) -1)) {
            logError(printf("filInputReady(%d): _get_osfhandle(%d) failed:\n"
                            "errno=%d\nerror: %s\n",
                            safe_fileno(aFile), safe_fileno(aFile),
                            errno, strerror(errno)););
            raise_error(FILE_ERROR);
            result = FALSE;
          } else {
            if (PeekNamedPipe(fileHandle, NULL, 0, NULL, &totalBytesAvail, NULL) != 0) {
              result = totalBytesAvail >= 1;
            } else if (GetLastError() == ERROR_BROKEN_PIPE || feof(aFile)) {
              result = TRUE;
            } else {
              logError(printf("filInputReady(%d): PeekNamedPipe(" FMT_U_MEM ", ...) failed:\n"
                              "errno=%d\nerror: %s\n",
                              safe_fileno(aFile), (memSizeType) fileHandle,
                              errno, strerror(errno)););
              raise_error(FILE_ERROR);
              result = FALSE;
            } /* if */
          } /* if */
        } /* if */
      } else {
        logError(printf("filInputReady(%d): Unknown file type %d.\n",
                        safe_fileno(aFile), stat_buf.st_mode & S_IFMT););
        raise_error(FILE_ERROR);
        result = FALSE;
      } /* if */
    } /* if */
    logFunction(printf("filInputReady(%d) --> %d\n",
                       safe_fileno(aFile), result););
    return result;
  } /* filInputReady */



void filPipe (fileType *inFile, fileType *outFile)

  {
    SECURITY_ATTRIBUTES saAttr;
    HANDLE pipeReadHandle = INVALID_HANDLE_VALUE;
    HANDLE pipeWriteHandle = INVALID_HANDLE_VALUE;
    int pipeReadFildes;
    int pipeWriteFildes;

  /* filPipe */
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;
    if (unlikely(CreatePipe(&pipeReadHandle,
                            &pipeWriteHandle, &saAttr, 0) == 0)) {
      logError(printf("filPipe: CreatePipe() failed.\n"););
      raise_error(FILE_ERROR);
    } else {
      if (unlikely((pipeReadFildes =
          _open_osfhandle((intptr_t) (pipeReadHandle), _O_TEXT)) == -1)) {
        logError(printf("filPipe: _open_osfhandle() failed:\n"););
        CloseHandle(pipeReadHandle);
        CloseHandle(pipeWriteHandle);
        *inFile = NULL;
        *outFile = NULL;
        raise_error(FILE_ERROR);
      } else if (unlikely((pipeWriteFildes =
          _open_osfhandle((intptr_t) (pipeWriteHandle), _O_TEXT)) == -1)) {
        logError(printf("filPipe: _open_osfhandle() failed:\n"););
        _close(pipeReadFildes);
        CloseHandle(pipeWriteHandle);
        *inFile = NULL;
        *outFile = NULL;
        raise_error(FILE_ERROR);
      } else if (unlikely((*inFile = fdopen(pipeReadFildes, "rb")) == NULL)) {
        logError(printf("filPipe: fdopen(%d, \"rb\") failed:\n"
                      "errno=%d\nerror: %s\n",
                      pipeReadFildes, errno, strerror(errno)););
        _close(pipeReadFildes);
        _close(pipeWriteFildes);
        *outFile = NULL;
        raise_error(FILE_ERROR);
      } else if (unlikely((*outFile = fdopen(pipeWriteFildes, "wb")) == NULL)) {
        logError(printf("filPipe: fdopen(%d, \"wb\") failed:\n"
                      "errno=%d\nerror: %s\n",
                      pipeWriteFildes, errno, strerror(errno)););
        fclose(*inFile);
        *inFile = NULL;
        _close(pipeWriteFildes);
        raise_error(FILE_ERROR);
      } /* if */
    } /* if */
    logFunction(printf("filPipe --> {%d, %d}\n",
                       safe_fileno(*inFile), safe_fileno(*outFile)));
  } /* filPipe */



#if !HAS_SNPRINTF
int snprintf (char *buffer, size_t bufsize, const char *fmt, ...)

  {
    va_list ap;
    int result;

  /* snprintf */
    if (bufsize == 0) {
      result = 0;
    } else {
      va_start(ap, fmt);
      result = _vsnprintf(buffer, bufsize - 1, fmt, ap);
      if (result < 0) {
        buffer[bufsize - 1] = '\0';
        result = (int) bufsize;
      } else if (result == bufsize - 1) {
        buffer[bufsize - 1] = '\0';
      } /* if */      
      va_end(ap);
    } /* if */
    return result;
  } /* snprintf */
#endif



void setupFiles (void)

  {
    HANDLE hConsole;
    DWORD mode;

  /* setupFiles */
    /* Redirected files are set to _O_BINARY mode.       */
    /* Only real console files are left in _O_TEXT mode. */
    /* This way the ENTER key is translated to '\n'.     */
    hConsole = GetStdHandle(STD_INPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE ||
        GetFileType(hConsole) != FILE_TYPE_CHAR ||
        GetConsoleMode(hConsole, &mode) == 0) {
      setmode(fileno(stdin), _O_BINARY);
    } /* if */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE ||
        GetFileType(hConsole) != FILE_TYPE_CHAR ||
        GetConsoleMode(hConsole, &mode) == 0) {
      setmode(fileno(stdout), _O_BINARY);
    } /* if */
    hConsole = GetStdHandle(STD_ERROR_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE ||
        GetFileType(hConsole) != FILE_TYPE_CHAR ||
        GetConsoleMode(hConsole, &mode) == 0) {
      setmode(fileno(stderr), _O_BINARY);
    } /* if */
  } /* setupFiles */

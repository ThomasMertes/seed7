/********************************************************************/
/*                                                                  */
/*  fil_win.c     File functions which call the Windows API.        */
/*  Copyright (C) 1989 - 2013, 2015 - 2019, 2021  Thomas Mertes     */
/*                2024, 2025  Thomas Mertes                         */
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
/*  Changes: 2011 - 2013, 2015 - 2019, 2021, 2024  Thomas Mertes    */
/*           2025  Thomas Mertes                                    */
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
#if STDINT_H_PRESENT
#include "stdint.h"
#endif
#if UNISTD_H_PRESENT
#include "unistd.h"
#endif
#include "errno.h"

#include "common.h"
#include "os_decls.h"
#include "heaputl.h"
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



/**
 *  Read a character from 'inFile' and check if ctrl-c is pressed.
 *  @param inFile File from which the character is read.
 *  @param sigintReceived Flag indicating if ctrl-c has been pressed.
 *  @return the character read.
 */
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
    DWORD numEventsRead;
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
        if (PeekConsoleInputW(hKeyboard, events, numEvents, &numEventsRead) != 0) {
          for (idx = 0; idx < numEventsRead; idx++) {
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



/**
 *  Determine if at least one character can be read without blocking.
 *  Blocking means that 'getc' would wait until a character is
 *  received. Blocking can last for a period of unspecified length.
 *  Regular files do not block.
 *  @return TRUE if 'getc' would not block, FALSE otherwise.
 */
boolType filInputReady (fileType inFile)

  {
    cFileType cInFile;
    int file_no;
    HANDLE fileHandle;
    os_fstat_struct stat_buf;
    DWORD totalBytesAvail;
    boolType inputReady;

  /* filInputReady */
    logFunction(printf("filInputReady(%s%d)\n",
                       inFile == NULL ? "NULL " : "",
                       inFile != NULL ? safe_fileno(inFile->cFile) : 0););
    cInFile = inFile->cFile;
    if (unlikely(cInFile == NULL)) {
      logError(printf("filInputReady: Called with a closed file.\n"););
      raise_error(FILE_ERROR);
      inputReady = FALSE;
    } else {
      file_no = os_fileno(cInFile);
      if (unlikely(file_no == -1)) {
        logError(printf("filInputReady(%d): os_fileno(%d) failed:\n"
                        "errno=%d\nerror: %s\n",
                        safe_fileno(cInFile), safe_fileno(cInFile),
                        errno, strerror(errno)););
        raise_error(FILE_ERROR);
        inputReady = FALSE;
      } else if (unlikely(os_fstat(file_no, &stat_buf) != 0)) {
        logError(printf("filInputReady(%d): fstat(%d, *) failed:\n"
                        "errno=%d\nerror: %s\n",
                        safe_fileno(cInFile), safe_fileno(cInFile),
                        errno, strerror(errno)););
        raise_error(FILE_ERROR);
        inputReady = FALSE;
      } else {
        if (S_ISREG(stat_buf.st_mode)) {
          inputReady = TRUE;
        } else if (S_ISCHR(stat_buf.st_mode)) {
          /* printf("read_buffer_empty(cInFile)=%d\n", read_buffer_empty(cInFile)); */
          if (!read_buffer_empty(cInFile)) {
            inputReady = TRUE;
          } else if (file_no == 0) {
            inputReady = stdinReady();
            /* printf("stdinReady()=%d\n", result); */
          } else {
            fileHandle = (HANDLE) _get_osfhandle(file_no);
            if (unlikely(fileHandle == (HANDLE) -1)) {
              logError(printf("filInputReady(%d): _get_osfhandle(%d) failed:\n"
                              "errno=%d\nerror: %s\n",
                              safe_fileno(cInFile), safe_fileno(cInFile),
                              errno, strerror(errno)););
              raise_error(FILE_ERROR);
              inputReady = FALSE;
            } else {
              inputReady = WaitForSingleObject(fileHandle, 0) == WAIT_OBJECT_0;
            } /* if */
          } /* if */
        } else if (S_ISFIFO(stat_buf.st_mode)) {
          /* printf("read_buffer_empty(cInFile)=%d\n", read_buffer_empty(cInFile)); */
          if (!read_buffer_empty(cInFile)) {
            inputReady = TRUE;
          } else {
            fileHandle = (HANDLE) _get_osfhandle(file_no);
            if (unlikely(fileHandle == (HANDLE) -1)) {
              logError(printf("filInputReady(%d): _get_osfhandle(%d) failed:\n"
                              "errno=%d\nerror: %s\n",
                              safe_fileno(cInFile), safe_fileno(cInFile),
                              errno, strerror(errno)););
              raise_error(FILE_ERROR);
              inputReady = FALSE;
            } else {
              if (PeekNamedPipe(fileHandle, NULL, 0, NULL, &totalBytesAvail, NULL) != 0) {
                inputReady = totalBytesAvail >= 1;
              } else if (GetLastError() == ERROR_BROKEN_PIPE || feof(cInFile)) {
                inputReady = TRUE;
              } else {
                logError(printf("filInputReady(%d): PeekNamedPipe(" FMT_U_MEM ", ...) failed:\n"
                                "errno=%d\nerror: %s\n",
                                safe_fileno(cInFile), (memSizeType) fileHandle,
                                errno, strerror(errno)););
                raise_error(FILE_ERROR);
                inputReady = FALSE;
              } /* if */
            } /* if */
          } /* if */
        } else {
          logError(printf("filInputReady(%d): Unknown file type %d.\n",
                          safe_fileno(cInFile), stat_buf.st_mode & S_IFMT););
          raise_error(FILE_ERROR);
          inputReady = FALSE;
        } /* if */
      } /* if */
    } /* if */
    logFunction(printf("filInputReady(%d) --> %d\n",
                       safe_fileno(cInFile), inputReady););
    return inputReady;
  } /* filInputReady */



void filPipe (fileType *inFile, fileType *outFile)

  {
    fileType pipeInFile = NULL;
    fileType pipeOutFile = NULL;
    SECURITY_ATTRIBUTES saAttr;
    HANDLE pipeReadHandle = INVALID_HANDLE_VALUE;
    HANDLE pipeWriteHandle = INVALID_HANDLE_VALUE;
    int pipeReadFildes;
    int pipeWriteFildes;
    cFileType cPipeInFile;
    cFileType cPipeOutFile;
    errInfoType err_info = OKAY_NO_ERROR;

  /* filPipe */
    if (unlikely(!ALLOC_RECORD(pipeInFile, fileRecord, count.files) ||
                 !ALLOC_RECORD(pipeOutFile, fileRecord, count.files))) {
      err_info = MEMORY_ERROR;
    } else {
      saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
      saAttr.bInheritHandle = TRUE;
      saAttr.lpSecurityDescriptor = NULL;
      if (unlikely(CreatePipe(&pipeReadHandle,
                              &pipeWriteHandle, &saAttr, 0) == 0)) {
        logError(printf("filPipe: CreatePipe() failed.\n"););
        err_info = FILE_ERROR;
      } else {
        if (unlikely((pipeReadFildes =
            _open_osfhandle((intptr_t) (pipeReadHandle), _O_TEXT)) == -1)) {
          logError(printf("filPipe: _open_osfhandle() failed:\n"););
          CloseHandle(pipeReadHandle);
          CloseHandle(pipeWriteHandle);
          err_info = FILE_ERROR;
        } else if (unlikely((pipeWriteFildes =
            _open_osfhandle((intptr_t) (pipeWriteHandle), _O_TEXT)) == -1)) {
          logError(printf("filPipe: _open_osfhandle() failed:\n"););
          _close(pipeReadFildes);
          CloseHandle(pipeWriteHandle);
          err_info = FILE_ERROR;
        } else if (unlikely((cPipeInFile = os_fdopen(pipeReadFildes, "rb")) == NULL)) {
          logError(printf("filPipe: os_fdopen(%d, \"rb\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        pipeReadFildes, errno, strerror(errno)););
          _close(pipeReadFildes);
          _close(pipeWriteFildes);
          err_info = FILE_ERROR;
        } else if (unlikely((cPipeOutFile = os_fdopen(pipeWriteFildes, "wb")) == NULL)) {
          logError(printf("filPipe: os_fdopen(%d, \"wb\") failed:\n"
                        "errno=%d\nerror: %s\n",
                        pipeWriteFildes, errno, strerror(errno)););
          fclose(cPipeInFile);
          _close(pipeWriteFildes);
          err_info = FILE_ERROR;
        } else {
          filDestr(*inFile);
          initFileType(pipeInFile, TRUE, FALSE);
          pipeInFile->cFile = cPipeInFile;
          *inFile = pipeInFile;
          filDestr(*outFile);
          initFileType(pipeOutFile, FALSE, TRUE);
          pipeOutFile->cFile = cPipeOutFile;
          *outFile = pipeOutFile;
        } /* if */
      } /* if */
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      if (pipeInFile != NULL) {
        FREE_RECORD(pipeInFile, fileRecord, count.files);
      } /* if */
      if (pipeOutFile != NULL) {
        FREE_RECORD(pipeOutFile, fileRecord, count.files);
      } /* if */
    } /* if */
    logFunction(printf("filPipe --> {%s%d, %s%d}\n",
                       *inFile == NULL ? "NULL " : "",
                       *inFile != NULL ? safe_fileno((*inFile)->cFile) : 0,
                       *outFile == NULL ? "NULL " : "",
                       *outFile != NULL ? safe_fileno((*outFile)->cFile) : 0));
  } /* filPipe */



#ifdef DEFINE_FTELLI64_EXT
#if DEFINE_FTELLI64_EXT == 1
os_off_t ftelli64Ext (FILE *aFile)

  {
    fpos_t pos;
    os_off_t filePosition;

  /* ftelli64Ext */
    if (fgetpos(aFile, &pos) == 0) {
      memcpy(&filePosition, &pos, sizeof(os_off_t));
    } else {
      filePosition = -1;
    } /* if */
    return filePosition;
  } /* ftelli64Ext */



#elif DEFINE_FTELLI64_EXT == 2
os_off_t ftelli64Ext (FILE *aFile)

  {
    int file_no;
    fpos_t pos;
    os_off_t filePosition;

  /* ftelli64Ext */
    file_no = os_fileno(aFile);
    if (file_no == -1) {
      filePosition = -1;
    } else {
      /* Use fgetpos() and fsetpos() to ensure, that the internal buffer */
      /* of aFile is synchronized with the underlying file descriptor file. */
      /* This way _telli64() returns the same value as _ftelli64() would do. */
      if (fgetpos(aFile, &pos) == 0 && fsetpos(aFile, &pos) == 0) {
        filePosition = _telli64(file_no);
      } else {
        filePosition = -1;
      } /* if */
    } /* if */
    return filePosition;
  } /* ftelli64Ext */

#endif
#endif



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
      } else if ((unsigned int) result == bufsize - 1) {
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
    stdinFileRecord.cFile = stdin;
    stdoutFileRecord.cFile = stdout;
    stderrFileRecord.cFile = stderr;
    /* Redirected files are set to _O_BINARY mode.       */
    /* Only real console files are left in _O_TEXT mode. */
    /* This way the ENTER key is translated to '\n'.     */
    hConsole = GetStdHandle(STD_INPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE ||
        GetFileType(hConsole) != FILE_TYPE_CHAR ||
        GetConsoleMode(hConsole, &mode) == 0) {
      os_setmode(os_fileno(stdin), _O_BINARY);
    } /* if */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE ||
        GetFileType(hConsole) != FILE_TYPE_CHAR ||
        GetConsoleMode(hConsole, &mode) == 0) {
      os_setmode(os_fileno(stdout), _O_BINARY);
    } /* if */
    hConsole = GetStdHandle(STD_ERROR_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE ||
        GetFileType(hConsole) != FILE_TYPE_CHAR ||
        GetConsoleMode(hConsole, &mode) == 0) {
      os_setmode(os_fileno(stderr), _O_BINARY);
    } /* if */
  } /* setupFiles */

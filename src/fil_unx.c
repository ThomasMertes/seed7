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
#include "heaputl.h"
#include "striutl.h"
#include "cmd_drv.h"
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
#if defined read_buffer_empty && HAS_POLL
boolType filInputReady (fileType inFile)

  {
    cFileType cInFile;
    int file_no;
    struct pollfd pollFd[1];
    int poll_result;
    boolType inputReady;

  /* filInputReady */
    logFunction(printf("filInputReady(%s%d)\n",
                       inFile == NULL ? "NULL " : "",
                       inFile != NULL ? safe_fileno(inFile->cFile) : 0););
    cInFile = inFile->cFile;
    if (!read_buffer_empty(cInFile)) {
      inputReady = TRUE;
    } else {
      file_no = fileno(cInFile);
      if (unlikely(file_no == -1)) {
        logError(printf("filInputReady(%d): fileno(%d) failed:\n"
                        "errno=%d\nerror: %s\n",
                        safe_fileno(cInFile), safe_fileno(cInFile),
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
                          safe_fileno(cInFile), safe_fileno(cInFile),
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
                       safe_fileno(cInFile), inputReady););
    return inputReady;
  } /* filInputReady */

#else



boolType filInputReady (fileType inFile)

  {
    cFileType cInFile;
    int file_no;
    int flags;
    int ch;
    int saved_errno;
    boolType inputReady;

  /* filInputReady */
    logFunction(printf("filInputReady(%s%d)\n",
                       inFile == NULL ? "NULL " : "",
                       inFile != NULL ? safe_fileno(inFile->cFile) : 0););
    cInFile = inFile->cFile;
    file_no = fileno(cInFile);
    if (unlikely(file_no == -1)) {
      logError(printf("filInputReady(%d): fileno(%d) failed:\n"
                      "errno=%d\nerror: %s\n",
                      safe_fileno(cInFile), safe_fileno(cInFile),
                      errno, strerror(errno)););
      raise_error(FILE_ERROR);
      inputReady = FALSE;
    } else {
      /* printf("file_no=%d\n", file_no); */
      flags = fcntl(file_no, F_GETFL);
      fcntl(file_no, F_SETFL, flags|O_NONBLOCK);
      ch = getc(cInFile);
      saved_errno = errno;
      /* printf("errno=%d ", saved_errno); */
      if (ch == EOF) {
        if (feof(cInFile)) {
          clearerr(cInFile);
          inputReady = TRUE;
        } else if (saved_errno == EAGAIN || saved_errno == EIO) {
          inputReady = FALSE;
        } else {
          inputReady = TRUE;
        } /* if */
      } else {
        ungetc(ch, cInFile);
        inputReady = TRUE;
      } /* if */
      fcntl(file_no, F_SETFL, flags);
    } /* if */
    logFunction(printf("filInputReady(%d) --> %d\n",
                       safe_fileno(cInFile), inputReady););
    return inputReady;
  } /* filInputReady */
#endif



#ifdef OUT_OF_ORDER
boolType filInputReady (fileType inFile)

  {
    cFileType cInFile;
    int file_no;
    int nfds;
    fd_set readfds;
    struct timeval timeout;
    int select_result;
    boolType inputReady;

  /* filInputReady */
    logFunction(printf("filInputReady(%s%d)\n",
                       inFile == NULL ? "NULL " : "",
                       inFile != NULL ? safe_fileno(inFile->cFile) : 0););
    cInFile = inFile->cFile;
    file_no = fileno(cInFile);
    if (unlikely(file_no == -1)) {
      logError(printf("filInputReady(%d): fileno(%d) failed:\n"
                      "errno=%d\nerror: %s\n",
                      safe_fileno(cInFile), safe_fileno(cInFile),
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
                        safe_fileno(cInFile), nfds, sock,
                        errno, strerror(errno)););
        raise_error(FILE_ERROR);
        inputReady = FALSE;
      } else {
        inputReady = FD_ISSET(file_no, &readfds);
      } /* if */
    } /* if */
    logFunction(printf("filInputReady(%d) --> %d\n",
                       safe_fileno(cInFile), inputReady););
    return inputReady;
  } /* filInputReady */
#endif



void filPipe (fileType *inFile, fileType *outFile)

  {
    fileType pipeInFile = NULL;
    fileType pipeOutFile = NULL;
    int aPipe[2];
    cFileType cPipeInFile;
    cFileType cPipeOutFile;
    errInfoType err_info = OKAY_NO_ERROR;

  /* filPipe */
    if (unlikely(!ALLOC_RECORD(pipeInFile, fileRecord, count.files) ||
                 !ALLOC_RECORD(pipeOutFile, fileRecord, count.files))) {
      err_info = MEMORY_ERROR;
    } else if (unlikely(pipe(aPipe) != 0)) {
      logError(printf("filPipe: pipe(aPipe) failed:\n"
                      "errno=%d\nerror: %s\n",
                      errno, strerror(errno)););
      err_info = FILE_ERROR;
    } else {
      if (unlikely((cPipeInFile = fdopen(aPipe[0], "rb")) == NULL)) {
        logError(printf("filPipe: fdopen(%d, \"rb\") failed:\n"
                      "errno=%d\nerror: %s\n",
                      aPipe[0], errno, strerror(errno)););
        close(aPipe[0]);
        close(aPipe[1]);
        err_info = FILE_ERROR;
      } else if (unlikely((cPipeOutFile = fdopen(aPipe[1], "wb")) == NULL)) {
        logError(printf("filPipe: fdopen(%d, \"wb\") failed:\n"
                      "errno=%d\nerror: %s\n",
                      aPipe[1], errno, strerror(errno)););
        fclose(cPipeInFile);
        close(aPipe[1]);
        err_info = FILE_ERROR;
      } else {
        filDestr(*inFile);
        initFileType(pipeInFile, 1);
        pipeInFile->cFile = cPipeInFile;
        *inFile = pipeInFile;
        filDestr(*outFile);
        initFileType(pipeOutFile, 1);
        pipeOutFile->cFile = cPipeOutFile;
        *outFile = pipeOutFile;
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



#ifdef DETERMINE_OS_PROPERTIES_AT_RUNTIME
EMSCRIPTEN_KEEPALIVE void setOsProperties (char *nullDeviceName, unsigned char *pathDelimiter,
                                           int useDriveLetters, int envCaseInsensitive)

  { /* setOsProperties */
    logFunction(printf("setOsProperties(\"%s\")\n", ););
    if ((nullDevice = malloc(strlen(nullDeviceName) + NULL_TERMINATION_LEN)) != NULL) {
      strcpy(nullDevice, nullDeviceName);
    } /* if */
    shellPathDelimiter = pathDelimiter[0];
    shellUsesDriveLetters = useDriveLetters;
#ifdef EMULATE_ENVIRONMENT
    if (envCaseInsensitive) {
      environmentStrncmp = strncasecmp;
    } else {
      environmentStrncmp = strncmp;
    } /* if */
#endif
  } /* setOsProperties */
#endif



#ifdef EMULATE_NODE_ENVIRONMENT
EMSCRIPTEN_KEEPALIVE void setEnvironmentVar (char *key, char *value)

  { /* setEnvironmentVar */
    logFunction(printf("setEnvironmentVar(\"%s\", \"%s\")\n", key, value););
    setenv7(key, value, 1);
  } /* setEnvironmentVar */
#endif



void setupFiles (void)

  { /* setupFiles */
    logFunction(printf("setupFiles\n"););
    stdinFileRecord.cFile = stdin;
    stdoutFileRecord.cFile = stdout;
    stderrFileRecord.cFile = stderr;
#ifdef MOUNT_NODEFS
    EM_ASM(
      let bslash = String.fromCharCode(92);
      let setEnvironmentVar = Module.cwrap('setEnvironmentVar', 'number', ['string', 'string']);
      let setOsProperties = Module.cwrap('setOsProperties', 'number', ['string', 'string', 'number']);
      if (typeof require === 'function') {
        let fs;
        let os;
        try {
          fs = require('fs');
          os = require('os');
        } catch (e) {
          fs = null;
          os = null;
        }
        if (fs !== null) {
          let statData;
          if (os.platform() === 'win32') {
            for (let drive = 0; drive < 26; drive++) {
              let ch = String.fromCharCode('a'.charCodeAt(0) + drive);
              try {
                statData = fs.statSync(ch + ':/');
                // console.log('drive: ' + ch + ' exists');
                if (statData.isDirectory()) {
                  try {
                    statData = FS.stat('/' + ch);
                  } catch (e) {
                    // console.log('mkdir: ' + '/' + ch);
                    FS.mkdir('/' + ch);
                  }
                  FS.mount(NODEFS, { root: ch + ':/' }, '/' + ch);
                }
              } catch (e) {
              }
            }
          } else {
            let files = fs.readdirSync('/');
            for (let idx in files) {
              // console.log('file: ' + files[idx]);
              if (fs.statSync('/' + files[idx]).isDirectory()) {
                try {
                  statData = FS.stat('/' + files[idx]);
                } catch (e) {
                  // console.log('mkdir: ' + '/' + files[idx]);
                  FS.mkdir('/' + files[idx]);
                }
                FS.mount(NODEFS, { root: '/' + files[idx] }, '/' + files[idx]);
              }
            }
          }
          let workDir = process.cwd().replace(new RegExp(bslash + bslash, 'g'), '/');
          if (workDir.charAt(1) === ':' && workDir.charAt(2) === '/') {
            workDir = '/' + workDir.charAt(0).toLowerCase() + workDir.substring(2);
          }
          // console.log('workDir: ' + workDir);
          FS.chdir(workDir);
        }
#ifdef DETERMINE_OS_PROPERTIES_AT_RUNTIME
        // Setup nullDevice, shellPathDelimiter and shellUsesDriveLetters variables.
        if (process.platform === "win32") {
          setOsProperties('NUL:', bslash, 1, 1);
        } else {
          setOsProperties('/dev/null', '/', 0, 0);
        }
#endif
#ifdef EMULATE_NODE_ENVIRONMENT
        // Setup environment
        Object.keys(process.env).forEach(function(key) {
          setEnvironmentVar(key, process.env[key]);
        });
#endif
      } else {
        let scripts = document.getElementsByTagName('script');
        let index = scripts.length - 1;
        let myScript = scripts[index];
        // console.log('myScript.src is ' + myScript.src);
        let src = myScript.src;
        let n = src.search(bslash + '?');
        let queryString = "";
        if (n !== -1) {
          queryString = myScript.src.substring(n + 1).replace('+', '%2B');
        }
#ifdef DETERMINE_OS_PROPERTIES_AT_RUNTIME
        setOsProperties('/dev/null', '/', 0, 0);
#endif
        setEnvironmentVar('QUERY_STRING', queryString);
        setEnvironmentVar('HOME', '/home/web_user');
        // console.log('queryString is ' + queryString);
      }
    );
#endif
    logFunction(printf("setupFiles -->\n"););
  } /* setupFiles */

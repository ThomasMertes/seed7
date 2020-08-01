/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000, 2014, 2016, 2017  Thomas Mertes      */
/*                                                                  */
/*  This program is free software; you can redistribute it and/or   */
/*  modify it under the terms of the GNU General Public License as  */
/*  published by the Free Software Foundation; either version 2 of  */
/*  the License, or (at your option) any later version.             */
/*                                                                  */
/*  This program is distributed in the hope that it will be useful, */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   */
/*  GNU General Public License for more details.                    */
/*                                                                  */
/*  You should have received a copy of the GNU General Public       */
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: General                                                 */
/*  File: seed7/src/sigutl.c                                        */
/*  Changes: 1993, 1994  Thomas Mertes                              */
/*  Content: Driver shutdown and signal handling.                   */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdio.h"
#include "stdlib.h"
#include "signal.h"
#include "sys/types.h"

#include "common.h"
#include "fil_drv.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "sigutl.h"


typedef void (*signalHandlerType) (int signalNum);

#if HAS_SIGACTION || HAS_SIGNAL
static const int normalSignals[] = {SIGABRT, SIGILL, SIGINT, SIGFPE};
volatile static suspendInterprType suspendInterpreter;
#endif



void shutDrivers (void)

  { /* shutDrivers */
    logFunction(printf("shutDrivers\n"););
    /* The actual shut functionality is now via atexit(). */
    fflush(NULL);
    logFunction(printf("shutDrivers -->\n"););
  } /* shutDrivers */



const_cstriType signalName (int signalNum)

  {
    static char buffer[20];
    const_cstriType sigName;

  /* signalName */
    logFunction(printf("signalName(%d)\n", signalNum););
    switch (signalNum) {
      case SIGABRT: sigName = "SIGABRT"; break;
      case SIGFPE:  sigName = "SIGFPE";  break;
      case SIGILL:  sigName = "SIGILL";  break;
      case SIGINT:  sigName = "SIGINT";  break;
      case SIGSEGV: sigName = "SIGSEGV"; break;
      case SIGTERM: sigName = "SIGTERM"; break;
#ifdef SIGALRM
      case SIGALRM: sigName = "SIGALRM"; break;
#endif
#ifdef SIGPIPE
      case SIGPIPE: sigName = "SIGPIPE"; break;
#endif
      default:
        sprintf(buffer, "%d", signalNum);
        sigName = buffer;
        break;
    } /* switch */
    logFunction(printf("signalName(%d) --> \"%s\"\n",
                       signalNum, sigName););
    return sigName;
  } /* signalName */



void triggerSigfpe (void)

  {
    int number;

  /* triggerSigfpe */
    signal(SIGFPE, SIG_DFL);
    number = 0;
#ifdef DO_SIGFPE_WITH_DIV_BY_ZERO
    printf("%d", 1 / number); /* trigger SIGFPE on purpose */
#else
    raise(SIGFPE);
#endif
    printf("\n*** Continue after SIGFPE.\n");
  } /* triggerSigfpe */



static boolType signalDecision (int signalNum, boolType inHandler)

  {
    int ch;
    boolType sigintReceived;
    int position;
    char buffer[10];
    long unsigned int exceptionNum;
    boolType resume = FALSE;

  /* signalDecision */
    logFunction(printf("signalDecision(%d, %d)\n",
                       signalNum, inHandler););
    printf("\n*** SIGNAL %s RAISED\n"
           "\n*** The following commands are possible:\n"
           "  RETURN  Continue\n"
           "  *       Terminate\n"
           "  /       Trigger SIGFPE\n"
           "  !n      Raise exception with number (e.g.: !1 raises MEMORY_ERROR)\n",
           signalName(signalNum));
    if (suspendInterpreter != NULL) {
      printf("  c       Suspend the program\n");
    } /* if */
    if (inHandler) {
      do {
        ch = fgetc(stdin);
      } while (ch == ' ');
    } else {
      do {
        ch = readCharChkCtrlC(stdin, &sigintReceived);
      } while (sigintReceived || ch == ' ');
    } /* if */
    if (ch == '*') {
      shutDrivers();
      exit(1);
    } else if (ch == '/') {
      triggerSigfpe();
    } else if (suspendInterpreter != NULL && ch == 'c') {
      suspendInterpreter(signalNum);
    } else if (ch == '!') {
      position = 0;
      while ((ch = fgetc(stdin)) >= (int) ' ' && ch <= (int) '~' && position < 4) {
        buffer[position] = (char) ch;
        position++;
      } /* while */
      buffer[position] = '\0';
      if (position > 0 && buffer[0] >= '0' && buffer[0] <= '9') {
        exceptionNum = strtoul(buffer, NULL, 10);
        raise_error((int) exceptionNum);
      } /* if */
    } else {
      resume = TRUE;
    } /* if */
    while (ch != EOF && ch != '\n') {
      ch = fgetc(stdin);
    } /* while */
    return resume;
  } /* signalDecision */



#if HAS_SIGACTION || HAS_SIGNAL
static void handleTracedSignals (int signalNum)

  { /* handleTracedSignals */
#if defined SIGALRM && !HAS_SIGACTION
    signal(SIGALRM, SIG_IGN);
#endif
#ifdef DIALOG_IN_SIGNAL_HANDLER
    (void) signalDecision(signalNum, TRUE);
#else
    if (suspendInterpreter != NULL) {
      suspendInterpreter(signalNum);
    } /* if */
#endif
#if SIGNAL_RESETS_HANDLER
    signal(signalNum, handleTracedSignals);
#endif
  } /* handleTracedSignals */



static void handleNumericError (int signalNum)

  { /* handleNumericError */
#if SIGNAL_RESETS_HANDLER
    signal(signalNum, handleNumericError);
#endif
    raise_error(NUMERIC_ERROR);
  } /* handleNumericError */



#if OVERFLOW_SIGNAL
static void handleOverflowError (int signalNum)

  {
#if SIGNAL_RESETS_HANDLER
    signal(signalNum, handleOverflowError);
#endif
    raise_error(OVERFLOW_ERROR);
  }
#endif



static void handleTermSignal (int signalNum)

  { /* handleTermSignal */
    printf("\n*** SIGNAL %s RAISED\n", signalName(signalNum));
    printf("\n*** Program terminated.\n");
    shutDrivers();
    exit(1);
  } /* handleTermSignal */



static void handleSegvSignal (int signalNum)

  { /* handleSegvSignal */
    shutDrivers();
    printf("\n*** SIGNAL SEGV RAISED\n"
           "\n*** Program terminated.\n");
#if HAS_SIGACTION
    {
      struct sigaction sigAct;
      sigemptyset(&sigAct.sa_mask);
      sigAct.sa_flags = SA_RESTART;
      sigAct.sa_handler = SIG_DFL;
      sigaction(SIGABRT, &sigAct, NULL);
    }
#elif HAS_SIGNAL
    signal(SIGABRT, SIG_DFL);
#endif
    abort();
  } /* handleSegvSignal */



#if HAS_SIGACTION
void setupSignalHandlers (boolType handleSignals,
    boolType traceSignals, boolType overflowSigError,
    boolType fpeNumericError, suspendInterprType suspendInterpr)

  {
    int pos;
    int signalNum;
    struct sigaction sigAct;
    boolType okay = TRUE;

  /* setupSignalHandlers */
    logFunction(printf("setupSignalHandlers(%d, %d, %d, %d, " FMT_U_MEM ")\n",
                       handleSignals, traceSignals, overflowSigError,
                       fpeNumericError, (memSizeType) suspendInterpr););
    suspendInterpreter = suspendInterpr;
    if (handleSignals) {
      sigemptyset(&sigAct.sa_mask);
#ifdef SIGALRM
      sigaddset(&sigAct.sa_mask, SIGALRM);
#endif
      sigAct.sa_flags = SA_RESTART;
      for (pos = 0; pos < sizeof(normalSignals) / sizeof(int); pos++) {
        signalNum = normalSignals[pos];
#if OVERFLOW_SIGNAL
        if (signalNum == OVERFLOW_SIGNAL && overflowSigError) {
          sigAct.sa_handler = handleOverflowError;
        } else
#endif
        if (signalNum == SIGFPE && fpeNumericError) {
          sigAct.sa_handler = handleNumericError;
        } else if (traceSignals) {
          sigAct.sa_handler = handleTracedSignals;
        } else {
          sigAct.sa_handler = handleTermSignal;
        } /* if */
        okay = okay && sigaction(signalNum, &sigAct, NULL) == 0;
      } /* for */
      sigAct.sa_handler = handleTermSignal;
      okay = okay && sigaction(SIGTERM,  &sigAct, NULL) == 0;
      if (traceSignals) {
        sigAct.sa_handler = handleSegvSignal;
      } else {
        sigAct.sa_handler = SIG_DFL;
      } /* if */
      okay = okay && sigaction(SIGSEGV, &sigAct, NULL) == 0;
#ifdef SIGPIPE
      sigAct.sa_handler = SIG_IGN;
      okay = okay && sigaction(SIGPIPE, &sigAct, NULL) == 0;
#endif
    } /* if */
    if (!okay) {
      printf("\n*** Activating signal handlers failed.\n");
    } /* if */
    logFunction(printf("setupSignalHandlers -->\n"););
  } /* setupSignalHandlers */

#elif HAS_SIGNAL



void setupSignalHandlers (boolType handleSignals,
    boolType traceSignals, boolType overflowSigError,
    boolType fpeNumericError, suspendInterprType suspendInterpr)

  {
    int pos;
    int signalNum;
    signalHandlerType sigHandler;
    boolType okay = TRUE;

  /* setupSignalHandlers */
    logFunction(printf("setupSignalHandlers(%d, %d, %d, %d, " FMT_U_MEM ")\n",
                       handleSignals, traceSignals, overflowSigError,
                       fpeNumericError, (memSizeType) suspendInterpr););
    suspendInterpreter = suspendInterpr;
    if (handleSignals) {
      for (pos = 0; pos < sizeof(normalSignals) / sizeof(int); pos++) {
        signalNum = normalSignals[pos];
#if OVERFLOW_SIGNAL
        if (signalNum == OVERFLOW_SIGNAL && overflowSigError) {
          sigHandler = handleOverflowError;
        } else
#endif
        if (signalNum == SIGFPE && fpeNumericError) {
          sigHandler = handleNumericError;
        } else if (traceSignals) {
          sigHandler = handleTracedSignals;
        } else {
          sigHandler = handleTermSignal;
        } /* if */
        okay = okay && signal(signalNum, sigHandler) != SIG_ERR;
      } /* for */
      okay = okay && signal(SIGTERM, handleTermSignal) != SIG_ERR;
      if (traceSignals) {
        sigHandler = handleSegvSignal;
      } else {
        sigHandler = SIG_DFL;
      } /* if */
      okay = okay && signal(SIGSEGV, sigHandler) != SIG_ERR;
#ifdef SIGPIPE
      signal(SIGPIPE, SIG_IGN);
#endif
    } /* if */
    if (!okay) {
      printf("\n*** Activating signal handlers failed.\n");
    } /* if */
    logFunction(printf("setupSignalHandlers -->\n"););
  } /* setupSignalHandlers */

#else



void setupSignalHandlers (boolType handleSignals,
    boolType traceSignals, boolType overflowSigError,
    boolType fpeNumericError, suspendInterprType suspendInterpr)

  { /* setupSignalHandlers */
    logFunction(printf("setupSignalHandlers(%d, %d, %d, %d, " FMT_U_MEM ")\n",
                       handleSignals, traceSignals, overflowSigError,
                       fpeNumericError, (memSizeType) suspendInterpr););
    logFunction(printf("setupSignalHandlers -->\n"););
  } /* setupSignalHandlers */
#endif
#endif



static signalHandlerType getCurrentSignalHandler (int signalNum)

  {
#if HAS_SIGACTION
    struct sigaction oldAction;
#endif
    signalHandlerType currentHandler;

  /* getCurrentSignalHandler */
    logFunction(printf("getCurrentSignalHandler(%d)\n", signalNum););
#if HAS_SIGACTION
    if (likely(sigaction(signalNum, NULL, &oldAction) == 0)) {
      currentHandler = oldAction.sa_handler;
#elif HAS_SIGNAL
    currentHandler = signal(signalNum, SIG_IGN);
    if (likely(currentHandler != SIG_ERR) &&
               signal(signalNum, currentHandler) != SIG_ERR) {
#endif
#if HAS_SIGACTION || HAS_SIGNAL
    } else {
      logError(printf("callSignalHandler(%d) failed.\n", signalNum););
      currentHandler = SIG_ERR;
    } /* if */
#else
    currentHandler = SIG_ERR;
#endif
    logFunction(printf("getCurrentSignalHandler(%d) --> " FMT_U_MEM "\n",
                       signalNum, (memSizeType) currentHandler););
    return currentHandler;
  } /* getCurrentSignalHandler */



boolType callSignalHandler (int signalNum)

  {
    signalHandlerType currentHandler;
    boolType resume = FALSE;

  /* callSignalHandler */
    logFunction(printf("callSignalHandler(%d)\n", signalNum););
    currentHandler = getCurrentSignalHandler(signalNum);
    if (currentHandler == handleTracedSignals) {
      resume = signalDecision(signalNum, FALSE);
    } else if (currentHandler == SIG_DFL) {
      raise(signalNum);
    } else if (currentHandler != SIG_IGN && currentHandler != SIG_ERR) {
      currentHandler(signalNum);
    } /* if */
    return resume;
  } /* callSignalHandler */

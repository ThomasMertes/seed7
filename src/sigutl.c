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
#include "os_decls.h"
#include "heaputl.h"
#include "fil_drv.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "sigutl.h"


typedef void (*signalHandlerType) (int signalNum);

#if HAS_SIGACTION || HAS_SIGNAL
static const int normalSignals[] = {SIGABRT, SIGILL, SIGINT, SIGFPE};
#endif
volatile static suspendInterprType suspendInterpreter;



void shutDrivers (void)

  { /* shutDrivers */
    logSignalFunction(printf("shutDrivers\n"););
    /* The actual shut functionality is now via atexit(). */
    fflush(NULL);
    logSignalFunction(printf("shutDrivers -->\n"););
  } /* shutDrivers */



/**
 *  Determine the name of the given signal 'signalNum'.
 *  @param signalNum Number of the signal.
 *  @return the name of the signal.
 */
const_cstriType signalName (int signalNum)

  {
    static char buffer[INT_DECIMAL_SIZE + NULL_TERMINATION_LEN];
    const_cstriType sigName;

  /* signalName */
    logSignalFunction(printf("signalName(%d)\n", signalNum););
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
    logSignalFunction(printf("signalName(%d) --> \"%s\"\n",
                             signalNum, sigName););
    return sigName;
  } /* signalName */



/**
 *  Trigger the signal SIGFPE such that a debugger can catch it.
 *  The compiler option -e causes that SIGFPE is raised with
 *  triggerSigFpe(), if an uncaught exception occurs. This way a
 *  debugger can handle uncaught Seed7 exceptions.
 */
void triggerSigfpe (void)

  {
#if DO_SIGFPE_WITH_DIV_BY_ZERO
    int number;
#endif

  /* triggerSigfpe */
#if HAS_SIGACTION
    {
      struct sigaction sig_act;
      sigemptyset(&sig_act.sa_mask);
      sig_act.sa_flags = SA_RESTART;
      sig_act.sa_handler = SIG_DFL;
      sigaction(SIGFPE, &sig_act, NULL);
    }
#elif HAS_SIGNAL
    signal(SIGFPE, SIG_DFL);
#endif
#if DO_SIGFPE_WITH_DIV_BY_ZERO
    number = 0;
    /* Under Windows it is necessary to trigger SIGFPE    */
    /* this way to assure that the debugger can catch it. */
    printf("%d", 1 / number); /* trigger SIGFPE on purpose */
#else
    raise(SIGFPE);
#endif
    printf("\n*** Continue after SIGFPE.\n");
  } /* triggerSigfpe */



/**
 *  Dialog to decide how to continue after a signal has been received.
 *  This function might be called from a signal handler. Note that this
 *  function does things that are beyond the specification of what
 *  signal handlers are allowed to do. Signal handlers should be left
 *  quickly and several functions (including I/O functions) should never
 *  be called from them. Waiting in a signal handler for the user to
 *  respond violates that. Besides that this function works under Linux
 *  BSD and Windows with various run-time libraries.
 */
static boolType signalDecision (int signalNum, boolType inHandler)

  {
    int ch;
    boolType sigintReceived;
    unsigned int position;
    char buffer[ULONG_DECIMAL_SIZE + NULL_TERMINATION_LEN];
    long unsigned int exceptionNum;
    boolType resume = FALSE;

  /* signalDecision */
    logSignalFunction(printf("signalDecision(%d, %d)\n",
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
      os_exit(1);
    } else if (ch == '/') {
      triggerSigfpe();
    } else if (suspendInterpreter != NULL && ch == 'c') {
      suspendInterpreter(signalNum);
    } else if (ch == '!') {
      position = 0;
      while ((ch = fgetc(stdin)) >= (int) ' ' && ch <= (int) '~') {
        if (position < sizeof(buffer) - 1) {
          buffer[position] = (char) ch;
          position++;
        } /* if */
      } /* while */
      buffer[position] = '\0';
      if (position > 0 && buffer[0] >= '0' && buffer[0] <= '9') {
        exceptionNum = strtoul(buffer, NULL, 10);
        if (exceptionNum > OKAY_NO_ERROR &&
            exceptionNum <= ACTION_ERROR && signalNum != SIGSEGV) {
          raise_error((int) exceptionNum);
        } /* if */
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
/**
 *  Tracing signal handler for normalSignals.
 *  Tracing signals is activated in interpreter and compiler with the
 *  option -ts. This signal handler is used for normalSignals
 *  (e.g.: SIGABRT, SIGILL, SIGINT, SIGFPE).
 */
static void handleTracedSignals (int signalNum)

  { /* handleTracedSignals */
#if defined SIGALRM && !HAS_SIGACTION
    signal(SIGALRM, SIG_IGN);
#endif
#if DIALOG_IN_SIGNAL_HANDLER
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



/**
 *  Signal handler for the signal SIGFPE.
 */
static void handleNumericError (int signalNum)

  { /* handleNumericError */
#if SIGNAL_RESETS_HANDLER
    signal(signalNum, handleNumericError);
#endif
    raise_error(NUMERIC_ERROR);
  } /* handleNumericError */



#if OVERFLOW_SIGNAL
/**
 *  Signal handler for the OVERFLOW_SIGNAL (SIGILL, SIGABRT or SIGTRAP).
 */
static void handleOverflowError (int signalNum)

  {
#if SIGNAL_RESETS_HANDLER
    signal(signalNum, handleOverflowError);
#endif
    raise_error(OVERFLOW_ERROR);
  }
#endif



/**
 *  Signal handler for signals that terminate the program.
 *  This signal handler is used for SIGTERM and for normalSignals
 *  (e.g.: SIGABRT, SIGILL, SIGINT, SIGFPE).
 */
static void handleTermSignal (int signalNum)

  { /* handleTermSignal */
    printf("\n*** SIGNAL %s RAISED\n"
           "\n*** Program terminated.\n", signalName(signalNum));
    shutDrivers();
    os_exit(1);
  } /* handleTermSignal */



/**
 *  Tracing signal handler for the signal SIGSEGV.
 *  Tracing signals is activated in interpreter and
 *  compiler with the option -ts.
 */
static void handleTracedSegvSignal (int signalNum)

  { /* handleTracedSegvSignal */
#if defined SIGALRM && !HAS_SIGACTION
    signal(SIGALRM, SIG_IGN);
#endif
#if DIALOG_IN_SIGNAL_HANDLER
    (void) signalDecision(signalNum, TRUE);
#else
    if (suspendInterpreter != NULL) {
      suspendInterpreter(signalNum);
    } /* if */
#endif
#if HAS_SIGALTSTACK
#if SIGNAL_RESETS_HANDLER
    signal(signalNum, handleTracedSegvSignal);
#endif
    no_memory(SOURCE_POSITION(3011));
#else
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
#endif
  } /* handleTracedSegvSignal */



/**
 *  Signal handler for the signal SIGSEGV.
 */
static void handleSegvSignal (int signalNum)

  { /* handleSegvSignal */
#if HAS_SIGALTSTACK
#if SIGNAL_RESETS_HANDLER
    signal(signalNum, handleSegvSignal);
#endif
    no_memory(SOURCE_POSITION(3011));
#else
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
#endif
  } /* handleSegvSignal */



/**
 *  Initialize the signal handlers.
 *  @param handleSignals Specifies if signals should be handled at all.
 *  @param traceSignals Specifies if signals should trigger a dialog at
 *                      the console.
 *  @param overflowSigError Specifies if an OVERFLOW_SIGNAL should
 *                          raise OVERFLOW_ERROR.
 *  @param fpeNumericError Specifies if SIGFPE should raise NUMERIC_ERROR.
 */
#if HAS_SIGACTION
void setupSignalHandlers (boolType handleSignals,
    boolType traceSignals, boolType overflowSigError,
    boolType fpeNumericError, suspendInterprType suspendInterpr)

  {
    unsigned int pos;
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
#if OVERFLOW_SIGNAL && defined SIGTRAP && OVERFLOW_SIGNAL == SIGTRAP
      if (overflowSigError) {
        sigAct.sa_handler = handleOverflowError;
        okay = okay && sigaction(SIGTRAP, &sigAct, NULL) == 0;
      } else if (traceSignals) {
        sigAct.sa_handler = handleTracedSignals;
        okay = okay && sigaction(SIGTRAP, &sigAct, NULL) == 0;
      } /* if */
#endif
      sigAct.sa_handler = handleTermSignal;
      okay = okay && sigaction(SIGTERM,  &sigAct, NULL) == 0;
      sigAct.sa_flags = SA_ONSTACK;
      if (traceSignals) {
        sigAct.sa_handler = handleTracedSegvSignal;
      } else {
        sigAct.sa_handler = handleSegvSignal;
      } /* if */
      okay = okay && sigaction(SIGSEGV, &sigAct, NULL) == 0;
#ifdef SIGPIPE
      sigAct.sa_flags = SA_RESTART;
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
#if OVERFLOW_SIGNAL && defined SIGTRAP && OVERFLOW_SIGNAL == SIGTRAP
      if (overflowSigError) {
        okay = okay && signal(SIGTRAP, handleOverflowError) != SIG_ERR;
      } else if (traceSignals) {
        okay = okay && signal(SIGTRAP, handleTracedSignals) != SIG_ERR;
      } /* if */
#endif
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

#endif
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



/**
 *  Determine the current signal handler of the given signal 'signalNum'.
 *  @param signalNum Number of the signal.
 *  @return signal handler that corresponds to 'signalNum', or
 *          SIG_ERR if no signal handler could be found.
 */
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
      logError(printf("getCurrentSignalHandler(%d) failed.\n", signalNum););
      currentHandler = SIG_ERR;
    } /* if */
#else
    currentHandler = SIG_ERR;
#endif
    logFunction(printf("getCurrentSignalHandler(%d) --> " FMT_U_MEM "\n",
                       signalNum, (memSizeType) currentHandler););
    return currentHandler;
  } /* getCurrentSignalHandler */



/**
 *  Call the current signal handler without raising the signal.
 *  This function is called if ctrl-c has been pressed.
 *  If signal tracing is switched on signalDecision() is called
 *  to determine if the program should resume.
 *  @param signalNum Number of the signal.
 *  @return TRUE if the program should resume, or
 *          FALSE if the program should not resume.
 */
boolType callSignalHandler (int signalNum)

  {
    signalHandlerType currentHandler;
    boolType resume = FALSE;

  /* callSignalHandler */
    logFunction(printf("callSignalHandler(%d)\n", signalNum););
    currentHandler = getCurrentSignalHandler(signalNum);
#if HAS_SIGACTION || HAS_SIGNAL
    if (currentHandler == handleTracedSignals) {
      resume = signalDecision(signalNum, FALSE);
    } else
#endif
    if (currentHandler == SIG_DFL) {
      raise(signalNum);
    } else if (currentHandler != SIG_IGN && currentHandler != SIG_ERR) {
      currentHandler(signalNum);
    } /* if */
    return resume;
  } /* callSignalHandler */

/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000  Thomas Mertes                        */
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

#include "common.h"
#include "kbd_drv.h"
#include "con_drv.h"

#undef EXTERN
#define EXTERN
#include "sigutl.h"


#if HAS_SIGACTION || HAS_SIGNAL
volatile boolType trace_signals = FALSE;

static void activate_signal_handlers (void);
#endif



void shut_drivers (void)

  { /* shut_drivers */
    logFunction(printf("shut_drivers\n"););
    kbdShut();
    conShut();
    fflush(NULL);
    logFunction(printf("shut_drivers -->\n"););
  } /* shut_drivers */



const_cstriType signal_name (int sig_num)

  {
    static char buffer[20];
    const_cstriType sig_name;

  /* signal_name */
    logFunction(printf("signal_name(%d)\n", sig_num););
    if (sig_num == SIGABRT) {
      sig_name = "ABORT";
    } else if (sig_num == SIGFPE) {
      sig_name = "FPE";
    } else if (sig_num == SIGILL) {
      sig_name = "ILL";
    } else if (sig_num == SIGINT) {
      sig_name = "INTR";
    } else if (sig_num == SIGSEGV) {
      sig_name = "SEGV";
    } else if (sig_num == SIGTERM) {
      sig_name = "TERM";
#ifdef SIGALRM
    } else if (sig_num == SIGALRM) {
      sig_name = "ALARM";
#endif
#ifdef SIGPIPE
    } else if (sig_num == SIGPIPE) {
      sig_name = "SIGPIPE";
#endif
    } else {
      sprintf(buffer, "%d", sig_num);
      sig_name = buffer;
    } /* if */
    logFunction(printf("signal_name(%d) --> \"%s\"\n",
                       sig_num, sig_name););
    return sig_name;
  } /* signal_name */



#if HAS_SIGACTION || HAS_SIGNAL
static void handle_signals (int sig_num)

  {
#ifdef DIALOG_IN_SIGNAL_HANDLER
    int ch;
#endif

  /* handle_signals */
#ifdef SIGALRM
    signal(SIGALRM, SIG_IGN);
#endif
#ifdef DIALOG_IN_SIGNAL_HANDLER
    printf("\n*** SIGNAL %s RAISED\n\n"
           "*** (Type RETURN to continue, '*' to terminate or 'c' to stop)\n",
           signal_name(sig_num));
    ch = fgetc(stdin);
    if (ch == '*') {
      shut_drivers();
      exit(1);
#ifdef CTRL_C_SENDS_EOF
    } else if (ch == 'c' || ch == EOF) {
#else
    } else if (ch == 'c') {
#endif
      interrupt_flag = TRUE;
      signal_number = sig_num;
    } /* if */
    while (ch != EOF && ch != '\n') {
      ch = fgetc(stdin);
    } /* while */
#else
    interrupt_flag = TRUE;
    signal_number = sig_num;
#endif
#if !HAS_SIGACTION && HAS_SIGNAL
    activate_signal_handlers();
#endif
  } /* handle_signals */



static void handle_term_signal (int sig_num)

  { /* handle_term_signal */
    shut_drivers();
    printf("\n*** SIGNAL %s RAISED\n", signal_name(sig_num));
    printf("\n*** Program terminated.\n");
    exit(1);
  } /* handle_term_signal */



static void handle_segv_signal (int sig_num)

  { /* handle_segv_signal */
    shut_drivers();
    printf("\n*** SIGNAL SEGV RAISED\n"
           "\n*** Program terminated.\n");
    signal(SIGABRT, SIG_DFL);
    abort();
  } /* handle_segv_signal */



static void activate_signal_handlers (void)

  { /* activate_signal_handlers */
#if HAS_SIGACTION
    {
      struct sigaction sig_act;
      boolType okay;

      if (trace_signals) {
        sig_act.sa_handler = handle_signals;
      } else {
        sig_act.sa_handler = handle_term_signal;
      } /* if */
      sigemptyset(&sig_act.sa_mask);
      sig_act.sa_flags = SA_RESTART;
      okay = sigaction(SIGABRT,  &sig_act, NULL) == 0 &&
             sigaction(SIGFPE,   &sig_act, NULL) == 0 &&
             sigaction(SIGILL,   &sig_act, NULL) == 0 &&
             sigaction(SIGINT,   &sig_act, NULL) == 0;
      sig_act.sa_handler = handle_term_signal;
      okay = okay &&
             sigaction(SIGTERM,  &sig_act, NULL) == 0;
      if (trace_signals) {
        sig_act.sa_handler = handle_segv_signal;
      } else {
        sig_act.sa_handler = SIG_DFL;
      } /* if */
      okay = okay &&
             sigaction(SIGSEGV,  &sig_act, NULL) == 0;
      if (!okay) {
        printf("\n*** Activating signal handlers failed.\n");
      } /* if */
    }
#elif HAS_SIGNAL
    if (trace_signals) {
      signal(SIGABRT, handle_signals);
      signal(SIGFPE,  handle_signals);
      signal(SIGILL,  handle_signals);
      signal(SIGINT,  handle_signals);
    } else {
      signal(SIGABRT, handle_term_signal);
      signal(SIGFPE,  handle_term_signal);
      signal(SIGILL,  handle_term_signal);
      signal(SIGINT,  handle_term_signal);
    } /* if */
    signal(SIGTERM, handle_term_signal);
    if (trace_signals) {
      signal(SIGSEGV, handle_segv_signal);
    } else {
      signal(SIGSEGV, SIG_DFL);
    } /* if */
#else
#error "Neither sigaction() nor signal() are available."
#endif
#ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN);
#endif
  } /* activate_signal_handlers */
#endif



void setup_signal_handlers (boolType do_handle_signals, boolType do_trace_signals)

  { /* setup_signal_handlers */
    logFunction(printf("setup_signal_handlers(%d, %d)\n",
                       do_handle_signals, do_trace_signals););
#if HAS_SIGACTION || HAS_SIGNAL
    trace_signals = do_trace_signals;
    if (do_handle_signals) {
      activate_signal_handlers();
    } /* if */
#endif
    logFunction(printf("setup_signal_handlers -->\n"););
  } /* setup_signal_handlers */

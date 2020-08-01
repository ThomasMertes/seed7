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



void shut_drivers (void)

  { /* shut_drivers */
    kbdShut();
    conShut();
    fflush(NULL);
  } /* shut_drivers */



const_cstriType signal_name (int sig_num)

  {
    static char buffer[20];
    const_cstriType sig_name;

  /* signal_name */
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
    return sig_name;
  } /* signal_name */



#ifdef CATCH_SIGNALS
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
    printf("\n*** SIGNAL %s RAISED\n\n*** (Type RETURN to continue or '*' to terminate)\n",
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
#ifndef HAS_SIGACTION
    activate_signal_handlers();
#endif
  } /* handle_signals */



static void handle_segv_signal (int sig_num)

  { /* handle_segv_signal */
    shut_drivers();
    printf("\n*** SIGNAL SEGV RAISED\n"
           "\n*** Program terminated.\n");
    exit(1);
  } /* handle_segv_signal */



static void handle_term_signal (int sig_num)

  { /* handle_term_signal */
    shut_drivers();
    printf("\n*** SIGNAL %s RAISED\n", signal_name(sig_num));
    printf("\n*** Program terminated.\n");
    exit(1);
  } /* handle_term_signal */



void activate_signal_handlers (void)

  { /* activate_signal_handlers */
#ifdef HAS_SIGACTION
    {
      struct sigaction sig_act;
      boolType okay;

      sig_act.sa_handler = handle_signals;
      sigemptyset(&sig_act.sa_mask);
      sig_act.sa_flags = SA_RESTART;
      okay = sigaction(SIGABRT,  &sig_act, NULL) == 0 &&
             sigaction(SIGFPE,   &sig_act, NULL) == 0 &&
             sigaction(SIGILL,   &sig_act, NULL) == 0 &&
             sigaction(SIGINT,   &sig_act, NULL) == 0;
      sig_act.sa_handler = handle_segv_signal;
      okay = okay &&
             sigaction(SIGSEGV,  &sig_act, NULL) == 0;
      sig_act.sa_handler = handle_term_signal;
      okay = okay &&
             sigaction(SIGTERM,  &sig_act, NULL) == 0;
      if (!okay) {
        printf("\n*** Activating signal handlers failed.\n");
      } /* if */
    }
#else
    signal(SIGABRT, handle_signals);
    signal(SIGFPE,  handle_signals);
    signal(SIGILL,  handle_signals);
    signal(SIGINT,  handle_signals);
    signal(SIGSEGV, handle_segv_signal);
    signal(SIGTERM, handle_term_signal);
#endif
#ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN);
#endif
  } /* activate_signal_handlers */
#endif

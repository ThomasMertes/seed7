/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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



#ifdef ANSI_C

void shut_drivers (void)
#else

void shut_drivers ()
#endif

  { /* shut_drivers */
    kbdShut();
    conShut();
    fflush(NULL);
  } /* shut_drivers */



#ifdef CATCH_SIGNALS
#ifdef ANSI_C

static void handle_signals (int sig_num)
#else

static void handle_signals (sig_num)
int sig_num;
#endif

  {
    int ch;
    const_cstritype sig_name;
    char buffer[20];

  /* handle_signals */
#ifdef SIGALRM
    signal(SIGALRM, SIG_IGN);
#endif
/*  shut_drivers(); */
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
    printf("\n*** SIGNAL %s RAISED\n", sig_name);
    printf("\n*** (Type RETURN to continue or '*' to terminate)\n");
    ch = fgetc(stdin);
    if (ch == '*') {
      shut_drivers();
      exit(1);
    } /* if */
    activate_signal_handlers();
/*  exit(1); */
  } /* handle_signals */



#ifdef ANSI_C

static void handle_term_signal (int sig_num)
#else

static void handle_term_signal (sig_num)
int sig_num;
#endif

  { /* handle_term_signal */
    shut_drivers();
    printf("\n*** SIGNAL TERM RAISED\n");
    printf("\n*** Program terminated\n");
    exit(1);
  } /* handle_term_signal */



#ifdef ANSI_C

void activate_signal_handlers (void)
#else

void activate_signal_handlers ()
#endif

  { /* activate_signal_handlers */
    signal(SIGABRT, handle_signals);
    signal(SIGFPE, handle_signals);
    signal(SIGILL, handle_signals);
    signal(SIGINT, handle_signals);
    signal(SIGSEGV, handle_signals);
    signal(SIGTERM, handle_term_signal);
#ifdef SIGPIPE
    signal(SIGPIPE, handle_signals);
#endif
  } /* activate_signal_handlers */
#endif

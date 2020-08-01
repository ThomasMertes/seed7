/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2009  Thomas Mertes                        */
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
/*  Module: Main                                                    */
/*  File: seed7/src/hi.c                                            */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Main program of the hi Interpreter.                    */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "sigutl.h"
#include "data.h"
#include "infile.h"
#include "syvarutl.h"
#include "identutl.h"
#include "entutl.h"
#include "findid.h"
#include "symbol.h"
#include "analyze.h"
#include "traceutl.h"
#include "exec.h"
#include "option.h"
#include "runerr.h"
#include "level.h"
#include "flt_rtl.h"
#include "scr_drv.h"



#ifdef ANSI_C

int main (int argc, char **argv)
#else

int main (argc, argv)
int argc;
char **argv;
#endif

  {
    progtype currentProg;

  /* main */
#ifdef TRACE_HI
    printf("BEGIN HI\n");
#endif
    set_trace(NULL, -1, NULL);
    options(argc, argv, 1);
    if (option.version_info) {
      printf("HI INTERPRETER Version 4.5.%d  Copyright (c) 1990-2009 Thomas Mertes\n", LEVEL);
    } /* if */
    if (argc == 1) {
      printf("usage: hi [options] sourcefile [parameters]\n");
    } else {
      setupFloat();
/*    printf("source_file_name: \"%s\"\n", option.source_file_name);
      printf("prot_file_name: \"%s\"\n", option.prot_file_name); */
      if (option.source_file_name == NULL) {
        printf("*** Sourcefile missing\n");
      } else {
        currentProg = analyze((const_ustritype) option.source_file_name);
        if (!option.analyze_only && currentProg != NULL &&
            (currentProg->error_count == 0 || option.execute_always)) {
/*        PRIME_OBJECTS(); */
/*        printf("%d%d\n",
              trace.actions,
              trace.check_actions); */
          interpr(currentProg);
        } /* if */
      } /* if */
    } /* if */
    shut_drivers();
    if (fail_flag) {
      printf("\n*** Uncaught EXCEPTION ");
      printobject(fail_value);
      printf(" raised with\n");
      prot_list(fail_expression);
      printf("\n\nStack:\n");
      write_call_stack(fail_stack);
    } /* if */
    /* getchar(); */
#ifdef TRACE_HI
    printf("END HI\n");
#endif
    return(0);
  } /* main */

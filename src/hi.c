/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2011  Thomas Mertes                        */
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
/*  Changes: 1990 - 1994, 2010, 2011  Thomas Mertes                 */
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
#include "data_rtl.h"
#include "infile.h"
#include "heaputl.h"
#include "striutl.h"
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
#include "arr_rtl.h"
#include "cmd_rtl.h"
#include "con_drv.h"

stritype programPath;



#ifdef ANSI_C

void raise_error2 (int exception_num, const_cstritype filename, int line)
#else

void raise_error2 (exception_num, filename, line)
int exception_num;
char *filename;
int line;
#endif

  { /* raise_error2 */
    raise_exception(prog.sys_var[exception_num]);
  } /* raise_error2 */



#ifdef ANSI_C

static void processOptions (rtlArraytype arg_v)
#else

static void processOptions (arg_v)
rtlArraytype arg_v;
#endif

  {
    int position;
    stritype opt;
    cstritype cstri_opt;

  /* processOptions */
#ifdef TRACE_OPTION
    printf("BEGIN processOptions\n");
#endif
    option.source_file_name = NULL;
    option.analyze_only = FALSE;
    option.show_ident_table = FALSE;
    for (position = 0; position < arg_v->max_position; position++) {
      if (option.source_file_name == NULL) {
        opt = arg_v->arr[position].value.strivalue;
        /* prot_stri(opt);
           printf("\n"); */
        if (opt->size >= 2 && opt->mem[0] == '-') {
          switch (opt->mem[1]) {
            case 'a':
              option.analyze_only = TRUE;
              break;
            case 'd':
              if (opt->size >= 3) {
                if (ALLOC_CSTRI(cstri_opt, opt->size - 2)) {
                  stri_compress((ustritype) cstri_opt,
                      &opt->mem[2], opt->size - 2);
                  cstri_opt[opt->size - 2] = '\0';
                  option.comp_trace_level = cstri_opt;
                } else {
                  option.comp_trace_level = "";
                } /* if */
              } else {
                option.comp_trace_level = "a";
              } /* if */
              break;
            case 'h':
            case '?':
              break;
            case 'i':
              option.show_ident_table = TRUE;
              break;
            case 'm':
              option.get_infile_buffer = FALSE;
              break;
            case 'p':
              if (opt->size >= 3) {
                if (ALLOC_CSTRI(cstri_opt, opt->size - 2)) {
                  stri_compress((ustritype) cstri_opt,
                      &opt->mem[2], opt->size - 2);
                  cstri_opt[opt->size - 2] = '\0';
                  option.prot_file_name = cstri_opt;
                } else {
                  option.prot_file_name = "";
                } /* if */
              } else {
                if (position < arg_v->max_position - 1) {
                  position++;
                  opt = arg_v->arr[position].value.strivalue;
                  if (ALLOC_CSTRI(cstri_opt, opt->size)) {
                    stri_compress((ustritype) cstri_opt,
                        opt->mem, opt->size);
                    cstri_opt[opt->size] = '\0';
                    option.prot_file_name = cstri_opt;
                  } else {
                    option.prot_file_name = "";
                  } /* if */
                } /* if */
              } /* if */
              break;
            case 'q':
              option.version_info = FALSE;
              option.compilation_info = FALSE;
              option.linecount_info = FALSE;
              break;
            case 's':
              option.catch_signals = FALSE;
              break;
            case 't':
              if (opt->size >= 3) {
                if (ALLOC_CSTRI(cstri_opt, opt->size - 2)) {
                  stri_compress((ustritype) cstri_opt,
                      &opt->mem[2], opt->size - 2);
                  cstri_opt[opt->size - 2] = '\0';
                  option.exec_trace_level = cstri_opt;
                } else {
                  option.exec_trace_level = "";
                } /* if */
              } else {
                option.exec_trace_level = "a";
              } /* if */
              break;
            case 'v':
              option.compilation_info = TRUE;
              option.linecount_info = TRUE;
              if (opt->size >= 3 && opt->mem[2] == '2') {
                option.incr_message_line = 0;
              } /* if */
              break;
            case 'x':
              option.execute_always = TRUE;
              break;
            default:
              printf("*** Unknown option ");
              prot_stri(opt);
              printf("\n");
              break;
          } /* switch */
        } else {
          option.source_file_name = stri_to_standard_path(opt);
        } /* if */
      } else {
        if (option.argv == NULL) {
          option.argv = (void *) arg_v;
          option.argv_start = (memsizetype) position;
          /* printf("argv_start = %d\n", position); */
        } /* if */
      } /* if */
    } /* for */
    /* printf("%d %d %d\n",
        option.version_info,
        option.compilation_info,
        option.linecount_info); */
#ifdef TRACE_OPTION
    printf("END processOptions\n");
#endif
  } /* processOptions */



#ifdef USE_WMAIN
#ifdef ANSI_C

int wmain (int argc, wchar_t **argv)
#else

int wmain (argc, argv)
int argc;
wchar_t **argv;
#endif
#else
#ifdef ANSI_C

int main (int argc, char **argv)
#else

int main (argc, argv)
int argc;
char **argv;
#endif
#endif

  {
    rtlArraytype arg_v;
    stritype arg_0;
    progtype currentProg;

  /* main */
#ifdef TRACE_HI
    printf("BEGIN HI\n");
#endif
    set_trace(NULL, -1, NULL);
    arg_v = getArgv(argc, argv, &arg_0, &programPath);
    if (arg_v == NULL) {
      printf("\n*** No more memory. Program terminated.\n");
    } else {
      FREE_STRI(arg_0, arg_0->size);
      processOptions(arg_v);
      if (option.version_info) {
        printf("HI INTERPRETER Version 4.5.%d  Copyright (c) 1990-2011 Thomas Mertes\n", LEVEL);
      } /* if */
#ifdef CATCH_SIGNALS
      if (option.catch_signals) {
        activate_signal_handlers();
      } /* if */
#endif
      if (argc == 1) {
        printf("usage: hi [options] sourcefile [parameters]\n");
      } else {
        setupFloat();
        /* printf("source_file_name: \"");
           prot_stri(option.source_file_name);
           printf("\"\n");
           printf("prot_file_name: \"%s\"\n", option.prot_file_name); */
        if (option.source_file_name == NULL) {
          printf("*** Sourcefile missing\n");
        } else {
          currentProg = analyze(option.source_file_name);
          if (!option.analyze_only && currentProg != NULL &&
              (currentProg->error_count == 0 || option.execute_always)) {
            /* PRIME_OBJECTS(); */
            /* printf("%d%d\n",
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
    } /* if */
    /* getchar(); */
#ifdef TRACE_HI
    printf("END HI\n");
#endif
    return(0);
  } /* main */

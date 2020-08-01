/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2012  Thomas Mertes                        */
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

static void writeHelp (void)
#else

static void writeHelp ()
#endif

  { /* writeHelp */
    printf("usage: hi [options] sourcefile [parameters]\n\n");
    printf("Options:\n");
    printf("  -?   Write hi interpreter usage.\n");
    printf("  -a   Analyze only and suppress the execution phase.\n");
    printf("  -dx  Set compile time trace level to x. Where x is a string consisting of:\n");
    printf("         a Trace primitive actions\n");
    printf("         c Do action check\n");
    printf("         d Trace dynamic calls\n");
    printf("         e Trace exceptions and handlers\n");
    printf("         h Trace heap size (in combination with 'a')\n");
    printf("  -d   Equivalent to -da\n");
    printf("  -i   Show the identifier table after the analyzing phase.\n");
    printf("  -m   Use less memory during the analyzing phase.\n");
    printf("       If there is a \"No more memory\" error you can try this option.\n");
    printf("  -q   Compile quiet. Line and file information and compilation\n");
    printf("       statistics are suppressed.\n");
    printf("  -tx  Set runtime trace level to x. Where x is a string consisting of:\n");
    printf("         a Trace primitive actions\n");
    printf("         c Do action check\n");
    printf("         d Trace dynamic calls\n");
    printf("         e Trace exceptions and handlers\n");
    printf("         h Trace heap size (in combination with 'a')\n");
    printf("  -t   Equivalent to -ta\n");
    printf("  -vn  Set verbosity level of analyse phase to n. Where n is one of:\n");
    printf("         0 Compile quiet (equivalent to -q)\n");
    printf("         1 Write just the header with version information (default)\n");
    printf("         2 Write a list of include libraries\n");
    printf("         3 Write line numbers, while analyzing\n");
    printf("  -v   Equivalent to -v2\n");
    printf("  -x   Execute even when the program contains errors.\n\n");
  } /* writeHelp */



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
    int verbosity_level;

  /* processOptions */
#ifdef TRACE_OPTION
    printf("BEGIN processOptions\n");
#endif
    option.source_file_argument = NULL;
    option.analyze_only = FALSE;
    option.show_ident_table = FALSE;
    for (position = 0; position < arg_v->max_position; position++) {
      if (option.source_file_argument == NULL) {
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
              option.write_help = TRUE;
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
              if (opt->size >= 3 && opt->mem[2] >= '0' && opt->mem[2] <= '3') {
                verbosity_level = (int) opt->mem[2] - '0';
              } else {
                verbosity_level = 2;
              } /* if */
              if (verbosity_level <= 1) {
                option.version_info = verbosity_level != 0;
                option.compilation_info = FALSE;
                option.linecount_info = FALSE;
              } else {
                option.compilation_info = TRUE;
                option.linecount_info = TRUE;
                if (verbosity_level == 3) {
                  option.incr_message_line = 0;
                } /* if */
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
          option.source_file_argument = stri_to_standard_path(opt);
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
    stritype programName;
    progtype currentProg;

  /* main */
#ifdef TRACE_HI
    printf("BEGIN HI\n");
#endif
    set_trace(NULL, -1, NULL);
    arg_v = getArgv(argc, argv, &arg_0, &programName, &programPath);
    if (arg_v == NULL) {
      printf("\n*** No more memory. Program terminated.\n");
    } else {
      FREE_STRI(programName, programName->size);
      FREE_STRI(arg_0, arg_0->size);
      processOptions(arg_v);
      if (option.version_info) {
        printf("HI INTERPRETER Version 4.5.%d  Copyright (c) 1990-2012 Thomas Mertes\n", LEVEL);
      } /* if */
#ifdef CATCH_SIGNALS
      if (option.catch_signals) {
        activate_signal_handlers();
      } /* if */
#endif
      if (argc == 1) {
        printf("This is free software; see the source for copying conditions.  There is NO\n");
        printf("warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
        printf("Homepage: http://seed7.sourceforge.net\n\n");
        printf("usage: hi [options] sourcefile [parameters]\n\n");
        printf("Use  hi -?  to get more information about hi.\n\n");
      } else if (option.write_help) {
        writeHelp();
      } else {
        setupFloat();
        /* printf("source_file_argument: \"");
           prot_stri(option.source_file_argument);
           printf("\"\n");
           printf("prot_file_name: \"%s\"\n", option.prot_file_name); */
        if (option.source_file_argument == NULL) {
          printf("*** Sourcefile missing\n");
        } else {
          currentProg = analyze(option.source_file_argument);
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
    return 0;
  } /* main */

/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2013  Thomas Mertes                        */
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
/*  File: seed7/src/s7.c                                            */
/*  Changes: 1990 - 1994, 2010, 2011  Thomas Mertes                 */
/*  Content: Main program of the Seed7 interpreter.                 */
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
#include "prg_comp.h"
#include "traceutl.h"
#include "exec.h"
#include "option.h"
#include "runerr.h"
#include "level.h"
#include "flt_rtl.h"
#include "arr_rtl.h"
#include "cmd_rtl.h"
#include "con_drv.h"

#ifdef USE_WINMAIN
typedef struct {
    int dummy;
  } HINSTANCE__;
typedef HINSTANCE__* HINSTANCE;
#endif

stritype programPath;

#define VERSION_INFO "SEED7 INTERPRETER Version 5.0.%d  Copyright (c) 1990-2013 Thomas Mertes\n"



#ifdef ANSI_C

void raise_error2 (int exception_num, const_cstritype filename, int line)
#else

void raise_error2 (exception_num, filename, line)
int exception_num;
char *filename;
int line;
#endif

  { /* raise_error2 */
    (void) raise_exception(prog.sys_var[exception_num]);
  } /* raise_error2 */



#ifdef ANSI_C

static void writeHelp (void)
#else

static void writeHelp ()
#endif

  { /* writeHelp */
    printf("usage: s7 [options] sourcefile [parameters]\n\n");
    printf("Options:\n");
    printf("  -?   Write Seed7 interpreter usage.\n");
    printf("  -a   Analyze only and suppress the execution phase.\n");
    printf("  -dx  Set compile time trace level to x. Where x is a string consisting of:\n");
    printf("         a Trace primitive actions\n");
    printf("         c Do action check\n");
    printf("         d Trace dynamic calls\n");
    printf("         e Trace exceptions and handlers\n");
    printf("         h Trace heap size (in combination with 'a')\n");
    printf("  -d   Equivalent to -da\n");
    printf("  -i   Show the identifier table after the analyzing phase.\n");
    printf("  -l   Add a directory to the include library search path (e.g.: -l ../lib).\n");
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
    const_cstritype comp_trace_level = NULL;
    const_cstritype exec_trace_level = NULL;
    int verbosity_level = 1;
    rtlArraytype seed7_libraries;
    booltype error = FALSE;

  /* processOptions */
#ifdef TRACE_OPTION
    printf("BEGIN processOptions\n");
#endif
    option.source_file_argument = NULL;
    option.analyze_only = FALSE;
    if (ALLOC_RTL_ARRAY(seed7_libraries, 0)) {
      seed7_libraries->min_position = 1;
      seed7_libraries->max_position = 0;
    } /* if */
    for (position = 0; position < arg_v->max_position; position++) {
      if (option.source_file_argument == NULL) {
        opt = arg_v->arr[position].value.strivalue;
        /* prot_stri(opt);
           printf("\n"); */
        if (opt->size == 2 && opt->mem[0] == '-') {
          switch (opt->mem[1]) {
            case 'a':
              option.analyze_only = TRUE;
              break;
            case 'd':
              comp_trace_level = "a";
              break;
            case 'h':
            case '?':
              option.write_help = TRUE;
              break;
            case 'i':
              option.parser_options |= SHOW_IDENT_TABLE;
              break;
            case 'p':
              if (position < arg_v->max_position - 1) {
                arg_v->arr[position].value.strivalue = NULL;
                FREE_STRI(opt, opt->size);
                position++;
                option.prot_file_name = stri_to_standard_path(arg_v->arr[position].value.strivalue);
                arg_v->arr[position].value.strivalue = NULL;
                opt = NULL;
              } /* if */
              break;
            case 'q':
              verbosity_level = 0;
              break;
            case 's':
              option.catch_signals = FALSE;
              break;
            case 't':
              exec_trace_level = "a";
              break;
            case 'v':
              verbosity_level = 2;
              break;
            case 'x':
              option.execute_always = TRUE;
              break;
            case 'l':
              if (position < arg_v->max_position - 1) {
                arg_v->arr[position].value.strivalue = NULL;
                FREE_STRI(opt, opt->size);
                position++;
                opt = stri_to_standard_path(arg_v->arr[position].value.strivalue);
                if (seed7_libraries != NULL) {
                  arrPush(&seed7_libraries, (rtlGenerictype) opt);
                } /* if */
                arg_v->arr[position].value.strivalue = NULL;
                opt = NULL;
              } /* if */
              break;
            default:
              if (!error) {
                printf(VERSION_INFO, LEVEL);
                error = TRUE;
              } /* if */
              printf("*** Ignore unsupported option: ");
              prot_stri_unquoted(opt);
              printf("\n");
              break;
          } /* switch */
        } else if (opt->size >= 3 && opt->mem[0] == '-') {
          switch (opt->mem[1]) {
            case 'd':
              if (ALLOC_CSTRI(cstri_opt, opt->size - 2)) {
                stri_compress((ustritype) cstri_opt,
                    &opt->mem[2], opt->size - 2);
                cstri_opt[opt->size - 2] = '\0';
                comp_trace_level = cstri_opt;
              } else {
                comp_trace_level = "";
              } /* if */
              break;
            case 't':
              if (ALLOC_CSTRI(cstri_opt, opt->size - 2)) {
                stri_compress((ustritype) cstri_opt,
                    &opt->mem[2], opt->size - 2);
                cstri_opt[opt->size - 2] = '\0';
                exec_trace_level = cstri_opt;
              } else {
                exec_trace_level = "";
              } /* if */
              break;
            case 'v':
              if (opt->mem[2] >= '0' && opt->mem[2] <= '3') {
                verbosity_level = (int) opt->mem[2] - '0';
              } else {
                verbosity_level = 2;
              } /* if */
              break;
            default:
              if (!error) {
                printf(VERSION_INFO, LEVEL);
                error = TRUE;
              } /* if */
              printf("*** Ignore unsupported option: ");
              prot_stri_unquoted(opt);
              printf("\n");
              break;
          } /* switch */
        } else {
          option.source_file_argument = stri_to_standard_path(opt);
          arg_v->arr[position].value.strivalue = NULL;
          opt = NULL;
        } /* if */
        if (opt != NULL) {
          arg_v->arr[position].value.strivalue = NULL;
          FREE_STRI(opt, opt->size);
        } /* if */
      } else {
        if (option.argv == NULL) {
          option.argv = arg_v;
          option.argv_start = (memsizetype) position;
          /* printf("argv_start = %d\n", position); */
        } /* if */
      } /* if */
    } /* for */
    mapTraceFlags2(comp_trace_level, &option.parser_options);
    mapTraceFlags2(exec_trace_level, &option.exec_options);
    option.seed7_libraries = seed7_libraries;
    if (verbosity_level >= 1) {
      if (verbosity_level >= 2) {
        option.parser_options |= WRITE_LIBRARY_NAMES;
        option.parser_options |= SHOW_STATISTICS;
        if (verbosity_level >= 3) {
          option.parser_options |= WRITE_LINE_NUMBERS;
        } /* if */
      } /* if */
      if (!error) {
        printf(VERSION_INFO, LEVEL);
      } /* if */
    } /* if */
#ifdef TRACE_OPTION
    printf("END processOptions\n");
#endif
  } /* processOptions */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

static void printOptions (void)
#else

static void printOptions ()
#endif

  { /* printOptions */
    printf("source_file_argument: "); prot_stri( option.source_file_argument);    printf("\n");
    printf("prot_file_name:       "); prot_stri( option.prot_file_name);          printf("\n");
    printf("write_help:           "); prot_int(  option.write_help);              printf("\n");
    printf("analyze_only:         "); prot_int(  option.analyze_only);            printf("\n");
    printf("execute_always:       "); prot_int(  option.execute_always);          printf("\n");
    printf("parser_options:       "); prot_int(  option.parser_options);          printf("\n");
    printf("catch_signals:        "); prot_int(  option.catch_signals);           printf("\n");
    printf("seed7_libraries:      "); prot_int((inttype) option.seed7_libraries); printf("\n");
    printf("argv:                 "); prot_int((inttype) option.argv);            printf("\n");
    printf("argv_start:           "); prot_int( option.argv_start);               printf("\n");
  } /* printOptions */
#endif



#ifdef ANSI_C

#ifdef USE_WMAIN
int wmain (int argc, wchar_t **argv)
#elif defined USE_WINMAIN
int WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, char *lpCmdLine, int nShowCmd)
#else
int main (int argc, char **argv)
#endif
#else

#ifdef USE_WMAIN
int wmain (argc, argv)
int argc;
wchar_t **argv;
#elif defined USE_WINMAIN
int WinMain (hInstance, hPrevInstance, lpCmdLine, nShowCmd)
HINSTANCE hInstance;
HINSTANCE hPrevInstance;
char *lpCmdLine;
int nShowCmd;
#else
int main (argc, argv)
int argc;
char **argv;
#endif
#endif

  {
    rtlArraytype arg_v;
    progtype currentProg;

  /* main */
#ifdef TRACE_S7
    printf("BEGIN S7\n");
#endif
    set_protfile_name(NULL);
#ifdef USE_WINMAIN
    arg_v = getArgv(0, NULL, NULL, NULL, &programPath);
#else
    arg_v = getArgv(argc, argv, NULL, NULL, &programPath);
#endif
    if (arg_v == NULL) {
      printf(VERSION_INFO, LEVEL);
      printf("\n*** No more memory. Program terminated.\n");
    } else {
      processOptions(arg_v);
      /* printOptions(); */
#ifdef CATCH_SIGNALS
      if (option.catch_signals) {
        activate_signal_handlers();
      } /* if */
#endif
      if (arg_v->max_position < arg_v->min_position) {
        printf("This is free software; see the source for copying conditions.  There is NO\n");
        printf("warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
        printf("Homepage: http://seed7.sourceforge.net\n\n");
        printf("usage: s7 [options] sourcefile [parameters]\n\n");
        printf("Use  s7 -?  to get more information about s7.\n\n");
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
          currentProg = analyze(option.source_file_argument, option.parser_options,
                                option.seed7_libraries, option.prot_file_name);
          if (!option.analyze_only && currentProg != NULL &&
              (currentProg->error_count == 0 || option.execute_always)) {
            /* PRIME_OBJECTS(); */
            /* printf("%d%d\n",
               trace.actions,
               trace.check_actions); */
            interpret(currentProg, option.argv, option.argv_start,
                      option.exec_options, option.prot_file_name);
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
#ifdef TRACE_S7
    printf("END S7\n");
#endif
    return 0;
  } /* main */

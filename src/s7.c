/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2025  Thomas Mertes                        */
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
/*  Changes: 1990 - 1994, 2010, 2011, 2013, 2015  Thomas Mertes     */
/*           2021  Thomas Mertes                                    */
/*  Content: Main program of the Seed7 interpreter.                 */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "sigutl.h"
#include "data.h"
#include "data_rtl.h"
#include "os_decls.h"
#include "infile.h"
#include "heaputl.h"
#include "flistutl.h"
#include "actutl.h"
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
#include "int_rtl.h"
#include "flt_rtl.h"
#include "arr_rtl.h"
#include "cmd_rtl.h"
#include "str_rtl.h"
#include "sql_rtl.h"
#include "con_rtl.h"
#include "con_drv.h"
#include "fil_drv.h"
#include "big_drv.h"
#include "drw_drv.h"

#ifdef USE_WINMAIN
typedef struct {
    int dummy;
  } HINSTANCE__;
typedef HINSTANCE__ *HINSTANCE;
#endif

#define VERSION_INFO "SEED7 INTERPRETER Version 5.3.%d  Copyright (c) 1990-2025 Thomas Mertes\n"



void raise_error2 (int exception_num, const_cstriType filename, int line)

  { /* raise_error2 */
    /* printf("raise_error2(%d, %s, %d)\n", exception_num, filename, line); */
    if (prog != NULL) {
      (void) raise_exception(prog->sys_var[exception_num]);
    } else {
      (void) raise_with_arguments(NULL, NULL);
    } /* if */
  } /* raise_error2 */



static void writeHelp (void)

  { /* writeHelp */
    printf("usage: s7 [options] sourcefile [parameters]\n\n");
    printf("Options:\n");
    printf("  -? or -h  Write Seed7 interpreter usage.\n");
    printf("  -a   Analyze only and suppress the execution phase.\n");
    printf("  -dx  Set compile time trace level to x. Where x is a string consisting of:\n");
    printf("         a Trace primitive actions\n");
    printf("         c Do action check\n");
    printf("         d Trace dynamic calls\n");
    printf("         e Trace exceptions and handlers\n");
    printf("         h Trace heap size (in combination with 'a')\n");
    printf("         s Trace signals\n");
    printf("  -d   Equivalent to -da\n");
    printf("  -i   Show the identifier table after the analysis phase.\n");
    printf("  -l   Add a directory to the include library search path (e.g.: -l ../lib).\n");
    printf("  -p   Specify a protocol file, for trace output (e.g.: -p prot.txt).\n");
    printf("  -q   Compile quiet. Line and file information and compilation\n");
    printf("       statistics are suppressed.\n");
    printf("  -s   Deactivate signal handlers.\n");
    printf("  -tx  Set runtime trace level to x. Where x is a string consisting of:\n");
    printf("         a Trace primitive actions\n");
    printf("         c Do action check\n");
    printf("         d Trace dynamic calls\n");
    printf("         e Trace exceptions and handlers\n");
    printf("         h Trace heap size (in combination with 'a')\n");
    printf("         s Trace signals\n");
    printf("  -t   Equivalent to -ta\n");
    printf("  -vn  Set verbosity level of analysis phase to n. Where n is one of:\n");
    printf("         0 Compile quiet (equivalent to -q)\n");
    printf("         1 Write just the header with version information (default)\n");
    printf("         2 Write a list of include libraries\n");
    printf("         3 Write line numbers, while analyzing\n");
    printf("  -v   Equivalent to -v2\n");
    printf("  -x   Execute even if the program contains errors.\n\n");
  } /* writeHelp */



#if ANY_LOG_ACTIVE
static void printArray (const const_rtlArrayType array)

  {
    memSizeType position;

  /* printArray */
    if (array == NULL) {
      printf("*NULL*");
    } else if (arraySize(array) != 0) {
      if (array->arr[0].value.striValue == NULL) {
        printf("NULL");
      } else {
        printf("\"%s\"", striAsUnquotedCStri(array->arr[0].value.striValue));
      } /* if */
      for (position = 1; position < arraySize(array); position++) {
        if (array->arr[position].value.striValue == NULL) {
          printf(", NULL");
        } else {
          printf(", \"%s\"", striAsUnquotedCStri(array->arr[position].value.striValue));
        } /* if */
      } /* for */
    } /* if */
    printf("\n");
  } /* printArray */



static void printOptions (const optionType option)

  { /* printOptions */
    printf("sourceFileArgument: \"%s\"\n",
           striAsUnquotedCStri(option->sourceFileArgument));
    printf("protFileName:       \"%s\"\n", striAsUnquotedCStri(option->protFileName));
    printf("writeHelp:          %s\n", option->writeHelp ? "TRUE" : "FALSE");
    printf("analyzeOnly:        %s\n", option->analyzeOnly ? "TRUE" : "FALSE");
    printf("executeAlways:      %s\n", option->executeAlways ? "TRUE" : "FALSE");
    printf("parserOptions:      " FMT_U "\n", option->parserOptions);
    printf("libraryDirs:        ");
    printArray(option->libraryDirs);
    printf("argv:               ");
    printArray(option->argv);
    printf("argvStart:          " FMT_U_MEM "\n", option->argvStart);
  } /* printOptions */
#endif



void freeOptions (optionType option)

  { /* freeOptions */
    strDestr(option->sourceFileArgument);
    strDestr(option->protFileName);
    freeStringArray(option->libraryDirs);
  } /* freeOptions */



static void processOptions (rtlArrayType arg_v, const optionType option)

  {
    int position;
    striType opt;
    striType traceLevel;
    int verbosity_level = 1;
    boolType handleSignals = TRUE;
    rtlArrayType libraryDirs;
    rtlObjectType pathObj;
    boolType error = FALSE;

  /* processOptions */
    logFunction(printf("processOptions\n"););
    option->sourceFileArgument = NULL;
    option->analyzeOnly = FALSE;
    if (ALLOC_RTL_ARRAY(libraryDirs, 0)) {
      libraryDirs->min_position = 1;
      libraryDirs->max_position = 0;
    } /* if */
    for (position = 0; position < arg_v->max_position; position++) {
      if (option->sourceFileArgument == NULL) {
        opt = arg_v->arr[position].value.striValue;
        /* printf("opt=\"%s\"\n", striAsUnquotedCStri(opt)); */
        if (opt->size == 2 && opt->mem[0] == '-') {
          switch (opt->mem[1]) {
            case 'a':
              option->analyzeOnly = TRUE;
              break;
            case 'd':
              if (ALLOC_STRI_SIZE_OK(traceLevel, 1)) {
                traceLevel->mem[0] = 'a';
                traceLevel->size = 1;
                mapTraceFlags(traceLevel, &option->parserOptions);
                FREE_STRI(traceLevel, 1);
              } /* if */
              break;
            case 'h':
            case '?':
              option->writeHelp = TRUE;
              break;
            case 'i':
              option->parserOptions |= SHOW_IDENT_TABLE;
              break;
            case 'p':
              if (position < arg_v->max_position - 1) {
                arg_v->arr[position].value.striValue = NULL;
                FREE_STRI(opt, opt->size);
                position++;
                opt = arg_v->arr[position].value.striValue;
                option->protFileName = stri_to_standard_path(opt);
                arg_v->arr[position].value.striValue = NULL;
                opt = NULL;
              } /* if */
              break;
            case 'q':
              verbosity_level = 0;
              break;
            case 's':
              handleSignals = FALSE;
              break;
            case 't':
              if (ALLOC_STRI_SIZE_OK(traceLevel, 1)) {
                traceLevel->mem[0] = 'a';
                traceLevel->size = 1;
                mapTraceFlags(traceLevel, &option->execOptions);
                FREE_STRI(traceLevel, 1);
              } /* if */
              break;
            case 'v':
              verbosity_level = 2;
              break;
            case 'x':
              option->executeAlways = TRUE;
              break;
            case 'l':
              if (position < arg_v->max_position - 1) {
                arg_v->arr[position].value.striValue = NULL;
                FREE_STRI(opt, opt->size);
                position++;
                opt = arg_v->arr[position].value.striValue;
                pathObj.value.striValue = stri_to_standard_path(opt);
                if (libraryDirs != NULL && pathObj.value.striValue != NULL) {
                  arrPush(&libraryDirs, pathObj.value.genericValue);
                } /* if */
                arg_v->arr[position].value.striValue = NULL;
                opt = NULL;
              } /* if */
              break;
            default:
              if (!error) {
                printf(VERSION_INFO, LEVEL);
                error = TRUE;
              } /* if */
              printf("*** Ignore unsupported option: ");
              conWrite(opt);
              printf("\n");
              break;
          } /* switch */
        } else if (opt->size >= 3 && opt->mem[0] == '-') {
          switch (opt->mem[1]) {
            case 'd':
              if (ALLOC_STRI_SIZE_OK(traceLevel, opt->size - 2)) {
                memcpy(traceLevel->mem, &opt->mem[2],
                       (opt->size - 2) * sizeof(strElemType));
                traceLevel->size = opt->size - 2;
                mapTraceFlags(traceLevel, &option->parserOptions);
                FREE_STRI(traceLevel, 1);
              } /* if */
              break;
            case 't':
              if (ALLOC_STRI_SIZE_OK(traceLevel, opt->size - 2)) {
                memcpy(traceLevel->mem, &opt->mem[2],
                       (opt->size - 2) * sizeof(strElemType));
                traceLevel->size = opt->size - 2;
                mapTraceFlags(traceLevel, &option->execOptions);
                FREE_STRI(traceLevel, 1);
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
              conWrite(opt);
              printf("\n");
              break;
          } /* switch */
        } else {
          option->sourceFileArgument = stri_to_standard_path(opt);
          arg_v->arr[position].value.striValue = NULL;
          opt = NULL;
        } /* if */
        if (opt != NULL) {
          arg_v->arr[position].value.striValue = NULL;
          FREE_STRI(opt, opt->size);
        } /* if */
      } else {
        if (option->argv == NULL) {
          option->argv = arg_v;
          option->argvStart = (memSizeType) position;
          /* printf("argvStart = %d\n", position); */
        } /* if */
      } /* if */
    } /* for */
    option->libraryDirs = libraryDirs;
    if (verbosity_level >= 1) {
      if (verbosity_level >= 2) {
        option->parserOptions |= WRITE_LIBRARY_NAMES;
        option->parserOptions |= SHOW_STATISTICS;
        if (verbosity_level >= 3) {
          option->parserOptions |= WRITE_LINE_NUMBERS;
        } /* if */
      } /* if */
      if (!error) {
        printf(VERSION_INFO, LEVEL);
      } /* if */
    } /* if */
    if (handleSignals) {
      option->parserOptions |= HANDLE_SIGNALS;
      option->execOptions   |= HANDLE_SIGNALS;
    } /* if */
    logFunction(printf("processOptions -->\n");
                printOptions(option););
  } /* processOptions */



#ifdef USE_WMAIN
int wmain (int argc, wchar_t **argv)
#elif defined USE_WINMAIN
int WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, char *lpCmdLine, int nShowCmd)
#else
int main (int argc, char **argv)
#endif

  {
    rtlArrayType arg_v;
    progType currentProg;
    optionRecord option = {
        NULL,  /* sourceFileArgument */
        NULL,  /* protFileName       */
        FALSE, /* writeHelp          */
        FALSE, /* analyzeOnly        */
        FALSE, /* executeAlways      */
        0,     /* parserOptions      */
        0,     /* execOptions        */
        NULL,  /* libraryDirs        */
        NULL,  /* argv               */
        0,     /* argvStart          */
      };

  /* main */
    logFunction(printf("main\n"););
    setupStack(DEFAULT_STACK_SIZE);
    setupRand();
    setupFiles();
    set_protfile_name(NULL);
#ifdef USE_WINMAIN
    arg_v = getArgv(0, NULL, NULL, NULL, NULL);
#else
    arg_v = getArgv(argc, argv, NULL, NULL, NULL);
#endif
    if (arg_v == NULL) {
      printf(VERSION_INFO, LEVEL);
      printf("\n*** No more memory. Program terminated.\n");
    } else {
      processOptions(arg_v, &option);
      setupSignalHandlers((option.parserOptions & HANDLE_SIGNALS) != 0,
                          (option.parserOptions & TRACE_SIGNALS) != 0,
                          FALSE, FALSE, doSuspendInterpreter);
      if (fail_flag) {
        printf("\n*** Processing the options failed. Program terminated.\n");
      } else {
        if (arg_v->max_position < arg_v->min_position) {
          printf("This is free software; see the source for copying conditions.  There is NO\n");
          printf("warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
          printf("Homepage: http://seed7.sourceforge.net\n\n");
          printf("usage: s7 [options] sourcefile [parameters]\n\n");
          printf("Use  s7 -?  to get more information about s7.\n\n");
        } else if (option.writeHelp) {
          writeHelp();
        } else {
          setupFloat();
          setupBig();
          drawInit();
          /* printf("sourceFileArgument: \"");
             prot_stri(option.sourceFileArgument);
             printf("\"\n");
             printf("protFileName: \"%s\"\n", option.protFileName); */
          if (option.sourceFileArgument == NULL) {
            printf("*** Sourcefile missing\n");
          } else {
            currentProg = analyze(option.sourceFileArgument, option.parserOptions,
                                  option.libraryDirs, option.protFileName);
            if (!option.analyzeOnly && currentProg != NULL &&
                (currentProg->error_count == 0 || option.executeAlways)) {
              /* PRIME_OBJECTS(); */
              /* printf("%d%d\n",
                 trace.actions,
                 trace.check_actions); */
              if (currentProg->main_object == NULL ||
                  CATEGORY_OF_OBJ(currentProg->main_object) == FORWARDOBJECT) {
                printf("*** Declaration for main missing\n");
              } else {
                interpret(currentProg, option.argv, option.argvStart,
                          option.execOptions, option.protFileName);
              } /* if */
              if (fail_flag) {
                uncaught_exception(currentProg);
                if (fail_value == DB_EXCEPTION(currentProg)) {
                  striType message;

                  message = sqlErrMessage();
                  printf("\nMessage from the DATABASE_ERROR exception:\n");
                  conWrite(message);
                  printf("\n");
                  FREE_STRI(message, message->size);
                } /* if */
              } /* if */
            } /* if */
#if HEAP_STATISTIC_AT_PROGRAM_EXIT
            prgDestr(currentProg);
#endif
          } /* if */
        } /* if */
        shutDrivers();
        freeOptions(&option);
      } /* if */
      freeStringArray(arg_v);
    } /* if */
    /* getchar(); */
#if HEAP_STATISTIC_AT_PROGRAM_EXIT
    leaveExceptionHandling();
    freeActPtrTable();
    closeBig();
    heapStatistic();
#endif
#if SHOW_OBJECT_MEMORY_LEAKS
    listAllObjects();
#endif
#if CHECK_STACK
    printf("max_stack_size: " FMT_U_MEM "\n", getMaxStackSize());
#endif
    logFunction(printf("main --> 0\n"););
#ifdef USE_DO_EXIT
    doExit(0);
#endif
    return 0;
  } /* main */

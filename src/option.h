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
/*  Module: Main                                                    */
/*  File: seed7/src/option.h                                        */
/*  Changes: 1994  Thomas Mertes                                    */
/*  Content: Reads and interprets the command line options.         */
/*                                                                  */
/********************************************************************/

#define TRACE_ACTIONS            1
#define TRACE_DO_ACTION_CHECK    2
#define TRACE_DYNAMIC_CALLS      4
#define TRACE_EXCEPTIONS         8
#define TRACE_HEAP_SIZE         16
#define TRACE_MATCH             32
#define TRACE_EXECUTIL          64
#define TRACE_SIGNALS          128
#define HANDLE_SIGNALS         256
#define WRITE_LIBRARY_NAMES    512
#define WRITE_LINE_NUMBERS    1024
#define SHOW_IDENT_TABLE      2048
#define SHOW_STATISTICS       4096

typedef struct {
    striType          source_file_argument;
    const_striType    prot_file_name;
    boolType          write_help;
    boolType          analyze_only;
    boolType          execute_always;
    uintType          parser_options;
    uintType          exec_options;
    boolType          handle_signals;
    rtlArrayType      seed7_libraries;
    rtlArrayType      argv;
    memSizeType       argv_start;
  } optionType;

#ifdef DO_INIT
optionType option = {
    NULL,  /* source_file_name  */
    NULL,  /* prot_file_name    */
    FALSE, /* write_help        */
    FALSE, /* analyze_only      */
    FALSE, /* execute_always    */
    0,     /* parser_options    */
    0,     /* exec_options      */
    TRUE,  /* handle_signals    */
    NULL,  /* seed7_libraries   */
    NULL,  /* argv              */
    0,     /* argv_start        */
  };
#else
EXTERN optionType option;
#endif

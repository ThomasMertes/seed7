/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000, 2015  Thomas Mertes                  */
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
/*  Changes: 1994, 2015  Thomas Mertes                              */
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

typedef struct optionStruct {
    striType          sourceFileArgument;
    const_striType    protFileName;
    boolType          writeHelp;
    boolType          analyzeOnly;
    boolType          executeAlways;
    uintType          parserOptions;
    uintType          execOptions;
    rtlArrayType      libraryDirs;
    rtlArrayType      argv;
    memSizeType       argvStart;
  } optionRecord, *optionType;

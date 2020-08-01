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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: Analyzer                                                */
/*  File: seed7/src/info.c                                          */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Procedures for compile time info.                      */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "infile.h"
#include "option.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "info.h"



#ifdef WITH_COMPILATION_INFO
#ifdef ANSI_C

void display_compilation_info (void)
#else

void display_compilation_info ()
#endif

  { /* display_compilation_info */
#ifdef TRACE_INFILE
    printf("BEGIN display_compilation_info\n");
#endif
    if (option.compilation_info) {
      CR_FIL_LIN_INFO();
      fflush(stdout);
    } /* if */
#ifdef TRACE_INFILE
    printf("END display_compilation_info\n");
#endif
  } /* display_compilation_info */
#endif



#ifdef WITH_COMPILATION_INFO
#ifdef ANSI_C

void line_compilation_info (void)
#else

void line_compilation_info ()
#endif

  { /* line_compilation_info */
#ifdef TRACE_INFILE
    printf("BEGIN line_compilation_info\n");
#endif
    if (option.compilation_info) {
      CR_LIN_INFO();
      fflush(stdout);
      in_file.next_msg_line = in_file.line + option.incr_message_line;
    } /* if */
#ifdef TRACE_INFILE
    printf("END line_compilation_info\n");
#endif
  } /* line_compilation_info */
#endif



#ifdef WITH_COMPILATION_INFO
#ifdef ANSI_C

void open_compilation_info (void)
#else

void open_compilation_info ()
#endif

  {
    unsigned int name_length;
    unsigned int next_name_length;
    unsigned int number;

  /* open_compilation_info */
#ifdef TRACE_INFILE
    printf("BEGIN open_compilation_info\n");
#endif
    if (option.compilation_info) {
      printf("     1 %s", in_file.name);
      if (in_file.next != NULL) {
        name_length = strlen((cstritype) in_file.name);
        next_name_length = strlen((cstritype) in_file.next->name);
        for (number = name_length; number < next_name_length;
            number++) {
          fputc(' ', stdout);
        } /* for */
      } /* if */
      fputc('\r', stdout);
      fflush(stdout);
    } /* if */
#ifdef TRACE_INFILE
    printf("END open_compilation_info\n");
#endif
  } /* open_compilation_info */
#endif

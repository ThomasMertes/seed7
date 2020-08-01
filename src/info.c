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

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "info.h"



#ifdef WITH_COMPILATION_INFO
void display_compilation_info (void)

  { /* display_compilation_info */
#ifdef TRACE_INFILE
    printf("BEGIN display_compilation_info\n");
#endif
    if (in_file.write_line_numbers) {
      CR_FIL_LIN_INFO();
      fflush(stdout);
    } /* if */
#ifdef TRACE_INFILE
    printf("END display_compilation_info\n");
#endif
  } /* display_compilation_info */
#endif



#ifdef WITH_COMPILATION_INFO
void line_compilation_info (void)

  { /* line_compilation_info */
#ifdef TRACE_INFILE
    printf("BEGIN line_compilation_info\n");
#endif
    if (in_file.write_line_numbers) {
      CR_LIN_INFO();
      fflush(stdout);
      in_file.next_msg_line = in_file.line + in_file.incr_message_line;
    } /* if */
#ifdef TRACE_INFILE
    printf("END line_compilation_info\n");
#endif
  } /* line_compilation_info */
#endif



#ifdef WITH_COMPILATION_INFO
void open_compilation_info (boolType write_library_names, boolType write_line_numbers)

  {
    memSizeType new_name_length;
    memSizeType curr_name_length;
    memSizeType number;

  /* open_compilation_info */
#ifdef TRACE_INFILE
    printf("BEGIN open_compilation_info\n");
#endif
    in_file.write_library_names = write_library_names;
    in_file.write_line_numbers = write_line_numbers;
    if (write_line_numbers) {
      in_file.incr_message_line = 0;
    } else {
      in_file.incr_message_line = LARGE_INCR;
    } /* if */
    in_file.next_msg_line = 1 + in_file.incr_message_line;
    if (write_line_numbers) {
      printf("     1 %s", in_file.name_ustri);
      if (in_file.curr_infile != NULL) {
        new_name_length = strlen((const_cstriType) in_file.name_ustri);
        curr_name_length = strlen((const_cstriType) in_file.curr_infile->name_ustri);
        for (number = new_name_length; number < curr_name_length;
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

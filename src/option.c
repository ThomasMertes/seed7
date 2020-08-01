/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2005  Thomas Mertes                        */
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
/*  Module: Main                                                    */
/*  File: seed7/src/option.c                                        */
/*  Changes: 1994  Thomas Mertes                                    */
/*  Content: Reads and interprets the command line options.         */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"

#include "version.h"
#include "common.h"
#include "data.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "option.h"



#ifdef ANSI_C

void options (int argc, char **argv, int start)
#else

void options (argc, argv, start)
int argc;
char **argv;
int start;
#endif

  {
    int position;
    char *opt;
    int numeric_value;
    booltype has_numeric_value;

  /* options */
#ifdef TRACE_OPTION
    printf("BEGIN options\n");
#endif
    option.source_file_name = NULL;
    option.analyze_only = FALSE;
    option.show_ident_table = FALSE;
    for (position = start; position < argc; position++) {
      if (option.source_file_name == NULL) {
        opt = argv[position];
        if (opt[0] == '-' && opt[1] != '\0') {
          has_numeric_value = FALSE;
          if (opt[2] >= '0' && opt[2] <= '9') {
            if (sscanf(&opt[2], "%d", &numeric_value) != 1) {
              printf("*** Illegal numeric value \"%s\"\n",
                  &opt[2]);
            } else {
              has_numeric_value = TRUE;
            } /* if */
          } /* if */
          switch (opt[1]) {
            case 'a':
              option.analyze_only = TRUE;
              break;
            case 'd':
              if (opt[2] != '\0') {
                option.comp_trace_level = &opt[2];
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
              if (opt[2] != '\0') {
                option.prot_file_name = &opt[2];
              } else {
                if (position < argc) {
                  position++;
                  option.prot_file_name = argv[position];
                } /* if */
              } /* if */
              break;
            case 'q':
              option.compilation_info = FALSE;
              option.linecount_info = FALSE;
              break;
            case 't':
              if (opt[2] != '\0') {
                option.exec_trace_level = &opt[2];
              } else {
                option.exec_trace_level = "a";
              } /* if */
              break;
            default:
              printf("*** Unknown option %s\n", opt);
              break;
          } /* switch */
        } else {
          option.source_file_name = opt;
        } /* if */
      } else {
        if (option.arg_v == NULL) {
          option.arg_c = argc - position;
          option.arg_v = &argv[position];
        } /* if */
      } /* if */
    } /* for */
#ifdef TRACE_OPTION
    printf("END options\n");
#endif
  } /* options */

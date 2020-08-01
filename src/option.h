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

typedef struct {
    stritype          source_file_name;
    const_cstritype   prot_file_name;
    booltype          write_help;
    booltype          analyze_only;
    booltype          execute_always;
    booltype          show_ident_table;
    booltype          get_infile_buffer;
    booltype          version_info;
    booltype          compilation_info;
    booltype          linecount_info;
    booltype          catch_signals;
    unsigned int      incr_message_line;
    const_cstritype   comp_trace_level;
    const_cstritype   exec_trace_level;
    void             *argv;
    memsizetype       argv_start;
    objecttype        arg_v;
  } opttype;

#ifdef DO_INIT
opttype option = {
    NULL,  /* source_file_name  */
    NULL,  /* prot_file_name    */
    FALSE, /* write_help        */
    FALSE, /* analyze_only      */
    FALSE, /* execute_always    */
    FALSE, /* show_ident_table  */
    TRUE,  /* get_infile_buffer */
    TRUE,  /* version_info      */
    FALSE, /* compilation_info  */
    FALSE, /* linecount_info    */
    TRUE,  /* catch_signals     */
    16383, /* incr_message_line */
    NULL,  /* comp_trace_level  */
    NULL,  /* exec_trace_level  */
    NULL,  /* argv              */
    0,     /* argv_start        */
    NULL,  /* arg_v             */
  };
#else
EXTERN opttype option;
#endif

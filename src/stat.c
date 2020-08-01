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
/*  File: seed7/src/stat.c                                          */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: Procedures for maintaining an analyze phase statistic. */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdio.h"
#include "time.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "symbol.h"
#include "infile.h"

#undef EXTERN
#define EXTERN
#include "stat.h"


static clock_t start_time;



#ifdef ANSI_C

void show_statistic (void)
#else

void show_statistic ()
#endif

  {
#ifdef WITH_COMPILATION_INFO
    clock_t stop_time;
    long c_per_msec;
    long msecs_total;
#endif

  /* show_statistic */
#ifdef WITH_COMPILATION_INFO
    printf("%6ld lines total\n", total_lines);
    stop_time = clock();
/*  printf("%6d start clock\n", start_time);
    printf("%6d stop clock\n", stop_time); */
    if (CLOCKS_PER_SEC >= 1000) {
      c_per_msec = CLOCKS_PER_SEC / 1000;
      msecs_total = ((long) (stop_time - start_time)) / c_per_msec;
    } else if (CLOCKS_PER_SEC > 0) {
      c_per_msec = 1000 / CLOCKS_PER_SEC;
      msecs_total = ((long) (stop_time - start_time)) * c_per_msec;
    } else {
      msecs_total = (long) 0;
    } /* if */
/*  printf("%6d milliseconds\n", msecs_total); */
    if (msecs_total != 0) {
      printf("%6d lines per second\n",
        (int) ((total_lines * 1000) / msecs_total));
    } /* if */
#endif
#ifdef WITH_STATISTIK
    printf("%6d literals\n", literal_count);
    printf("%6d lists\n", list_count);
    printf("%6d comments\n", comment_count);
#endif
#ifdef DO_HEAP_STATISTIK
    heap_statistic();
#else
#ifdef DO_HEAPSIZE_COMPUTATION
    printf("%6lu bytes\n", heapsize());
#endif
#endif
    if (error_count >= 1) {
      printf("%6d error%s found\n", error_count, error_count > 1 ? "s" : "");
    } /* if */
  } /* show_statistic */



#ifdef ANSI_C

void reset_statistic (void)
#else

void reset_statistic ()
#endif

  { /* reset_statistic */
#ifdef WITH_COMPILATION_INFO
    total_lines = 0;
#endif
#ifdef WITH_STATISTIK
    literal_count = 0;
    list_count = 0;
    comment_count = 0;
#endif
    error_count = 0;
    start_time = clock();
  } /* reset_statistic */

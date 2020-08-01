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
/*  Module: Analyzer - Error                                        */
/*  File: seed7/src/fatal.c                                         */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: Submit fatal compile time error messages.              */
/*                                                                  */
/*  This file contains the procedure fatal_memory_error which is    */
/*  called when there is no more memory in the analysation phase.   */
/*  (In the interpreting phase an EXCEPTION is called in this case) */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "infile.h"
#include "error.h"
#include "sigutl.h"

#undef EXTERN
#define EXTERN
#include "fatal.h"



void no_memory (const_cstriType source_file, int source_line)

  { /* no_memory */
#ifdef TRACE_HEAPUTIL
    printf("BEGIN no_memory\n");
#endif
    printf("\n*** %s(%1d): No more memory. Program terminated.\n",
        source_file, source_line);
#ifdef DO_HEAPSIZE_COMPUTATION
    printf("%6lu bytes in heap\n", (unsigned long) hs);
    printf("%6lu bytes used\n", (unsigned long) heapsize());
    printf("%6lu bytes in free lists\n", (unsigned long) (hs - heapsize()));
#endif
#ifdef USE_CHUNK_ALLOCS
    printf("%6lu bytes in chunks\n", (unsigned long) chunk.total_size);
    printf("%6u unused bytes in last chunk\n",
        (unsigned) (chunk.beyond - chunk.freemem));
    printf("%6lu lost bytes in chunks\n", (unsigned long) chunk.lost_bytes);
#endif
    shut_drivers();
#ifdef TRACE_HEAPUTIL
    printf("END no_memory\n");
#endif
    exit(1);
  } /* no_memory */



void fatal_memory_error (const_cstriType source_file, int source_line)

  { /* fatal_memory_error */
    place_of_error(OUT_OF_HEAP_SPACE);
    printf("Fatal Error: Out of heap space");
    no_memory(source_file, source_line);
  } /* fatal_memory_error */

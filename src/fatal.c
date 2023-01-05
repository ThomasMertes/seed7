/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000, 2010, 2021  Thomas Mertes            */
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
/*  Changes: 1992 - 1994, 2010, 2021  Thomas Mertes                 */
/*  Content: Submit fatal compile time error messages.              */
/*                                                                  */
/*  This file contains the procedure fatal_memory_error which is    */
/*  called if there is no more memory in the parsing phase.         */
/*  (In the interpreting phase an EXCEPTION is called in this case) */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "setjmp.h"

#include "common.h"
#include "data.h"
#include "os_decls.h"
#include "heaputl.h"
#include "flistutl.h"
#include "traceutl.h"
#include "infile.h"
#include "error.h"
#include "sigutl.h"

#undef EXTERN
#define EXTERN
#include "fatal.h"


/* The long jump position memoryErrorOcurred is */
/* only used during the analyze phase (parsing). */
boolType currentlyAnalyzing = FALSE;
longjmpPosition memoryErrorOcurred;



static void no_memory (const_cstriType source_file, int source_line)

  { /* no_memory */
    logFunction(printf("no_memory(\"%s\", %d)\n", source_file, source_line););
#ifdef WITH_PROTOCOL
    if (trace.exceptions) {
      printf("\n*** %s(%1d): No more memory. Parsing terminated.\n",
             source_file, source_line);
#if DO_HEAPSIZE_COMPUTATION
      printf("%6lu bytes in heap\n", (unsigned long) hs);
      printf("%6lu bytes used\n", (unsigned long) heapsize());
      printf("%6lu bytes in free lists\n", (unsigned long) (hs - heapsize()));
#endif
#if USE_CHUNK_ALLOCS
      printf("%6lu bytes in chunks\n", (unsigned long) chunk.total_size);
      printf("%6u unused bytes in last chunk\n",
          (unsigned) (chunk.beyond - chunk.freemem));
      printf("%6lu lost bytes in chunks\n", (unsigned long) chunk.lost_bytes);
#endif
    } /* if */
#endif
    if (currentlyAnalyzing) {
      logFunction(printf("no_memory(\"%s\", %d) --> longjmp\n",
                         source_file, source_line););
      do_longjmp(memoryErrorOcurred, 1);
    } else {
      shutDrivers();
      logFunction(printf("no_memory(\"%s\", %d) --> exit\n",
                         source_file, source_line););
      os_exit(1);
    } /* if */
  } /* no_memory */



void fatal_memory_error (const_cstriType source_file, int source_line)

  { /* fatal_memory_error */
    logFunction(printf("fatal_memory_error(\"%s\", %d)\n",
                       source_file, source_line););
    place_of_error(OUT_OF_HEAP_SPACE);
    printf("Fatal Error: Out of heap space");
    no_memory(source_file, source_line);
  } /* fatal_memory_error */

/********************************************************************/
/*                                                                  */
/*  segv_win.c    Stack overflow handler for Windows.               */
/*  Copyright (C) 2026  Thomas Mertes                               */
/*                                                                  */
/*  This file is part of the Seed7 Runtime Library.                 */
/*                                                                  */
/*  The Seed7 Runtime Library is free software; you can             */
/*  redistribute it and/or modify it under the terms of the GNU     */
/*  Lesser General Public License as published by the Free Software */
/*  Foundation; either version 2.1 of the License, or (at your      */
/*  option) any later version.                                      */
/*                                                                  */
/*  The Seed7 Runtime Library is distributed in the hope that it    */
/*  will be useful, but WITHOUT ANY WARRANTY; without even the      */
/*  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR */
/*  PURPOSE.  See the GNU Lesser General Public License for more    */
/*  details.                                                        */
/*                                                                  */
/*  You should have received a copy of the GNU Lesser General       */
/*  Public License along with this program; if not, write to the    */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/segv_win.c                                      */
/*  Changes: 2026  Thomas Mertes                                    */
/*  Content: Stack overflow handler for Windows.                    */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#define _WIN32_WINNT 0x500
#include <windows.h>
#include <stdio.h>
#include <setjmp.h>

#include "common.h"
#include "heaputl.h"

static boolType stackOverflow = FALSE;


static LONG WINAPI segmentationViolationHandler (PEXCEPTION_POINTERS pExp)

  { /* segmentationViolationHandler */
    logFunction(printf("segmentationViolationHandler\n"););
    stackOverflow = pExp->ExceptionRecord->ExceptionCode ==
                    EXCEPTION_STACK_OVERFLOW;
    no_memory(SOURCE_POSITION(3021));
    return 0;
  } /* segmentationViolationHandler */



boolType setupSegmentationViolationHandler (void)

  {
    boolType okay;

  /* setupSegmentationViolationHandler */
    logFunction(printf("setupSegmentationViolationHandler\n"););
    okay = AddVectoredExceptionHandler(1, segmentationViolationHandler) != NULL;
    logFunction(printf("setupSegmentationViolationHandler --> %d\n",
                       okay););
    return okay;
  } /* setupSegmentationViolationHandler */



void resetExceptionCheck (void)

  { /* resetExceptionCheck */
    logFunction(printf("resetExceptionCheck\n"););
    if (stackOverflow) {
      _resetstkoflw();
      stackOverflow = FALSE;
    } /* if */
  } /* resetExceptionCheck */

/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2013  Thomas Mertes                        */
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
/*  Module: General                                                 */
/*  File: seed7/src/syvarutl.c                                      */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: Maintains the interpreter system variables.            */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "striutl.h"

#undef EXTERN
#define EXTERN
#include "syvarutl.h"

#undef TRACE_SYSVAR


#define MAX_CSTRI_BUFFER_LEN 25

static const_cstriType sys_name[NUMBER_OF_SYSVARS] = {
    "empty",
    "memory_error",
    "numeric_error",
    "overflow_error",
    "range_error",
    "file_error",
    "illegal_action",
    "false",
    "true",
    "type",
    "expr",
    "integer",
    "bigInteger",
    "char",
    "string",
    "proc",
    "float",
    "assign",
    "create",
    "destroy",
    "ord",
    "in",
    "prot_outfile",
    "flush",
    "write",
    "writeln",
    "main",
  };



/**
 *  Determine the index of a system variable with a given name.
 *  @return the index of the system variable, or
 *          -1 when no system variable with the name is found.
 */
int find_sysvar (const_striType stri)

  {
    char sysvar_name[MAX_CSTRI_BUFFER_LEN + 1];
    errInfoType err_info = OKAY_NO_ERROR;
    int result;

  /* find_sysvar */
#ifdef TRACE_SYSVAR
    printf("BEGIN find_sysvar\n");
#endif
    if (stri->size > MAX_CSTRI_BUFFER_LEN) {
      result = -1;
    } else {
      conv_to_cstri(sysvar_name, stri, &err_info);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        result = -1;
      } else {
        result = NUMBER_OF_SYSVARS - 1;
        while (result >= 0 &&
            strcmp(sysvar_name, sys_name[result]) != 0) {
          result--;
        } /* while */
        /* printf("find_sysvar: %s -> %d\n", sysvar_name, result); */
      } /* if */
    } /* if */
#ifdef TRACE_SYSVAR
    printf("END find_sysvar -> %d\n", result);
#endif
    return result;
  } /* find_sysvar */



void init_sysvar (void)

  {
    int number;

  /* init_sysvar */
#ifdef TRACE_SYSVAR
    printf("BEGIN init_sysvar\n");
#endif
    for (number = 0; number < NUMBER_OF_SYSVARS; number++) {
      prog.sys_var[number] = NULL;
    } /* for */
#ifdef TRACE_SYSVAR
    printf("END init_sysvar\n");
#endif
  } /* init_sysvar */

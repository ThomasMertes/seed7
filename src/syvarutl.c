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

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

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


#define MAX_CSTRI_BUFFER_LEN 25

static const const_cstriType sys_name[NUMBER_OF_SYSVARS] = {
    "empty",
    "memory_error",
    "numeric_error",
    "overflow_error",
    "range_error",
    "file_error",
    "database_error",
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
    "dot",
    "value",
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
int findSysvar (const_striType stri)

  {
    char sysvar_name[MAX_CSTRI_BUFFER_LEN + NULL_TERMINATION_LEN];
    errInfoType err_info = OKAY_NO_ERROR;
    int result;

  /* findSysvar */
    logFunction(printf("findSysvar(\"%s\")\n",
                       striAsUnquotedCStri(stri)););
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
        /* printf("findSysvar: %s -> %d\n", sysvar_name, result); */
      } /* if */
    } /* if */
    logFunction(printf("findSysvar --> %d\n", result););
    return result;
  } /* findSysvar */



void init_sysvar (progType aProgram)

  {
    int number;

  /* init_sysvar */
    logFunction(printf("init_sysvar\n"););
    for (number = 0; number < NUMBER_OF_SYSVARS; number++) {
      aProgram->sys_var[number] = NULL;
    } /* for */
    logFunction(printf("init_sysvar -->\n"););
  } /* init_sysvar */

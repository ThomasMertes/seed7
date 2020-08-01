/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2014  Thomas Mertes                        */
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
/*  Module: Library                                                 */
/*  File: seed7/src/pcslib.c                                        */
/*  Changes: 2014  Thomas Mertes                                    */
/*  Content: All primitive actions for the process type.            */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "syvarutl.h"
#include "striutl.h"
#include "arrutl.h"
#include "objutl.h"
#include "runerr.h"
#include "pcs_rtl.h"
#include "pcs_drv.h"

#undef EXTERN
#define EXTERN
#include "pcslib.h"



/**
 *  Returns the error output file (stderr) of the given child process.
 *  If the standard error file of the subprocess has been redirected
 *  then this function will return NULL.
 *  @return the error output file of ''process'' or
 *          NULL, if stderr has been redirected.
 */
objectType pcs_childStdErr (listType arguments)

  { /* pcs_childStdErr */
    isit_process(arg_1(arguments));
    return bld_file_temp(
        pcsChildStdErr(take_process(arg_1(arguments))));
  } /* pcs_childStdErr */



/**
 *  Returns the standard input file (stdin) of the given child process.
 *  If the standard input file of the subprocess has been redirected
 *  then this function will return NULL.
 *  @return the standard input file of ''process'' or
 *          NULL, if stdin has been redirected.
 */
objectType pcs_childStdIn (listType arguments)

  { /* pcs_childStdIn */
    isit_process(arg_1(arguments));
    return bld_file_temp(
        pcsChildStdIn(take_process(arg_1(arguments))));
  } /* pcs_childStdIn */



/**
 *  Returns the standard output file (stdout) of the given child process.
 *  If the standard output file of the subprocess has been redirected
 *  then this function will return NULL.
 *  @return the standard output file of ''process'' or
 *          NULL, if stdout has been redirected.
 */
objectType pcs_childStdOut (listType arguments)

  { /* pcs_childStdOut */
    isit_process(arg_1(arguments));
    return bld_file_temp(
        pcsChildStdOut(take_process(arg_1(arguments))));
  } /* pcs_childStdOut */



/**
 *  Compare two processes.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
objectType pcs_cmp (listType arguments)

  { /* pcs_cmp */
    isit_process(arg_1(arguments));
    isit_process(arg_2(arguments));
    return bld_int_temp(
        pcsCmp(take_process(arg_1(arguments)), take_process(arg_2(arguments))));
  } /* pcs_cmp */



/**
 *  Assign source/arg_3 to dest/arg_1.
 *  A copy function assumes that dest/arg_1 contains a legal value.
 */
objectType pcs_cpy (listType arguments)

  {
    objectType dest;
    objectType source;
    processType process_source;
    processType old_process;

  /* pcs_cpy */
    dest = arg_1(arguments);
    source = arg_3(arguments);
    isit_process(dest);
    isit_process(source);
    is_variable(dest);
    process_source = take_process(source);
    if (TEMP_OBJECT(source)) {
      source->value.processValue = NULL;
    } else {
      if (process_source != NULL) {
        process_source->usage_count++;
      } /* if */
    } /* if */
    old_process = take_process(dest);
    if (old_process != NULL) {
      old_process->usage_count--;
      if (old_process->usage_count == 0) {
        pcsFree(old_process);
      } /* if */
    } /* if */
    dest->value.processValue = process_source;
    return SYS_EMPTY_OBJECT;
  } /* pcs_cpy */



/**
 *  Initialize dest/arg_1 and assign source/arg_3 to it.
 *  A create function assumes that the contents of dest/arg_1
 *  is undefined. Create functions can be used to initialize
 *  constants.
 */
objectType pcs_create (listType arguments)

  {
    objectType source;
    processType process_value;

  /* pcs_create */
    source = arg_3(arguments);
    isit_process(source);
    SET_CATEGORY_OF_OBJ(arg_1(arguments), PROCESSOBJECT);
    process_value = take_process(source);
    arg_1(arguments)->value.processValue = process_value;
    if (TEMP_OBJECT(source)) {
      source->value.processValue = NULL;
    } else {
      if (process_value != NULL) {
        process_value->usage_count++;
      } /* if */
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* pcs_create */



/**
 *  Free the memory referred by 'old_process/arg_1'.
 *  After pcs_destr is left 'old_process/arg_1' is NULL.
 *  The memory where 'old_process/arg_1' is stored can be freed afterwards.
 */
objectType pcs_destr (listType arguments)

  {
    processType old_process;

  /* pcs_destr */
    isit_process(arg_1(arguments));
    old_process = take_process(arg_1(arguments));
    if (old_process != NULL) {
      old_process->usage_count--;
      if (old_process->usage_count == 0) {
        pcsFree(old_process);
      } /* if */
      arg_1(arguments)->value.processValue = NULL;
    } /* if */
    SET_UNUSED_FLAG(arg_1(arguments));
    return SYS_EMPTY_OBJECT;
  } /* pcs_destr */



objectType pcs_empty (listType arguments)

  { /* pcs_empty */
    return bld_process_temp(NULL);
  } /* pcs_empty */



/**
 *  Check if two processes are equal.
 *  @return TRUE if both processes are equal,
 *          FALSE otherwise.
 */
objectType pcs_eq (listType arguments)

  { /* pcs_eq */
    isit_process(arg_1(arguments));
    isit_process(arg_3(arguments));
    if (pcsEq(take_process(arg_1(arguments)),
              take_process(arg_3(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* pcs_eq */



/**
 *  Return the exit value of the specified process.
 *  By convention, the value 0 indicates normal termination.
 *  @return the exit value of the specified process.
 */
objectType pcs_exit_value (listType arguments)

  { /* pcs_exit_value */
    isit_process(arg_1(arguments));
    return bld_int_temp(
        pcsExitValue(take_process(arg_1(arguments))));
  } /* pcs_exit_value */



/**
 *  Compute the hash value of a process.
 *  @return the hash value.
 */
objectType pcs_hashcode (listType arguments)

  { /* pcs_hashcode */
    isit_process(arg_1(arguments));
    return bld_int_temp(
        pcsHashCode(take_process(arg_1(arguments))));
  } /* pcs_hashcode */



/**
 *  Test whether the specified process is alive.
 *  @return TRUE if the specified process has not yet terminated,
 *          FALSE otherwise.
 */
objectType pcs_is_alive (listType arguments)

  { /* pcs_is_alive */
    isit_process(arg_1(arguments));
    if (pcsIsAlive(take_process(arg_1(arguments)))) {
      return SYS_TRUE_OBJECT;
    } else {
      return SYS_FALSE_OBJECT;
    } /* if */
  } /* pcs_is_alive */



/**
 *  Kill the specified process.
 *  @exception FILE_ERROR It was not possible to kill the process.
 */
objectType pcs_kill (listType arguments)

  { /* pcs_kill */
    isit_process(arg_1(arguments));
    pcsKill(take_process(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pcs_kill */



/**
 *  Check if two processes are not equal.
 *  @return TRUE if both processes are not equal,
 *          FALSE otherwise.
 */
objectType pcs_ne (listType arguments)

  { /* pcs_ne */
    isit_process(arg_1(arguments));
    isit_process(arg_3(arguments));
    if (pcsEq(take_process(arg_1(arguments)),
              take_process(arg_3(arguments)))) {
      return SYS_FALSE_OBJECT;
    } else {
      return SYS_TRUE_OBJECT;
    } /* if */
  } /* pcs_ne */



objectType pcs_pipe2 (listType arguments)

  {
    objectType childStdin_variable;
    objectType childStdout_variable;
    rtlArrayType parameters;

  /* pcs_pipe2 */
    isit_stri(arg_1(arguments));
    isit_array(arg_2(arguments));
    childStdin_variable = arg_3(arguments);
    isit_file(childStdin_variable);
    childStdout_variable = arg_4(arguments);
    isit_file(childStdout_variable);
    parameters = gen_rtl_array(take_array(arg_2(arguments)));
    if (parameters == NULL) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      pcsPipe2(take_stri(arg_1(arguments)), parameters,
               &childStdin_variable->value.fileValue,
               &childStdout_variable->value.fileValue);
      FREE_RTL_ARRAY(parameters, ARRAY_LENGTH(parameters));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* pcs_pipe2 */



objectType pcs_pty (listType arguments)

  {
    objectType childStdin_variable;
    objectType childStdout_variable;
    rtlArrayType parameters;

  /* pcs_pty */
    isit_stri(arg_1(arguments));
    isit_array(arg_2(arguments));
    childStdin_variable = arg_3(arguments);
    isit_file(childStdin_variable);
    childStdout_variable = arg_4(arguments);
    isit_file(childStdout_variable);
    parameters = gen_rtl_array(take_array(arg_2(arguments)));
    if (parameters == NULL) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      pcsPty(take_stri(arg_1(arguments)), parameters,
               &childStdin_variable->value.fileValue,
               &childStdout_variable->value.fileValue);
      FREE_RTL_ARRAY(parameters, ARRAY_LENGTH(parameters));
    } /* if */
    return SYS_EMPTY_OBJECT;
  } /* pcs_pty */



objectType pcs_start (listType arguments)

  {
    rtlArrayType parameters;
    processType process;

  /* pcs_start */
    isit_stri(arg_1(arguments));
    isit_array(arg_2(arguments));
    isit_file(arg_3(arguments));
    isit_file(arg_4(arguments));
    isit_file(arg_5(arguments));
    parameters = gen_rtl_array(take_array(arg_2(arguments)));
    if (parameters == NULL) {
      return raise_exception(SYS_MEM_EXCEPTION);
    } else {
      process = pcsStart(take_stri(arg_1(arguments)), parameters,
                         take_file(arg_3(arguments)),
                         take_file(arg_4(arguments)),
                         take_file(arg_5(arguments)));
      FREE_RTL_ARRAY(parameters, ARRAY_LENGTH(parameters));
    } /* if */
    return bld_process_temp(process);
  } /* pcs_start */



/**
 *  Convert a 'process' to a string.
 *  The process is converted to a string with the process identifier (PID).
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR  Not enough memory to represent the result.
 */
objectType pcs_str (listType arguments)

  { /* pcs_str */
    isit_process(arg_1(arguments));
    return bld_stri_temp(
        pcsStr(take_process(arg_1(arguments))));
  } /* pcs_str */



/**
 *  Get 'process' value of the object referenced by 'aReference/arg_1'.
 *  @return the 'process' value of the referenced object.
 *  @exception RANGE_ERROR When 'aReference/arg_1' is NIL or
 *             category(aReference) <> PROCESSOBJECT holds.
 */
objectType pcs_value (listType arguments)

  {
    objectType aReference;
    processType process_value;

  /* pcs_value */
    isit_reference(arg_1(arguments));
    aReference = take_reference(arg_1(arguments));
    if (unlikely(aReference == NULL ||
                 CATEGORY_OF_OBJ(aReference) != PROCESSOBJECT)) {
      logError(printf("pcs_value(");
               trace1(aReference);
               printf("): Category is not PROCESSOBJECT.\n"););
      return raise_exception(SYS_RNG_EXCEPTION);
    } else {
      process_value = take_process(aReference);
      if (process_value != NULL) {
        process_value->usage_count++;
      } /* if */
      return bld_process_temp(process_value);
    } /* if */
  } /* pcs_value */



/**
 *  Wait until the specified child process has terminated.
 *  Suspend the execution of the calling process until the
 *  specified child has terminated.
 */
objectType pcs_wait_for (listType arguments)

  { /* pcs_wait_for */
    isit_process(arg_1(arguments));
    pcsWaitFor(take_process(arg_1(arguments)));
    return SYS_EMPTY_OBJECT;
  } /* pcs_wait_for */

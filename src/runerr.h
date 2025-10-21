/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000, 2010 - 2011, 2014 - 2017             */
/*                Thomas Mertes                                     */
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
/*  Module: Runtime                                                 */
/*  File: seed7/src/runerr.h                                        */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Runtime error and exception handling functions.        */
/*                                                                  */
/********************************************************************/

EXTERN objectType curr_action_object;
EXTERN objectType curr_exec_object;
EXTERN listType curr_argument_list;
EXTERN boolType fail_flag;
EXTERN objectType fail_value;
EXTERN listType fail_stack;

#ifdef DO_INIT
listType fail_expression = NULL;
striType fail_expr_stri = NULL;
fileNumType fail_file_number = 0;
lineNumType fail_line_number = 0;
#else
EXTERN listType fail_expression;
EXTERN striType fail_expr_stri;
EXTERN fileNumType fail_file_number;
EXTERN lineNumType fail_line_number;
#endif

#ifdef DO_INIT
boolType catch_exceptions = TRUE;
#else
EXTERN boolType catch_exceptions;
#endif

#define set_fail_flag(failValue) interrupt_flag = (failValue); fail_flag = (failValue);
/* printf("set_fail_flag(%d) at %s(%d)\n", failValue, __FILE__, __LINE__); */

typedef struct {
    boolType fail_flag;
    objectType fail_value;
    listType fail_expression;
    striType fail_expr_stri;
    listType fail_stack;
    fileNumType fail_file_number;
    lineNumType fail_line_number;
  } failStateStruct;


#define category_required(cat,arg) categoryRequired(cat, arg, __FILE__, __LINE__)
#define empty_value(arg) emptyValue(arg, __FILE__, __LINE__)
#define var_required(arg) varRequired(arg, __FILE__, __LINE__)

void categoryRequired (objectCategory required, objectType argument,
                       const_cstriType fileName, int line);
void emptyValue (objectType argument,
                  const_cstriType fileName, int line);
void varRequired (objectType argument,
                  const_cstriType fileName, int line);
void write_call_stack (const_listType stack_elem);
void uncaught_exception (progType aProg);
void write_exception_info (void);
objectType raise_with_obj_and_args (objectType exception,
                                    objectType object, listType list);
objectType raise_with_arguments (objectType exception, listType list);
objectType raise_exception (objectType exception);
void leaveExceptionHandling (void);
void saveFailState (failStateStruct *failState);
void restoreFailState (failStateStruct *failState);
errInfoType getErrInfoFromFailValue (objectType failValue);
void show_signal (void);

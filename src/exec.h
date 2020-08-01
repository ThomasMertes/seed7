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
/*  Module: Interpreter                                             */
/*  File: seed7/src/exec.h                                          */
/*  Changes: 1999  Thomas Mertes                                    */
/*  Content: Main interpreter procedures.                           */
/*                                                                  */
/********************************************************************/

#ifdef DO_INIT
volatile boolType interrupt_flag = FALSE;
#else
EXTERN volatile boolType interrupt_flag;
#endif
EXTERN volatile int signal_number;


void doSuspendInterpreter (int signalNum);
objectType exec_object (register objectType object);
objectType exec_call (objectType object);
objectType evaluate (objectType object);
objectType eval_expression (objectType object);
objectType exec_dynamic (listType expr_list);
objectType exec_expr (const progType currentProg, objectType object,
                      errInfoType *err_info);

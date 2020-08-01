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
/*  Module: Runtime                                                 */
/*  File: seed7/src/runerr.h                                        */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Runtime error and exception handling procedures.       */
/*                                                                  */
/********************************************************************/

EXTERN objecttype curr_action_object;
EXTERN objecttype curr_exec_object;
EXTERN listtype curr_argument_list;
EXTERN booltype fail_flag;
EXTERN objecttype fail_value;
EXTERN listtype fail_expression;
EXTERN listtype fail_stack;


#ifdef ANSI_C

void continue_question(void);
void run_error (objectclass, objecttype);
void empty_value (objecttype);
void var_required (objecttype);
objecttype undef_objectclass (objecttype, listtype);
void write_call_stack (const_listtype);
objecttype raise_with_arguments (objecttype, listtype);
objecttype raise_exception (objecttype);
void raise_error (int);

#else

void continue_question();
void run_error ();
void empty_value ();
void var_required ();
objecttype undef_objectclass ();
void write_call_stack ();
objecttype raise_with_arguments ();
objecttype raise_exception ();
void raise_error ();

#endif

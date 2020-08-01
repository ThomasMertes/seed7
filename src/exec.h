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
/*  Module: Interpreter                                             */
/*  File: seed7/src/exec.h                                          */
/*  Changes: 1999  Thomas Mertes                                    */
/*  Content: Main interpreter procedures.                           */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype exec_object (register objecttype);
objecttype exec_call (objecttype);
objecttype evaluate (objecttype);
objecttype eval_expression (objecttype);
objecttype exec_dynamic (listtype);
objecttype exec_expr (progtype, objecttype);
void interpr (progtype);

#else

objecttype exec_object ();
objecttype exec_call ();
objecttype evaluate ();
objecttype exec_dynamic ();
objecttype eval_expression ();
objecttype exec_expr ();
void interpr ();

#endif

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
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/dcllib.h                                        */
/*  Changes: 1999  Thomas Mertes                                    */
/*  Content: Primitive actions to for simple declarations.          */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype dcl_attr (listtype);
objecttype dcl_const (listtype);
objecttype dcl_elements (listtype);
objecttype dcl_fwd (listtype);
objecttype dcl_getfunc (listtype);
objecttype dcl_getobj (listtype);
objecttype dcl_global (listtype);
objecttype dcl_in1var (listtype);
objecttype dcl_in2var (listtype);
objecttype dcl_inout1 (listtype);
objecttype dcl_inout2 (listtype);
objecttype dcl_param_attr (listtype);
objecttype dcl_ref1 (listtype);
objecttype dcl_ref2 (listtype);
objecttype dcl_symb (listtype);
objecttype dcl_val1 (listtype);
objecttype dcl_val2 (listtype);
objecttype dcl_var (listtype);

#else

objecttype dcl_attr ();
objecttype dcl_const ();
objecttype dcl_elements ();
objecttype dcl_fwd ();
objecttype dcl_getfunc ();
objecttype dcl_getobj ();
objecttype dcl_global ();
objecttype dcl_in1var ();
objecttype dcl_in2var ();
objecttype dcl_inout1 ();
objecttype dcl_inout2 ();
objecttype dcl_param_attr ();
objecttype dcl_ref1 ();
objecttype dcl_ref2 ();
objecttype dcl_symb ();
objecttype dcl_val1 ();
objecttype dcl_val2 ();
objecttype dcl_var ();

#endif

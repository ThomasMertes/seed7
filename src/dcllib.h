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
/*  Module: Library                                                 */
/*  File: seed7/src/dcllib.h                                        */
/*  Changes: 1999  Thomas Mertes                                    */
/*  Content: Primitive actions to for simple declarations.          */
/*                                                                  */
/********************************************************************/

objectType dcl_attr       (listType arguments);
objectType dcl_const      (listType arguments);
objectType dcl_elements   (listType arguments);
objectType dcl_fwd        (listType arguments);
objectType dcl_fwdvar     (listType arguments);
objectType dcl_getfunc    (listType arguments);
objectType dcl_getobj     (listType arguments);
objectType dcl_global     (listType arguments);
objectType dcl_in1        (listType arguments);
objectType dcl_in2        (listType arguments);
objectType dcl_in1var     (listType arguments);
objectType dcl_in2var     (listType arguments);
objectType dcl_inout1     (listType arguments);
objectType dcl_inout2     (listType arguments);
objectType dcl_param_attr (listType arguments);
objectType dcl_ref1       (listType arguments);
objectType dcl_ref2       (listType arguments);
objectType dcl_symb       (listType arguments);
objectType dcl_syntax     (listType arguments);
objectType dcl_val1       (listType arguments);
objectType dcl_val2       (listType arguments);
objectType dcl_var        (listType arguments);

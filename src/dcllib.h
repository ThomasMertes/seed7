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

objecttype dcl_attr       (listtype arguments);
objecttype dcl_const      (listtype arguments);
objecttype dcl_elements   (listtype arguments);
objecttype dcl_fwd        (listtype arguments);
objecttype dcl_fwdvar     (listtype arguments);
objecttype dcl_getfunc    (listtype arguments);
objecttype dcl_getobj     (listtype arguments);
objecttype dcl_global     (listtype arguments);
objecttype dcl_in1var     (listtype arguments);
objecttype dcl_in2var     (listtype arguments);
objecttype dcl_inout1     (listtype arguments);
objecttype dcl_inout2     (listtype arguments);
objecttype dcl_param_attr (listtype arguments);
objecttype dcl_ref1       (listtype arguments);
objecttype dcl_ref2       (listtype arguments);
objecttype dcl_symb       (listtype arguments);
objecttype dcl_val1       (listtype arguments);
objecttype dcl_val2       (listtype arguments);
objecttype dcl_var        (listtype arguments);

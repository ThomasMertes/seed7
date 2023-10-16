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
/*  File: seed7/src/doany.h                                         */
/*  Changes: 1993, 1994  Thomas Mertes                              */
/*  Content: Functions to call several Seed7 functions from C.      */
/*                                                                  */
/********************************************************************/

objectType exec1 (listType list);
boolType do_flush (objectType outfile);
boolType do_wrnl (objectType outfile);
boolType do_wrstri (objectType outfile, striType stri);
boolType do_wrcstri (objectType outfile, const_cstriType stri);
void init_do_any (void);

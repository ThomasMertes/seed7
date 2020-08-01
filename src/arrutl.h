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
/*  File: seed7/src/arrutl.h                                        */
/*  Changes: 2013  Thomas Mertes                                    */
/*  Content: Procedures to maintain objects of type arraytype.      */
/*                                                                  */
/********************************************************************/

#define ARRAY_LENGTH(anArray) (uinttype) ((anArray)->max_position - (anArray)->min_position + 1)


#ifdef ANSI_C

rtlArraytype gen_rtl_array (arraytype anArray);

#else

rtlArraytype gen_rtl_array ();

#endif

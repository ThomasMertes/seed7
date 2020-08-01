/********************************************************************/
/*                                                                  */
/*  arr_rtl.h     Primitive actions for the array type.             */
/*  Copyright (C) 1989 - 2006  Thomas Mertes                        */
/*                                                                  */
/*  This file is part of the Seed7 Runtime Library.                 */
/*                                                                  */
/*  The Seed7 Runtime Library is free software; you can             */
/*  redistribute it and/or modify it under the terms of the GNU     */
/*  Lesser General Public License as published by the Free Software */
/*  Foundation; either version 2.1 of the License, or (at your      */
/*  option) any later version.                                      */
/*                                                                  */
/*  The Seed7 Runtime Library is distributed in the hope that it    */
/*  will be useful, but WITHOUT ANY WARRANTY; without even the      */
/*  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR */
/*  PURPOSE.  See the GNU Lesser General Public License for more    */
/*  details.                                                        */
/*                                                                  */
/*  You should have received a copy of the GNU Lesser General       */
/*  Public License along with this program; if not, write to the    */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/arr_rtl.h                                       */
/*  Changes: 1991, 1992, 1993, 1994, 2005, 2006  Thomas Mertes      */
/*  Content: Primitive actions for the array type.                  */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

void arrAppend (rtlArraytype *arr_variable, rtlArraytype arr_from);
rtlArraytype arrArrlit2 (inttype start_position, rtlArraytype arr1);
rtlArraytype arrBaselit (rtlObjecttype element);
rtlArraytype arrBaselit2 (inttype start_position, rtlObjecttype element);
rtlArraytype arrCat (rtlArraytype arr1, rtlArraytype arr2);
rtlArraytype arrExtend (rtlArraytype arr1, rtlObjecttype element);
rtlArraytype arrGen (rtlObjecttype element1, rtlObjecttype element2);
rtlArraytype arrHead (rtlArraytype arr1, inttype stop);
rtlArraytype arrHeadTemp (rtlArraytype *arr_temp, inttype stop);
rtlGenerictype arrIdxTemp (rtlArraytype *arr_temp, inttype pos);
rtlArraytype arrRange (rtlArraytype arr1, inttype start, inttype stop);
rtlArraytype arrRangeTemp (rtlArraytype *arr_temp, inttype start, inttype stop);
rtlGenerictype arrRemove (rtlArraytype *arr_to, inttype position);
rtlArraytype arrSort (rtlArraytype arr1, inttype cmp_func (rtlGenerictype, rtlGenerictype));
rtlArraytype arrTail (rtlArraytype arr1, inttype start);
rtlArraytype arrTailTemp (rtlArraytype *arr_temp, inttype start);

#else

void arrAppend ();
rtlArraytype arrArrlit2 ();
rtlArraytype arrBaselit ();
rtlArraytype arrBaselit2 ();
rtlArraytype arrCat ();
rtlArraytype arrExtend ();
rtlArraytype arrGen ();
rtlArraytype arrHead ();
rtlArraytype arrHeadTemp ();
rtlGenerictype arrIdxTemp ();
rtlArraytype arrRange ();
rtlArraytype arrRangeTemp ();
rtlGenerictype arrRemove ();
rtlArraytype arrSort ();
rtlArraytype arrTail ();
rtlArraytype arrTailTemp ();

#endif

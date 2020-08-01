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
/*  Free Software Foundation, Inc., 59 Temple Place, Suite 330,     */
/*  Boston, MA 02111-1307 USA                                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/arr_rtl.h                                       */
/*  Changes: 1991, 1992, 1993, 1994, 2005, 2006  Thomas Mertes      */
/*  Content: Primitive actions for the array type.                  */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

void arrAppend (rtlArraytype *, rtlArraytype);
rtlArraytype arrArrlit (rtlArraytype);
rtlArraytype arrArrlit2 (inttype, rtlArraytype);
rtlArraytype arrBaselit (rtlObjecttype);
rtlArraytype arrBaselit2 (inttype, rtlObjecttype);
rtlArraytype arrCat (rtlArraytype, rtlArraytype);
rtlArraytype arrExtend (rtlArraytype, rtlObjecttype);
rtlArraytype arrGen (rtlObjecttype, rtlObjecttype);
rtlArraytype arrHead (rtlArraytype, inttype);
rtlArraytype arrRange (rtlArraytype, inttype, inttype);
rtlArraytype arrSort (rtlArraytype, inttype (rtlGenerictype, rtlGenerictype));
rtlArraytype arrTail (rtlArraytype, inttype);

#else

void arrAppend ();
rtlArraytype arrArrlit ();
rtlArraytype arrArrlit2 ();
rtlArraytype arrBaselit ();
rtlArraytype arrBaselit2 ();
rtlArraytype arrCat ();
rtlArraytype arrExtend ();
rtlArraytype arrGen ();
rtlArraytype arrHead ();
rtlArraytype arrRange ();
rtlArraytype arrSort ();
rtlArraytype arrTail ();

#endif

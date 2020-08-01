/********************************************************************/
/*                                                                  */
/*  big_rtl.h     Primitive actions for the bigInteger type.        */
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
/*  File: seed7/src/big_rtl.h                                       */
/*  Changes: 2005, 2006  Thomas Mertes                              */
/*  Content: Primitive actions for the bigInteger type.             */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

biginttype bigAbs (biginttype);
biginttype bigAdd (biginttype, biginttype);
stritype bigCLit (biginttype);
inttype bigCmp (biginttype, biginttype);
void bigCpy (biginttype *, biginttype);
void bigDecr (biginttype *);
void bigDestr (biginttype);
biginttype bigDiv (biginttype, biginttype);
booltype bigEq (biginttype, biginttype);
void bigGrow (biginttype *, biginttype);
void bigIncr (biginttype *);
biginttype bigIPow (biginttype, inttype);
biginttype bigMDiv (biginttype, biginttype);
biginttype bigMinus (biginttype);
biginttype bigMod (biginttype, biginttype);
biginttype bigMult (biginttype, biginttype);
booltype bigNe (biginttype, biginttype);
biginttype bigParse (stritype);
biginttype bigPred (biginttype);
biginttype bigRand (biginttype, biginttype);
biginttype bigRem (biginttype, biginttype);
biginttype bigSbtr (biginttype, biginttype);
void bigShrink (biginttype *, biginttype);
stritype bigStr (biginttype);
biginttype bigSucc (biginttype);

#else

biginttype bigAbs ();
biginttype bigAdd ();
stritype bigCLit ();
inttype bigCmp ();
void bigCpy ();
void bigDecr ();
void bigDestr ();
biginttype bigDiv ();
booltype bigEq ();
void bigGrow ();
void bigIncr ();
biginttype bigIPow ();
biginttype bigMDiv ();
biginttype bigMinus ();
biginttype bigMod ();
biginttype bigMult ();
booltype bigNe ();
biginttype bigParse ();
biginttype bigPred ();
biginttype bigRand ();
biginttype bigRem ();
biginttype bigSbtr ();
void bigShrink ();
stritype bigStr ();
biginttype bigSucc ();

#endif

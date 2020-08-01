/********************************************************************/
/*                                                                  */
/*  str_rtl.h     Primitive actions for the string type.            */
/*  Copyright (C) 1989 - 2005  Thomas Mertes                        */
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
/*  File: seed7/src/str_rtl.h                                       */
/*  Changes: 1991, 1992, 1993, 1994, 2005  Thomas Mertes            */
/*  Content: Primitive actions for the string type.                 */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

void strAppend (stritype *, stritype);
inttype strCompare (stritype, stritype);
stritype strConcat (stritype, stritype);
void strCopy (stritype *, stritype);
stritype strCreate (stritype);
void strDestr (stritype);
booltype strGe (stritype, stritype);
booltype strGt (stritype, stritype);
inttype strHashCode (stritype);
stritype strHead (stritype, inttype);
inttype strIpos (stritype, stritype, inttype);
booltype strLe (stritype, stritype);
stritype strLit (stritype);
stritype strLow (stritype);
stritype strLpad (stritype, inttype);
booltype strLt (stritype, stritype);
stritype strMult (stritype, inttype);
inttype strPos (stritype, stritype);
stritype strRange (stritype, inttype, inttype);
stritype strRepl (stritype, stritype, stritype);
stritype strRpad (stritype, inttype);
inttype strRpos (stritype, stritype);
stritype strSubstr (stritype, inttype, inttype);
stritype strTail (stritype, inttype);
stritype strTrim (stritype);
stritype strUp (stritype);

#else

void strAppend ();
inttype strCompare ();
stritype strConcat ();
void strCopy ();
stritype strCreate ();
void strDestr ();
booltype strGe ();
booltype strGt ();
inttype strHashCode ();
stritype strHead ();
inttype strIpos ();
booltype strLe ();
stritype strLit ();
stritype strLow ();
stritype strLpad ();
booltype strLt ();
stritype strMult ();
inttype strPos ();
stritype strRange ();
stritype strRepl ();
stritype strRpad ();
inttype strRpos ();
stritype strSubstr ();
stritype strTail ();
stritype strTrim ();
stritype strUp ();

#endif

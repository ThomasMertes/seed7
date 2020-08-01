/********************************************************************/
/*                                                                  */
/*  str_rtl.h     Primitive actions for the string type.            */
/*  Copyright (C) 1989 - 2008  Thomas Mertes                        */
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
/*  File: seed7/src/str_rtl.h                                       */
/*  Changes: 1991, 1992, 1993, 1994, 2005  Thomas Mertes            */
/*  Content: Primitive actions for the string type.                 */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

void strAppend (stritype *const stri_to, const const_stritype stri_from);
inttype strChIpos (const const_stritype, const chartype, const inttype);
inttype strChPos (const const_stritype, const chartype);
/* rtlArraytype strChSplit (const const_stritype, const chartype); */
stritype strCLit (const const_stritype);
inttype strCompare (const const_stritype, const const_stritype);
stritype strConcat (const const_stritype, const const_stritype);
stritype strConcatTemp (stritype, const const_stritype);
void strCopy (stritype *const, const const_stritype);
stritype strCreate (const const_stritype);
void strDestr (const stritype);
stritype strEmpty (void);
booltype strGe (const const_stritype, const const_stritype);
stritype strGetenv (const const_stritype stri);
booltype strGt (const const_stritype, const const_stritype);
inttype strHashCode (const const_stritype);
stritype strHead (const const_stritype, const inttype);
inttype strIpos (const const_stritype, const const_stritype, const inttype);
booltype strLe (const const_stritype, const const_stritype);
stritype strLit (const const_stritype);
stritype strLow (const const_stritype);
stritype strLowTemp (const stritype);
stritype strLpad (const const_stritype, const inttype);
stritype strLpad0 (const const_stritype stri, const inttype pad_size);
stritype strLpad0Temp (const stritype stri, const inttype pad_size);
booltype strLt (const const_stritype, const const_stritype);
stritype strMult (const const_stritype, const inttype);
inttype strPos (const const_stritype, const const_stritype);
stritype strRange (const const_stritype, inttype, inttype);
inttype strRChPos (const const_stritype, const chartype);
stritype strRepl (const const_stritype, const const_stritype, const const_stritype);
stritype strRpad (const const_stritype, const inttype);
inttype strRpos (const const_stritype, const const_stritype);
/* rtlArraytype strSplit (const const_stritype, const const_stritype); */
stritype strSubstr (const const_stritype, inttype, inttype);
stritype strTail (const const_stritype, inttype);
stritype strToUtf8 (const const_stritype stri);
stritype strTrim (const const_stritype stri);
stritype strUp (const const_stritype);
stritype strUpTemp (const stritype);
stritype strUtf8ToStri (const_stritype stri8);

#else

void strAppend ();
inttype strChIpos ();
inttype strChPos ();
/* rtlArraytype strChSplit (); */
stritype strCLit ();
inttype strCompare ();
stritype strConcat ();
stritype strConcatTemp ();
void strCopy ();
stritype strCreate ();
void strDestr ();
stritype strEmpty ();
booltype strGe ();
stritype strGetenv ();
booltype strGt ();
inttype strHashCode ();
stritype strHead ();
inttype strIpos ();
booltype strLe ();
stritype strLit ();
stritype strLow ();
stritype strLowTemp ();
stritype strLpad ();
stritype strLpad0 ();
stritype strLpad0Temp ();
booltype strLt ();
stritype strMult ();
inttype strPos ();
stritype strRange ();
inttype strRChPos ();
stritype strRepl ();
stritype strRpad ();
inttype strRpos ();
/* rtlArraytype strSplit (); */
stritype strSubstr ();
stritype strTail ();
stritype strToUtf8 ();
stritype strTrim ();
stritype strUp ();
stritype strUpTemp ();
stritype strUtf8ToStri ();

#endif

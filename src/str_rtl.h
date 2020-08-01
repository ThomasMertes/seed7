/********************************************************************/
/*                                                                  */
/*  str_rtl.h     Primitive actions for the string type.            */
/*  Copyright (C) 1989 - 2014  Thomas Mertes                        */
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
/*  Changes: 1991 - 1994, 2005, 2008 - 2014  Thomas Mertes          */
/*  Content: Primitive actions for the string type.                 */
/*                                                                  */
/********************************************************************/

/**
 *  Macro to compute the hashcode of a string.
 *  A corresponding macro is inlined by the compiler. If this macro
 *  is changed the code in the compiler must be changed as well.
 */
#define hashCode(stri) (inttype) ((stri)->size == 0 ? 0 : \
                                     ((stri)->mem[0] << 5 ^ \
                                      (stri)->mem[(stri)->size >> 1] << 3 ^ \
                                      (stri)->mem[(stri)->size - 1] << 1 ^ \
                                      (stri)->size))


void toLower (const strelemtype *const stri, memsizetype length,
    strelemtype *const dest);
void toUpper (const strelemtype *const stri, memsizetype length,
    strelemtype *const dest);
stritype concat_path (const const_stritype absolutePath,
    const const_stritype relativePath);
void strAppend (stritype *const destination, const_stritype stri_from);
void strAppendTemp (stritype *const destination, const stritype stri_from);
inttype strChIPos (const const_stritype main_stri, const chartype searched,
    const inttype from_index);
stritype strChMult (const chartype ch, const inttype factor);
inttype strChPos (const const_stritype main_stri, const chartype searched);
/* rtlArraytype strChSplit (const const_stritype main_stri,
    const chartype delimiter); */
stritype strCLit (const const_stritype stri);
inttype strCompare (const const_stritype stri1, const const_stritype stri2);
stritype strConcat (const const_stritype stri1, const const_stritype stri2);
stritype strConcatN (const const_stritype striArray[], memsizetype arraySize);
stritype strConcatTemp (stritype stri1, const const_stritype stri2);
void strCopy (stritype *const stri_to, const const_stritype stri_from);
stritype strCreate (const const_stritype stri_from);
void strDestr (const const_stritype old_string);
stritype strEmpty (void);
booltype strGe (const const_stritype stri1, const const_stritype stri2);
booltype strGt (const const_stritype stri1, const const_stritype stri2);
inttype strHashCode (const const_stritype stri);
#ifdef ALLOW_STRITYPE_SLICES
void strHeadSlice (const const_stritype stri, const inttype stop, stritype slice);
#else
stritype strHead (const const_stritype stri, const inttype stop);
#endif
stritype strHeadTemp (const stritype stri, const inttype stop);
inttype strIPos (const const_stritype main_stri, const const_stritype searched,
    const inttype from_index);
booltype strLe (const const_stritype stri1, const const_stritype stri2);
stritype strLit (const const_stritype stri);
stritype strLow (const const_stritype stri);
stritype strLowTemp (const stritype stri);
stritype strLpad (const const_stritype stri, const inttype pad_size);
stritype strLpadTemp (const stritype stri, const inttype pad_size);
stritype strLpad0 (const const_stritype stri, const inttype pad_size);
stritype strLpad0Temp (const stritype stri, const inttype pad_size);
booltype strLt (const const_stritype stri1, const const_stritype stri2);
stritype strLtrim (const const_stritype stri);
stritype strMult (const const_stritype stri, const inttype factor);
inttype strPos (const const_stritype main_stri, const const_stritype searched);
void strPush (stritype *const destination, const chartype extension);
#ifdef ALLOW_STRITYPE_SLICES
void strRangeSlice (const const_stritype stri, inttype start, inttype stop, stritype slice);
#else
stritype strRange (const const_stritype stri, inttype start, inttype stop);
#endif
inttype strRChIPos (const const_stritype main_stri, const chartype searched,
    const inttype from_index);
inttype strRChPos (const const_stritype main_stri, const chartype searched);
stritype strRepl (const const_stritype main_stri,
    const const_stritype searched, const const_stritype replace);
inttype strRIPos (const const_stritype main_stri, const const_stritype searched,
    const inttype from_index);
stritype strRpad (const const_stritype stri, const inttype pad_size);
inttype strRPos (const const_stritype main_stri, const const_stritype searched);
stritype strRtrim (const const_stritype stri);
/* rtlArraytype strSplit (const const_stritype main_stri,
    const const_stritype delimiter); */
#ifdef ALLOW_STRITYPE_SLICES
void strSubstrSlice (const const_stritype stri, inttype start, inttype len, stritype slice);
#else
stritype strSubstr (const const_stritype stri, inttype start, inttype len);
#endif
#ifdef ALLOW_STRITYPE_SLICES
void strTailSlice (const const_stritype stri, inttype start, stritype slice);
#else
stritype strTail (const const_stritype stri, inttype start);
#endif
stritype strToUtf8 (const const_stritype stri);
stritype strTrim (const const_stritype stri);
stritype strUp (const const_stritype stri);
stritype strUpTemp (const stritype stri);
stritype strUtf8ToStri (const_stritype utf8);
stritype strZero (const inttype factor);

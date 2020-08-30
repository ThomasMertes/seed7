/********************************************************************/
/*                                                                  */
/*  str_rtl.h     Primitive actions for the string type.            */
/*  Copyright (C) 1989 - 2015  Thomas Mertes                        */
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
/*  Changes: 1991 - 1994, 2005, 2008 - 2015  Thomas Mertes          */
/*  Content: Primitive actions for the string type.                 */
/*                                                                  */
/********************************************************************/

/**
 *  Macro to compute the hashcode of a string.
 *  A corresponding macro is inlined by the compiler. If this macro
 *  is changed the code in the compiler must be changed as well.
 */
#define hashCode(stri) (intType) ((stri)->size == 0 ? 0 : \
                                     ((stri)->mem[0] << 5 ^ \
                                      (stri)->mem[(stri)->size >> 1] << 3 ^ \
                                      (stri)->mem[(stri)->size - 1] << 1 ^ \
                                      (stri)->size))


void toLower (const strElemType *const stri, memSizeType length,
    strElemType *const dest);
void toUpper (const strElemType *const stri, memSizeType length,
    strElemType *const dest);
striType concatPath (const const_striType absolutePath,
    const const_striType relativePath);
void strAppend (striType *const destination, const_striType extension);
void strAppendN (striType *const destination,
    const const_striType extensionArray[], memSizeType arraySize);
void strAppendTemp (striType *const destination, const striType extension);
striType strChChRepl (const const_striType mainStri,
    const charType searched, const charType replacement);
intType strChIPos (const const_striType mainStri, const charType searched,
    const intType fromIndex);
striType strChMult (const charType ch, const intType factor);
intType strChPos (const const_striType mainStri, const charType searched);
striType strChRepl (const const_striType mainStri,
    const charType searched, const const_striType replacement);
/* rtlArrayType strChSplit (const const_striType mainStri,
    const charType delimiter); */
striType strCLit (const const_striType stri);
intType strCompare (const const_striType stri1, const const_striType stri2);
striType strConcat (const const_striType stri1, const const_striType stri2);
striType strConcatN (const const_striType striArray[], memSizeType arraySize);
striType strConcatTemp (striType stri1, const const_striType stri2);
void strCopy (striType *const dest, const const_striType source);
striType strCreate (const const_striType source);
void strDestr (const const_striType old_string);
striType strEmpty (void);
boolType strGe (const const_striType stri1, const const_striType stri2);
boolType strGt (const const_striType stri1, const const_striType stri2);
intType strHashCode (const const_striType stri);
#if ALLOW_STRITYPE_SLICES
void strHeadSlice (const const_striType stri, const intType stop, striType slice);
#endif
striType strHead (const const_striType stri, const intType stop);
striType strHeadTemp (const striType stri, const intType stop);
intType strIPos (const const_striType mainStri, const const_striType searched,
    const intType fromIndex);
boolType strLe (const const_striType stri1, const const_striType stri2);
striType strLit (const const_striType stri);
striType strLow (const const_striType stri);
striType strLowTemp (const striType stri);
striType strLpad (const const_striType stri, const intType padSize);
striType strLpadTemp (const striType stri, const intType padSize);
striType strLpad0 (const const_striType stri, const intType padSize);
striType strLpad0Temp (const striType stri, const intType padSize);
boolType strLt (const const_striType stri1, const const_striType stri2);
striType strLtrim (const const_striType stri);
striType strMult (const const_striType stri, const intType factor);
intType strPos (const const_striType mainStri, const const_striType searched);
void strPush (striType *const destination, const charType extension);
#if ALLOW_STRITYPE_SLICES
void strRangeSlice (const const_striType stri, intType start, intType stop, striType slice);
#endif
striType strRange (const const_striType stri, intType start, intType stop);
intType strRChIPos (const const_striType mainStri, const charType searched,
    const intType fromIndex);
intType strRChPos (const const_striType mainStri, const charType searched);
striType strRepl (const const_striType mainStri,
    const const_striType searched, const const_striType replace);
intType strRIPos (const const_striType mainStri, const const_striType searched,
    const intType fromIndex);
striType strRpad (const const_striType stri, const intType padSize);
intType strRPos (const const_striType mainStri, const const_striType searched);
striType strRtrim (const const_striType stri);
/* rtlArrayType strSplit (const const_striType main_stri,
    const const_striType delimiter); */
#if ALLOW_STRITYPE_SLICES
void strSubstrSlice (const const_striType stri, intType start, intType len, striType slice);
#endif
striType strSubstr (const const_striType stri, intType start, intType len);
#if ALLOW_STRITYPE_SLICES
void strTailSlice (const const_striType stri, intType start, striType slice);
#endif
striType strTail (const const_striType stri, intType start);
striType strTailTemp (const striType stri, intType start);
striType strToUtf8 (const const_striType stri);
striType strTrim (const const_striType stri);
striType strUp (const const_striType stri);
striType strUpTemp (const striType stri);
striType strUtf8ToStri (const const_striType utf8);
striType strZero (const intType factor);

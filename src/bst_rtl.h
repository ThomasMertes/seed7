/********************************************************************/
/*                                                                  */
/*  bst_rtl.h     Primitive actions for the byte string type.       */
/*  Copyright (C) 1989 - 2010  Thomas Mertes                        */
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
/*  File: seed7/src/bst_rtl.h                                       */
/*  Changes: 2007, 2010  Thomas Mertes                              */
/*  Content: Primitive actions for the byte string type.            */
/*                                                                  */
/********************************************************************/

/**
 *  Macro to compute the hashcode of a bstring.
 *  A corresponding macro is inlined by the compiler. If this macro
 *  is changed the code in the compiler must be changed as well.
 */
#define bstringHashCode(bstri) (intType) ((bstri)->size == 0 ? 0 : \
                                             ((memSizeType) (bstri)->mem[0] << 5 ^ \
                                                            (bstri)->size << 3 ^ \
                                                            (bstri)->mem[(bstri)->size - 1]))


void bstAppend (bstriType *const destination, const_bstriType extension);
bstriType bstCat (const const_bstriType bstri1, const const_bstriType bstri2);
intType bstCmp (const const_bstriType bstri1, const const_bstriType bstri2);
void bstCpy (bstriType *const dest, const const_bstriType source);
bstriType bstCreate (const const_bstriType source);
void bstDestr (const const_bstriType old_bstring);
intType bstHashCode (const const_bstriType bstri);
bstriType bstParse (const const_striType stri);
striType bstStr (const const_bstriType bstri);
bstriType bstTail (const const_bstriType stri, intType start);

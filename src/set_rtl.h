/********************************************************************/
/*                                                                  */
/*  set_rtl.h     Primitive actions for the set type.               */
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
/*  File: seed7/src/set_rtl.h                                       */
/*  Changes: 2004, 2005, 2010, 2012 - 2014  Thomas Mertes           */
/*  Content: Primitive actions for the set type.                    */
/*                                                                  */
/********************************************************************/

#define byte_sft(b) ((b)==1?3:((b)==2?4:((b)==4?5:((b)==8?6:((b)==16?7:8)))))
#define bitset_shift byte_sft(sizeof(bitSetType))
#define bitset_mask  ((1 << bitset_shift) - 1)

#if RSHIFT_DOES_SIGN_EXTEND
#define bitset_pos(number) ((number)>>bitset_shift)
#else
#define bitset_pos(number) ((number)<0?~(~(number)>>bitset_shift):(number)>>bitset_shift)
#endif


setType setBaselit (const intType number);
intType setCard (const const_setType set1);
intType setCmp (const const_setType set1, const const_setType set2);
void setCpy (setType *const dest, const const_setType source);
setType setCreate (const const_setType source);
void setDestr (const const_setType old_set);
setType setDiff (const const_setType set1, const const_setType set2);
boolType setElem (const intType number, const const_setType set1);
setType setEmpty (void);
boolType setEq (const const_setType set1, const const_setType set2);
void setExcl (setType *const set_to, const intType number);
intType setHashCode (const const_setType set1);
setType setIConv (intType number);
void setIncl (setType *const set_to, const intType number);
setType setIntersect (const const_setType set1, const const_setType set2);
boolType setIsEmpty (const const_setType set1);
boolType setIsProperSubset (const const_setType set1, const const_setType set2);
boolType setIsSubset (const const_setType set1, const const_setType set2);
intType setMax (const const_setType set1);
intType setMin (const const_setType set1);
intType setNext (const const_setType set1, const intType number);
intType setRand (const const_setType set1);
setType setRangelit (const intType lowerValue, const intType upperValue);
intType setSConv (const const_setType set1);
setType setSymdiff (const const_setType set1, const const_setType set2);
uintType setToUInt (const const_setType set1, const intType lowestBitNum);
setType setUnion (const const_setType set1, const const_setType set2);

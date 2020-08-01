/********************************************************************/
/*                                                                  */
/*  set_rtl.h     Primitive actions for the set type.               */
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
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/set_rtl.h                                       */
/*  Changes: 2004, 2005  Thomas Mertes                              */
/*  Content: Primitive actions for the set type.                    */
/*                                                                  */
/********************************************************************/

#define byte_sft(b) ((b)==1?3:((b)==2?4:((b)==4?5:((b)==8?6:((b)==16?7:8)))))
#define bitset_shift byte_sft(sizeof(bitsettype))
#define bitset_mask  ((1 << bitset_shift) - 1)

#ifdef RSHIFT_DOES_SIGN_EXTEND
#define bitset_pos(number) ((number)>>bitset_shift)
#else
#define bitset_pos(number) ((number)<0?~(~(number)>>bitset_shift):(number)>>bitset_shift)
#endif


#ifdef ANSI_C

settype setBaselit (const inttype number);
inttype setCard (const const_settype set1);
inttype setCmp (const const_settype set1, const const_settype set2);
void setCpy (settype *const set_to, const const_settype set_from);
settype setCreate (const const_settype set_from);
void setDestr (const settype old_set);
settype setDiff (const const_settype set1, const const_settype set2);
booltype setElem (const inttype number, const const_settype set1);
booltype setEq (const const_settype set1, const const_settype set2);
void setExcl (settype *const set_to, const inttype number);
inttype setHashCode (const const_settype set1);
settype setIConv (inttype number);
void setIncl (settype *const set_to, const inttype number);
settype setIntersect (const const_settype set1, const const_settype set2);
booltype setIsProperSubset (const const_settype set1, const const_settype set2);
booltype setIsSubset (const const_settype set1, const const_settype set2);
inttype setMax (const const_settype set1);
inttype setMin (const const_settype set1);
booltype setNotElem (const inttype number, const const_settype set1);
inttype setRand (const const_settype set1);
inttype setSConv (const const_settype set1);
settype setSymdiff (const const_settype set1, const const_settype set2);
inttype setToInt (const const_settype set1, const inttype lowestBitNum);
settype setUnion (const const_settype set1, const const_settype set2);

#else

settype setBaselit ();
inttype setCard ();
inttype setCmp ();
void setCpy ();
settype setCreate ();
void setDestr ();
settype setDiff ();
booltype setElem ();
booltype setEq ();
void setExcl ();
inttype setHashCode ();
settype setIConv ();
void setIncl ();
settype setIntersect ();
booltype setIsProperSubset ();
booltype setIsSubset ()
inttype setMax ();
inttype setMin ();
booltype setNotElem ();
inttype setRand ();
inttype setSConv ();
settype setSymdiff ();
settype setUnion ();

#endif

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


#ifdef ANSI_C

settype setBaselit (const inttype);
inttype setCard (const const_settype);
inttype setCmp (const const_settype, const const_settype);
void setCpy (settype *const, const const_settype);
settype setCreate (const const_settype);
void setDestr (const const_settype);
settype setDiff (const const_settype, const const_settype);
booltype setElem (const inttype, const const_settype);
booltype setEq (const const_settype, const const_settype);
void setExcl (settype *const, const inttype);
booltype setGe (const const_settype, const const_settype);
booltype setGt (const const_settype, const const_settype);
inttype setHashCode (const const_settype);
void setIncl (settype *const, const inttype);
settype setIntersect (const const_settype, const const_settype);
booltype setLe (const const_settype, const const_settype);
booltype setLt (const const_settype, const const_settype);
inttype setMax (const const_settype);
inttype setMin (const const_settype);
booltype setNe (const const_settype, const const_settype);
booltype setNotElem (const inttype, const const_settype);
inttype setRand (const const_settype);
settype setSymdiff (const const_settype, const const_settype);
settype setUnion (const const_settype, const const_settype);

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
booltype setGe ();
booltype setGt ();
inttype setHashCode ();
void setIncl ();
settype setIntersect ();
booltype setLe ();
booltype setLt ();
inttype setMax ();
inttype setMin ();
booltype setNe ();
booltype setNotElem ();
inttype setRand ();
settype setSymdiff ();
settype setUnion ();

#endif

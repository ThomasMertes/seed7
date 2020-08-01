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
/*  Free Software Foundation, Inc., 59 Temple Place, Suite 330,     */
/*  Boston, MA 02111-1307 USA                                       */
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

inttype setCard (settype);
void setCpy (settype *, settype);
settype setDiff (settype, settype);
booltype setElem (inttype, settype);
booltype setEq (settype, settype);
void setExcl (settype *, inttype);
booltype setGe (settype, settype);
booltype setGt (settype, settype);
void setIncl (settype *, inttype);
settype setIntersect (settype, settype);
booltype setLe (settype, settype);
booltype setLt (settype, settype);
inttype setMax (settype);
inttype setMin (settype);
booltype setNe (settype, settype);
booltype setNotElem (inttype, settype);
inttype setRand (settype);
settype setUnion (settype, settype);

#else

inttype setCard ();
void setCpy ();
settype setDiff ();
booltype setElem ();
booltype setEq ();
void setExcl ();
booltype setGe ();
booltype setGt ();
void setIncl ();
settype setIntersect ();
booltype setLe ();
booltype setLt ();
inttype setMax ();
inttype setMin ();
booltype setNe ();
booltype setNotElem ();
inttype setRand ();
settype setUnion ();

#endif

/********************************************************************/
/*                                                                  */
/*  chr_rtl.h     Primitive actions for the char type.              */
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
/*  File: seed7/src/chr_rtl.h                                       */
/*  Changes: 1992, 1993, 1994, 2005  Thomas Mertes                  */
/*  Content: Primitive actions for the char type.                   */
/*                                                                  */
/********************************************************************/

#if ALLOW_STRITYPE_SLICES
#define chrStrMacro(ch,str) (str.size=1,str.mem=str.mem1,str.mem1[0]=(strElemType)(ch),&str)
#else
#define chrStrMacro(ch,str) (str.size=1,str.mem[0]=(strElemType)(ch),&str)
#endif


striType chrCLit (charType character);
#if ALLOW_STRITYPE_SLICES
striType chrCLitToBuffer (charType character, striType buffer);
#endif
intType chrCmp (charType char1, charType char2);
boolType chrIsLetter (charType ch);
charType chrLow (charType ch);
striType chrStr (charType ch);
charType chrUp (charType ch);
intType chrWidth (charType ch);

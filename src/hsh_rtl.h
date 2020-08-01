/********************************************************************/
/*                                                                  */
/*  hsh_rtl.h     Primitive actions for the string type.            */
/*  Copyright (C) 1989 - 2007  Thomas Mertes                        */
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
/*  File: seed7/src/hsh_rtl.h                                       */
/*  Changes: 2007  Thomas Mertes                                    */
/*  Content: Primitive actions for the hash type.                   */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

booltype hshContains (const const_rtlHashtype, const rtlGenerictype,
                      inttype, comparetype);
void hshCpy (rtlHashtype *const, const const_rtlHashtype,
             createfunctype, createfunctype, destrfunctype, destrfunctype);
rtlHashtype hshCreate (const const_rtlHashtype, createfunctype, createfunctype,
                       destrfunctype, destrfunctype);
void hshDestr (const const_rtlHashtype, destrfunctype, destrfunctype);
rtlHashtype hshEmpty (void);
void hshExcl (const rtlHashtype, const rtlGenerictype, inttype,
              comparetype, destrfunctype, destrfunctype);
rtlGenerictype hshIdx (const const_rtlHashtype, const rtlGenerictype, inttype, comparetype);
rtlObjecttype *hshIdxAddr (const const_rtlHashtype, const rtlGenerictype, inttype, comparetype);
void hshIncl (const rtlHashtype, rtlGenerictype, rtlGenerictype , inttype,
	      comparetype, createfunctype, createfunctype, copyfunctype);
rtlArraytype hshKeys (const const_rtlHashtype, createfunctype, destrfunctype);
rtlArraytype hshValues (const const_rtlHashtype, createfunctype, destrfunctype);

#else

booltype hshContains ();
void hshCpy ();
rtlHashtype hshCreate ();
void hshDestr ();
rtlHashtype hshEmpty ();
void hshExcl ();
rtlGenerictype hshIdx ();
rtlObjecttype *hshIdxAddr ();
void hshIncl ();
rtlArraytype hshKeys ();
rtlArraytype hshValues ();

#endif

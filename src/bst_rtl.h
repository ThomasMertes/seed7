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

#ifdef ANSI_C

void bstAppend (bstritype *const bstri_to, const_bstritype bstri_from);
bstritype bstCat (const const_bstritype bstri1, const const_bstritype bstri2);
inttype bstCmp (const const_bstritype bstri1, const const_bstritype bstri2);
void bstCpy (bstritype *const bstri_to, const const_bstritype bstri_from);
bstritype bstCreate (const const_bstritype bstri_from);
void bstDestr (const bstritype old_bstring);
inttype bstHashCode (const const_bstritype bstri);
bstritype bstParse (const const_stritype stri);
stritype bstStr (const const_bstritype bstri);

#else

void bstAppend ();
bstritype bstCat ();
inttype bstCmp ();
void bstCpy ();
bstritype bstCreate ();
void bstDestr ();
inttype bstHashCode ();
bstritype bstParse ();
stritype bstStr ();

#endif

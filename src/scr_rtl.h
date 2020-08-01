/********************************************************************/
/*                                                                  */
/*  scr_rtl.h     Primitive actions for screen output.              */
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
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/scr_rtl.h                                       */
/*  Changes: 2007  Thomas Mertes                                    */
/*  Content: Primitive actions for screen output.                   */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

void scrHScroll (inttype, inttype, inttype, inttype, inttype);
void scrSetpos (inttype, inttype);
void scrVScroll (inttype, inttype, inttype, inttype, inttype);
void scrWrite (stritype);

#else

void scrHScroll ();
void scrSetpos ();
void scrVScroll ();
void scrWrite ();

#endif

/********************************************************************/
/*                                                                  */
/*  ut8_rtl.h     Primitive actions for the UTF-8 file type.        */
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
/*  File: seed7/src/ut8_rtl.h                                       */
/*  Changes: 2005  Thomas Mertes                                    */
/*  Content: Primitive actions for the UTF-8 file type.             */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

chartype ut8Getc (filetype aFile);
stritype ut8Gets (filetype aFile, inttype length);
stritype ut8LineRead (filetype aFile, chartype *termination_char);
void ut8Seek (filetype aFile, inttype file_position);
stritype ut8WordRead (filetype aFile, chartype *termination_char);
void ut8Write (filetype aFile, const const_stritype stri);

#else

chartype ut8Getc ();
stritype ut8Gets ();
stritype ut8LineRead ();
void ut8Seek ();
stritype ut8WordRead ();
void ut8Write ();

#endif

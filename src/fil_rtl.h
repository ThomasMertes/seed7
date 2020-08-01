/********************************************************************/
/*                                                                  */
/*  fil_rtl.h     Primitive actions for the primitive file type.    */
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
/*  File: seed7/src/fil_rtl.h                                       */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: Primitive actions for the primitive file type.         */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

biginttype filBigLng (filetype aFile);
void filBigSeek (filetype aFile, biginttype big_position);
biginttype filBigTell (filetype aFile);
stritype filGets (filetype aFile, inttype length);
booltype filHasNext (filetype aFile);
stritype filLineRead (filetype aFile, chartype *termination_char);
stritype filLit (filetype aFile);
inttype filLng (filetype aFile);
filetype filOpen (stritype file_name, stritype file_mode);
filetype filPopen (stritype command, stritype file_mode);
void filPrint (stritype stri);
void filSeek (filetype aFile, inttype file_position);
void filSetbuf (filetype aFile, inttype mode, inttype size);
inttype filTell (filetype aFile);
stritype filWordRead (filetype aFile, chartype *termination_char);
void filWrite (filetype aFile, stritype stri);

#else

biginttype filBigLng ();
void filBigSeek ();
biginttype filBigTell ();
stritype filGets ();
booltype filHasNext ();
stritype filLineRead ();
stritype filLit ();
inttype filLng ();
filetype filOpen ();
filetype filPopen ();
void filPrint ();
void filSeek ();
void filSetbuf ();
inttype filTell ();
stritype filWordRead ();
void filWrite ();

#endif

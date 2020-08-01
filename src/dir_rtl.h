/********************************************************************/
/*                                                                  */
/*  dir_rtl.h     Primitive actions for the directory type.         */
/*  Copyright (C) 1989 - 2008  Thomas Mertes                        */
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
/*  File: seed7/src/dir_rtl.h                                       */
/*  Changes: 2008  Thomas Mertes                                    */
/*  Content: Primitive actions for the directory type.              */
/*                                                                  */
/********************************************************************/

typedef os_DIR *dirType;

#ifdef OS_STRI_WCHAR
extern wchar_t dot[];
extern wchar_t dotdot[];
extern wchar_t slash[];
#else
extern char dot[];
extern char dotdot[];
extern char slash[];
#endif


void dirClose (dirType directory);
dirType dirOpen (const const_striType file_name);
striType dirRead (dirType directory);

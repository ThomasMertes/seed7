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

#include "dir_drv.h"

#ifdef USE_WOPENDIR
typedef WDIR *dirtype;
typedef struct wdirent os_dirent_struct;
extern wchar_t dot[];
extern wchar_t dotdot[];
extern wchar_t slash[];
#define os_closedir wclosedir
#define os_opendir  wopendir
#define os_readdir  wreaddir
#else
typedef DIR *dirtype;
typedef struct dirent os_dirent_struct;
extern char dot[];
extern char dotdot[];
extern char slash[];
#define os_closedir closedir
#define os_opendir  opendir
#define os_readdir  readdir
#endif


#ifdef ANSI_C

void dirClose (dirtype directory);
dirtype dirOpen (stritype file_name);
stritype dirRead (dirtype directory);

#else

void dirClose ();
dirtype dirOpen ();
stritype dirRead ();

#endif

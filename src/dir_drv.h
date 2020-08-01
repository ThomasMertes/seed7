/********************************************************************/
/*                                                                  */
/*  dir_drv.h     Prototypes of OS specific directory functions.    */
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
/*  File: seed7/src/dir_drv.h                                       */
/*  Changes: 1993, 1994, 2007  Thomas Mertes                        */
/*  Content: Prototypes of OS specific directory functions.         */
/*                                                                  */
/********************************************************************/

/* Note that DIR_LIB has as value one of DIRENT_DIRECTORY,          */
/* DIRECT_DIRECTORY, DIRDOS_DIRECTORY or DIRWIN_DIRECTORY.          */

#if DIR_LIB == DIRENT_DIRECTORY
#include "sys/types.h"
#include "sys/stat.h"
#include "dirent.h"

#elif DIR_LIB == DIRECT_DIRECTORY
#include "sys/types.h"
#include "sys/stat.h"
#include "direct.h"

#elif DIR_LIB == DIRDOS_DIRECTORY
#include "sys/types.h"
#include "sys/stat.h"
#include "direct.h"
#include "dir_dos.h"

#elif DIR_LIB == DIRWIN_DIRECTORY
#include "sys/types.h"
#include "sys/stat.h"
#include "direct.h"
#include "dir_win.h"
#endif

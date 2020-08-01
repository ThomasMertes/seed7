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

/* Note that only one of USE_DIRENT, USE_DIRECT, USE_DIRDOS or      */
/* USE_DIRWIN is active.                                            */

#ifdef USE_DIRENT
#include "sys/types.h"
#include "sys/stat.h"
#include "dirent.h"
#endif

#ifdef USE_DIRECT
#include "sys/types.h"
#include "sys/stat.h"
#include "direct.h"
#endif

#ifdef USE_DIRDOS
#include "sys/types.h"
#include "sys/stat.h"
#include "direct.h"
#include "dir_dos.h"
#endif

#ifdef USE_DIRWIN
#include "sys/types.h"
#include "sys/stat.h"
#include "direct.h"
#include "dir_win.h"
#endif

/********************************************************************/
/*                                                                  */
/*  dir_drv.h     Driver functions for directorys.                  */
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
/*  File: seed7/src/dir_drv.h                                       */
/*  Changes: 1993, 1994, 2007  Thomas Mertes                        */
/*  Content: Driver functions for directorys.                       */
/*                                                                  */
/********************************************************************/

/* Note that only one of USE_DIRENT, USE_DIRECT, USE_DIRDOS or      */
/* USE_DIRWIN is active.                                            */

#ifdef USE_DIRENT
#include "sys/types.h"
#include "sys/stat.h"
#include "dirent.h"

#ifdef OUT_OF_ORDER
#ifdef ANSI_C
#ifdef C_PLUS_PLUS
extern "C" int chdir (char *);
extern "C" char* getcwd (char *, int);
extern "C" int rmdir(char *);
#else
extern int chdir (char *);
extern char* getcwd (char *, int);
extern int rmdir(char *);
#endif
#else
extern int chdir ();
extern char* getcwd ();
extern int rmdir();
#endif
#endif

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


#ifdef MKDIR_WITH_ONE_PARAMETER
#define mkdir(NAME,MODE) mkdir(NAME)
#endif

#ifdef CHOWN_MISSING
#define chown(NAME,UID,GID)
#endif

#ifdef CHMOD_MISSING
#define chmod(NAME,MODE)
#endif

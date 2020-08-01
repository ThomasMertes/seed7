/********************************************************************/
/*                                                                  */
/*  dir_win.h     Directory access using the windows capabilitys.   */
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
/*  File: seed7/src/dir_win.h                                       */
/*  Changes: 1993, 1994, 2007  Thomas Mertes                        */
/*  Content: Directory access using _findfirst and _findnext.       */
/*                                                                  */
/*  Implements opendir, readdir and closedir in the way it is       */
/*  defined in Unix.                                                */
/*                                                                  */
/********************************************************************/

#include "windows.h"

#if MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
#include "vol_drv.h"
#endif


#ifdef OS_STRI_WCHAR

struct wdirent {
    wchar_t *d_name;
  };

typedef struct {
    /* FirstElement can only be 0 or 1. This way a WDIR can never */
    /* start with UINT32TYPE_MAX, which is the magic value of a   */
    /* volumeListType value.                                      */
    int firstElement;
    HANDLE dirHandle;
    WIN32_FIND_DATAW findData;
    struct wdirent dirEntry;
  } WDIR;

WDIR *wopendir (const wchar_t *name);
struct wdirent *wreaddir (WDIR *curr_dir);
int wclosedir (WDIR *curr_dir);


#else


struct dirent {
    char *d_name;
  };

typedef struct {
    /* FirstElement can only be 0 or 1. This way a DIR can never  */
    /* start with UINT32TYPE_MAX, which is the magic value of a   */
    /* volumeListType value.                                      */
    int firstElement;
    HANDLE dirHandle;
    WIN32_FIND_DATAA findData;
    struct dirent dirEntry;
  } DIR;

DIR *opendir (const char *name);
struct dirent *readdir (DIR *curr_dir);
int closedir (DIR *curr_dir);

#endif

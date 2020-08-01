/********************************************************************/
/*                                                                  */
/*  dir_dos.h     Directory functions which call the Dos API.       */
/*  Copyright (C) 1989 - 2014  Thomas Mertes                        */
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
/*  File: seed7/src/dir_dos.h                                       */
/*  Changes: 1993, 1994  Thomas Mertes                              */
/*  Content: Directory access using DOS findfirst and findnext.     */
/*                                                                  */
/*  Implements opendir, readdir and closedir in the way it is       */
/*  defined in Unix.                                                */
/*                                                                  */
/********************************************************************/

#ifdef DIR_WIN_DOS

#include "io.h"

struct dirent {
    char d_name[260];
  };

typedef struct {
    long dir_handle;
    struct _finddata_t find_record;
    int first_element;
    struct dirent dir_entry;
  } DIR;

#else

#include "dos.h"

struct dirent {
    char d_name[260];
  };

typedef struct {
    struct find_t find_record;
    int first_element;
    struct dirent dir_entry;
  } DIR;

#endif


DIR *opendir (char *);
struct dirent *readdir (DIR *);
int closedir (DIR *);

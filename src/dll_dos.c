/********************************************************************/
/*                                                                  */
/*  dll_dos.c     Dynamic link library support (dummy).             */
/*  Copyright (C) 1989 - 2018  Thomas Mertes                        */
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
/*  File: seed7/src/dll_dos.c                                       */
/*  Changes: 2018  Thomas Mertes                                    */
/*  Content: Dynamic link library support (dummy).                  */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdio.h"

#include "common.h"
#include "dll_drv.h"



/**
 *  Open a DLL respectively shared library with the given 'dllName'.
 *  @return a reference to the library, or NULL if it was not found.
 */
void *dllOpen (const char *dllName)

  { /* dllOpen */
    return NULL;
  } /* dllOpen */



/**
 *  Get the function specified with 'symbol' from the library 'dll'.
 *  @return a pointer to the function, or NULL if it was not found.
 */
funcPtrType dllFunc (void *dll, const char *symbol)

  { /* dllFunc*/
    return NULL;
  } /* dllFunc */

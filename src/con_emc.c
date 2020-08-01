/********************************************************************/
/*                                                                  */
/*  con_emc.c     Driver for emcc (JavaScript) console access.      */
/*  Copyright (C) 1989 - 2011  Thomas Mertes                        */
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
/*  File: seed7/src/con_emc.c                                       */
/*  Changes: 2015  Thomas Mertes                                    */
/*  Content: Driver for emcc (JavaScript) console access.           */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "heaputl.h"
#include "ut8_rtl.h"

#include "con_drv.h"



void kbdShut (void)

  { /* kbdShut */
  } /* kbdShut */



boolType kbdKeyPressed (void)

  { /* kbdKeyPressed */
    return FALSE;
  } /* kbdKeyPressed */



charType kbdGetc (void)

  { /* kbdGetc */
    return (charType) EOF;
  } /* kbdGetc */



charType kbdRawGetc (void)

  { /* kbdRawRead */
    return (charType) EOF;
  } /* kbdRawRead */



int conHeight (void)

  { /* conHeight */
    return 0;
  } /* conHeight */



int conWidth (void)

  { /* conWidth */
    return 0;
  } /* conWidth */



void conFlush (void)

  { /* conFlush */
  } /* conFlush */



void conCursor (boolType on)

  { /* conCursor */
  } /* conCursor */



void conSetCursor (intType line, intType column)

  { /* conSetCursor */
  } /* conSetCursor */



/**
 *  Writes the string stri to the console at the current position.
 */
void conWrite (const const_striType stri)

  { /* conWrite */
    ut8Write(stdout, stri);
  } /* conWrite */



void conClear (intType startlin, intType startcol,
    intType stoplin, intType stopcol)

  { /* conClear */
  } /* conClear */



void conUpScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  { /* conUpScroll */
  } /* conUpScroll */



void conDownScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  { /* conDownScroll */
  } /* conDownScroll */



void conLeftScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  { /* conLeftScroll */
  } /* conLeftScroll */



void conRightScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  { /* conRightScroll */
  } /* conRightScroll */



void conShut (void)

  { /* conShut */
  } /* conShut */



int conOpen (void)

  { /* conOpen */
    return 1;
  } /* conOpen */

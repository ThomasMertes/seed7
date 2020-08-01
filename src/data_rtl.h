/********************************************************************/
/*                                                                  */
/*  data_rtl.h    Basic type definitions and settings.              */
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
/*  Free Software Foundation, Inc., 59 Temple Place, Suite 330,     */
/*  Boston, MA 02111-1307 USA                                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/data_rtl.h                                      */
/*  Changes: 1990 - 1994, 2000, 2005  Thomas Mertes                 */
/*  Content: Type definitions for general data structures.          */
/*                                                                  */
/********************************************************************/

typedef struct objectstruct  *objecttype;
typedef struct arraystruct   *arraytype;
typedef struct helemstruct   *helemtype;
typedef struct hashstruct    *hashtype;

typedef struct objectstruct {
    long value;
  } objectrecord;

typedef struct arraystruct {
    inttype min_position;
    inttype max_position;
    objectrecord arr[1];
  } arrayrecord;

typedef struct helemstruct {
    helemtype next_less;
    helemtype next_greater;
    objectrecord key;
    objectrecord data;
  } helemrecord;

typedef struct hashstruct {
    int bits;
    int mask;
    int table_size;
    helemtype table[1];
  } hashrecord;

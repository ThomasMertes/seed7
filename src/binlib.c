/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2013  Thomas Mertes                        */
/*                                                                  */
/*  This program is free software; you can redistribute it and/or   */
/*  modify it under the terms of the GNU General Public License as  */
/*  published by the Free Software Foundation; either version 2 of  */
/*  the License, or (at your option) any later version.             */
/*                                                                  */
/*  This program is distributed in the hope that it will be useful, */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   */
/*  GNU General Public License for more details.                    */
/*                                                                  */
/*  You should have received a copy of the GNU General Public       */
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/binlib.c                                        */
/*  Changes: 2013  Thomas Mertes                                    */
/*  Content: All primitive actions for the type binary.             */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "objutl.h"
#include "runerr.h"
#include "big_drv.h"

#undef EXTERN
#define EXTERN
#include "biglib.h"



objectType bin_and (listType arguments)

  { /* bin_and */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigAnd(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* bin_and */



objectType bin_or (listType arguments)

  { /* bin_or */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigOr(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* bin_or */



objectType bin_xor (listType arguments)

  { /* bin_xor */
    isit_bigint(arg_1(arguments));
    isit_bigint(arg_3(arguments));
    return bld_bigint_temp(
        bigXor(take_bigint(arg_1(arguments)), take_bigint(arg_3(arguments))));
  } /* bin_xor */

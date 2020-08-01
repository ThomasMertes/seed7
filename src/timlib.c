/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2000  Thomas Mertes                        */
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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/timlib.c                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions to do time operations.           */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "time.h"

#include "version.h"
#include "common.h"
#include "data.h"
#include "syvarutl.h"
#include "runerr.h"
#include "memory.h"
#include "tim_drv.h"

#undef EXTERN
#define EXTERN
#include "timlib.h"



#ifdef ANSI_C

objecttype tim_await (listtype arguments)
#else

objecttype tim_await (arguments)
listtype arguments;
#endif

  { /* tim_await */
    isit_int(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    isit_int(arg_7(arguments));
    isit_int(arg_8(arguments));
    timAwait(arg_1(arguments)->value.intvalue,
             arg_2(arguments)->value.intvalue,
             arg_3(arguments)->value.intvalue,
             arg_4(arguments)->value.intvalue,
             arg_5(arguments)->value.intvalue,
             arg_6(arguments)->value.intvalue,
             arg_7(arguments)->value.intvalue,
             arg_8(arguments)->value.intvalue);
    return(SYS_EMPTY_OBJECT);
  } /* tim_await */



#ifdef ANSI_C

objecttype tim_now (listtype arguments)
#else

objecttype tim_now (arguments)
listtype arguments;
#endif

  { /* tim_now */
    isit_int(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    isit_int(arg_7(arguments));
    isit_int(arg_8(arguments));
    timNow(&arg_1(arguments)->value.intvalue,
           &arg_2(arguments)->value.intvalue,
           &arg_3(arguments)->value.intvalue,
           &arg_4(arguments)->value.intvalue,
           &arg_5(arguments)->value.intvalue,
           &arg_6(arguments)->value.intvalue,
           &arg_7(arguments)->value.intvalue,
           &arg_8(arguments)->value.intvalue);
/*  fprintf(stderr, "now      %04ld/%02ld/%02ld %02ld:%02ld:%02ld %7ld\n",
        arg_1(arguments)->value.intvalue,
        arg_2(arguments)->value.intvalue,
        arg_3(arguments)->value.intvalue,
        arg_4(arguments)->value.intvalue,
        arg_5(arguments)->value.intvalue,
        arg_6(arguments)->value.intvalue,
        arg_7(arguments)->value.intvalue); */
    return(SYS_EMPTY_OBJECT);
  } /* tim_now */

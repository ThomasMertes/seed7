/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
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
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/timlib.c                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions to do time operations.           */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "time.h"

#include "common.h"
#include "data.h"
#include "syvarutl.h"
#include "objutl.h"
#include "runerr.h"
#include "tim_rtl.h"
#include "tim_drv.h"

#undef EXTERN
#define EXTERN
#include "timlib.h"



/**
 *  Wait until the given time is reached
 *  @param time_zone/arg_8 Difference to UTC in minutes
 *                         (for UTC+1 it is 60). The time_zone includes
 *                         the effect of a daylight saving time.
 */
objectType tim_await (listType arguments)

  { /* tim_await */
    isit_int(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    isit_int(arg_7(arguments));
    isit_int(arg_8(arguments));
    logFunction(printf("tim_await(" F_D(04) "-" F_D(02) "-" F_D(02) " "
                                    F_D(02) ":" F_D(02) ":" F_D(02) "."
                                    F_D(06) ", " FMT_D ")\n",
                       take_int(arg_1(arguments)),
                       take_int(arg_2(arguments)),
                       take_int(arg_3(arguments)),
                       take_int(arg_4(arguments)),
                       take_int(arg_5(arguments)),
                       take_int(arg_6(arguments)),
                       take_int(arg_7(arguments)),
                       take_int(arg_8(arguments))););
    timAwait(take_int(arg_1(arguments)),
             take_int(arg_2(arguments)),
             take_int(arg_3(arguments)),
             take_int(arg_4(arguments)),
             take_int(arg_5(arguments)),
             take_int(arg_6(arguments)),
             take_int(arg_7(arguments)),
             take_int(arg_8(arguments)));
    logFunction(printf("tim_await -->\n"););
    return SYS_EMPTY_OBJECT;
  } /* tim_await */



/**
 *  Convert a timestamp into a time from the local time zone.
 *  The timestamp is expressed in seconds since the Unix Epoch.
 *  The Unix Epoch (1970-01-01 00:00:00 UTC) corresponds to 0.
 *  @return the local time that corresponds to the timestamp.
 */
objectType tim_from_timestamp (listType arguments)

  {
    boolType is_dst;

  /* tim_from_timestamp */
    isit_int(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    isit_int(arg_7(arguments));
    isit_int(arg_8(arguments));
    isit_int(arg_9(arguments));
    isit_bool(arg_10(arguments));
    timFromIntTimestamp(arg_1(arguments)->value.intValue,
                       &arg_2(arguments)->value.intValue,
                       &arg_3(arguments)->value.intValue,
                       &arg_4(arguments)->value.intValue,
                       &arg_5(arguments)->value.intValue,
                       &arg_6(arguments)->value.intValue,
                       &arg_7(arguments)->value.intValue,
                       &arg_8(arguments)->value.intValue,
                       &arg_9(arguments)->value.intValue,
                       &is_dst);
    if (is_dst) {
      arg_10(arguments)->value.objValue = SYS_TRUE_OBJECT;
    } else {
      arg_10(arguments)->value.objValue = SYS_FALSE_OBJECT;
    } /* if */
/*  fprintf(stderr, "timestamp %10ld  %04ld-%02ld-%02ld %02ld:%02ld:%02ld %7ld\n",
        arg_1(arguments)->value.intValue,
        arg_2(arguments)->value.intValue,
        arg_3(arguments)->value.intValue,
        arg_4(arguments)->value.intValue,
        arg_5(arguments)->value.intValue,
        arg_6(arguments)->value.intValue,
        arg_7(arguments)->value.intValue,
        arg_8(arguments)->value.intValue); */
    return SYS_EMPTY_OBJECT;
  } /* tim_from_timestamp */



/**
 *  Determine the current local time.
 *  @param time_zone/arg_8 Difference to UTC in minutes
 *                         (for UTC+1 it is 60). The time_zone includes
 *                         the effect of a daylight saving time.
 *  @param is_dst/arg_9 Is TRUE, if a daylight saving time is
 *                      currently in effect.
 */
objectType tim_now (listType arguments)

  {
    boolType is_dst;

  /* tim_now */
    isit_int(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    isit_int(arg_7(arguments));
    isit_int(arg_8(arguments));
    isit_bool(arg_9(arguments));
    timNow(&arg_1(arguments)->value.intValue,
           &arg_2(arguments)->value.intValue,
           &arg_3(arguments)->value.intValue,
           &arg_4(arguments)->value.intValue,
           &arg_5(arguments)->value.intValue,
           &arg_6(arguments)->value.intValue,
           &arg_7(arguments)->value.intValue,
           &arg_8(arguments)->value.intValue,
           &is_dst);
    if (is_dst) {
      arg_9(arguments)->value.objValue = SYS_TRUE_OBJECT;
    } else {
      arg_9(arguments)->value.objValue = SYS_FALSE_OBJECT;
    } /* if */
/*  fprintf(stderr, "now      %04ld-%02ld-%02ld %02ld:%02ld:%02ld %7ld\n",
        arg_1(arguments)->value.intValue,
        arg_2(arguments)->value.intValue,
        arg_3(arguments)->value.intValue,
        arg_4(arguments)->value.intValue,
        arg_5(arguments)->value.intValue,
        arg_6(arguments)->value.intValue,
        arg_7(arguments)->value.intValue); */
    return SYS_EMPTY_OBJECT;
  } /* tim_now */



/**
 *  Sets timeZone and daylightSavingTime for a given time.
 *  @return the time in the local time zone.
 */
objectType tim_set_local_tz (listType arguments)

  {
    boolType is_dst;

  /* tim_set_local_tz */
    isit_int(arg_1(arguments));
    isit_int(arg_2(arguments));
    isit_int(arg_3(arguments));
    isit_int(arg_4(arguments));
    isit_int(arg_5(arguments));
    isit_int(arg_6(arguments));
    isit_int(arg_7(arguments));
    isit_bool(arg_8(arguments));
    timSetLocalTZ(take_int(arg_1(arguments)),
                  take_int(arg_2(arguments)),
                  take_int(arg_3(arguments)),
                  take_int(arg_4(arguments)),
                  take_int(arg_5(arguments)),
                  take_int(arg_6(arguments)),
                  &arg_7(arguments)->value.intValue,
                  &is_dst);
    if (is_dst) {
      arg_8(arguments)->value.objValue = SYS_TRUE_OBJECT;
    } else {
      arg_8(arguments)->value.objValue = SYS_FALSE_OBJECT;
    } /* if */
/*  fprintf(stderr, "tim_set_local_tz %04ld-%02ld-%02ld %02ld:%02ld:%02ld %7ld %d\n",
        take_int(arg_1(arguments)),
        take_int(arg_2(arguments)),
        take_int(arg_3(arguments)),
        take_int(arg_4(arguments)),
        take_int(arg_5(arguments)),
        take_int(arg_6(arguments)),
        take_int(arg_7(arguments)),
        is_dst); */
    return SYS_EMPTY_OBJECT;
  } /* tim_set_local_tz */

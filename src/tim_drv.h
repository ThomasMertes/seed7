/********************************************************************/
/*                                                                  */
/*  tim_drv.h     Prototypes for time handling functions.           */
/*  Copyright (C) 1989 - 2009  Thomas Mertes                        */
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
/*  File: seed7/src/tim_drv.h                                       */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: Prototypes for time handling functions.                */
/*                                                                  */
/********************************************************************/

#ifdef USE_ALTERNATE_LOCALTIME_R
#define localtime_r alternate_localtime_r
#endif

#ifdef ANSI_C

void timAwait (inttype year, inttype month, inttype day, inttype hour,
    inttype min, inttype sec, inttype mycro_sec, inttype time_zone);
void timNow (inttype *year, inttype *month, inttype *day, inttype *hour,
    inttype *min, inttype *sec, inttype *mycro_sec, inttype *time_zone,
    booltype *is_dst);
#ifdef USE_ALTERNATE_LOCALTIME_R
struct tm *alternate_localtime_r (time_t *utc_seconds, struct tm *tm_result);
#endif

#else

void timAwait ();
void timNow ();
#ifdef USE_ALTERNATE_LOCALTIME_R
struct tm *alternate_localtime_r ();
#endif

#endif

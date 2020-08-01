/********************************************************************/
/*                                                                  */
/*  tim_rtl.h     Time access using the C capabilitys.              */
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
/*  File: seed7/src/tim_rtl.h                                       */
/*  Changes: 2009  Thomas Mertes                                    */
/*  Content: Time access using the C capabilitys.                   */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

time_t mkutc (struct tm *timeptr);
void timFromTimestamp (time_t st_time,
    inttype *year, inttype *month, inttype *day, inttype *hour,
    inttype *min, inttype *sec, inttype *mycro_sec, inttype *time_zone,
    booltype *is_dst);
time_t timToTimestamp (inttype year, inttype month, inttype day, inttype hour,
    inttype min, inttype sec, inttype mycro_sec, inttype time_zone);

#else

time_t mkutc ();
void timFromTimestamp ();
time_t timToTimestamp ();

#endif

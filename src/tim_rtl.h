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

time_t mkutc (struct tm *timeptr);
time_t unchecked_mkutc (struct tm *timeptr);
void timFromTimestamp (time_t st_time,
    intType *year, intType *month, intType *day, intType *hour,
    intType *min, intType *sec, intType *micro_sec, intType *time_zone,
    boolType *is_dst);
void timFromIntTimestamp (intType timestamp,
    intType *year, intType *month, intType *day, intType *hour,
    intType *min, intType *sec, intType *micro_sec, intType *time_zone,
    boolType *is_dst);
time_t timToTimestamp (intType year, intType month, intType day, intType hour,
    intType min, intType sec, intType micro_sec, intType time_zone);
void timSetLocalTZ (intType year, intType month, intType day, intType hour,
    intType min, intType sec, intType *time_zone, boolType *is_dst);

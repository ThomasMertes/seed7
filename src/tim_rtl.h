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

time_t unchecked_mkutc (struct tm *timeptr);
void timFromTimestamp (time_t timestamp,
    intType *year, intType *month, intType *day, intType *hour,
    intType *minute, intType *second, intType *micro_sec, intType *timeZone,
    boolType *isDst);
void timFromIntTimestamp (intType timestamp,
    intType *year, intType *month, intType *day, intType *hour,
    intType *minute, intType *second, intType *micro_sec, intType *timeZone,
    boolType *isDst);
void timUtcFromTimestamp (timeStampType timestamp,
    intType *year, intType *month, intType *day,
    intType *hour, intType *minute, intType *second);
timeStampType timToTimestamp (intType year, intType month, intType day,
    intType hour, intType minute, intType second, intType timeZone);

#if TIME_T_SIZE == TIMESTAMPTYPE_SIZE && TIME_T_SIGNED
#define timToOsTimestamp(year, month, day, hour, minute, second, timeZone) \
    (time_t) timToTimestamp(year, month, day, hour, minute, second, timeZone)
#else
time_t timToOsTimestamp (intType year, intType month, intType day,
    intType hour, intType minute, intType second, intType timeZone);
#endif

void timSetLocalTZ (intType year, intType month, intType day, intType hour,
    intType minute, intType second, intType *timeZone, boolType *isDst);
void dateFromDaysSince1900 (int32Type daysSince1900_01_01,
    intType *year, intType *month, intType *day);
boolType dateIsOkay (intType year, intType month, intType day);
boolType assignTime (const_cstriType isoDate, intType *year, intType *month,
    intType *day, intType *hour, intType *minute, intType *second,
    intType *microSecond, boolType *isTime);
boolType assignIsoDuration (const const_ustriType isoDuration,
    intType *year, intType *month, intType *day, intType *hour,
    intType *minute, intType *second, intType *micro_second);

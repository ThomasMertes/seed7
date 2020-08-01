/********************************************************************/
/*                                                                  */
/*  tim_dos.c     Time access using the dos capabilitys.            */
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
/*  File: seed7/src/tim_dos.c                                       */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: Time access using the dos capabilitys.                 */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include "sys/types.h"
#include "sys/timeb.h"

#include "version.h"
#include "common.h"

#undef EXTERN
#define EXTERN
#include "tim_drv.h"



#ifdef ANSI_C

void get_time (time_t *calendar_time, long *mycro_secs, long *time_zone)
#else

void get_time (calendar_time, mycro_secs, time_zone)
time_t *calendar_time;
long *mycro_secs;
long *time_zone;
#endif

  {
    struct timeb tstruct;

  /* get_time */
    ftime(&tstruct);
    *calendar_time = tstruct.time;
    *mycro_secs = 1000 * ((long) tstruct.millitm);
    *time_zone = (long) timezone;
  } /* get_time */



#ifdef ANSI_C

void await_time (time_t calendar_time, long mycro_secs)
#else

void await_time (calendar_time, mycro_secs)
time_t calendar_time;
long mycro_secs;
#endif

  {
    time_t time_now;
    struct timeb tstruct;

  /* await_time */
/*  time_now = time(NULL); */
    ftime(&tstruct);
    time_now = tstruct.time;
/*  calendar_time = calendar_time + 60 * (int) tstruct.timezone; */
/*  printf("%d %d %d %d\n",
        tstruct.time, (int) tstruct.timezone, time_now, calendar_time); */
/*  printf("time_now < calendar_time: %d < %d\n", time_now, calendar_time); */
    if (time_now < calendar_time) {
      do {
        ftime(&tstruct);
/*      printf("%ld ?= %ld\n", tstruct.time, calendar_time); */
      } while (tstruct.time < calendar_time);
    } /* if */
    if (mycro_secs != 0) {
      do {
        ftime(&tstruct);
/*      printf("%ld.%ld000 ?= %ld.%ld\n",
            tstruct.time, (long) tstruct.millitm,
            calendar_time, mycro_secs); */
      } while (tstruct.time <= calendar_time &&
          1000 * ((long) tstruct.millitm) < mycro_secs);
    } /* if */
  } /* await_time */

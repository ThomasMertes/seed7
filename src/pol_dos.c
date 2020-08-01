/********************************************************************/
/*                                                                  */
/*  pol_dos.c     Poll type and function using DOS capabilities.    */
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
/*  File: seed7/src/pol_dos.c                                       */
/*  Changes: 2011  Thomas Mertes                                    */
/*  Content: Poll type and function using DOS capabilities.         */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "pol_drv.h"



#ifdef ANSI_C

void polAddCheck (const polltype pollData, const sockettype aSocket,
    inttype eventsToCheck, const rtlGenerictype fileObj)
#else

void polAddCheck (pollData, aSocket, eventsToCheck, fileObj)
polltype pollData;
sockettype aSocket;
inttype eventsToCheck;
rtlGenerictype fileObj;
#endif

  { /* polAddCheck */
  } /* polAddCheck */



#ifdef ANSI_C

void polClear (const polltype pollData)
#else

void polClear (pollData)
polltype pollData;
#endif

  { /* polClear */
  } /* polClear */



#ifdef ANSI_C

void polCpy (const polltype poll_to, const const_polltype pollDataFrom)
#else

void polCpy (poll_to, pollDataFrom)
polltype poll_to;
polltype pollDataFrom;
#endif

  {  /* polCpy */
  }  /* polCpy */



#ifdef ANSI_C

polltype polCreate (const const_polltype pollDataFrom)
#else

polltype polCreate (pollDataFrom)
polltype pollDataFrom;
#endif

  { /* polCreate */
    return NULL;
  } /* polCreate */



#ifdef ANSI_C

void polDestr (const polltype oldPollData)
#else

void polDestr (oldPollData)
polltype oldPollData;
#endif

  { /* polDestr */
  } /* polDestr */



#ifdef ANSI_C

polltype polEmpty (void)
#else

polltype polEmpty ()
#endif

  { /* polEmpty */
    return NULL;
  } /* polEmpty */



#ifdef ANSI_C

inttype polGetCheck (const const_polltype pollData, const sockettype aSocket)
#else

inttype polGetCheck (pollData, aSocket)
polltype pollData;
sockettype aSocket;
#endif

  { /* polGetCheck */
    return POLL_NOTHING;
  } /* polGetCheck */



#ifdef ANSI_C

inttype polGetFinding (const const_polltype pollData, const sockettype aSocket)
#else

inttype polGetFinding (pollData, aSocket)
polltype pollData;
sockettype aSocket;
#endif

  { /* polGetFinding */
    return POLL_NOTHING;
  } /* polGetFinding */


#ifdef ANSI_C

booltype polHasNext (const polltype pollData)
#else

booltype polHasNext (pollData)
polltype pollData;
#endif

  { /* polHasNext */
    return FALSE;
  } /* polHasNext */



#ifdef ANSI_C

void polIterChecks (const polltype pollData, inttype pollMode)
#else

void polIterChecks (pollData, pollMode)
polltype pollData;
inttype pollMode;
#endif

  { /* polIterChecks */
  } /* polIterChecks */



#ifdef ANSI_C

void polIterFindings (const polltype pollData, inttype pollMode)
#else

void polIterFindings (pollData, pollMode)
polltype pollData;
inttype pollMode;
#endif

  { /* polIterFindings */
  } /* polIterFindings */



#ifdef ANSI_C

rtlGenerictype polNextFile (const polltype pollData, const rtlGenerictype nullFile)
#else

rtlGenerictype polNextFile (pollData, nullFile)
polltype pollData;
rtlGenerictype nullFile;
#endif

  { /* polNextFile */
    return nullFile;
  } /* polNextFile */



#ifdef ANSI_C

void polPoll (const polltype pollData)
#else

void polPoll (pollData)
polltype pollData;
#endif

  { /* polPoll */
    raise_error(FILE_ERROR);
  } /* polPoll */



#ifdef ANSI_C

void polRemoveCheck (const polltype pollData, const sockettype aSocket,
    inttype eventsToCheck)
#else

void polRemoveCheck (pollData, aSocket, eventsToCheck)
polltype pollData;
sockettype aSocket;
inttype eventsToCheck;
#endif

  { /* polRemoveCheck */
  } /* polRemoveCheck */

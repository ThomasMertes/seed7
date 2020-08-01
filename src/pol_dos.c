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


#ifdef DO_HEAP_STATISTIC
size_t sizeof_pollrecord = sizeof(pollrecord);
#endif



void initPollOperations (const createfunctype incrUsageCount,
    const destrfunctype decrUsageCount)

  { /* initPollOperations */
  } /* initPollOperations */



void polAddCheck (const polltype pollData, const sockettype aSocket,
    inttype eventsToCheck, const generictype fileObj)

  { /* polAddCheck */
  } /* polAddCheck */



void polClear (const polltype pollData)

  { /* polClear */
  } /* polClear */



void polCpy (const polltype poll_to, const const_polltype pollDataFrom)

  {  /* polCpy */
  }  /* polCpy */



polltype polCreate (const const_polltype pollDataFrom)

  { /* polCreate */
    return NULL;
  } /* polCreate */



void polDestr (const polltype oldPollData)

  { /* polDestr */
  } /* polDestr */



polltype polEmpty (void)

  { /* polEmpty */
    return NULL;
  } /* polEmpty */



inttype polGetCheck (const const_polltype pollData, const sockettype aSocket)

  { /* polGetCheck */
    return POLL_NOTHING;
  } /* polGetCheck */



inttype polGetFinding (const const_polltype pollData, const sockettype aSocket)

  { /* polGetFinding */
    return POLL_NOTHING;
  } /* polGetFinding */


booltype polHasNext (const polltype pollData)

  { /* polHasNext */
    return FALSE;
  } /* polHasNext */



void polIterChecks (const polltype pollData, inttype pollMode)

  { /* polIterChecks */
  } /* polIterChecks */



void polIterFindings (const polltype pollData, inttype pollMode)

  { /* polIterFindings */
  } /* polIterFindings */



generictype polNextFile (const polltype pollData, const generictype nullFile)

  { /* polNextFile */
    return nullFile;
  } /* polNextFile */



void polPoll (const polltype pollData)

  { /* polPoll */
    raise_error(FILE_ERROR);
  } /* polPoll */



void polRemoveCheck (const polltype pollData, const sockettype aSocket,
    inttype eventsToCheck)

  { /* polRemoveCheck */
  } /* polRemoveCheck */

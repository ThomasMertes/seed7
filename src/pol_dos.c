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


#if DO_HEAP_STATISTIC
size_t sizeof_pollRecord = sizeof(pollRecord);
#endif



void initPollOperations (const createFuncType incrUsageCount,
    const destrFuncType decrUsageCount)

  { /* initPollOperations */
  } /* initPollOperations */



void polAddCheck (const pollType pollData, const socketType aSocket,
    intType eventsToCheck, const genericType fileObj)

  { /* polAddCheck */
  } /* polAddCheck */



/**
 *  Clears 'pollData'.
 *  All sockets and all events are removed from 'pollData' and
 *  the iterator is reset, such that polHasNext() returns FALSE.
 */
void polClear (const pollType pollData)

  { /* polClear */
  } /* polClear */



/**
 *  Assign source to dest.
 *  A copy function assumes that dest contains a legal value.
 *  @exception MEMORY_ERROR Not enough memory to create dest.
 */
void polCpy (const pollType dest, const const_pollType source)

  {  /* polCpy */
  }  /* polCpy */



/**
 *  Return a copy of source, that can be assigned to a new destination.
 *  It is assumed that the destination of the assignment is undefined.
 *  Create functions can be used to initialize Seed7 constants.
 *  @return a copy of source.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
pollType polCreate (const const_pollType source)

  { /* polCreate */
    return NULL;
  } /* polCreate */



/**
 *  Free the memory referred by 'oldPollData'.
 *  After polDestr is left 'oldPollData' refers to not existing memory.
 *  The memory where 'oldPollData' is stored can be freed afterwards.
 */
void polDestr (const pollType oldPollData)

  { /* polDestr */
  } /* polDestr */



/**
 *  Create an empty poll data value.
 *  @return an empty poll data value.
 *  @exception MEMORY_ERROR Not enough memory for the result.
 */
pollType polEmpty (void)

  { /* polEmpty */
    return NULL;
  } /* polEmpty */



/**
 *  Return the checkedEvents field from 'pollData' for 'aSocket'.
 *  The polPoll function uses the checkedEvents as input.
 *  The following checkedEvents can be returned:
 *  - POLLNOTHING no data can be read or written.
 *  - POLLIN data can be read from the corresponding socket.
 *  - POLLOUT data can be written to the corresponding socket.
 *  - POLLINOUT data can be read and written (POLLIN and POLLOUT).
 *
 *  @param pollData Poll data from which the checkedEvents are
 *         retrieved.
 *  @param aSocket Socket for which the checkedEvents are retrived.
 *  @return POLLNOTHING, POLLIN, POLLOUT or POLLINOUT, depending on
 *          the events added and removed for 'aSocket' with
 *          'addCheck' and 'removeCheck'.
 */
intType polGetCheck (const const_pollType pollData, const socketType aSocket)

  { /* polGetCheck */
    return POLL_NOTHING;
  } /* polGetCheck */



/**
 *  Return the eventFindings field from 'pollData' for 'aSocket'.
 *  The polPoll function assigns the
 *  eventFindings for 'aSocket' to 'pollData'. The following
 *  eventFindings can be returned:
 *  - POLLNOTHING no data can be read or written.
 *  - POLLIN data can be read from the corresponding socket.
 *  - POLLOUT data can be written to the corresponding socket.
 *  - POLLINOUT data can be read and written (POLLIN and POLLOUT).
 *
 *  @return POLLNOTHING, POLLIN, POLLOUT or POLLINOUT, depending on
 *          the findings of polPoll concerning 'aSocket'.
 */
intType polGetFinding (const const_pollType pollData, const socketType aSocket)

  { /* polGetFinding */
    return POLL_NOTHING;
  } /* polGetFinding */


/**
 *  Determine if the 'pollData' iterator can deliver another file.
 *  @return TRUE if 'nextFile' would return another file from the
 *          'pollData' iterator, FALSE otherwise.
 */
boolType polHasNext (const pollType pollData)

  { /* polHasNext */
    return FALSE;
  } /* polHasNext */



/**
 *  Reset the 'pollData' iterator to process checkedEvents.
 *  The following calls of 'hasNext' and 'nextFile' refer to
 *  the checkedEvents of the given 'pollMode'. 'PollMode'
 *  can have one of the following values:
 *  - POLLNOTHING don't iterate ('hasNext' returns FALSE).
 *  - POLLIN data can be read from the corresponding socket.
 *  - POLLOUT data can be written to the corresponding socket.
 *  - POLLINOUT data can be read or written (POLLIN and POLLOUT).
 *
 *  @exception RANGE_ERROR Illegal value for 'pollMode'.
 */
void polIterChecks (const pollType pollData, intType pollMode)

  { /* polIterChecks */
  } /* polIterChecks */



/**
 *  Reset the 'pollData' iterator to process eventFindings.
 *  The following calls of 'hasNext' and 'nextFile' refer to
 *  the eventFindings of the given 'pollMode'. 'PollMode'
 *  can have one of the following values:
 *  - POLLNOTHING don't iterate ('hasNext' returns FALSE).
 *  - POLLIN data can be read from the corresponding socket.
 *  - POLLOUT data can be written to the corresponding socket.
 *  - POLLINOUT data can be read or written (POLLIN and POLLOUT).
 *
 *  @exception RANGE_ERROR Illegal value for 'pollMode'.
 */
void polIterFindings (const pollType pollData, intType pollMode)

  { /* polIterFindings */
  } /* polIterFindings */



/**
 *  Get the next file from the 'pollData' iterator.
 *  Successive calls of 'nextFile' return all files from the 'pollData'
 *  iterator. The file returned by 'nextFile' is determined with the
 *  function 'addCheck'. The files covered by the 'pollData' iterator
 *  are determined with 'iterChecks' or 'iterFindings'.
 *  @return the next file from the 'pollData' iterator, or
 *          STD_NULL if no file from the 'pollData' iterator is available.
 */
genericType polNextFile (const pollType pollData, const genericType nullFile)

  { /* polNextFile */
    return nullFile;
  } /* polNextFile */



/**
 *  Waits for one or more of the checkedEvents from 'pollData'.
 *  polPoll waits until one of the checkedEvents for a
 *  corresponding socket occurs. If a checked event occurs
 *  the eventFindings field is assigned a value. The following
 *  eventFindings values are assigned:
 *  - POLLIN data can be read from the corresponding socket.
 *  - POLLOUT data can be written to the corresponding socket.
 *  - POLLINOUT data can be read and written (POLLIN and POLLOUT).
 *
 *  @exception FILE_ERROR The system function returns an error.
 */
void polPoll (const pollType pollData)

  { /* polPoll */
    raise_error(FILE_ERROR);
  } /* polPoll */



/**
 *  Remove 'eventsToCheck' for 'aSocket' from 'pollData'.
 *  'EventsToCheck' can have one of the following values:
 *  - POLLIN check if data can be read from the corresponding socket.
 *  - POLLOUT check if data can be written to the corresponding socket.
 *  - POLLINOUT check if data can be read or written (POLLIN or POLLOUT).
 *
 *  @param pollData Poll data from which the event checks are removed.
 *  @param aSocket Socket for which the events should not be checked.
 *  @param eventsToCheck Events to be removed from the checkedEvents
 *         field of 'pollData'.
 *  @exception RANGE_ERROR Illegal value for 'eventsToCheck'.
 */
void polRemoveCheck (const pollType pollData, const socketType aSocket,
    intType eventsToCheck)

  { /* polRemoveCheck */
  } /* polRemoveCheck */

/********************************************************************/
/*                                                                  */
/*  pol_drv.h     Prototypes of functions to support poll.          */
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
/*  File: seed7/src/pol_drv.h                                       */
/*  Changes: 2011  Thomas Mertes                                    */
/*  Content: Prototypes of functions to support poll.               */
/*                                                                  */
/********************************************************************/

#define POLL_NOTHING 0
#define POLL_IN      1
#define POLL_OUT     2
#define POLL_INOUT   3

typedef struct {
    createfunctype incrUsageCount;
    destrfunctype decrUsageCount;
  } fileObjectOpType;

#ifdef DO_INIT
fileObjectOpType fileObjectOps = {NULL, NULL};
#else
EXTERN fileObjectOpType fileObjectOps;
#endif


void initPollOperations (const createfunctype incrUsageCount,
                         const destrfunctype decrUsageCount);
void polAddCheck (const polltype pollData, const sockettype aSocket,
                  inttype eventsToCheck, const rtlGenerictype fileObj);
void polClear (const polltype pollData);
void polCpy (const polltype poll_to, const const_polltype pollDataFrom);
polltype polCreate (const const_polltype pollDataFrom);
void polDestr (const polltype oldPollData);
polltype polEmpty (void);
inttype polGetCheck (const const_polltype pollData, const sockettype aSocket);
inttype polGetFinding (const const_polltype pollData, const sockettype aSocket);
booltype polHasNext (const polltype pollData);
void polIterChecks (const polltype pollData, inttype pollMode);
void polIterFindings (const polltype pollData, inttype pollMode);
rtlGenerictype polNextFile (const polltype pollData, const rtlGenerictype nullFile);
void polPoll (const polltype pollData);
void polRemoveCheck (const polltype pollData, const sockettype aSocket,
                     inttype eventsToCheck);

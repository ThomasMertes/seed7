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
    createFuncType incrUsageCount;
    destrFuncType decrUsageCount;
  } fileObjectOpType;

#ifdef DO_INIT
fileObjectOpType fileObjectOps = {NULL, NULL};
#else
EXTERN fileObjectOpType fileObjectOps;
#endif


void initPollOperations (const createFuncType incrUsageCount,
                         const destrFuncType decrUsageCount);
void polAddCheck (const pollType pollData, const socketType aSocket,
                  intType eventsToCheck, const genericType fileObj);
void polClear (const pollType pollData);
void polCpy (const pollType poll_to, const const_pollType pollDataFrom);
pollType polCreate (const const_pollType pollDataFrom);
void polDestr (const pollType oldPollData);
pollType polEmpty (void);
intType polGetCheck (const const_pollType pollData, const socketType aSocket);
intType polGetFinding (const const_pollType pollData, const socketType aSocket);
boolType polHasNext (const pollType pollData);
void polIterChecks (const pollType pollData, intType pollMode);
void polIterFindings (const pollType pollData, intType pollMode);
genericType polNextFile (const pollType pollData, const genericType nullFile);
void polPoll (const pollType pollData);
void polRemoveCheck (const pollType pollData, const socketType aSocket,
                     intType eventsToCheck);

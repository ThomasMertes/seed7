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


#ifdef ANSI_C

void polAddCheck (polltype pollData, const sockettype aSocket,
                  inttype pollMode, const rtlGenerictype fileObj);
void polClear (polltype pollData);
void polCpy (polltype poll_to, const const_polltype pollDataFrom);
polltype polCreate (const const_polltype pollDataFrom);
void polDestr (const const_polltype oldPollData);
polltype polEmpty (void);
inttype polGetCheck (polltype pollData, const sockettype aSocket);
inttype polGetFinding (polltype pollData, const sockettype aSocket);
booltype polHasNext (polltype pollData);
void polIterChecks (polltype pollData, inttype pollMode);
void polIterFindings (polltype pollData, inttype pollMode);
rtlGenerictype polNextFile (polltype pollData, const rtlGenerictype nullFile);
void polPoll (polltype pollData);
void polRemoveCheck (polltype pollData, const sockettype aSocket, inttype pollMode);

#else

void polAddCheck ();
void polClear ();
void polCpy ();
polltype polCreate ();
void polDestr ();
polltype polEmpty ();
inttype polGetCheck ();
inttype polGetFinding ();
booltype polHasNext ();
void polIterChecks ();
void polIterFindings ();
rtlObjecttype polNextFile ();
void polPoll ();
void polRemoveCheck ();

#endif

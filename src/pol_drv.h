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

#ifdef ANSI_C

void polAddReadCheck (polltype pollData, const sockettype aSocket,
                      const rtlGenerictype fileObj);
void polAddWriteCheck (polltype pollData, const sockettype aSocket,
                       const rtlGenerictype fileObj);
void polClear (polltype pollData);
void polCpy (polltype poll_to, const const_polltype pollDataFrom);
polltype polCreate (const const_polltype pollDataFrom);
void polDestr (const const_polltype oldPollData);
polltype polEmpty (void);
rtlArraytype polFiles (const const_polltype pollData);
booltype polHasNextReadFile (polltype pollData);
booltype polHasNextWriteFile (polltype pollData);
rtlGenerictype polNextReadFile (polltype pollData, const rtlGenerictype nullFile);
rtlGenerictype polNextWriteFile (polltype pollData, const rtlGenerictype nullFile);
void polPoll (polltype pollData);
booltype polReadyForRead (polltype pollData, const sockettype aSocket);
booltype polReadyForWrite (polltype pollData, const sockettype aSocket);
void polRemoveReadCheck (polltype pollData, const sockettype aSocket);
void polRemoveWriteCheck (polltype pollData, const sockettype aSocket);

#else

void polAddReadCheck ();
void polAddWriteCheck ();
void polClear ();
void polCpy ();
polltype polCreate ();
void polDestr ();
polltype polEmpty ();
rtlArraytype polFiles ():
booltype polHasNextReadFile ();
booltype polHasNextWriteFile ();
rtlObjecttype polNextReadFile ();
rtlObjecttype polNextWriteFile ();
void polPoll ();
booltype polReadyForRead ();
booltype polReadyForWrite ();
void polRemoveReadCheck ();
void polRemoveWriteCheck ();

#endif

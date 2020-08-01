/********************************************************************/
/*                                                                  */
/*  pcs_drv.h     Prototypes of OS specific process functions.      */
/*  Copyright (C) 1989 - 2014  Thomas Mertes                        */
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
/*  File: seed7/src/pcs_drv.h                                       */
/*  Changes: 2010, 2014  Thomas Mertes                              */
/*  Content: Prototypes of OS specific process functions.           */
/*                                                                  */
/********************************************************************/

intType pcsCmp (const const_processType process1, const const_processType process2);
boolType pcsEq (const const_processType process1, const const_processType process2);
intType pcsExitValue (const const_processType process);
void pcsFree (processType old_process);
intType pcsHashCode (const const_processType aProcess);
boolType pcsIsAlive (const processType aProcess);
void pcsKill (const processType aProcess);
void pcsPipe2 (const const_striType command, const const_rtlArrayType parameters,
    fileType *childStdin, fileType *childStdout);
void pcsPty (const const_striType command, const const_rtlArrayType parameters,
    fileType *childStdin, fileType *childStdout);
processType pcsStart (const const_striType command, const const_rtlArrayType parameters,
    fileType childStdin, fileType childStdout, fileType childStderr);
striType pcsStr (const const_processType process);
void pcsWaitFor (const processType process);

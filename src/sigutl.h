/********************************************************************/
/*                                                                  */
/*  sigutl.h      Driver shutdown and signal handling.              */
/*  Copyright (C) 1990 - 2000, 2014, 2016, 2017  Thomas Mertes      */
/*                2019, 2020, 2021, 2025  Thomas Mertes             */
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
/*  File: seed7/src/sigutl.h                                        */
/*  Changes: 1990 - 2000, 2014, 2016, 2017, 2019  Thomas Mertes     */
/*           2020, 2021, 2025  Thomas Mertes                        */
/*  Content: Driver shutdown and signal handling.                   */
/*                                                                  */
/********************************************************************/

typedef void (*suspendInterprType) (int signalNum);


void shutDrivers (void);
const_cstriType signalName (int signalNum);
void triggerSigfpe (void);
void setupSignalHandlers (boolType handleSignals,
    boolType traceSignals, boolType overflowSigError,
    boolType fpeNumericError, suspendInterprType suspendInterpr);
boolType callSignalHandler (int signalNum);

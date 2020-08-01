/********************************************************************/
/*                                                                  */
/*  fil_drv.h     Prototypes of OS specific file functions.         */
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
/*  File: seed7/src/fil_drv.h                                       */
/*  Changes: 2011  Thomas Mertes                                    */
/*  Content: Prototypes of OS specific file functions.              */
/*                                                                  */
/********************************************************************/


int readCharChkCtrlC (fileType inFile, boolType *sigintReceived);
boolType filInputReady (fileType aFile);
void filPipe (fileType *inFile, fileType *outFile);
#ifdef DEFINE_FTELLI64_EXT
os_off_t ftelli64Ext (FILE *aFile);
#endif
void setupFiles (void);

/********************************************************************/
/*                                                                  */
/*  cmd_rtl.h     Directory, file and other system functions.       */
/*  Copyright (C) 1989 - 2009  Thomas Mertes                        */
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
/*  File: seed7/src/cmd_rtl.h                                       */
/*  Changes: 1994, 2006, 2009  Thomas Mertes                        */
/*  Content: Directory, file and other system functions.            */
/*                                                                  */
/********************************************************************/

#if HAS_SYMBOLIC_LINKS
striType followLink (striType path);
#endif
#ifdef EMULATE_ROOT_CWD
void initEmulatedCwd (errInfoType *err_info);
#endif
bigIntType cmdBigFileSize (const const_striType file_name);
void cmdChdir (const const_striType dir_name);
void cmdCloneFile (const const_striType source_name, const const_striType dest_name);
striType cmdConfigValue (const const_striType name);
void cmdCopyFile (const const_striType source_name, const const_striType dest_name);
rtlArrayType cmdEnvironment (void);
setType cmdFileMode (const const_striType file_name);
intType cmdFileSize (const const_striType file_name);
intType cmdFileType (const const_striType file_name);
intType cmdFileTypeSL (const const_striType file_name);
striType cmdGetcwd (void);
striType cmdGetenv (const const_striType stri);
void cmdGetATime (const const_striType file_name,
    intType *year, intType *month, intType *day, intType *hour,
    intType *min, intType *sec, intType *micro_sec, intType *time_zone,
    boolType *is_dst);
void cmdGetCTime (const const_striType file_name,
    intType *year, intType *month, intType *day, intType *hour,
    intType *min, intType *sec, intType *micro_sec, intType *time_zone,
    boolType *is_dst);
void cmdGetMTime (const const_striType file_name,
    intType *year, intType *month, intType *day, intType *hour,
    intType *min, intType *sec, intType *micro_sec, intType *time_zone,
    boolType *is_dst);
rtlArrayType cmdGetSearchPath (void);
striType cmdHomeDir (void);
void cmdMkdir (const const_striType dir_name);
void cmdMove (const const_striType source_name, const const_striType dest_name);
void cmdProcessCpy (processType *const process_to, const processType process_from);
processType cmdProcessCreate (const processType process_from);
void cmdProcessDestr (const processType old_process);
striType cmdReadlink (const const_striType link_name);
void cmdRemoveFile (const const_striType file_name);
void cmdRemoveTree (const const_striType file_name);
void cmdSetenv (const const_striType name, const const_striType value);
void cmdSetATime (const const_striType file_name,
    intType year, intType month, intType day, intType hour,
    intType min, intType sec, intType micro_sec, intType time_zone);
void cmdSetFileMode (const const_striType file_name, const const_setType mode);
void cmdSetMTime (const const_striType file_name,
    intType year, intType month, intType day, intType hour,
    intType min, intType sec, intType micro_sec, intType time_zone);
void cmdSetSearchPath (rtlArrayType searchPath);
intType cmdShell (const const_striType command, const const_striType parameters);
striType cmdShellEscape (const const_striType stri);
void cmdSymlink (const const_striType source_name, const const_striType dest_name);
striType cmdToOsPath (const const_striType standardPath);

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

#ifdef FILE_UNKNOWN
#undef FILE_UNKNOWN
#endif

#define FILE_ABSENT   0 /* A component of path does not exist */
#define FILE_UNKNOWN  1 /* File exists but has an unknown type */
#define FILE_REGULAR  2
#define FILE_DIR      3
#define FILE_CHAR     4
#define FILE_BLOCK    5
#define FILE_FIFO     6
#define FILE_SYMLINK  7
#define FILE_SOCKET   8

#if HAS_READLINK || defined HAS_DEVICE_IO_CONTROL
striType followLink (striType path, errInfoType *err_info);
#endif
#if defined USE_EXTENDED_LENGTH_PATH && USE_EXTENDED_LENGTH_PATH
void adjustCwdForShell (errInfoType *err_info);
#endif
#if EMULATE_ROOT_CWD
void initEmulatedCwd (errInfoType *err_info);
#endif
striType doGetCwd (errInfoType *err_info);
striType getAbsolutePath (const const_striType aPath);
bigIntType cmdBigFileSize (const const_striType filePath);
void cmdChdir (const const_striType dir_name);
void cmdCloneFile (const const_striType source_name, const const_striType dest_name);
striType cmdConfigValue (const const_striType name);
void cmdCopyFile (const const_striType source_name, const const_striType dest_name);
rtlArrayType cmdEnvironment (void);
intType cmdFileSize (const const_striType filePath);
intType cmdFileType (const const_striType filePath);
intType cmdFileTypeSL (const const_striType filePath);
striType cmdFinalPath (const const_striType filePath);
striType cmdGetcwd (void);
striType cmdGetenv (const const_striType stri);
void cmdGetATime (const const_striType filePath,
    intType *year, intType *month, intType *day, intType *hour,
    intType *min, intType *sec, intType *micro_sec, intType *time_zone,
    boolType *is_dst);
void cmdGetATimeOfSymlink (const const_striType filePath,
    intType *year, intType *month, intType *day, intType *hour,
    intType *min, intType *sec, intType *micro_sec, intType *time_zone,
    boolType *is_dst);
void cmdGetCTime (const const_striType filePath,
    intType *year, intType *month, intType *day, intType *hour,
    intType *min, intType *sec, intType *micro_sec, intType *time_zone,
    boolType *is_dst);
setType cmdGetFileMode (const const_striType filePath);
setType cmdGetFileModeOfSymlink (const const_striType filePath);
void cmdGetMTime (const const_striType filePath,
    intType *year, intType *month, intType *day, intType *hour,
    intType *min, intType *sec, intType *micro_sec, intType *time_zone,
    boolType *is_dst);
void cmdGetMTimeOfSymlink (const const_striType filePath,
    intType *year, intType *month, intType *day, intType *hour,
    intType *min, intType *sec, intType *micro_sec, intType *time_zone,
    boolType *is_dst);
rtlArrayType cmdGetSearchPath (void);
striType cmdHomeDir (void);
void cmdMkdir (const const_striType dir_name);
void cmdMove (const const_striType source_name, const const_striType dest_name);
rtlArrayType cmdReadDir (const const_striType dirPath);
striType cmdReadLink (const const_striType link_name);
striType cmdReadLinkAbsolute (const const_striType filePath);
void cmdRemoveFile (const const_striType filePath);
void cmdRemoveTree (const const_striType filePath);
void cmdSetenv (const const_striType name, const const_striType value);
void cmdSetATime (const const_striType filePath,
    intType year, intType month, intType day, intType hour,
    intType min, intType sec, intType micro_sec, intType time_zone);
void cmdSetFileMode (const const_striType filePath, const const_setType mode);
void cmdSetMTime (const const_striType filePath,
    intType year, intType month, intType day, intType hour,
    intType min, intType sec, intType micro_sec, intType time_zone);
void cmdSetSearchPath (rtlArrayType searchPath);
intType cmdShell (const const_striType command, const const_striType parameters);
striType cmdShellEscape (const const_striType stri);
void cmdSymlink (const const_striType source_name, const const_striType dest_name);
striType cmdToOsPath (const const_striType standardPath);
void cmdUnsetenv (const const_striType name);

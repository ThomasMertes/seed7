/********************************************************************/
/*                                                                  */
/*  cmd_rtl.h     Primitive actions for various commands.           */
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
/*  Content: Primitive actions for various commands.                */
/*                                                                  */
/********************************************************************/

#ifdef FTELL_WRONG_FOR_PIPE
#define ftell improved_ftell
#endif

#ifdef ANSI_C

biginttype cmdBigFileSize (stritype file_name);
void cmdChdir (stritype dir_name);
void cmdCloneFile (stritype source_name, stritype dest_name);
stritype cmdConfigValue (stritype name);
void cmdCopyFile (stritype source_name, stritype dest_name);
settype cmdFileMode (stritype file_name);
inttype cmdFileSize (stritype file_name);
inttype cmdFileType (stritype file_name);
inttype cmdFileTypeSL (stritype file_name);
stritype cmdGetcwd (void);
void cmdGetATime (stritype file_name,
    inttype *year, inttype *month, inttype *day, inttype *hour,
    inttype *min, inttype *sec, inttype *mycro_sec, inttype *time_zone,
    booltype *is_dst);
void cmdGetCTime (stritype file_name,
    inttype *year, inttype *month, inttype *day, inttype *hour,
    inttype *min, inttype *sec, inttype *mycro_sec, inttype *time_zone,
    booltype *is_dst);
void cmdGetMTime (stritype file_name,
    inttype *year, inttype *month, inttype *day, inttype *hour,
    inttype *min, inttype *sec, inttype *mycro_sec, inttype *time_zone,
    booltype *is_dst);
void cmdMkdir (stritype dir_name);
void cmdMove (stritype source_name, stritype dest_name);
stritype cmdReadlink (stritype link_name);
void cmdRemove (stritype file_name);
void cmdRemoveAnyFile (stritype file_name);
void cmdSetATime (stritype file_name,
    inttype year, inttype month, inttype day, inttype hour,
    inttype min, inttype sec, inttype mycro_sec, inttype time_zone);
void cmdSetFileMode (stritype file_name, settype mode);
void cmdSetMTime (stritype file_name,
    inttype year, inttype month, inttype day, inttype hour,
    inttype min, inttype sec, inttype mycro_sec, inttype time_zone);
inttype cmdShell (stritype command_stri);
void cmdSymlink (stritype source_name, stritype dest_name);
#ifdef FTELL_WRONG_FOR_PIPE
long improved_ftell (FILE *stream);
#endif

#else

biginttype cmdBigFileSize ();
void cmdChdir ();
void cmdCloneFile ();
stritype cmdConfigValue ();
void cmdCopyFile ();
settype cmdFileMode ();
inttype cmdFileSize ();
inttype cmdFileType ();
inttype cmdFileTypeSL ()
stritype cmdGetcwd ();
void cmdGetATime ();
void cmdGetCTime ();
void cmdGetMTime ();
void cmdMkdir ();
void cmdMove ();
stritype cmdReadlink ();
void cmdRemove ();
void cmdRemoveAnyFile ();
void cmdSetATime ();
void cmdSetFileMode ();
void cmdSetMTime ();
inttype cmdShell ();
void cmdSymlink ();
#ifdef FTELL_WRONG_FOR_PIPE
long improved_ftell ();
#endif

#endif

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

#ifdef ANSI_C

stritype getProgramPath (const const_stritype source_file_name);
biginttype cmdBigFileSize (const const_stritype file_name);
void cmdChdir (const const_stritype dir_name);
void cmdCloneFile (const const_stritype source_name, const const_stritype dest_name);
stritype cmdConfigValue (const const_stritype name);
void cmdCopyFile (const const_stritype source_name, const const_stritype dest_name);
settype cmdFileMode (const const_stritype file_name);
inttype cmdFileSize (const const_stritype file_name);
inttype cmdFileType (const const_stritype file_name);
inttype cmdFileTypeSL (const const_stritype file_name);
stritype cmdGetcwd (void);
void cmdGetATime (const const_stritype file_name,
    inttype *year, inttype *month, inttype *day, inttype *hour,
    inttype *min, inttype *sec, inttype *micro_sec, inttype *time_zone,
    booltype *is_dst);
void cmdGetCTime (const const_stritype file_name,
    inttype *year, inttype *month, inttype *day, inttype *hour,
    inttype *min, inttype *sec, inttype *micro_sec, inttype *time_zone,
    booltype *is_dst);
void cmdGetMTime (const const_stritype file_name,
    inttype *year, inttype *month, inttype *day, inttype *hour,
    inttype *min, inttype *sec, inttype *micro_sec, inttype *time_zone,
    booltype *is_dst);
void cmdMkdir (const const_stritype dir_name);
void cmdMove (const const_stritype source_name, const const_stritype dest_name);
stritype cmdReadlink (const const_stritype link_name);
void cmdRemove (const const_stritype file_name);
void cmdRemoveAnyFile (const const_stritype file_name);
void cmdSetATime (const const_stritype file_name,
    inttype year, inttype month, inttype day, inttype hour,
    inttype min, inttype sec, inttype micro_sec, inttype time_zone);
void cmdSetFileMode (const const_stritype file_name, settype mode);
void cmdSetMTime (const const_stritype file_name,
    inttype year, inttype month, inttype day, inttype hour,
    inttype min, inttype sec, inttype micro_sec, inttype time_zone);
inttype cmdShell (const const_stritype command_stri);
void cmdSymlink (const const_stritype source_name, const const_stritype dest_name);

#else

stritype getProgramPath ();
biginttype cmdBigFileSize ();
void cmdChdir ();
void cmdCloneFile ();
stritype cmdConfigValue ();
void cmdCopyFile ();
settype cmdFileMode ();
inttype cmdFileSize ();
inttype cmdFileType ();
inttype cmdFileTypeSL ();
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

#endif

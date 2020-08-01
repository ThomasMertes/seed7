/********************************************************************/
/*                                                                  */
/*  cmd_drv.h     Driver functions for commands.                    */
/*  Copyright (C) 1989 - 2010  Thomas Mertes                        */
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
/*  File: seed7/src/cmd_drv.h                                       */
/*  Changes: 2010  Thomas Mertes                                    */
/*  Content: Driver functions for commands.                         */
/*                                                                  */
/********************************************************************/

#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
typedef struct {
    uint32type magicValue;
    uint32type driveBitmask;
    int currentDrive;
  } volumeListType;

#define IS_VOLUME_LIST(ptr) (ptr != NULL && ((volumeListType *) (ptr))->magicValue == UINT32TYPE_MAX)
#endif


#ifdef C_PLUS_PLUS
#define C "C"
#else
#define C
#endif

#ifdef DEFINE_OS_STAT_PROTOTYPE
extern C int __cdecl os_stat (const_os_stritype path, os_stat_struct *buffer);
#endif


#ifdef ANSI_C

os_stritype *getUtf16Argv (int *w_argc);
void freeUtf16Argv (os_stritype *w_argv);
stritype getExecutablePath (const const_stritype arg_0);
#ifdef DEFINE_WGETENV
os_stritype wgetenv (const const_os_stritype name);
#endif
#ifdef DEFINE_WSETENV
int wsetenv (const const_os_stritype name, const const_os_stritype value,
    int overwrite);
#endif
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
volumeListType *openVolumeList (void);
#endif

#else

os_stritype *getUtf16Argv ();
void freeUtf16Argv ();
stritype getExecutablePath ();
#ifdef DEFINE_WGETENV
os_stritype wgetenv ();
#endif
#ifdef DEFINE_WSETENV
int wsetenv ();
#endif
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
volumeListType *openVolumeList ();
#endif

#endif

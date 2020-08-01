/********************************************************************/
/*                                                                  */
/*  cmd_drv.h     Prototypes of OS specific command functions.      */
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
/*  File: seed7/src/cmd_drv.h                                       */
/*  Changes: 2010  Thomas Mertes                                    */
/*  Content: Prototypes of OS specific command functions.           */
/*                                                                  */
/********************************************************************/

#ifdef C_PLUS_PLUS
#define C "C"
#else
#define C
#endif

os_striType *getUtf16Argv (int *w_argc);
void freeUtf16Argv (os_striType *w_argv);
striType getExecutablePath (const const_striType arg_0);
#if USE_GET_ENVIRONMENT
os_striType *getEnvironment (void);
void freeEnvironment (os_striType *environment);
#endif
#ifdef EMULATE_ENVIRONMENT
char *getenv7 (const char *name);
int setenv7 (const char *name, const char *value, int overwrite);
int unsetenv7 (const char *name);
#endif
#ifdef DEFINE_WGETENV
os_striType wgetenv (const const_os_striType name);
#endif
#ifdef DEFINE_WSETENV
int wsetenv (const const_os_striType name, const const_os_striType value,
    int overwrite);
#endif
#ifdef DEFINE_WUNSETENV
int wunsetenv (const const_os_striType name);
#endif
striType cmdGetGroup (const const_striType filePath);
striType cmdGetOwner (const const_striType filePath);
void cmdSetGroup (const const_striType filePath, striType group);
void cmdSetOwner (const const_striType filePath, striType owner);

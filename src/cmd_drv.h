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

#ifdef DETERMINE_OS_PROPERTIES_AT_RUNTIME
extern char *nullDevice;
extern unsigned char shellPathDelimiter;
extern boolType shellUsesDriveLetters;
#ifdef EMULATE_ENVIRONMENT
extern int (*environmentStrncmp) (const char *s1, const char *s2, size_t n);
#endif
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
#ifdef EMULATE_NODE_ENVIRONMENT
int setenvForNodeJs (const char *name, const char *value, int overwrite);
int unsetenvForNodeJs (const char *name);
#endif
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
#ifdef DEFINE_WIN_RENAME
int winRename (const const_os_striType oldPath, const const_os_striType newPath);
#endif
#if HAS_READLINK
striType doReadLink (const const_striType filePath, errInfoType *err_info);
#define HAS_DO_READ_LINK
#else
#ifdef HAS_DEVICE_IO_CONTROL
striType winReadLink (const const_striType filePath, errInfoType *err_info);
#define doReadLink winReadLink
#define HAS_DO_READ_LINK
#endif
#endif
#ifdef DEFINE_WIN_SYMLINK
void winSymlink (const const_striType targetPath,
    const const_striType symlinkPath, errInfoType *err_info);
#endif
#if defined DEFINE_WCHMOD_EXT && defined HAS_DEVICE_IO_CONTROL
#ifndef HAS_GET_FILE_INFORMATION_BY_HANDLE_EX
int wchmodExt (const wchar_t *path, int pmode);
#endif
#endif
striType cmdGetGroup (const const_striType filePath);
striType cmdGetGroupOfSymlink (const const_striType filePath);
striType cmdGetOwner (const const_striType filePath);
striType cmdGetOwnerOfSymlink (const const_striType filePath);
void cmdSetGroup (const const_striType filePath, const const_striType group);
void cmdSetGroupOfSymlink (const const_striType filePath, const const_striType group);
void cmdSetOwner (const const_striType filePath, const const_striType owner);
void cmdSetOwnerOfSymlink (const const_striType filePath, const const_striType owner);

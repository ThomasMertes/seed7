/********************************************************************/
/*                                                                  */
/*  os_decls.h    Define macros for OS calls and structs.           */
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
/*  File: seed7/src/os_decls.h                                      */
/*  Changes: 2011  Thomas Mertes                                    */
/*  Content: Define macros for OS calls and structs.                */
/*                                                                  */
/********************************************************************/

#ifndef os_chdir
#define os_chdir chdir
#endif
#ifndef os_getcwd
#define os_getcwd getcwd
#endif
#ifndef os_mkdir
#define os_mkdir mkdir
#endif
#ifndef os_rmdir
#define os_rmdir rmdir
#endif
#ifndef os_opendir
#define os_opendir opendir
#endif
#ifndef os_readdir
#define os_readdir readdir
#endif
#ifndef os_closedir
#define os_closedir closedir
#endif
#ifndef os_DIR
#define os_DIR DIR
#endif
#ifndef os_dirent_struct
#define os_dirent_struct struct dirent
#endif
#ifndef os_fstat
#define os_fstat fstat
#endif
#ifndef os_lstat
#define os_lstat lstat
#endif
#ifndef os_stat
#define os_stat stat
#endif
#ifndef os_stat_struct
#define os_stat_struct struct stat
#endif
#ifndef os_fstat_struct
#define os_fstat_struct os_stat_struct
#endif
#ifndef os_chown
#define os_chown chown
#endif
#ifndef os_chmod
#define os_chmod chmod
#endif
#ifndef os_utime
#define os_utime utime
#endif
#ifndef os_utimbuf_struct
#define os_utimbuf_struct struct utimbuf
#endif
#ifndef os_remove
#define os_remove remove
#endif
#ifndef os_rename
#define os_rename rename
#endif
#ifndef os_system
#define os_system system
#endif
#ifndef os_pclose
#define os_pclose pclose
#endif
#ifndef os_popen
#define os_popen popen
#endif
#ifndef os_fopen
#define os_fopen fopen
#endif
#ifndef os_fdopen
#define os_fdopen fdopen
#endif
#ifndef os_poll
#define os_poll poll
#endif
#ifndef os_environ
#define os_environ environ
#endif
#ifndef os_getenv
#define os_getenv getenv
#endif
#ifndef os_isatty
#define os_isatty isatty
#endif
#ifndef os_setmode
#define os_setmode setmode
#endif

#ifdef OS_STRI_WCHAR
#define os_mode_rb L"rb"
#define os_mode_wb L"wb"
#else
#define os_mode_rb "rb"
#define os_mode_wb "wb"
#endif

#if OS_OFF_T_SIZE == 32
#define unsigned_os_off_t uint32Type
#elif OS_OFF_T_SIZE == 64
#define unsigned_os_off_t uint64Type
#else
#error "sizeof(os_off_t) is neither 4 nor 8."
#endif

#ifdef DEFINE_WGETENV
#define os_getenv_string_free(env_var) FREE_OS_STRI(env_var)
#else
#define os_getenv_string_free(env_var)
#endif

/* The definition of fileno below take place */
/* after the system include files. This avoids warnings */
/* about and _fileno being deprecated.          */
#ifdef os_fileno
#define fileno os_fileno
#endif

#if FILENO_WORKS_FOR_NULL
#define safe_fileno(stream) fileno(stream)
#else
#define safe_fileno(stream) ((stream) == NULL ? -1 : fileno(stream))
#endif

#ifndef CC_OPT_OPTIMIZE_1
#define CC_OPT_OPTIMIZE_1 "-O1"
#endif

#ifndef CC_OPT_OPTIMIZE_2
#define CC_OPT_OPTIMIZE_2 "-O2"
#endif

#ifndef CC_OPT_OPTIMIZE_3
#define CC_OPT_OPTIMIZE_3 "-O3"
#endif

#ifndef REDIRECT_FILEDES_1
#define REDIRECT_FILEDES_1 ">"
#endif

#ifndef REDIRECT_FILEDES_2
#define REDIRECT_FILEDES_2 "2>"
#endif

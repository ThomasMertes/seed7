/********************************************************************/
/*                                                                  */
/*  striutl.h     Procedures to work with wide char strings.        */
/*  Copyright (C) 1989 - 2005  Thomas Mertes                        */
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
/*  File: seed7/src/striutl.h                                       */
/*  Changes: 1991, 1992, 1993, 1994, 2005  Thomas Mertes            */
/*  Content: Procedures to work with wide char strings.             */
/*                                                                  */
/********************************************************************/

extern const_cstritype cstri_escape_sequence[];

#define MAX_UTF8_EXPANSION_FACTOR 6
#define max_utf8_size(size) (MAX_UTF8_EXPANSION_FACTOR * (size))
#define free_cstri(cstri,stri) UNALLOC_CSTRI(cstri, max_utf8_size((stri)->size));
#define free_wstri(wstri,stri) free(wstri);
#define cstri_expand(stri,cstri,size) ustri_expand(stri, (const_ustritype) cstri, size)


#ifdef OS_STRI_WCHAR
typedef wchar_t          os_chartype;
typedef uint16type       os_uchartype;
typedef wstritype        os_stritype;
typedef const_wstritype  const_os_stritype;
#define os_stri_strlen   wcslen
#define os_stri_strcpy   wcscpy
#define os_stri_strcat   wcscat
#define os_stri_strchr   wcschr
#define os_stri_alloc    ALLOC_WSTRI
#define os_stri_realloc  REALLOC_WSTRI
#define os_stri_free     free
#else
typedef char             os_chartype;
typedef unsigned char    os_uchartype;
typedef cstritype        os_stritype;
typedef const_cstritype  const_os_stritype;
#define os_stri_strlen   strlen
#define os_stri_strcpy   strcpy
#define os_stri_strcat   strcat
#define os_stri_strchr   strchr
#define os_stri_alloc    ALLOC_CSTRI
#define os_stri_realloc  REALLOC_CSTRI
#define os_stri_free     free
#endif

#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
#ifdef EMULATE_ROOT_CWD
extern os_stritype current_emulated_cwd;
#endif
extern os_chartype emulated_root[];

#define IS_EMULATED_ROOT(os_path) (os_path == emulated_root)
#endif

#define PATH_IS_NORMAL        0
#define PATH_IS_EMULATED_ROOT 1
#define PATH_NOT_MAPPED       2



memsizetype utf8_to_stri (strelemtype *dest_stri, memsizetype *dest_len,
                          const_ustritype ustri, size_t len);
memsizetype utf8_bytes_missing (const_ustritype ustri, size_t len);
memsizetype stri_to_utf8 (ustritype out_stri, register const strelemtype *strelem,
                          memsizetype len);
void stri_export_utf8 (ustritype out_stri, const_stritype in_stri);
void ustri_expand (strelemtype *stri, const_ustritype ustri, size_t len);
void stri_compress (ustritype ustri, const strelemtype *stri, size_t len);
#ifdef OS_STRI_WCHAR
memsizetype stri_to_wstri (wstritype out_wstri, register const strelemtype *strelem,
                           memsizetype len, errinfotype *err_info);
#endif
cstritype cp_to_cstri8 (const_stritype stri);
bstritype stri_to_bstri (const_stritype stri);
bstritype stri_to_bstri8 (const_stritype stri);
#ifdef CONSOLE_WCHAR
bstritype stri_to_bstriw (const_stritype stri);
#endif
stritype cstri_to_stri (const_cstritype cstri);
stritype cstri8_to_stri (const_cstritype cstri);
stritype cstri8_or_cstri_to_stri (const_cstritype cstri);
stritype conv_from_os_stri (const const_os_stritype os_stri, memsizetype length);
os_stritype stri_to_os_stri (const_stritype stri, errinfotype *err_info);
stritype os_stri_to_stri (const_os_stritype os_stri, errinfotype *err_info);
stritype stri_to_standard_path (stritype stri);
stritype cp_from_os_path (const_os_stritype os_path, errinfotype *err_info);
#ifdef EMULATE_ROOT_CWD
void setEmulatedCwd (const os_stritype os_path);
#endif
os_stritype cp_to_os_path (const_stritype std_path, int *path_info,
    errinfotype *err_info);
os_stritype cp_to_command (const const_stritype commandPath,
    const const_stritype parameters, errinfotype *err_info);
#ifdef PATHS_RELATIVE_TO_EXECUTABLE
stritype relativeToProgramPath (const const_stritype basePath,
    const const_cstritype dir);
#endif

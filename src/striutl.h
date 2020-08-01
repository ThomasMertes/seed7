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


#ifdef OS_PATH_WCHAR
typedef wchar_t         *wstritype;
typedef const wchar_t   *const_wstritype;

typedef wchar_t          os_chartype;
typedef uint16type       os_uchartype;
typedef wstritype        os_stritype;
typedef const_wstritype  const_os_stritype;
#define os_stri_strlen   wcslen
#define os_stri_strcpy   wcscpy
#define os_stri_strcat   wcscat
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
#define os_stri_alloc    ALLOC_CSTRI
#define os_stri_realloc  REALLOC_CSTRI
#define os_stri_free     free
#endif


#ifdef ANSI_C

const strelemtype *stri_charpos (const_stritype stri, strelemtype ch);
memsizetype utf8_to_stri (strelemtype *dest_stri, memsizetype *dest_len,
                          const_ustritype ustri, size_t len);
memsizetype utf8_bytes_missing (const_ustritype ustri, size_t len);
cstritype cp_to_cstri (const_stritype stri);
bstritype stri_to_bstri (const_stritype stri);
bstritype stri_to_bstri8 (const_stritype stri);
stritype cstri_to_stri (const_cstritype cstri);
stritype cstri8_or_cstri_to_stri (const_cstritype cstri);
os_stritype stri_to_os_stri (const_stritype stri, errinfotype *err_info);
stritype os_stri_to_stri (const_os_stritype os_stri, errinfotype *err_info);
stritype stri_to_standard_path (stritype stri);
stritype cp_from_os_path (const_os_stritype os_stri, errinfotype *err_info);
os_stritype cp_to_os_path (const_stritype stri, errinfotype *err_info);
os_stritype cp_to_command (const_stritype stri, errinfotype *err_info);
#ifdef PATHS_RELATIVE_TO_EXECUTABLE
stritype relativeToProgramPath (const const_stritype basePath,
    const const_cstritype dir);
#endif

#else

strelemtype *stri_charpos ();
memsizetype utf8_to_stri ();
memsizetype utf8_bytes_missing ();
cstritype cp_to_cstri ();
bstritype stri_to_bstri ();
bstritype stri_to_bstri8 ();
stritype cstri_to_stri ();
stritype cstri8_or_cstri_to_stri ();
stritype stri_to_standard_path ():
os_stritype stri_to_os_stri ();
stritype os_stri_to_stri ();
stritype cp_from_os_path ();
os_stritype cp_to_os_path ();
os_stritype cp_to_command ();
#ifdef PATHS_RELATIVE_TO_EXECUTABLE
stritype relativeToProgramPath ();
#endif

#endif


#ifdef UTF32_STRINGS

#ifdef ANSI_C

memsizetype stri_to_utf8 (ustritype out_stri, register const strelemtype *stri,
                          memsizetype len);
void stri_export (ustritype out_stri, const_stritype in_stri);
void ustri_expand (strelemtype *stri, const_ustritype ustri, size_t len);
void stri_compress (ustritype ustri, const strelemtype *stri, size_t len);

#else

memsizetype stri_to_utf8 ();
void stri_export ();
void ustri_expand ();
void stri_compress ();

#endif

#else

#define stri_export(ustri,stri) memcpy((ustri), (stri)->mem, (stri)->size); (ustri)[(stri)->size] = '\0';
#define ustri_expand(stri,ustri,len) memcpy((stri), (ustri), (len))
#define stri_compress(ustri,stri,len) memcpy((ustri), (stri), (len))

#endif

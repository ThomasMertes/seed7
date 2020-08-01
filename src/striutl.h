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

#define compr_size(stri) (6 * (stri)->size)
#define free_cstri(cstri,stri) UNALLOC_CSTRI(cstri, compr_size(stri));
#define free_wstri(wstri,stri) free(wstri);
#define cstri_expand(stri,cstri,size) ustri_expand(stri, (const_ustritype) cstri, size)
#define cstri0_expand(stri,cstri) ustri0_expand(stri, (const_ustritype) cstri)

#ifdef OS_PATH_WCHAR
typedef wchar_t          os_chartype;
typedef wchar_t         *os_stritype;
#define os_stri_strlen   wcslen
#define os_stri_strcpy   wcscpy
#define os_stri_strcat   wcscat
#define os_stri_alloc    ALLOC_WSTRI
#define os_stri_realloc  REALLOC_WSTRI
#define os_stri_free     free
#else
typedef char             os_chartype;
typedef cstritype        os_stritype;
#define os_stri_strlen   strlen
#define os_stri_strcpy   strcpy
#define os_stri_strcat   strcat
#define os_stri_alloc    ALLOC_CSTRI
#define os_stri_realloc  REALLOC_CSTRI
#define os_stri_free     free
#endif


#ifdef ANSI_C

memsizetype utf8_to_stri (strelemtype *, memsizetype *, const_ustritype, size_t);
memsizetype utf8_bytes_missing (const_ustritype, size_t);
void ustri0_expand (strelemtype *stri, const_ustritype ustri);
cstritype cp_to_cstri (const_stritype);
os_stritype cp_to_os_path (const_stritype, errinfotype *);
bstritype stri_to_bstri (const_stritype);
bstritype stri_to_bstri8 (const_stritype);
stritype cstri_to_stri (const_cstritype cstri);
stritype cstri8_or_cstri_to_stri (const_cstritype cstri);
stritype os_stri_to_stri (os_stritype os_stri);
strelemtype *stri_charpos (stritype, strelemtype);
os_stritype cp_to_command (stritype, errinfotype *);

#else

memsizetype utf8_to_stri ();
memsizetype utf8_bytes_missing ();
void ustri0_expand ();
cstritype cp_to_cstri ();
os_stritype cp_to_os_path ();
bstritype stri_to_bstri ();
bstritype stri_to_bstri8 ();
stritype cstri_to_stri ();
stritype cstri8_or_cstri_to_stri ();
stritype os_stri_to_stri ();
strelemtype *stri_charpos ();
os_stritype cp_to_command ();

#endif


#ifdef UTF32_STRINGS

#ifdef ANSI_C

memsizetype stri_to_utf8 (ustritype, const_stritype);
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

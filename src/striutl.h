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

#define compr_size(stri) (6 * (stri)->size)
#define free_cstri(cstri,stri) UNALLOC_CSTRI(cstri, compr_size(stri));
#define cstri_expand(stri,cstri,size) ustri_expand(stri, (const_ustritype) cstri, size)

#ifdef ANSI_C

memsizetype stri_to_utf8 (ustritype, const_stritype);
memsizetype utf8_to_stri (strelemtype *, memsizetype *, const_ustritype, SIZE_TYPE);
memsizetype utf8_bytes_missing (const_ustritype, SIZE_TYPE);
cstritype cp_to_cstri (const_stritype);
wchar_t *cp_to_wstri (stritype);
bstritype stri_to_bstri (stritype);
bstritype stri_to_bstri8 (const_stritype);
stritype cstri_to_stri (const_cstritype);
stritype cstri8_to_stri (const_cstritype);
strelemtype *stri_charpos (stritype, strelemtype);
cstritype cp_to_command (stritype);

#else

memsizetype stri_to_utf8 ();
memsizetype utf8_to_stri ();
memsizetype utf8_bytes_missing ();
cstritype cp_to_cstri ();
wchar_t *cp_to_wstri ();
bstritype stri_to_bstri ();
bstritype stri_to_bstri8 ();
stritype cstri_to_stri ();
stritype cstri8_to_stri ();
strelemtype *stri_charpos ();
cstritype cp_to_command ();

#endif


#ifdef WIDE_CHAR_STRINGS

#ifdef ANSI_C

void stri_export (ustritype, const_stritype);
void ustri_expand (strelemtype *, const_ustritype, SIZE_TYPE);
void stri_compress (ustritype, const strelemtype *, SIZE_TYPE);

#else

void stri_export ();
void ustri_expand ();
void stri_compress ();

#endif

#else

#define stri_export(ustri,stri) memcpy((ustri), (stri)->mem, (stri)->size); (ustri)[(stri)->size] = '\0';
#define ustri_expand(stri,ustri,len) memcpy((stri), (ustri), (len))
#define stri_compress(ustri,stri,len) memcpy((ustri), (stri), (len))

#endif

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


#ifdef ANSI_C

memsizetype utf8_to_stri (strelemtype *, memsizetype *, const_ustritype, SIZE_TYPE);
memsizetype utf8_bytes_missing (const_ustritype, SIZE_TYPE);
cstritype cp_to_cstri (stritype);
bstritype stri_to_bstri (stritype);
bstritype stri_to_bstri8 (stritype);
stritype cstri_to_stri (const_cstritype);
stritype cstri8_to_stri (const_cstritype);
strelemtype *stri_charpos (stritype, strelemtype);

#else

memsizetype utf8_to_stri ();
memsizetype utf8_bytes_missing ();
cstritype cp_to_cstri ();
bstritype stri_to_bstri ();
bstritype stri_to_bstri8 ();
stritype cstri_to_stri ();
stritype cstri8_to_stri ();
strelemtype *stri_charpos ();

#endif


#ifdef WIDE_CHAR_STRINGS

#ifdef ANSI_C

void stri_export (ustritype, stritype);
void stri_expand (strelemtype *, const_ustritype, SIZE_TYPE);
void stri_compress (ustritype, const strelemtype *, SIZE_TYPE);

#else

void stri_export ();
void stri_expand ();
void stri_compress ();

#endif

#else

#define stri_export(ustri,stri) memcpy((ustri), (stri)->mem, (stri)->size); (ustri)[(stri)->size] = '\0';
#define stri_expand(stri,ustri,len) memcpy((stri), (ustri), (len))
#define stri_compress(ustri,stri,len) memcpy((ustri), (stri), (len))

#endif

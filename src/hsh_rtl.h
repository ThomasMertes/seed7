/********************************************************************/
/*                                                                  */
/*  hsh_rtl.h     Primitive actions for the string type.            */
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
/*  File: seed7/src/hsh_rtl.h                                       */
/*  Changes: 2007  Thomas Mertes                                    */
/*  Content: Primitive actions for the hash type.                   */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

booltype hshContains (const const_rtlHashtype hash1,
                      const rtlGenerictype key, inttype hashcode,
                      comparetype cmp_func);
void hshCpy (rtlHashtype *const hash_to,
             const const_rtlHashtype hash_from,
             createfunctype key_create_func, destrfunctype key_destr_func,
             createfunctype data_create_func, destrfunctype data_destr_func);
rtlHashtype hshCreate (const const_rtlHashtype hash_from,
                       createfunctype key_create_func,
                       destrfunctype key_destr_func,
                       createfunctype data_create_func,
                       destrfunctype data_destr_func);
void hshDestr (const const_rtlHashtype old_hash,
               destrfunctype key_destr_func, destrfunctype data_destr_func);
rtlHashtype hshEmpty (void);
void hshExcl (const rtlHashtype hash1, const rtlGenerictype key,
              inttype hashcode, comparetype cmp_func,
              destrfunctype key_destr_func, destrfunctype data_destr_func);
rtlGenerictype hshIdx (const const_rtlHashtype hash1,
                       const rtlGenerictype key, inttype hashcode,
                       comparetype cmp_func);
rtlObjecttype *hshIdxAddr (const const_rtlHashtype hash1,
                           const rtlGenerictype key,
                           inttype hashcode, comparetype cmp_func);
rtlGenerictype hshIdxWithDefault (const rtlHashtype hash1,
                                  const rtlGenerictype key,
                                  const rtlGenerictype data,
                                  inttype hashcode, comparetype cmp_func,
                                  createfunctype key_create_func,
                                  createfunctype data_create_func);
void hshIncl (const rtlHashtype hash1, const rtlGenerictype key,
              const rtlGenerictype data, inttype hashcode,
              comparetype cmp_func, createfunctype key_create_func,
              createfunctype data_create_func, copyfunctype data_copy_func);
rtlArraytype hshKeys (const const_rtlHashtype hash1,
                      createfunctype key_create_func,
                      destrfunctype key_destr_func);
rtlArraytype hshValues (const const_rtlHashtype hash1,
                        createfunctype value_create_func,
                        destrfunctype value_destr_func);

#else

booltype hshContains ();
void hshCpy ();
rtlHashtype hshCreate ();
void hshDestr ();
rtlHashtype hshEmpty ();
void hshExcl ();
rtlGenerictype hshIdx ();
rtlObjecttype *hshIdxAddr ();
void hshIncl ();
rtlArraytype hshKeys ();
rtlArraytype hshValues ();

#endif

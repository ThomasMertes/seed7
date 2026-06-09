/********************************************************************/
/*                                                                  */
/*  hsh_rtl.h     Primitive actions for the hash map type.          */
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
/*  Content: Primitive actions for the hash map type.               */
/*                                                                  */
/********************************************************************/

void freeGenericHash (rtlHashType genericHash);
boolType hashAdd (const rtlHashType aHashMap, const rtlValueUnion aKey,
                  const rtlValueUnion data, intType hashcode,
                  compareFuncType cmp_func,
                  const createFuncType key_create_func,
                  const createFuncType data_create_func);
rtlHashElemType hshConcatKeyValue (rtlHashElemType element1,
                                   rtlHashElemType element2);
boolType hshContainsGeneric (const const_rtlHashType aHashMap,
                             const genericType aKey, intType hashcode);
boolType hshContains (const const_rtlHashType aHashMap,
                      const rtlValueUnion aKey, intType hashcode,
                      compareFuncType cmp_func);
void hshCpy (rtlHashType *const dest, const const_rtlHashType source,
             const createFuncType key_create_func,
             const destrFuncType key_destr_func,
             const createFuncType data_create_func,
             const destrFuncType data_destr_func);
rtlHashType hshCreate (const const_rtlHashType source,
                       const createFuncType key_create_func,
                       const destrFuncType key_destr_func,
                       const createFuncType data_create_func,
                       const destrFuncType data_destr_func);
void hshDestr (const const_rtlHashType old_hash,
               const destrFuncType key_destr_func,
               const destrFuncType data_destr_func);
rtlHashType hshEmpty (void);
void hshExclGeneric (const rtlHashType hash1, const genericType key,
                     intType hashcode);
void hshExcl (const rtlHashType aHashMap, const rtlValueUnion aKey,
              intType hashcode, compareFuncType cmp_func,
              const destrFuncType key_destr_func,
              const destrFuncType data_destr_func);
rtlHashType hshGenHash (rtlHashElemType keyValuePairs,
                        const hashCodeFuncType key_hash_code_func,
                        compareFuncType cmp_func,
                        const destrFuncType key_destr_func,
                        const destrFuncType data_destr_func);
rtlHashElemType hshGenKeyValue (const rtlValueUnion aKey, const rtlValueUnion aValue);
rtlValueUnion hshIdx (const const_rtlHashType aHashMap,
                      const rtlValueUnion aKey, intType hashcode,
                      compareFuncType cmp_func);
rtlObjectType *hshIdxAddr (const const_rtlHashType aHashMap,
                           const rtlValueUnion aKey, intType hashcode,
                           compareFuncType cmp_func);
rtlObjectType *hshIdxAddr2 (const const_rtlHashType aHashMap,
                            const rtlValueUnion aKey, intType hashcode,
                            compareFuncType cmp_func);
void hshSetGeneric (const const_rtlHashType aHashMap,
                    const genericType aKey, intType hashcode,
                    const genericType newValue);
genericType hshIdxEnterGeneric (const rtlHashType aHashMap,
                                const genericType aKey,
                                const genericType defaultData,
                                intType hashcode);
genericType hshIdxWithDefaultGeneric (const const_rtlHashType aHashMap,
                                      const genericType aKey,
                                      const genericType defaultData,
                                      intType hashcode);
rtlValueUnion hshIdxWithDefault (const const_rtlHashType aHashMap,
                                 const rtlValueUnion aKey,
                                 const rtlValueUnion defaultData,
                                 intType hashcode,
                                 compareFuncType cmp_func);
rtlValueUnion hshIdxDefault0 (const const_rtlHashType aHashMap,
                              const rtlValueUnion aKey,
                              intType hashcode,
                              compareFuncType cmp_func);
void hshIncl (const rtlHashType aHashMap, const rtlValueUnion aKey,
              const rtlValueUnion data, intType hashcode,
              compareFuncType cmp_func,
              const createFuncType key_create_func,
              const createFuncType data_create_func,
              const copyFuncType data_copy_func);
rtlArrayType hshKeys (const const_rtlHashType aHashMap,
                      const createFuncType key_create_func,
                      const destrFuncType key_destr_func);
const_rtlHashElemType hshRand (const const_rtlHashType aHashMap);
rtlValueUnion hshUpdate (const rtlHashType aHashMap, const rtlValueUnion aKey,
                         const rtlValueUnion data, intType hashcode,
                         compareFuncType cmp_func,
                         const createFuncType key_create_func,
                         const createFuncType data_create_func);
rtlArrayType hshValues (const const_rtlHashType aHashMap,
                        const createFuncType value_create_func,
                        const destrFuncType value_destr_func);

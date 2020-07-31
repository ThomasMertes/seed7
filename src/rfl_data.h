/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2008  Thomas Mertes                        */
/*                                                                  */
/*  This program is free software; you can redistribute it and/or   */
/*  modify it under the terms of the GNU General Public License as  */
/*  published by the Free Software Foundation; either version 2 of  */
/*  the License, or (at your option) any later version.             */
/*                                                                  */
/*  This program is distributed in the hope that it will be useful, */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   */
/*  GNU General Public License for more details.                    */
/*                                                                  */
/*  You should have received a copy of the GNU General Public       */
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Compiler data reflection                                */
/*  File: seed7/src/rfl_data.h                                      */
/*  Changes: 1991-1995, 2007, 2008  Thomas Mertes                   */
/*  Content: Primitive actions for the ref_list type.               */
/*                                                                  */
/********************************************************************/

void rflAppend (listType *const list_to, const listType list_from);
listType rflCat (listType list1, const listType list2);
intType rflCmp (const_listType list1, const_listType list2);
void rflCpy (listType *const dest, const const_listType source);
listType rflCreate (const const_listType source);
void rflDestr (const listType old_list);
boolType rflElem (const const_objectType searched_object, const_listType list_element);
void rflElemcpy (listType list, intType position, objectType elem);
boolType rflEq (const_listType list1, const_listType list2);
listType rflHead (const listType list, intType stop);
objectType rflIdx (const_listType list, intType position);
void rflIncl (listType *list, objectType elem);
intType rflIpos (listType list_element, objectType searched_object,
    const intType from_index);
intType rflLng (const_listType list);
listType rflMklist (objectType elem);
intType rflPos (const_listType list_element, const const_objectType searched_object);
listType rflRange (const listType list, intType start, intType stop);
listType rflTail (const_listType list, intType start);

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

void rflAppend (listtype *const list_to, const listtype list_from);
listtype rflCat (listtype list1, const listtype list2);
inttype rflCmp (const_listtype list1, const_listtype list2);
void rflCpy (listtype *const list_to, const const_listtype list_from);
listtype rflCreate (const const_listtype list_from);
void rflDestr (const listtype old_list);
booltype rflElem (const const_objecttype searched_object, const_listtype list_element);
void rflElemcpy (listtype list, inttype position, objecttype elem);
booltype rflEq (const_listtype list1, const_listtype list2);
listtype rflHead (const listtype list, inttype stop);
objecttype rflIdx (const_listtype list, inttype position);
void rflIncl (listtype *list, objecttype elem);
inttype rflIpos (listtype list_element, objecttype searched_object,
    const inttype from_index);
inttype rflLng (const_listtype list);
listtype rflMklist (objecttype elem);
inttype rflPos (const_listtype list_element, const const_objecttype searched_object);
listtype rflRange (const listtype list, inttype start, inttype stop);
listtype rflTail (const_listtype list, inttype start);

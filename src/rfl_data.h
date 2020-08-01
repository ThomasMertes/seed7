/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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

#ifdef ANSI_C

void rflAppend (listtype *list_to, listtype list_from);
listtype rflCat (listtype list1, listtype list2);
void rflCpy (listtype *list_to, listtype list_from);
listtype rflCreate (listtype list_from);
void rflDestr (listtype old_list);
booltype rflElem (objecttype searched_object, listtype list_element);
void rflElemcpy (listtype list, inttype position, objecttype elem);
booltype rflEq (listtype list1, listtype list2);
listtype rflHead (listtype list, inttype stop);
objecttype rflIdx (listtype list, inttype position);
void rflIncl (listtype *list, objecttype elem);
inttype rflLng (listtype list);
listtype rflMklist (objecttype elem);
booltype rflNe (listtype list1, listtype list2);
inttype rflPos (listtype list_element, objecttype searched_object);
listtype rflRange (listtype list, inttype start, inttype stop);
listtype rflTail (listtype list, inttype start);

#else

void rflAppend ();
listtype rflCat ();
void rflCpy ();
listtype rflCreate ();
void rflDestr ();
booltype rflElem ();
void rflElemcpy ();
booltype rflEq ();
listtype rflHead ();
objecttype rflIdx ();
void rflIncl ();
inttype rflLng ();
listtype rflMklist ();
booltype rflNe ();
inttype rflPos ();
listtype rflRange ();
listtype rflTail ();

#endif

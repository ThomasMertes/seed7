/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000  Thomas Mertes                        */
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
/*  Module: Analyzer - Utility                                      */
/*  File: seed7/src/object.h                                        */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Procedures to maintain objects and lists.              */
/*                                                                  */
/********************************************************************/

objecttype new_empty_list_object (const_objecttype typeof_object);
objecttype new_nonempty_expression_object (objecttype first_element,
    listtype *list, const_objecttype typeof_object);
objecttype new_type_of_expression_object (objecttype first_element,
    listtype *list, typetype type_of);
objecttype new_expression_object (listtype *list);
listtype add_element_to_list (listtype list, objecttype object);

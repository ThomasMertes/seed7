/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2010  Thomas Mertes                        */
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
/*  Module: Library                                                 */
/*  File: seed7/src/bstlib.h                                        */
/*  Changes: 2007, 2010  Thomas Mertes                              */
/*  Content: All primitive actions for the byte string type.        */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype bst_append   (listtype arguments);
objecttype bst_cat      (listtype arguments);
objecttype bst_cmp      (listtype arguments);
objecttype bst_cpy      (listtype arguments);
objecttype bst_create   (listtype arguments);
objecttype bst_destr    (listtype arguments);
objecttype bst_empty    (listtype arguments);
objecttype bst_eq       (listtype arguments);
objecttype bst_hashcode (listtype arguments);
objecttype bst_idx      (listtype arguments);
objecttype bst_lng      (listtype arguments);
objecttype bst_ne       (listtype arguments);
objecttype bst_parse    (listtype arguments);
objecttype bst_str      (listtype arguments);
objecttype bst_value    (listtype arguments);

#else

objecttype bst_append ();
objecttype bst_cat ();
objecttype bst_cmp ();
objecttype bst_cpy ();
objecttype bst_create ();
objecttype bst_destr ();
objecttype bst_empty ();
objecttype bst_eq ();
objecttype bst_hashcode ();
objecttype bst_idx ();
objecttype bst_lng ();
objecttype bst_ne ();
objecttype bst_parse ();
objecttype bst_str ();
objecttype bst_value ();

#endif

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
/*  Module: Library                                                 */
/*  File: seed7/src/lstlib.h                                        */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: All primitive actions for the list type.               */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype lst_cat    (listtype arguments);
objecttype lst_cpy    (listtype arguments);
objecttype lst_create (listtype arguments);
objecttype lst_destr  (listtype arguments);
objecttype lst_elem   (listtype arguments);
objecttype lst_empty  (listtype arguments);
objecttype lst_excl   (listtype arguments);
objecttype lst_head   (listtype arguments);
objecttype lst_idx    (listtype arguments);
objecttype lst_incl   (listtype arguments);
objecttype lst_lng    (listtype arguments);
objecttype lst_range  (listtype arguments);
objecttype lst_tail   (listtype arguments);

#else

objecttype lst_cat ();
objecttype lst_cpy ();
objecttype lst_create ();
objecttype lst_destr ();
objecttype lst_elem ();
objecttype lst_empty ();
objecttype lst_excl ();
objecttype lst_head ();
objecttype lst_idx ();
objecttype lst_incl ();
objecttype lst_lng ();
objecttype lst_range ();
objecttype lst_tail ();

#endif

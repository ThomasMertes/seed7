/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/lstlib.h                                        */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: All primitive actions for the list type.               */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype lst_cat (listtype);
objecttype lst_cpy (listtype);
objecttype lst_create (listtype);
objecttype lst_destr (listtype);
objecttype lst_elem (listtype);
objecttype lst_empty (listtype);
objecttype lst_excl (listtype);
objecttype lst_head (listtype);
objecttype lst_idx (listtype);
objecttype lst_incl (listtype);
objecttype lst_lng (listtype);
objecttype lst_range (listtype);
objecttype lst_tail (listtype);

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

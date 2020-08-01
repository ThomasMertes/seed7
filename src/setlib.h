/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2005  Thomas Mertes                        */
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
/*  File: seed7/src/setlib.h                                        */
/*  Changes: 2004  Thomas Mertes                                    */
/*  Content: All primitive actions for the set type.                */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype set_baselit (listtype);
objecttype set_card (listtype);
objecttype set_conv (listtype);
objecttype set_cpy (listtype);
objecttype set_create (listtype);
objecttype set_destr (listtype);
objecttype set_diff (listtype);
objecttype set_elem (listtype);
objecttype set_empty (listtype);
objecttype set_eq (listtype);
objecttype set_excl (listtype);
objecttype set_ge (listtype);
objecttype set_gt (listtype);
objecttype set_incl (listtype);
objecttype set_intersect (listtype);
objecttype set_le (listtype);
objecttype set_lt (listtype);
objecttype set_max (listtype);
objecttype set_min (listtype);
objecttype set_ne (listtype);
objecttype set_not_elem (listtype);
objecttype set_rand (listtype);
objecttype set_symdiff (listtype);
objecttype set_union (listtype);
objecttype set_value (listtype);

#else

objecttype set_baselit ();
objecttype set_card ();
objecttype set_conv ();
objecttype set_cpy ();
objecttype set_create ();
objecttype set_destr ();
objecttype set_diff ();
objecttype set_elem ();
objecttype set_empty ();
objecttype set_eq ();
objecttype set_excl ();
objecttype set_ge ();
objecttype set_gt ();
objecttype set_in ();
objecttype set_incl ();
objecttype set_intersect ();
objecttype set_le ();
objecttype set_lt ();
objecttype set_max ();
objecttype set_min ();
objecttype set_ne ();
objecttype set_not_elem ();
objecttype set_rand ();
objecttype set_symdiff ();
objecttype set_union ();
objecttype set_value ();

#endif

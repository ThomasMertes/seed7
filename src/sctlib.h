/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2002  Thomas Mertes                        */
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
/*  File: seed7/src/arrlib.h                                        */
/*  Changes: 1993, 1994, 2002  Thomas Mertes                        */
/*  Content: All primitive actions for structure types.             */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype sct_alloc (listtype);
objecttype sct_cat (listtype);
objecttype sct_conv (listtype);
objecttype sct_cpy (listtype);
objecttype sct_create (listtype);
objecttype sct_destr (listtype);
objecttype sct_elem (listtype);
objecttype sct_empty (listtype);
objecttype sct_incl (listtype);
objecttype sct_lng (listtype);
objecttype sct_refidx (listtype);
objecttype sct_select (listtype);

#else

objecttype sct_alloc ();
objecttype sct_cat ();
objecttype sct_conv ();
objecttype sct_cpy ();
objecttype sct_create ();
objecttype sct_destr ();
objecttype sct_elem ();
objecttype sct_empty ();
objecttype sct_incl ();
objecttype sct_lng ();
objecttype sct_refidx ();
objecttype sct_select ();

#endif

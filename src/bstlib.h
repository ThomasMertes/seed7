/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2007  Thomas Mertes                        */
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
/*  File: seed7/src/bstlib.h                                        */
/*  Changes: 2007  Thomas Mertes                                    */
/*  Content: All primitive actions for the byte string type.        */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype bst_cpy (listtype);
objecttype bst_create (listtype);
objecttype bst_destr (listtype);
objecttype bst_empty (listtype);

#else

objecttype bst_cpy ();
objecttype bst_create ();
objecttype bst_destr ();
objecttype bst_empty ();

#endif

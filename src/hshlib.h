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
/*  File: seed7/src/hshlib.h                                        */
/*  Changes: 2005  Thomas Mertes                                    */
/*  Content: All primitive actions for hash types.                  */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype hsh_contains (listtype);
objecttype hsh_conv (listtype);
objecttype hsh_cpy (listtype);
objecttype hsh_create (listtype);
objecttype hsh_destr (listtype);
objecttype hsh_empty (listtype);
objecttype hsh_excl (listtype);
objecttype hsh_idx (listtype);
objecttype hsh_idx2 (listtype);
objecttype hsh_incl (listtype);
objecttype hsh_lng (listtype);
objecttype hsh_refidx (listtype);

#else

objecttype hsh_contains ();
objecttype hsh_conv ();
objecttype hsh_cpy ();
objecttype hsh_create ();
objecttype hsh_destr ();
objecttype hsh_empty ();
objecttype hsh_excl ();
objecttype hsh_idx ();
objecttype hsh_idx2 ();
objecttype hsh_incl ();
objecttype hsh_lng ();
objecttype hsh_refidx ();

#endif

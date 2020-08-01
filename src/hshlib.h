/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2009  Thomas Mertes                        */
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
/*  File: seed7/src/hshlib.h                                        */
/*  Changes: 2005  Thomas Mertes                                    */
/*  Content: All primitive actions for hash types.                  */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype hsh_contains     (listtype arguments);
objecttype hsh_conv         (listtype arguments);
objecttype hsh_cpy          (listtype arguments);
objecttype hsh_create       (listtype arguments);
objecttype hsh_destr        (listtype arguments);
objecttype hsh_empty        (listtype arguments);
objecttype hsh_excl         (listtype arguments);
objecttype hsh_for          (listtype arguments);
objecttype hsh_for_data_key (listtype arguments);
objecttype hsh_for_key      (listtype arguments);
objecttype hsh_idx          (listtype arguments);
objecttype hsh_idx2         (listtype arguments);
objecttype hsh_incl         (listtype arguments);
objecttype hsh_keys         (listtype arguments);
objecttype hsh_lng          (listtype arguments);
objecttype hsh_refidx       (listtype arguments);
objecttype hsh_values       (listtype arguments);

#else

objecttype hsh_contains ();
objecttype hsh_conv ();
objecttype hsh_cpy ();
objecttype hsh_create ();
objecttype hsh_destr ();
objecttype hsh_empty ();
objecttype hsh_excl ();
objecttype hsh_for ();
objecttype hsh_for_data_key ();
objecttype hsh_for_key ();
objecttype hsh_idx ();
objecttype hsh_idx2 ();
objecttype hsh_incl ();
objecttype hsh_keys ();
objecttype hsh_lng ();
objecttype hsh_refidx ();
objecttype hsh_values ();

#endif

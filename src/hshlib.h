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

objectType hsh_concat_key_value (listType arguments);
objectType hsh_contains         (listType arguments);
objectType hsh_conv             (listType arguments);
objectType hsh_cpy              (listType arguments);
objectType hsh_create           (listType arguments);
objectType hsh_destr            (listType arguments);
objectType hsh_empty            (listType arguments);
objectType hsh_excl             (listType arguments);
objectType hsh_for              (listType arguments);
objectType hsh_for_data_key     (listType arguments);
objectType hsh_for_key          (listType arguments);
objectType hsh_gen_hash         (listType arguments);
objectType hsh_gen_key_value    (listType arguments);
objectType hsh_idx              (listType arguments);
objectType hsh_idx2             (listType arguments);
objectType hsh_incl             (listType arguments);
objectType hsh_keys             (listType arguments);
objectType hsh_lng              (listType arguments);
objectType hsh_rand_key         (listType arguments);
objectType hsh_refidx           (listType arguments);
objectType hsh_update           (listType arguments);
objectType hsh_values           (listType arguments);

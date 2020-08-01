/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2015  Thomas Mertes                        */
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
/*  File: seed7/src/binlib.h                                        */
/*  Changes: 2015  Thomas Mertes                                    */
/*  Content: All primitive actions for the types bin64 and bin32.   */
/*                                                                  */
/********************************************************************/

objectType bin_add              (listType arguments);
objectType bin_add_assign       (listType arguments);
objectType bin_and              (listType arguments);
objectType bin_and_assign       (listType arguments);
objectType bin_big              (listType arguments);
objectType bin_binary           (listType arguments);
objectType bin_getBinaryFromSet (listType arguments);
objectType bin_lshift           (listType arguments);
objectType bin_lshift_assign    (listType arguments);
objectType bin_or               (listType arguments);
objectType bin_or_assign        (listType arguments);
objectType bin_radix            (listType arguments);
objectType bin_RADIX            (listType arguments);
objectType bin_rshift           (listType arguments);
objectType bin_rshift_assign    (listType arguments);
objectType bin_str              (listType arguments);
objectType bin_xor              (listType arguments);
objectType bin_xor_assign       (listType arguments);

/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2011  Thomas Mertes                        */
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
/*  File: seed7/src/pollib.h                                        */
/*  Changes: 2011  Thomas Mertes                                    */
/*  Content: All primitive actions to support poll.                 */
/*                                                                  */
/********************************************************************/

objectType pol_add_check     (listType arguments);
objectType pol_clear         (listType arguments);
objectType pol_cpy           (listType arguments);
objectType pol_create        (listType arguments);
objectType pol_destr         (listType arguments);
objectType pol_empty         (listType arguments);
objectType pol_get_check     (listType arguments);
objectType pol_get_finding   (listType arguments);
objectType pol_has_next      (listType arguments);
objectType pol_iter_checks   (listType arguments);
objectType pol_iter_findings (listType arguments);
objectType pol_next_file     (listType arguments);
objectType pol_poll          (listType arguments);
objectType pol_remove_check  (listType arguments);
objectType pol_value         (listType arguments);

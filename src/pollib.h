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

objectType pol_addCheck     (listType arguments);
objectType pol_clear        (listType arguments);
objectType pol_cpy          (listType arguments);
objectType pol_create       (listType arguments);
objectType pol_destr        (listType arguments);
objectType pol_empty        (listType arguments);
objectType pol_getCheck     (listType arguments);
objectType pol_getFinding   (listType arguments);
objectType pol_hasNext      (listType arguments);
objectType pol_iterChecks   (listType arguments);
objectType pol_iterFindings (listType arguments);
objectType pol_nextFile     (listType arguments);
objectType pol_poll         (listType arguments);
objectType pol_removeCheck  (listType arguments);
objectType pol_value        (listType arguments);

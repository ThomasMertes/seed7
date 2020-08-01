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

objecttype pol_addCheck     (listtype arguments);
objecttype pol_clear        (listtype arguments);
objecttype pol_cpy          (listtype arguments);
objecttype pol_create       (listtype arguments);
objecttype pol_destr        (listtype arguments);
objecttype pol_empty        (listtype arguments);
objecttype pol_getCheck     (listtype arguments);
objecttype pol_getFinding   (listtype arguments);
objecttype pol_hasNext      (listtype arguments);
objecttype pol_iterChecks   (listtype arguments);
objecttype pol_iterFindings (listtype arguments);
objecttype pol_nextFile     (listtype arguments);
objecttype pol_poll         (listtype arguments);
objecttype pol_removeCheck  (listtype arguments);
objecttype pol_value        (listtype arguments);

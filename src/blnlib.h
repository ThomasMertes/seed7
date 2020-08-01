/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000  Thomas Mertes                        */
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
/*  File: seed7/src/blnlib.h                                        */
/*  Changes: 1999  Thomas Mertes                                    */
/*  Content: All primitive actions for the boolean type.            */
/*                                                                  */
/********************************************************************/

objecttype bln_and    (listtype arguments);
objecttype bln_cpy    (listtype arguments);
objecttype bln_create (listtype arguments);
objecttype bln_ge     (listtype arguments);
objecttype bln_gt     (listtype arguments);
objecttype bln_iconv  (listtype arguments);
objecttype bln_le     (listtype arguments);
objecttype bln_lt     (listtype arguments);
objecttype bln_not    (listtype arguments);
objecttype bln_or     (listtype arguments);
objecttype bln_ord    (listtype arguments);

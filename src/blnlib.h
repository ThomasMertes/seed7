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

objectType bln_and     (listType arguments);
objectType bln_cpy     (listType arguments);
objectType bln_create  (listType arguments);
objectType bln_eq      (listType arguments);
objectType bln_ge      (listType arguments);
objectType bln_gt      (listType arguments);
objectType bln_iconv1  (listType arguments);
objectType bln_iconv3  (listType arguments);
objectType bln_le      (listType arguments);
objectType bln_lt      (listType arguments);
objectType bln_ne      (listType arguments);
objectType bln_not     (listType arguments);
objectType bln_or      (listType arguments);
objectType bln_ord     (listType arguments);
objectType bln_pred    (listType arguments);
objectType bln_succ    (listType arguments);
objectType bln_ternary (listType arguments);
objectType bln_value   (listType arguments);

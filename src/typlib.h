/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2008  Thomas Mertes                        */
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
/*  File: seed7/src/typlib.h                                        */
/*  Changes: 1993, 1994, 1999, 2000  Thomas Mertes                  */
/*  Content: All primitive actions for the type type.               */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype typ_addinterface (listtype arguments);
objecttype typ_cmp          (listtype arguments);
objecttype typ_cpy          (listtype arguments);
objecttype typ_create       (listtype arguments);
objecttype typ_destr        (listtype arguments);
objecttype typ_eq           (listtype arguments);
objecttype typ_func         (listtype arguments);
objecttype typ_gensub       (listtype arguments);
objecttype typ_gentype      (listtype arguments);
objecttype typ_hashcode     (listtype arguments);
objecttype typ_isdeclared   (listtype arguments);
objecttype typ_isderived    (listtype arguments);
objecttype typ_isforward    (listtype arguments);
objecttype typ_isfunc       (listtype arguments);
objecttype typ_isvarfunc    (listtype arguments);
objecttype typ_matchobj     (listtype arguments);
objecttype typ_meta         (listtype arguments);
objecttype typ_ne           (listtype arguments);
objecttype typ_num          (listtype arguments);
objecttype typ_result       (listtype arguments);
objecttype typ_str          (listtype arguments);
objecttype typ_value        (listtype arguments);
objecttype typ_varconv      (listtype arguments);
objecttype typ_varfunc      (listtype arguments);

#else

objecttype typ_addinterface ();
objecttype typ_cmp ();
objecttype typ_cpy ();
objecttype typ_create ();
objecttype typ_destr ();
objecttype typ_eq ();
objecttype typ_func ();
objecttype typ_gensub ();
objecttype typ_gentype ();
objecttype typ_hashcode ();
objecttype typ_isdeclared ();
objecttype typ_isderived ();
objecttype typ_isforward ();
objecttype typ_isfunc ();
objecttype typ_isvarfunc ();
objecttype typ_matchobj ();
objecttype typ_meta ();
objecttype typ_ne ();
objecttype typ_num ();
objecttype typ_result ();
objecttype typ_str ();
objecttype typ_value ();
objecttype typ_varconv ();
objecttype typ_varfunc ();

#endif

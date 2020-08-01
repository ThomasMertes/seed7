/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/typlib.h                                        */
/*  Changes: 1993, 1994, 1999, 2000  Thomas Mertes                  */
/*  Content: All primitive actions for the type type.               */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype typ_cmp (listtype);
objecttype typ_cpy (listtype);
objecttype typ_create (listtype);
objecttype typ_destr (listtype);
objecttype typ_eq (listtype);
objecttype typ_func (listtype);
objecttype typ_gensub (listtype);
objecttype typ_gentype (listtype);
objecttype typ_hashcode (listtype);
objecttype typ_isdeclared (listtype);
objecttype typ_isderived (listtype);
objecttype typ_isforward (listtype);
objecttype typ_isfunc (listtype);
objecttype typ_isvarfunc (listtype);
objecttype typ_matchobj (listtype);
objecttype typ_meta (listtype);
objecttype typ_ne (listtype);
objecttype typ_num (listtype);
objecttype typ_result (listtype);
objecttype typ_str (listtype);
objecttype typ_value (listtype);
objecttype typ_varconv (listtype);
objecttype typ_varfunc (listtype);

#else

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

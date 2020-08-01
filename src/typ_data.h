/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2010  Thomas Mertes                        */
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
/*  Module: Compiler data reflection                                */
/*  File: seed7/src/typ_data.h                                      */
/*  Changes: 1993, 1994, 1999, 2000, 2008, 2010  Thomas Mertes      */
/*  Content: Primitive actions for the type type.                   */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

inttype typCmpGeneric (rtlGenerictype type1, rtlGenerictype type2);
void typCpy (typetype *dest, typetype source);
typetype typCreate (typetype type_from);
void typDestr (typetype old_type);
typetype typFunc (typetype basic_type);
booltype typIsDerived (typetype any_type);
booltype typIsFunc (typetype any_type);
booltype typIsVarfunc (typetype any_type);
objecttype typMatchobj (typetype actual_type);
typetype typMeta (typetype any_type);
inttype typNum (typetype actual_type);
typetype typResult (typetype any_type);
stritype typStr (typetype type_arg);
typetype typVarfunc (typetype basic_type);

#else

inttype typCmpGeneric ();
void typCpy ();
typetype typCreate ();
void typDestr ();
typetype typFunc ();
booltype typIsDerived ();
booltype typIsFunc ();
booltype typIsVarfunc ();
objecttype typMatchobj ();
typetype typMeta ();
inttype typNum ();
typetype typResult ();
stritype typStr ();
typetype typVarfunc ();

#endif

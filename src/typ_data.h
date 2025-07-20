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

intType typCmp (const const_typeType type1, const const_typeType type2);
intType typCmpGeneric (const genericType value1, const genericType value2);
typeType typFunc (typeType basic_type);
listType typInterfaces (typeType basic_type);
boolType typIsDerived (typeType any_type);
boolType typIsFunc (typeType any_type);
boolType typIsVarfunc (typeType any_type);
objectType typMatchobj (typeType actual_type);
typeType typMeta (typeType any_type);
intType typNum (typeType actual_type);
typeType typResult (typeType any_type);
striType typStr (typeType type_arg);
typeType typVarfunc (typeType basic_type);

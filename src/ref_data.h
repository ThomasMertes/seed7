/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
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
/*  Module: Compiler data reflection                                */
/*  File: seed7/src/ref_data.h                                      */
/*  Changes: 1991-1994, 2004, 2007, 2008  Thomas Mertes             */
/*  Content: Primitive actions for the reference type.              */
/*                                                                  */
/********************************************************************/

objectType refAlloc (objectType obj_arg);
intType refArrMaxIdx (objectType obj_arg);
intType refArrMinIdx (objectType obj_arg);
listType refArrToList (objectType obj_arg);
objectType refBody (objectType obj_arg);
intType refCategory (objectType obj_arg);
intType refCatParse (striType category_name);
striType refCatStr (intType aCategory);
striType refFile (objectType obj_arg1);
listType refHshDataToList (objectType obj_arg);
listType refHshKeysToList (objectType obj_arg);
boolType refIsVar (objectType obj_arg);
objectType refItfToSct (objectType obj_arg);
intType refLine (objectType obj_arg);
listType refLocalConsts (objectType obj_arg);
listType refLocalVars (objectType obj_arg);
intType refNum (objectType obj_arg);
listType refParams (objectType obj_arg);
objectType refResini (objectType obj_arg);
objectType refResult (objectType obj_arg);
listType refSctToList (objectType obj_arg);
void refSetCategory (objectType obj_arg, intType aCategory);
void refSetParams (objectType obj_arg, const_listType params);
void refSetType (objectType obj_arg, typeType any_type);
void refSetVar (objectType obj_arg, boolType var_flag);
striType refStr (objectType obj_arg);
typeType refType (objectType obj_arg);
actType actValue (objectType obj_arg);
bigIntType bigValue (objectType obj_arg);
boolType blnValue (objectType obj_arg);
bstriType bstValue (objectType obj_arg);
charType chrValue (objectType obj_arg);
winType drwValue (objectType obj_arg);
fileType filValue (objectType obj_arg);
floatType fltValue (objectType obj_arg);
intType intValue (objectType obj_arg);
pollType polValue (objectType obj_arg);
progType prgValue (objectType obj_arg);
objectType refValue (objectType obj_arg);
listType rflValue (objectType obj_arg);
void rflSetValue (objectType list_to, listType list_from);
setType setValue (objectType obj_arg);
striType strValue (objectType obj_arg);
typeType typValue (objectType obj_arg);

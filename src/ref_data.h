/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2015  Thomas Mertes                        */
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
/*  Changes: 1991-1994, 2004, 2007, 2008, 2015  Thomas Mertes       */
/*  Content: Primitive actions for the reference type.              */
/*                                                                  */
/********************************************************************/

objectType refAlloc (const const_objectType obj_arg);
objectType refAllocStri (boolType var_flag, typeType any_type,
                         const const_striType stri_from);
intType refArrMaxIdx (const const_objectType obj_arg);
intType refArrMinIdx (const const_objectType obj_arg);
listType refArrToList (const const_objectType obj_arg);
objectType refBody (const const_objectType obj_arg);
intType refCategory (const const_objectType obj_arg);
intType refCatParse (striType category_name);
striType refCatStr (intType aCategory);
striType refFile (const const_objectType obj_arg1);
listType refHshDataToList (const const_objectType obj_arg);
listType refHshKeysToList (const const_objectType obj_arg);
boolType refIsVar (const const_objectType obj_arg);
objectType refItfToSct (const const_objectType obj_arg);
intType refLine (const const_objectType obj_arg);
listType refLocalConsts (const const_objectType obj_arg);
listType refLocalVars (const const_objectType obj_arg);
intType refNum (const const_objectType obj_arg);
listType refParams (const const_objectType obj_arg);
objectType refResini (const const_objectType obj_arg);
objectType refResult (const const_objectType obj_arg);
listType refSctToList (const const_objectType obj_arg);
void refSetCategory (objectType obj_arg, intType aCategory);
void refSetParams (objectType obj_arg, const_listType params);
void refSetType (objectType obj_arg, typeType any_type);
void refSetVar (objectType obj_arg, boolType var_flag);
striType refStr (const const_objectType obj_arg);
typeType refType (const const_objectType obj_arg);
actType actValue (const const_objectType obj_arg);
bigIntType bigValue (const const_objectType obj_arg);
boolType blnValue (const_objectType obj_arg);
bstriType bstValue (const const_objectType obj_arg);
charType chrValue (const const_objectType obj_arg);
winType drwValue (const const_objectType obj_arg);
fileType filValue (const const_objectType obj_arg);
floatType fltValue (const const_objectType obj_arg);
intType intValue (const const_objectType obj_arg);
processType pcsValue (const const_objectType obj_arg);
pollType polValue (const const_objectType obj_arg);
progType prgValue (const const_objectType obj_arg);
objectType refValue (const const_objectType obj_arg);
listType rflValue (const const_objectType obj_arg);
void rflSetValue (objectType list_to, listType list_from);
setType setValue (const const_objectType obj_arg);
striType strValue (const const_objectType obj_arg);
typeType typValue (const const_objectType obj_arg);

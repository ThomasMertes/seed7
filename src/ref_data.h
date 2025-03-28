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

objectType refAlloc (const const_objectType aReference);
objectType refAllocInt (boolType isVar, typeType aType,
                        const intType number);
objectType refAllocStri (boolType isVar, typeType aType,
                         const const_striType stri);
objectType refAllocVar (typeType aType, const intType aCategory);
intType refArrMaxIdx (const const_objectType arrayRef);
intType refArrMinIdx (const const_objectType arrayRef);
listType refArrToList (const const_objectType arrayRef);
objectType refBody (const const_objectType funcRef);
intType refCategory (const const_objectType aReference);
intType refCatParse (const const_striType category_name);
striType refCatStr (intType aCategory);
striType refFile (const const_objectType aReference1);
listType refHshDataToList (const const_objectType aReference);
listType refHshKeysToList (const const_objectType aReference);
intType refHshLength (const const_objectType aReference);
boolType refIsTemp (const const_objectType aReference);
boolType refIsVar (const const_objectType aReference);
objectType refItfToSct (const const_objectType aReference);
intType refLine (const const_objectType aReference);
listType refLocalConsts (const const_objectType funcRef);
listType refLocalVars (const const_objectType funcRef);
intType refNum (const const_objectType aReference);
listType refParams (const const_objectType funcRef);
striType refPath (const const_objectType aReference);
objectType refResini (const const_objectType funcRef);
objectType refResult (const const_objectType funcRef);
listType refSctToList (const const_objectType aReference);
void refSetCategory (objectType aReference, intType aCategory);
void refSetParams (objectType funcRef, const_listType params);
void refSetType (objectType aReference, typeType any_type);
void refSetVar (objectType aReference, boolType var_flag);
striType refStr (const const_objectType aReference);
typeType refType (const const_objectType aReference);
actType actValue (const const_objectType aReference);
bigIntType bigValue (const const_objectType aReference);
boolType blnValue (const_objectType aReference);
bstriType bstValue (const const_objectType aReference);
charType chrValue (const const_objectType aReference);
winType drwValue (const const_objectType aReference);
fileType filValue (const const_objectType aReference);
floatType fltValue (const const_objectType aReference);
intType intValue (const const_objectType aReference);
processType pcsValue (const const_objectType aReference);
bstriType pltValue (const const_objectType aReference);
pollType polValue (const const_objectType aReference);
progType prgValue (const const_objectType aReference);
objectType refValue (const const_objectType aReference);
listType rflValue (const const_objectType aReference);
void rflSetValue (objectType dest, listType source);
setType setValue (const const_objectType aReference);
striType strValue (const const_objectType aReference);
const_striType strValueRef (const const_objectType aReference);
typeType typValue (const const_objectType aReference);

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
/*  Module: Compiler data reflection                                */
/*  File: seed7/src/ref_data.h                                      */
/*  Changes: 1991-1994, 2004, 2007, 2008  Thomas Mertes             */
/*  Content: Primitive actions for the reference type.              */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype refAlloc (objecttype obj_arg);
inttype refArrmaxidx (objecttype obj_arg);
inttype refArrminidx (objecttype obj_arg);
listtype refArrtolist (objecttype obj_arg);
objecttype refBody (objecttype obj_arg);
inttype refCategory (objecttype obj_arg);
inttype refCatParse (stritype category_name);
stritype refCatStr (inttype aCategory);
void refCpy (objecttype *dest, objecttype source);
stritype refFile (objecttype obj_arg1);
listtype refHshDataToList (objecttype obj_arg);
listtype refHshKeyToList (objecttype obj_arg);
booltype refIsVar (objecttype obj_arg);
objecttype refItftosct (objecttype obj_arg);
inttype refLine (objecttype obj_arg);
listtype refLocalConsts (objecttype obj_arg);
listtype refLocalVars (objecttype obj_arg);
inttype refNum (objecttype obj_arg);
listtype refParams (objecttype obj_arg);
objecttype refResini (objecttype obj_arg);
objecttype refResult (objecttype obj_arg);
listtype refScttolist (objecttype obj_arg);
void refSetCategory (objecttype obj_arg, inttype aCategory);
void refSetParams (objecttype obj_arg, const_listtype params);
void refSetType (objecttype obj_arg, typetype any_type);
void refSetVar (objecttype obj_arg, booltype var_flag);
stritype refStr (objecttype obj_arg);
typetype refType (objecttype obj_arg);
acttype actValue (objecttype obj_arg);
biginttype bigValue (objecttype obj_arg);
bstritype bstValue (objecttype obj_arg);
chartype chrValue (objecttype obj_arg);
filetype filValue (objecttype obj_arg);
floattype fltValue (objecttype obj_arg);
inttype intValue (objecttype obj_arg);
progtype prgValue (objecttype obj_arg);
objecttype refValue (objecttype obj_arg);
listtype rflValue (objecttype obj_arg);
void rflSetvalue (objecttype list_to, listtype list_from);
settype setValue (objecttype obj_arg);
stritype strValue (objecttype obj_arg);
typetype typValue (objecttype obj_arg);

#else

objecttype refAlloc ();
inttype refArrmaxidx ();
inttype refArrminidx ();
listtype refArrtolist ();
objecttype refBody ();
inttype refCategory ();
inttype refCatParse ();
stritype refCatStr ();
void refCpy ();
stritype refFile ();
listtype refHshDataToList ();
listtype refHshKeyToList ();
booltype refIsVar ();
objecttype refItftosct ();
inttype refLine ();
listtype refLocalConsts ();
listtype refLocalVars ();
inttype refNum ();;
listtype refParams ();
objecttype refResini ();
objecttype refResult ();
listtype refScttolist ();
void refSetCategory ();
void refSetParams ();
void refSetType ();
void refSetVar ();
stritype refStr ();
typetype refType ();
acttype actValue ();
biginttype bigValue ();
bstritype bstValue ();
chartype chrValue ();
filetype filValue ();
floattype fltValue ();
inttype intValue ();
progtype prgValue ();
objecttype refValue ();
listtype rflValue ();
void rflSetvalue ();
settype setValue ();
stritype strValue ();
typetype typValue ();

#endif

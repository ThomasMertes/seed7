/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2025  Thomas Mertes                        */
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
/*  Module: Analyzer - Error                                        */
/*  File: seed7/src/msg_stri.h                                      */
/*  Changes: 2025  Thomas Mertes                                    */
/*  Content: Append various types to a string.                      */
/*                                                                  */
/********************************************************************/

#define copyCStri(msg, cstri) *(msg) = cstri_to_stri(cstri)
#define appendUStri(msg, ustri) appendCStri(msg, (const_cstriType) (ustri))
#define appendPtr(msg, ptr) appendInt(msg, (intType) (memSizeType) (ptr))


void appendString (striType *const msg, const const_striType stri);
void appendChar (striType *const msg, const charType aChar);
void appendCStri (striType *const msg, const const_cstriType cstri);
void appendCStri8 (striType *const msg, const const_cstriType cstri8);
void appendInt (striType *const msg, const intType number);
#if WITH_FLOAT
void appendFloat (striType *const msg, const floatType number);
#endif
void appendBigInt (striType *const msg, const const_bigIntType number);
void appendCharLiteral (striType *const msg, const charType aChar);
void appendStriLiteral (striType *const msg, const const_striType stri);
void appendBStriLiteral (striType *const msg, const const_bstriType bstri);
void appendCategory (striType *const msg, const objectCategory category);
void appendSymbol (striType *const msg, const symbolType *const aSymbol);
void appendType (striType *const msg, const const_typeType anytype);
void appendRealValue (striType *const msg, const const_objectType anyobject);
void appendFormalParam (striType *const msg, const const_objectType formalParam);
void appendListLimited (striType *const msg, const_listType list, int depthLimit);
void appendList (striType *const msg, const const_listType list);
void appendDotExpr (striType *const msg, const_listType list);
void appendObject (striType *const msg, const const_objectType anyobject);
void appendParams (striType *const msg, const const_listType params);
void appendNameList (striType *const msg, const_listType params);
void appendObjectWithParameters (striType *const msg, const_objectType obj_found);

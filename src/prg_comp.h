/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2013  Thomas Mertes                        */
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
/*  Module: Seed7 compiler library                                  */
/*  File: seed7/src/prg_comp.h                                      */
/*  Changes: 1991 - 1994, 2008, 2013  Thomas Mertes                 */
/*  Content: Primitive actions for the program type.                */
/*                                                                  */
/********************************************************************/

void addStructElement (progType aProg, objectType structElement,
    errInfoType *err_info);
void interpret (const const_progType currentProg, const const_rtlArrayType argv,
    memSizeType argvStart, uintType options, const const_striType protFileName);
progType prgBStriParse (const bstriType bstri, const const_setType options,
    const const_rtlArrayType libraryDirs, const fileType errorFile);
void prgCpy (progType *const dest, const progType source);
progType prgCreate (const progType source);
void prgDestr (progType old_prog);
intType prgErrorCount (const const_progType aProg);
objectType prgEval (progType aProgram, objectType anExpression);
objectType prgEvalWithArgs (progType aProgram, objectType object, listType args);
void prgExec (const const_progType aProgram, const const_rtlArrayType parameters,
    const const_setType options, const const_striType protFileName);
progType prgFilParse (const const_striType fileName, const const_setType options,
    const const_rtlArrayType libraryDirs, const fileType errorFile);
void prgGetError (const const_progType aProg, intType errorIndex,
    intType *errorNumber, striType *fileName, intType *lineNumber,
    intType *columnNumber, striType *msg, striType *errorLine);
listType prgGlobalObjects (const const_progType aProg);
objectType prgMatch (const const_progType aProg, listType curr_expr);
objectType prgMatchExpr (const progType aProg, listType curr_expr);
const_striType prgName (const const_progType aProg);
const_striType prgPath (const const_progType aProg);
listType prgStructSymbols (const const_progType aProgram);
progType prgStrParse (const const_striType stri, const const_setType options,
    const const_rtlArrayType libraryDirs, const fileType errorFile);
objectType prgSyobject (const progType aProg, const const_striType syobjectName);
objectType prgSysvar (const const_progType aProg, const const_striType sysvarName);

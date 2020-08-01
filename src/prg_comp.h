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
/*  Module: Seed7 compiler library                                  */
/*  File: seed7/src/prg_comp.h                                      */
/*  Changes: 1991, 1992, 1993, 1994, 2008  Thomas Mertes            */
/*  Content: Primitive actions for the program type.                */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

void prgCpy (progtype *const dest, const progtype source);
listtype prgDeclObjects (const const_progtype aProg);
void prgDestr (progtype old_prog);
inttype prgErrorCount (const const_progtype aProg);
objecttype prgEval (progtype currentProg, objecttype object);
void prgExec (progtype currentProg);
progtype prgFilParse (const const_stritype stri);
objecttype prgMatch (const const_progtype aProg, listtype curr_expr);
objecttype prgMatchExpr (const const_progtype aProg, listtype curr_expr);
progtype prgStrParse (const const_stritype stri);
objecttype prgSyobject (const progtype aProg, const const_stritype syobjectName);
objecttype prgSysvar (const const_progtype aProg, const const_stritype sysvarName);

#else

void prgCpy ();
listtype prgDeclObjects ();
void prgDestr ();
inttype prgErrorCount ();
objecttype prgEval ();
void prgExec ();
progtype prgFilParse ();
objecttype prgMatch ();
objecttype prgMatchExpr ();
progtype prgStrParse ();
objecttype prgSyobject ();
objecttype prgSysvar ();

#endif

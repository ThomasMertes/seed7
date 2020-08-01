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
/*  Module: Seed7 compiler library                                  */
/*  File: seed7/src/prg_comp.h                                      */
/*  Changes: 1991, 1992, 1993, 1994, 2008  Thomas Mertes            */
/*  Content: Primitive actions for the program type.                */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

progtype prgAnalyze (stritype stri);
void prgCpy (progtype *dest, progtype source);
listtype prgDeclObjects (progtype aProg);
void prgDestr (progtype old_prog);
inttype prgErrorCount (progtype aProg);
objecttype prgEval (progtype currentProg, objecttype object);
void prgExec (progtype currentProg);
objecttype prgMatch (progtype currentProg, listtype curr_expr);
progtype prgStrAnalyze (stritype stri);
objecttype prgSyobject (progtype currentProg, stritype stri1);
objecttype prgSysvar (progtype aProg, stritype sysvarName);

#else

progtype prgAnalyze ();
void prgCpy ();
listtype prgDeclObjects ();
void prgDestr (progtype old_prog);
inttype prgErrorCount ();
objecttype prgEval ();
void prgExec ();
objecttype prgMatch ();
progtype prgStrAnalyze ();
objecttype prgSyobject ();
objecttype prgSysvar ();

#endif

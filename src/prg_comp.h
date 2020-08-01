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

#ifdef ANSI_C

void interpret (const const_progtype currentProg, const const_rtlArraytype argv,
                memsizetype argv_start, uinttype options, const const_stritype prot_file_name);
void prgCpy (progtype *const prog_to, const progtype prog_from);
progtype prgCreate (const progtype prog_from);
void prgDestr (progtype old_prog);
inttype prgErrorCount (const const_progtype aProg);
objecttype prgEval (progtype currentProg, objecttype object);
void prgExec (const const_progtype currentProg, const const_rtlArraytype argv,
              const const_settype options, const const_stritype prot_file_name);
progtype prgFilParse (const const_stritype fileName, const const_settype options,
                      const const_rtlArraytype libraryDirs, const const_stritype prot_file_name);
listtype prgGlobalObjects (const const_progtype aProg);
objecttype prgMatch (const const_progtype aProg, listtype curr_expr);
objecttype prgMatchExpr (const const_progtype aProg, listtype curr_expr);
progtype prgStrParse (const const_stritype stri, const const_settype options,
                      const const_rtlArraytype libraryDirs, const const_stritype prot_file_name);
objecttype prgSyobject (const progtype aProg, const const_stritype syobjectName);
objecttype prgSysvar (const const_progtype aProg, const const_stritype sysvarName);

#else

void interpret ();
void prgCpy ();
progtype prgCreate ();
void prgDestr ();
inttype prgErrorCount ();
objecttype prgEval ();
void prgExec ();
progtype prgFilParse ();
listtype prgGlobalObjects ();
objecttype prgMatch ();
objecttype prgMatchExpr ();
progtype prgStrParse ();
objecttype prgSyobject ();
objecttype prgSysvar ();

#endif

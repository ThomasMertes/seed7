/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000  Thomas Mertes                        */
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
/*  Module: Interpreter                                             */
/*  File: seed7/src/doany.h                                         */
/*  Changes: 1993, 1994  Thomas Mertes                              */
/*  Content: Procedures to call several Seed7 functions from C.     */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype exec1 (listtype list);
booltype do_flush (objecttype outfile);
booltype do_wrnl (objecttype outfile);
booltype do_wrstri (objecttype outfile, stritype stri);
booltype do_wrcstri (objecttype outfile, const_cstritype stri);
void init_do_any (void);

#else

objecttype exec1 ();
booltype do_flush ();
booltype do_wrnl ();
booltype do_wrstri ();
booltype do_wrcstri ();
void init_do_any ();

#endif

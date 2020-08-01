/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2005  Thomas Mertes                        */
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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: General                                                 */
/*  File: seed7/src/arrutl.h                                        */
/*  Changes: 2005  Thomas Mertes                                    */
/*  Content: Procedures to maintain objects of type arraytype.      */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

void destroy_array (objecttype, memsizetype, objecttype);
void create_array (objecttype, objecttype,
                   memsizetype, objecttype, objecttype);
void copy_array (objecttype, objecttype, memsizetype, objecttype);
void qsort_array (objecttype, objecttype, objecttype);

#else

void destroy_array ();
void create_array ();
void copy_array ();
void qsort_array ();

#endif

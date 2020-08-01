/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: General                                                 */
/*  File: seed7/src/identutl.h                                      */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: Procedures to maintain objects of type identtype.      */
/*                                                                  */
/********************************************************************/

/* #define IDENT_TABLE(STRI, LEN) prog.ident.table[((STRI[0] & 63) << 4) | (LEN & 15)] */
/* #define IDENT_TABLE(STRI, LEN) prog.ident.table[((STRI[0] << 4) | LEN) & (ID_TABLE_SIZE - 1)] */
/* #define IDENT_TABLE(STRI, LEN) prog.ident.table[((STRI[0] << 4) ^ (STRI[1] << 2) ^ LEN) & (ID_TABLE_SIZE - 1)] */
#define IDENT_TABLE(STRI, LEN) prog.ident.table[((STRI[0] << 4) ^ (STRI[LEN - 1] << 2) ^ LEN) & (ID_TABLE_SIZE - 1)]


#ifdef ANSI_C

identtype new_ident (ustritype, unsigned int);
identtype get_ident (ustritype, unsigned int);
cstritype id_string (identtype);
void close_idents (progtype);
void init_idents (progtype, errinfotype *);

#else

identtype new_ident ();
identtype get_ident ();
cstritype id_string ();
void close_idents ();
void init_idents ();

#endif

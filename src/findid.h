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
/*  Module: Analyzer                                                */
/*  File: seed7/src/findid.h                                        */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: Procedures to maintain the identifier table.           */
/*                                                                  */
/********************************************************************/

#define STRONGEST_PRIORITY ((priorityType)   0)
#define SCOL_PRIORITY      ((priorityType)  49)
#define COM_PRIORITY       ((priorityType)  99)
#define WEAKEST_PRIORITY   ((priorityType) 127)

EXTERN identType current_ident;


#define find_1_ch_ident(ch)  current_ident = prog->ident.table1[(int) (ch)]
#define find_literal_ident() current_ident = prog->ident.literal
#define find_eof_ident()     current_ident = prog->ident.end_of_file


void find_normal_ident (sySizeType length);
void check_list_of_syntax_elements (const_listType elem_list);
void clean_idents (progType currentProg);
void write_idents (progType currentProg);
void init_findid (progType aProg, errInfoType *err_info);

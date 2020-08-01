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
/*  Module: General                                                 */
/*  File: seed7/src/chclsutl.h                                      */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Compute the type of a character very quickly.          */
/*                                                                  */
/********************************************************************/

typedef enum {ILLEGALCHAR, EOFCHAR, LETTERCHAR, DIGITCHAR,
    UNDERLINECHAR, SHARPCHAR, QUOTATIONCHAR, APOSTROPHECHAR,
    LEFTPARENCHAR, PARENCHAR, SPECIALCHAR, SPACECHAR,
    NEWLINECHAR} charclass;

EXTERN charclass ch_class[256 - EOF];
EXTERN booltype ch_name[256 - EOF];
EXTERN booltype ch_op[256 - EOF];
EXTERN booltype ch_not_esc[256 - EOF];
EXTERN booltype ch_std_comment[256 - EOF];
EXTERN inttype digit_value[128];


#define char_class(CHARACTER)       ch_class       [((int)(CHARACTER)) - EOF]
#define name_character(CHARACTER)   ch_name        [((int)(CHARACTER)) - EOF]
#define op_character(CHARACTER)     ch_op          [((int)(CHARACTER)) - EOF]
#define no_escape_char(CHARACTER)   ch_not_esc     [((int)(CHARACTER)) - EOF]
#define std_comment_char(CHARACTER) ch_std_comment [((int)(CHARACTER)) - EOF]


void init_chclass (void);

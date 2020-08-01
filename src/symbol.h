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
/*  Module: Analyzer - Scanner                                      */
/*  File: seed7/src/symbol.h                                        */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Maintains the current symbol of the scanner.           */
/*                                                                  */
/********************************************************************/

typedef enum {STOPSYMBOL, NAMESYMBOL, SPECIALSYMBOL,
    PARENSYMBOL, INTLITERAL, BIGINTLITERAL, CHARLITERAL, STRILITERAL,
    FLOATLITERAL} symbolcategory;

typedef struct {
    symbolcategory sycategory;
    ustritype name;
    unsigned int name_length;
    inttype intvalue;
    biginttype bigintvalue;
    chartype charvalue;
    stritype strivalue;
    memsizetype stri_max;
    floattype floatvalue;
  } symtype;

#ifdef DO_INIT
symtype symbol = {STOPSYMBOL, NULL, 0,
    (inttype) 0, (biginttype) NULL, (chartype) 0,
    (stritype) NULL, 0, (floattype) 0.0};
#else
EXTERN symtype symbol;
#endif


#define check_symb_length(LEN) if (LEN == symbol.name_length) extend_symb_length()
#define check_stri_length(LEN) if (LEN == symbol.stri_max) extend_stri_length()


#ifdef ANSI_C

void extend_symb_length (void);
void extend_stri_length (void);
void init_symbol (errinfotype *);
void close_symbol (void);

#else

void extend_symb_length ();
void extend_stri_length (void);
void init_symbol ();
void close_symbol ();

#endif

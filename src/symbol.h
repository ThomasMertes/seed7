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
/*  Module: Analyzer - Scanner                                      */
/*  File: seed7/src/symbol.h                                        */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Maintains the current symbol of the scanner.           */
/*                                                                  */
/********************************************************************/

typedef enum {STOPSYMBOL, NAMESYMBOL, SPECIALSYMBOL,
    PARENSYMBOL, INTLITERAL, BIGINTLITERAL, CHARLITERAL, STRILITERAL,
    FLOATLITERAL} symbolCategory;

typedef struct {
    symbolCategory sycategory;
    unsigned int syNumberInLine;
    ustriType name;
    sySizeType name_length;
    ucharType utf8_repr[4];
    sySizeType utf8_length;
    intType intValue;
    bigIntType bigIntValue;
    charType charValue;
    striType striValue;
    memSizeType stri_max;
    floatType floatValue;
    boolType unicodeNames;
  } symbolType;

#ifdef DO_INIT
symbolType symbol = {STOPSYMBOL, 0, NULL, 0, "\0\0\0\0", 0,
                     (intType) 0, (bigIntType) NULL, (charType) 0,
                     (striType) NULL, 0, (floatType) 0.0, FALSE};
#else
EXTERN symbolType symbol;
#endif


#define check_symb_length(len)              if (len == symbol.name_length) extend_symb_length()
#define check_symb_length_delta(len, delta) if (len + delta > symbol.name_length) extend_symb_length()
#define check_stri_length(len)              if (len == symbol.stri_max) extend_stri_length()


void extend_symb_length (void);
void extend_stri_length (void);
void init_symbol (errInfoType *err_info);
void close_symbol (void);

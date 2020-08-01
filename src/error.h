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
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Analyzer - Error                                        */
/*  File: seed7/src/error.h                                         */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Submit normal compile time error messages.             */
/*                                                                  */
/********************************************************************/

typedef enum {OUT_OF_HEAP_SPACE,
    NO_SOURCEFILE,
    FILENOTFOUND,
    EOF_ENCOUNTERED,
    CHAR_ILLEGAL,
    COMMENTOPEN,
    ILLEGALPRAGMA,
    WRONGACTION,
    WRONGSYSTEM,
    CARD_DECIMAL_TOO_BIG,
    NEGATIVEEXPONENT,
    DIGITEXPECTED,
    CARD_WITH_EXPONENT_TOO_BIG,
    BASE2TO36ALLOWED,
    EXTDIGITEXPECTED,
    ILLEGALBASEDDIGIT,
    CARD_BASED_TOO_BIG,
    APOSTROPHEXPECTED,
    CHAREXCEEDS,
    WRONG_QUOTATION_REPRESENTATION,
    WRONG_PATH_DELIMITER,
    STRINGESCAPE,
    WRONGNUMERICALESCAPE,
    BACKSLASHEXPECTED,
    STRINGEXCEEDS,
    NAMEEXPECTED,
    CARD_EXPECTED,
    STRI_EXPECTED,
    IDENT_EXPECTED,
    EXPR_EXPECTED,
    PARAM_DECL_FAILED,
    DECL_FAILED,
    EXCEPTION_RAISED,
    OBJTWICEDECLARED,
    OBJUNDECLARED,
    ILLEGAL_ASSOCIATIVITY,
    ILLEGAL_PRIORITY,
    TWO_PARAMETER_SYNTAX,
    EMPTY_SYNTAX,
    DOT_EXPR_REQUESTED,
    FALSE_INFIX_PRIORITY,
    FALSE_PREFIX_PRIORITY,
    WRONG_EXPR_PARAM_PRIORITY,
    WRONG_PREFIX_PRIORITY,
    DOT_EXPR_ILLEGAL,
    EXPECTED_SYMBOL,
    UNEXPECTED_SYMBOL,
    LITERAL_TYPE_UNDEFINED,
    DOLLAR_VALUE_WRONG,
    DOLLAR_TYPE_WRONG,
    NO_MATCH,
    WRONGACCESSRIGHT,
    TYPE_EXPECTED,
    PROC_EXPECTED,
    PARAM_SPECIFIER_EXPECTED,
    EVAL_TYPE_FAILED} errortype;


#ifdef ANSI_C

void place_of_error (errortype err);
void err_warning (errortype err);
void err_num_stri (errortype err, int num_found, int num_expected,
                   const_ustritype stri);
void err_ident (errortype err, const_identtype ident);
void err_object (errortype err, const_objecttype obj_found);
void err_type (errortype err, const_typetype type_found);
void err_expr_obj (errortype err, const_objecttype expr_object,
                   objecttype obj_found);
void err_match (errortype err, objecttype obj_found);
void err_string (errortype err, const_ustritype stri);
void err_stri (errortype err, const_stritype stri);
void err_integer (errortype err, inttype number);
void err_cchar (errortype err, int character);
void err_char (errortype err, chartype character);
void err_at_line (errortype err, linenumtype line);
void err_undeclared (errortype err, filenumtype file_num,
                     linenumtype line, const_ustritype stri);
void err_message (errortype err, const_stritype stri);

#else

void place_of_error ();
void err_warning ();
void err_num_stri ();
void err_ident ();
void err_object ();
void err_type ();
void err_expr_obj ();
void err_match ();
void err_string ();
void err_stri ();
void err_integer ();
void err_cchar ();
void err_char ();
void err_at_line ();
void err_undeclared ();
void err_message ();

#endif

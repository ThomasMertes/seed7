/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000, 2014, 2025  Thomas Mertes            */
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

typedef enum {
    OUT_OF_HEAP_SPACE,
    EOF_ENCOUNTERED,
    CHAR_ILLEGAL,
    COMMENTOPEN,
    FILENOTFOUND,
    WRONG_PATH_DELIMITER,
    ESSENTIAL_INCLUDE_FAILED,
    ILLEGALPRAGMA,
    WRONGACTION,
    WRONGSYSTEM,
    DOLLAR_VALUE_WRONG,
    DOLLAR_TYPE_WRONG,
    SYSTEM_MAIN_MISSING,
    CARD_DECIMAL_TOO_BIG,
    NEGATIVE_EXPONENT,
    DIGIT_EXPECTED,
    CARD_WITH_EXPONENT_TOO_BIG,
    BASE2TO36ALLOWED,
    EXTDIGITEXPECTED,
    ILLEGAL_BASED_DIGIT,
    CARD_BASED_TOO_BIG,
    APOSTROPHEXPECTED,
    CHAREXCEEDS,
    WRONG_QUOTATION_REPRESENTATION,
    STRINGESCAPE,
    WRONG_NUMERICAL_ESCAPE,
    NUMERICAL_ESCAPE_TOO_BIG,
    WRONG_STRING_CONTINUATION,
    STRING_EXCEEDS,
    NAMEEXPECTED,
    CARD_EXPECTED,
    STRI_EXPECTED,
    IDENT_EXPECTED,
    TYPE_EXPECTED,
    PROC_EXPECTED,
    PARAM_SPECIFIER_EXPECTED,
    PARAM_DECL_OR_SYMBOL_EXPECTED,
    EXCEPTION_EXPECTED,
    EXPR_EXPECTED,
    EXPECTED_SYMBOL,
    PARAM_DECL_FAILED,
    DECL_FAILED,
    OBJTWICEDECLARED,
    PREVIOUS_DECLARATION,
    EXCEPTION_RAISED,
    ILLEGAL_ASSOCIATIVITY,
    ILLEGAL_PRIORITY,
    TWO_PARAMETER_SYNTAX,
    EMPTY_SYNTAX,
    SYNTAX_DECLARED_TWICE,
    DOT_EXPR_EXPECTED,
    REDECLARED_INFIX_PRIORITY,
    REDECLARED_PREFIX_PRIORITY,
    WRONG_EXPR_PARAM_PRIORITY,
    WRONG_PREFIX_PRIORITY,
    DOT_EXPR_ILLEGAL,
    NO_MATCH,
    WRONGACCESSRIGHT,
    LITERAL_TYPE_UNDEFINED,
    KIND_OF_IN_PARAM_UNDEFINED,
    OVERLONG_UTF8_ENCODING,
    UTF16_SURROGATE_CHAR_FOUND,
    CHAR_NOT_UNICODE,
    UTF8_CONTINUATION_BYTE_EXPECTED,
    UNEXPECTED_UTF8_CONTINUATION_BYTE,
    SOLITARY_UTF8_START_BYTE,
    UTF16_BYTE_ORDER_MARK_FOUND
  } errorType;

#define err_cstri(err, cstri) err_ustri(err, (const_ustriType)(cstri))


void freeErrorList (parseErrorType error);
void err_warning (errorType err);
void err_num_stri (errorType err, int num_found, int num_expected,
                   const_ustriType stri);
void err_ident (errorType err, const_identType ident);
void err_object (errorType err, const_objectType obj_found);
void err_type (errorType err, const_typeType type_found);
void err_expr_type (errorType err, const_objectType expr_object,
                    const_typeType type_found);
void err_expr_obj (errorType err, const_objectType expr_object,
                   objectType obj_found);
void err_expr_obj_stri (errorType err, const_objectType expr_object,
                        objectType obj_found, const_cstriType stri);
void err_match (errorType err, objectType obj_found);
void err_ustri (errorType err, const const_ustriType stri);
void err_stri (errorType err, const const_striType stri);
void err_integer (errorType err, intType number);
void err_cchar (errorType err, int character);
void err_char (errorType err, charType character);
void err_at_line (errorType err, lineNumType line);
void err_at_file_in_line (errorType err, const_objectType obj_found,
                          fileNumType fileNumber, lineNumType lineNumber);
void err_existing_obj (errorType err, const_objectType obj_found);

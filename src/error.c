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
/*  Module: Analyzer - Error                                        */
/*  File: seed7/src/error.c                                         */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Submit normal compile time error messages.             */
/*                                                                  */
/*  Normal compile time error messages do not terminate the         */
/*  program.                                                        */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdio.h"
#include "stdlib.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "traceutl.h"
#include "identutl.h"
#include "infile.h"
#include "info.h"
#include "symbol.h"
#include "stat.h"

#undef EXTERN
#define EXTERN
#include "error.h"


#ifndef SEEK_SET
#define SEEK_SET 0
#endif


#define MAX_AREA_SIZE 256



#ifdef ANSI_C

static void print_line (linenumtype err_line)
#else

static void print_line (err_line)
linenumtype err_line;
#endif

  {
    long current_position;
    long buffer_start_position;
    int table_size;
    long *nl_table;
    int table_start;
    int table_pos;
    booltype searching;
    int area_size;
    int area_pos;
    int ch;

  /* print_line */
    /* printf("err_line=%lu in_file.line=%lu\n", err_line, in_file.line); */
    if (in_file.name != NULL && (current_position = IN_FILE_TELL()) >= 0L) {
      /* printf("current_position=%lu in_file.character=%d\n",
         current_position, in_file.character); */
      table_size = in_file.line - err_line + 1;
      if (ALLOC_HEAP(nl_table, long *, table_size * sizeof(long))) {
        CNT1_BYT(table_size * sizeof(long));
        if (in_file.character == EOF) {
          buffer_start_position = current_position;
        } else {
          buffer_start_position = current_position - 1;
        } /* if */
        table_pos = 0;
        searching = TRUE;
        do {
          if (buffer_start_position >= MAX_AREA_SIZE) {
            area_size = MAX_AREA_SIZE;
          } else {
            area_size = buffer_start_position;
          } /* if */
          buffer_start_position -= area_size;
          /* printf("buffer_start_position=%ld\n", buffer_start_position); */
          table_start = table_pos;
          IN_FILE_SEEK(buffer_start_position);
          area_pos = 0;
          while (area_pos < area_size) {
            if (next_character() == '\n') {
              nl_table[table_pos] = IN_FILE_TELL();
              table_pos++;
              if (table_pos >= table_size) {
                table_pos = table_start;
                searching = FALSE;
              } /* if */
            } /* if */
            area_pos++;
          } /* while */
        } while (searching && buffer_start_position > 0);
        if (!searching) {
          IN_FILE_SEEK(nl_table[table_pos]);
          if ((ch = next_character()) == '\r') {
            ch = next_character();
          } /* if */
          while (ch != '\r' && ch != '\n' && ch != EOF) {
            if (ch == '\t') {
              fputc(' ', stdout);
            } else if (ch < ' ' || ch > '~') {
              fputc('?', stdout);
            } else {
              fputc(ch, stdout);
            } /* if */
            ch = next_character();
          } /*while */
          fputc('\n', stdout);
          fputc('\n', stdout);
        } /* if */
      } /* if */
      IN_FILE_SEEK(current_position);
    } /* if */
  } /* print_line */



#ifdef ANSI_C

static void print_error_line (void)
#else

static void print_error_line ()
#endif

  {
    long current_position;
    long buffer_start_position;
    int start_index;
    uchartype buffer[1025];
    int buffer_size;
    int start;
    int stop;
    int number;
    int ch;

  /* print_error_line */
    if (in_file.name != NULL && (current_position = IN_FILE_TELL()) >= 0L) {
      if (current_position >= 512) {
        buffer_start_position = current_position - 512;
        start_index = 511;
      } else {
        buffer_start_position = 0;
        if (current_position > 0) {
          start_index = (int) (current_position - 1);
        } else {
          start_index = 0;
        } /* if */
      } /* if */
      IN_FILE_SEEK(buffer_start_position);
      buffer_size = 0;
      while (buffer_size < 1024 && (ch = next_character()) != EOF) {
        buffer[buffer_size] = (uchartype) ch;
        buffer_size++;
      } /* if */
      start = start_index - 1;
      while (start >= 0 && buffer[start] != '\n' &&
          buffer[start] != '\r') {
        start--;
      } /* while */
      stop = start_index;
      while (stop < buffer_size && buffer[stop] != '\n' &&
          buffer[stop] != '\r') {
        stop++;
      } /* while */
/*    printf("******************************\n\n"); */
/*    printf("%d\n", in_file.character); */
      for (number = start + 1; number < stop; number++) {
        if (buffer[number] == '\t') {
          buffer[number] = ' ';
        } else if (buffer[number] < ' ' || buffer[number] > '~') {
          buffer[number] = '?';
        } /* if */
      } /* for */
      fwrite(&buffer[start + 1], 1, (SIZE_TYPE) (stop - start - 1), stdout);
      printf("\n");
      for (number = 0; number < start_index - start - 1; number++) {
        fputc('-', stdout);
      } /* for */
      printf("^\n");
      IN_FILE_SEEK(current_position);
    } /* if */
  } /* print_error_line */



#ifdef ANSI_C

void place_of_error (errortype err)
#else

void place_of_error (err)
errortype err;
#endif

  { /* place_of_error */
/*  print_error_line(); */
    error_count++;
    if (in_file.name != NULL) {
      printf("*** %s(%1u):%d: ", in_file.name, in_file.line, ((int) err) + 1);
    } else {
      printf("*** ");
    } /* if */
  } /* place_of_error */



#ifdef ANSI_C

static void undef_err (void)
#else

static void undef_err ()
#endif

  { /* undef_err */
    printf("Undefined error\n");
  } /* undef_err */



#ifdef ANSI_C

static void write_symbol (void)
#else

static void write_symbol ()
#endif

  { /* write_symbol */
    if (symbol.syclass == PARENSYMBOL) {
      printf(" \"%c\"\n", symbol.name[0]);
    } else if (symbol.syclass == INTLITERAL) {
      printf(" \"%ld\"\n", symbol.intvalue);
    } else if (symbol.syclass == CHARLITERAL) {
      if (symbol.charvalue >= ' ' && symbol.charvalue <= '~') {
        printf(" \"'%c'\"\n", (char) symbol.charvalue);
      } else {
        printf(" \"'\\%lu\\'\"\n", symbol.charvalue);
      } /* if */
    } else if (symbol.syclass == STRILITERAL) {
      prot_stri(symbol.strivalue);
      printf("\n");
#ifdef WITH_FLOAT
    } else if (symbol.syclass == FLOATLITERAL) {
      printf(" \"%f\"\n", symbol.floatvalue);
#endif
    } else {
      printf(" \"%s\"\n", symbol.name);
    } /* if */
  } /* write_symbol */



#ifdef ANSI_C

static void write_type (typetype anytype)
#else

static void write_type (params)
typetype anytype;
#endif

  { /* write_type */
    if (anytype != NULL) {
      if (anytype->name != NULL) {
        printf("%s", anytype->name->entity->ident->name);
      } else if (anytype->result_type != NULL &&
          anytype->result_type->name != NULL) {
        printf("func %s", anytype->result_type->name->entity->ident->name);
      } else {
        printf(" *ANONYM_TYPE* ");
      } /* if */
    } else {
      printf(" *NULL_TYPE* ");
    } /* if */
  } /* write_type */



#ifdef ANSI_C

static void write_name_list (listtype params)
#else

static void write_name_list (params)
listtype params;
#endif

  {
    objecttype formal_param;
    int in_formal_param_list = 0;

  /* write_name_list */
    while (params != NULL) {
      if (CLASS_OF_OBJ(params->obj) == FORMPARAMOBJECT) {
        if (in_formal_param_list) {
          printf(", ");
        } else {
          printf(" (");
          in_formal_param_list = 1;
        } /* if */
        formal_param = params->obj->value.objvalue;
        switch (CLASS_OF_OBJ(formal_param)) {
          case VALUEPARAMOBJECT:
            printf("in ");
            if (VAR_OBJECT(formal_param)) {
              printf(" var ");
            } /* if */
            write_type(formal_param->type_of);
            if (HAS_DESCRIPTOR_ENTITY(formal_param)) {
              printf(": %s", formal_param->descriptor.entity->ident->name);
            } else {
              printf(" param");
            } /* if */
            break;
          case REFPARAMOBJECT:
            if (VAR_OBJECT(formal_param)) {
              printf("inout ");
            } else {
              printf("ref ");
            } /* if */
            write_type(formal_param->type_of);
            if (HAS_DESCRIPTOR_ENTITY(formal_param)) {
              printf(": %s", formal_param->descriptor.entity->ident->name);
            } else {
              printf(" param");
            } /* if */
            break;
          case TYPEOBJECT:
            if (HAS_DESCRIPTOR_ENTITY(formal_param)) {
              printf("attr %s", formal_param->descriptor.entity->ident->name);
            } else {
              printf("attr ");
              write_type(formal_param->value.typevalue);
            } /* if */
            break;
          default:
            printf("unknown formal ");
            trace1(formal_param);
            break;
        } /* switch */
      } else {
        if (in_formal_param_list) {
          printf(") ");
          in_formal_param_list = 0;
        } else {
          printf(" ");
        } /* if */
        switch (CLASS_OF_OBJ(params->obj)) {
          case SYMBOLOBJECT:
            printf("%s", params->obj->descriptor.entity->ident->name);
            break;
          default:
            printf("unknown param ");
            trace1(params->obj);
            break;
        } /* switch */
      } /* if */
      params = params->next;
    } /* while */
    if (in_formal_param_list) {
      printf(") ");
    } /* if */
  } /* write_name_list */



#ifdef ANSI_C

static void write_object (objecttype anyobject)
#else

static void write_object (anyobject)
objecttype anyobject;
#endif

  { /* write_object */
    if (anyobject == NULL) {
      printf("(NULL)");
    } else {
      switch (CLASS_OF_OBJ(anyobject)) {
        case TYPEOBJECT:
          printf("type ");
          write_type(anyobject->value.typevalue);
          break;
        case VALUEPARAMOBJECT:
          printf("parameter (in ");
          if (VAR_OBJECT(anyobject)) {
            printf(" var ");
          } /* if */
          write_type(anyobject->type_of);
          if (HAS_DESCRIPTOR_ENTITY(anyobject)) {
            printf(": %s", anyobject->descriptor.entity->ident->name);
          } else {
            printf(" param");
          } /* if */
          printf(")");
          break;
        case REFPARAMOBJECT:
          printf("parameter (");
          if (VAR_OBJECT(anyobject)) {
            printf("inout ");
          } else {
            printf("ref ");
          } /* if */
          write_type(anyobject->type_of);
          if (HAS_DESCRIPTOR_ENTITY(anyobject)) {
            printf(": %s", anyobject->descriptor.entity->ident->name);
          } else {
            printf(" param");
          } /* if */
          printf(")");
          break;
        case INTOBJECT:
        case CHAROBJECT:
        case STRIOBJECT:
        case FILEOBJECT:
        case FLOATOBJECT:
        case ARRAYOBJECT:
        case STRUCTOBJECT:
        case BLOCKOBJECT:
          if (VAR_OBJECT(anyobject)) {
            printf("variable ");
         } else {
            printf("constant ");
          } /* if */
          write_type(anyobject->type_of);
          if (HAS_DESCRIPTOR_ENTITY(anyobject)) {
            printf(": %s", anyobject->descriptor.entity->ident->name);
          } else {
            printvalue(anyobject);
          } /* if */
          break;
        case CALLOBJECT:
        case MATCHOBJECT:
          if (anyobject->value.listvalue != NULL &&
              anyobject->value.listvalue->obj != NULL) {
            printf("expression (");
            prot_list(anyobject->value.listvalue->next);
            printf(") of type ");
            write_type(anyobject->value.listvalue->obj->type_of);
          } /* if */
          break;
        default:
          if (HAS_DESCRIPTOR_ENTITY(anyobject)) {
            printf(id_string(anyobject->descriptor.entity->ident));
          } else {
            printf("%d ", CLASS_OF_OBJ(anyobject));
            printf(" *NULL_ENTITY_OBJECT*");
          } /* if */
          break;
      } /* switch */
    } /* if */
  } /* write_object */



#ifdef ANSI_C

void err_warning (errortype err)
#else

void err_warning (err)
errortype err;
#endif

  { /* err_warning */
    place_of_error(err);
    switch (err) {
      case OUT_OF_HEAP_SPACE:
        printf("No more memory\n");
        break;
      case EOF_ENCOUNTERED:
        printf("\"END OF FILE\" encountered\n");
        break;
      case ILLEGALPRAGMA:
        printf("Illegal pragma"); write_symbol();
        break;
      case WRONGSYSTEM:
        printf("Illegal system declaration"); write_symbol();
        break;
      case NEGATIVEEXPONENT:
        printf("Negative exponent in integer literal\n");
        break;
      case CHAREXCEEDS:
        printf("Character literal exceeds source line\n");
        break;
      case STRINGEXCEEDS:
        printf("String literal exceeds source line\n");
        break;
      case WRONG_QUOTATION_REPRESENTATION:
        printf("Use \\\" instead of \"\" to represent \" in a string\n");
        break;
      case WRONG_PATH_DELIMITER:
        printf("Use / instead of \\ as path delimiter\n");
        break;
      case NAMEEXPECTED:
        printf("Name expected found"); write_symbol();
        break;
      case STRI_EXPECTED:
        printf("String literal expected found"); write_symbol();
        break;
      case ILLEGAL_ASSOCIATIVITY:
        printf("Associativity expected found"); write_symbol();
        break;
      case EXPR_EXPECTED:
        printf("Expression expected found"); write_symbol();
        break;
      case TWO_PARAMETER_SYNTAX:
        printf("Syntax with two parameters before operator is illegal\n");
        break;
      case EMPTY_SYNTAX:
        printf("Empty syntax declaration\n");
        break;
      case DOT_EXPR_REQUESTED:
        printf("Dot expression requested as syntax description\n");
        break;
      case DECL_EXPECTED:
        printf("Declaration expected found"); write_symbol();
        break;
      case EXPR_CLASS_EXPECTED:
        printf("Expression expected after \"begin\"\n");
        break;
      case TYPE_EXPECTED:
        printf("Type expected found"); write_symbol();
        break;
      case LITERAL_TYPE_UNDEFINED:
        printf("Undefined type for literal"); write_symbol();
        break;
      case DOLLAR_VALUE_WRONG:
        printf("\"newtype\", \"subtype\", \"func\", \"enumlit\" or \"action\" expected found"); write_symbol();
        break;
      case DOLLAR_TYPE_WRONG:
        printf("\"func\" or \"type\" expected found"); write_symbol();
        break;
      default:
        undef_err();
        break;
    } /* switch */
    print_error_line();
    display_compilation_info();
  } /* err_warning */



#ifdef ANSI_C

void err_num_stri (errortype err, int num_found, int num_expected,
    ustritype stri)
#else

void err_num_stri (err, num_found, num_expected, stri)
errortype err;
int num_found;
int num_expected;
ustritype stri;
#endif

  { /* err_num_stri */
    place_of_error(err);
    switch (err) {
      case FALSE_INFIX_PRIORITY:
        printf("\"%s\" redeclared with infix priority %d not %d\n",
            stri, num_found, num_expected);
        break;
      case FALSE_PREFIX_PRIORITY:
        printf("\"%s\" redeclared with prefix priority %d not %d\n",
            stri, num_found, num_expected);
        break;
      case WRONG_EXPR_PARAM_PRIORITY:
        printf("Priority %d required for parameter after \"%s\" not %d\n",
            num_expected, stri, num_found);
        break;
      case WRONG_PREFIX_PRIORITY:
        printf("Priority <= %d expected found \"%s\" with priority %d\n",
            num_expected, stri, num_found);
        break;
      case DOT_EXPR_ILLEGAL:
        printf("\"%s\" must have priority %d not %d for dot expression\n",
            stri, num_expected, num_found);
        break;
      case ILLEGALBASEDDIGIT:
        printf("Illegal digit \"%c\" in based integer \"%d#%s\"\n",
            num_found, num_expected, stri);
        break;
      case CARD_BASED_TOO_BIG:
        printf("Based integer \"%d#%s\" too big\n",
            num_expected, stri);
        break;
      case CARD_WITH_EXPONENT_TOO_BIG:
        printf("Integer \"%dE%s\" too big\n",
            num_expected, stri);
        break;
      default:
        undef_err();
        break;
    } /* switch */
    print_error_line();
    display_compilation_info();
  } /* err_num_stri */



#ifdef ANSI_C

void err_ident (errortype err, identtype ident)
#else

void err_ident (err, ident)
errortype err;
identtype ident;
#endif

  { /* err_ident */
    place_of_error(err);
    switch (err) {
      case PARAM_SPECIFIER_EXPECTED:
        printf("Parameter specifier expected found \"%s\"\n",
            ident->name);
        break;
      case EXPECTED_SYMBOL:
        printf("\"%s\" expected found", ident->name);
        write_symbol();
        break;
      default:
        undef_err();
        break;
    } /* switch */
    print_error_line();
    display_compilation_info();
  } /* err_ident */



#ifdef ANSI_C

void err_object (errortype err, objecttype obj_found)
#else

void err_object (err, obj_found)
errortype err;
objecttype obj_found;
#endif

  { /* err_object */
    /* place_of_error(err); */
    error_count++;
    if (HAS_POSINFO(obj_found)){
      printf("*** %s(%1u):%d: ", file_name(GET_FILE_NUM(obj_found)),
          GET_LINE_NUM(obj_found), ((int) err) + 1);
    } else if (in_file.name != NULL) {
      printf("*** %s(%1u):%d: ", in_file.name, in_file.line, ((int) err) + 1);
    } else {
      printf("*** ");
    } /* if */
    switch (err) {
      case OBJTWICEDECLARED:
        if (obj_found->descriptor.entity->name_list == NULL) {
          printf("\"%s\" declared twice\n", obj_found->descriptor.entity->ident->name);
        } else {
          write_name_list(obj_found->descriptor.entity->name_list);
          printf(" declared twice\n");
        } /* if */
        break;
      case OBJUNDECLARED:
        printf("\"%s\" not declared\n", obj_found->descriptor.entity->ident->name);
        break;
      case PARAM_DECL_FAILED:
        printf("Declaration of parameter ");
        prot_list(obj_found->value.listvalue->next);
        printf(" failed\n");
        break;
      case DECL_FAILED:
        if (obj_found->descriptor.entity->name_list == NULL) {
          printf("Declaration of \"%s\" failed\n", obj_found->descriptor.entity->ident->name);
        } else {
          printf("Declaration of ");
          write_name_list(obj_found->descriptor.entity->name_list);
          printf(" failed\n");
        } /* if */
        break;
      case EXCEPTION_RAISED:
        printf("Exception \"%s\" raised\n", obj_found->descriptor.entity->ident->name);
        break;
      case IDENT_EXPECTED:
        printf("Identifier expected found \"");
        switch (CLASS_OF_OBJ(obj_found)) {
          case INTOBJECT:
            printf("%ld", obj_found->value.intvalue);
            break;
          case CHAROBJECT:
            fputc((char) obj_found->value.charvalue, stdout);
            break;
          case STRIOBJECT:
            fwrite(obj_found->value.strivalue->mem,
                (SIZE_TYPE) obj_found->value.strivalue->size,
                1, stdout);
            break;
#ifdef WITH_FLOAT
          case FLOATOBJECT:
            printf("%f", obj_found->value.floatvalue);
            break;
#endif
          default:
            printclass(CLASS_OF_OBJ(obj_found));
            printf(" ");
            trace1(obj_found);
            break;
        } /* switch */
        printf("\"\n");
        break;
      case NO_MATCH:
        printf("Match for ");
        prot_list(obj_found->value.listvalue);
        printf(" failed\n");
        break;
      case PROC_EXPECTED:
        printf("Procedure expected found ");
        printobject(obj_found);
        printf(" expression\n");
        break;
      case TYPE_EXPECTED:
        printf("Type expected found ");
        printobject(obj_found);
        printf(" \n");
        break;
      case EVAL_TYPE_FAILED:
        printf("Evaluate type expression ");
        trace1(obj_found);
        printf(" failed\n");
        break;
      default:
        undef_err();
        break;
    } /* switch */
    if (HAS_POSINFO(obj_found)){
      print_line(GET_LINE_NUM(obj_found));
    } else {
      print_error_line();
    } /* if */
    display_compilation_info();
  } /* err_object */



#ifdef ANSI_C

void err_type (errortype err, typetype type_found)
#else

void err_type (err, type_found)
errortype err;
typetype type_found;
#endif

  { /* err_type */
    place_of_error(err);
    switch (err) {
      case PROC_EXPECTED:
        printf("Procedure expected found ");
        printobject(type_found->match_obj);
        printf(" expression\n");
        break;
      default:
        undef_err();
        break;
    } /* switch */
    print_error_line();
    display_compilation_info();
  } /* err_type */



#ifdef ANSI_C

void err_expr_obj (errortype err, listtype expr, objecttype obj_found)
#else

void err_expr_obj (err, expr, obj_found)
errortype err;
listtype expr;
objecttype obj_found;
#endif

  { /* err_list_obj */
    place_of_error(err);
    switch (err) {
      case WRONGACCESSRIGHT:
        printf("Variable expected in ");
        prot_list(expr);
        printf(" found ");
        write_object(obj_found);
        printf("\n");
        break;
      default:
        undef_err();
        break;
    } /* switch */
    print_error_line();
    display_compilation_info();
  } /* err_list_obj */



#ifdef ANSI_C

void err_match (errortype err, objecttype obj_found)
#else

void err_match (err, obj_found)
errortype err;
objecttype obj_found;
#endif

  { /* err_match */
    /* place_of_error(err); */
    error_count++;
    if (HAS_POSINFO(obj_found)){
      printf("*** %s(%1u):%d: ", file_name(GET_FILE_NUM(obj_found)),
          GET_LINE_NUM(obj_found), ((int) err) + 1);
    } else if (in_file.name != NULL) {
      printf("*** %s(%1u):%d: ", in_file.name, in_file.line, ((int) err) + 1);
    } else {
      printf("*** ");
    } /* if */
    switch (err) {
      case NO_MATCH:
        printf("Match for ");
        prot_list(obj_found->value.listvalue);
        printf(" failed\n");
        break;
      default:
        undef_err();
        break;
    } /* switch */
    if (HAS_POSINFO(obj_found)){
      print_line(GET_LINE_NUM(obj_found));
    } else {
      print_error_line();
    } /* if */
    display_compilation_info();
  } /* err_match */



#ifdef ANSI_C

void err_string (errortype err, ustritype stri)
#else

void err_string (err, stri)
errortype err;
ustritype stri;
#endif

  { /* err_string */
    place_of_error(err);
    switch (err) {
      case UNEXPECTED_SYMBOL:
        printf("\"%s\" expected found", stri); write_symbol();
        break;
      case CARD_DECIMAL_TOO_BIG:
        printf("Integer \"%s\" too big\n", stri);
        break;
      case CARD_EXPECTED:
        printf("Integer literal expected found \"%s\"\n", stri);
        break;
      case FILENOTFOUND:
        printf("Include file \"%s\" not found\n", stri);
        break;
      default:
        undef_err();
        break;
    } /* switch */
    print_error_line();
    display_compilation_info();
  } /* err_string */



#ifdef ANSI_C

void err_stri (errortype err, stritype stri)
#else

void err_stri (err, stri)
errortype err;
stritype stri;
#endif

  { /* err_stri */
    place_of_error(err);
    switch (err) {
      case FILENOTFOUND:
        printf("Include file ");
        prot_stri(stri);
        printf(" not found\n");
        break;
      case WRONGACTION:
        printf("Illegal action ");
        prot_stri(stri);
        printf("\n");
        break;
      default:
        undef_err();
        break;
    } /* switch */
    print_error_line();
    display_compilation_info();
  } /* err_stri */



#ifdef ANSI_C

void err_integer (errortype err, inttype number)
#else

void err_integer (err, number)
errortype err;
inttype number;
#endif

  { /* err_integer */
    place_of_error(err);
    switch (err) {
      case BASE2TO36ALLOWED:
        printf("Integer base \"%ld\" not between 2 and 36\n", number);
        break;
      case ILLEGAL_PRIORITY:
        printf("Statement priority \"%ld\" too big\n", number);
        break;
      default:
        undef_err();
        break;
    } /* switch */
    print_error_line();
    display_compilation_info();
  } /* err_integer */



#ifdef ANSI_C

void err_cchar (errortype err, int character)
#else

void err_cchar (err, character)
errortype err;
int character;
#endif

  { /* err_cchar */
    place_of_error(err);
    switch (err) {
      case CHAR_ILLEGAL:
        printf("Illegal character in text \"");
        break;
      case DIGITEXPECTED:
        printf("Digit expected found \"");
        break;
      case EXTDIGITEXPECTED:
        printf("Extended digit expected found \"");
        break;
      case APOSTROPHEXPECTED:
        printf("\"'\" expected found \"");
        break;
      case BACKSLASHEXPECTED:
        printf("String continuations should end with \"\\\" not \"");
        break;
      case WRONGNUMERICALESCAPE:
        printf("Numerical escape sequences should end with \"\\\" not \"");
        break;
      case STRINGESCAPE:
        printf("Illegal string escape \"\\");
        break;
      default:
        undef_err();
        break;
    } /* switch */
    if (character >= ' ' && character <= '~') {
      printf("%c\"\n", character);
    } else {
      printf("\\%d\\\"\n", character);
    } /* if */
    print_error_line();
    display_compilation_info();
  } /* err_cchar */



#ifdef ANSI_C

void err_char (errortype err, chartype character)
#else

void err_char (err, character)
errortype err;
chartype character;
#endif

  { /* err_char */
    place_of_error(err);
    switch (err) {
      case CHAR_ILLEGAL:
        printf("Illegal character in text \"");
        break;
      default:
        undef_err();
        break;
    } /* switch */
    if (character >= ' ' && character <= '~') {
      printf("%c\"\n", (int) character);
    } else {
      printf("\\%lu\\\"\n", character);
    } /* if */
    print_error_line();
    display_compilation_info();
  } /* err_char */



#ifdef ANSI_C

void err_at_line (errortype err, linenumtype line)
#else

void err_at_line (err, line)
errortype err;
linenumtype line;
#endif

  { /* err_at_line */
    error_count++;
    printf("*** %s(%1u):%d: ", in_file.name, line, ((int) err) + 1);
    switch (err) {
      case COMMENTOPEN:
        printf("Unclosed comment\n");
        break;
      default:
        undef_err();
        break;
    } /* switch */
    print_line(line);
    display_compilation_info();
  } /* err_at_line */



#ifdef ANSI_C

void err_undeclared (errortype err, filenumtype file_num,
    linenumtype line, ustritype stri)
#else

void err_undeclared (err, file_num, line, stri)
errortype err;
filenumtype file_num;
linenumtype line;
ustritype stri;
#endif

  { /* err_undeclared */
    error_count++;
    printf("*** %s(%1u):%d: ", file_name(file_num),
        line, ((int) err) + 1);
    switch (err) {
      case OBJUNDECLARED:
        printf("\"%s\" not declared\n", stri);
        break;
      default:
        undef_err();
        break;
    } /* switch */
    print_error_line();
    display_compilation_info();
  } /* err_undeclared */



#ifdef ANSI_C

void err_message (errortype err, stritype stri)
#else

void err_message (err, stri)
errortype err;
stritype stri;
#endif

  { /* err_message */
    error_count++;
    printf("*** ");
    switch (err) {
      case NO_SOURCEFILE:
        printf("File ");
        prot_stri(stri);
        printf(" not found\n");
        break;
      default:
        undef_err();
        break;
    } /* switch */
  } /* err_message */

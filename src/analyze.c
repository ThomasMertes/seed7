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
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Analyzer - Main                                         */
/*  File: seed7/src/analyze.c                                       */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: Main procedure of the analyzing phase.                 */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "sigutl.h"
#include "data.h"
#include "heaputl.h"
#include "striutl.h"
#include "syvarutl.h"
#include "traceutl.h"
#include "chclsutl.h"
#include "identutl.h"
#include "entutl.h"
#include "fatal.h"
#include "scanner.h"
#include "parser.h"
#include "symbol.h"
#include "syntax.h"
#include "info.h"
#include "infile.h"
#include "error.h"
#include "findid.h"
#include "object.h"
#include "stat.h"
#include "atom.h"
#include "expr.h"
#include "match.h"
#include "name.h"
#include "exec.h"
#include "primitiv.h"
#include "doany.h"
#include "option.h"
#include "runerr.h"

#undef EXTERN
#define EXTERN
#include "analyze.h"

#undef TRACE_ANALYZE
#undef TRACE_DECL_ANY


/* when the analyzer is used from a compiled program this */
/* flag decides which exception handler should be called: */
booltype in_analyze = FALSE;

static booltype analyze_initialized = FALSE;
progrecord prog;



#ifdef ANSI_C

static void init_analyze (void)
#else

static void init_analyze ()
#endif

  { /* init_analyze */
    if (!analyze_initialized) {
      set_trace(NULL, -1, NULL);
      init_lib_path();
      init_chclass();
      init_primitiv();
      init_do_any();
      memset(&prog, 0, sizeof(progrecord)); /* not used, saved in analyze and interpr */
      analyze_initialized = TRUE;
    } /* if */
  } /* init_analyze */



#ifdef ANSI_C

static INLINE void system_var (void)
#else

static INLINE void system_var ()
#endif

  {
    int index_found;
    objecttype sys_object;

  /* system_var */
#ifdef TRACE_ANALYZE
    printf("BEGIN system_var\n");
#endif
    scan_symbol();
    if (symbol.sycategory == STRILITERAL) {
      index_found = find_sysvar(symbol.strivalue);
      if (index_found == -1) {
        err_warning(WRONGSYSTEM);
      } /* if */
    } else {
      err_warning(STRI_EXPECTED);
      index_found = -1;
    } /* if */
    scan_symbol();
    if (current_ident == prog.id_for.is) {
      scan_symbol();
    } else {
      err_ident(EXPECTED_SYMBOL, prog.id_for.is);
    } /* if */
    sys_object = read_atom();
    if (index_found != -1) {
      prog.sys_var[index_found] = sys_object;
    } /* if */
    if (current_ident == prog.id_for.semicolon) {
      scan_symbol();
    } else {
      err_ident(EXPECTED_SYMBOL, prog.id_for.semicolon);
    } /* if */
#ifdef TRACE_ANALYZE
    printf("END system_var\n");
#endif
  } /* system_var */



#ifdef ANSI_C

static INLINE void include_file (void)
#else

static INLINE void include_file ()
#endif

  {
    stritype include_file_name;
    errinfotype err_info = OKAY_NO_ERROR;

  /* include_file */
#ifdef TRACE_ANALYZE
    printf("BEGIN include_file\n");
#endif
    scan_symbol();
    if (symbol.sycategory == STRILITERAL) {
      if (!ALLOC_STRI(include_file_name, symbol.strivalue->size)) {
        err_warning(OUT_OF_HEAP_SPACE);
      } else {
        include_file_name->size = symbol.strivalue->size;
        memcpy(include_file_name->mem, symbol.strivalue->mem,
            (size_t) symbol.strivalue->size * sizeof(strelemtype));
        scan_symbol();
        if (current_ident != prog.id_for.semicolon) {
          err_ident(EXPECTED_SYMBOL, prog.id_for.semicolon);
        } /* if */
        if (stri_charpos(symbol.strivalue, '\\') != NULL) {
          err_stri(WRONG_PATH_DELIMITER, include_file_name);
          scan_symbol();
        } else {
          find_include_file(include_file_name, &err_info);
          if (err_info == MEMORY_ERROR) {
            err_warning(OUT_OF_HEAP_SPACE);
          } else if (err_info != OKAY_NO_ERROR) {
            /* FILE_ERROR or RANGE_ERROR */
            err_stri(FILENOTFOUND, include_file_name);
          } else {
            scan_byte_order_mark();
          } /* if */
          scan_symbol();
        } /* if */
        FREE_STRI(include_file_name, include_file_name->size);
      } /* if */
    } else {
      err_warning(STRI_EXPECTED);
      if (current_ident != prog.id_for.semicolon) {
        scan_symbol();
      } /* if */
      if (current_ident != prog.id_for.semicolon) {
        err_ident(EXPECTED_SYMBOL, prog.id_for.semicolon);
      } else {
        scan_symbol();
      } /* if */
    } /* if */
#ifdef TRACE_ANALYZE
    printf("END include_file\n");
#endif
  } /* include_file */



#ifdef ANSI_C

static void process_pragma (void)
#else

static void process_pragma ()
#endif

  {
    cstritype str1;
    errinfotype err_info = OKAY_NO_ERROR;

  /* process_pragma */
#ifdef TRACE_SCANNER
    printf("BEGIN process_pragma\n");
#endif
    if (symbol.sycategory != NAMESYMBOL) {
      err_warning(NAMEEXPECTED);
    } else {
      if (strcmp((cstritype) symbol.name, "library") == 0) {
        scan_symbol();
        if (symbol.sycategory == STRILITERAL) {
          if (stri_charpos(symbol.strivalue, '\\') != NULL) {
            err_warning(WRONG_PATH_DELIMITER);
            scan_symbol();
          } else {
            append_to_lib_path(symbol.strivalue, &err_info);
            if (err_info != OKAY_NO_ERROR) {
              fatal_memory_error(SOURCE_POSITION(2111));
            } /* if */
          } /* if */
        } else {
          err_warning(STRI_EXPECTED);
        } /* if */
      } else if (strcmp((cstritype) symbol.name, "message") == 0) {
        scan_symbol();
        if (symbol.sycategory == STRILITERAL) {
#ifdef WITH_COMPILATION_INFO
          if (option.compilation_info) {
            NL_LIN_INFO();
          } /* if */
#endif
          str1 = cp_to_cstri(symbol.strivalue);
          if (str1 != NULL) {
            puts(str1);
            fflush(stdout);
            free_cstri(str1, symbol.strivalue);
          } /* if */
          display_compilation_info();
        } else {
          err_warning(STRI_EXPECTED);
        } /* if */
      } else if (strcmp((cstritype) symbol.name, "info") == 0) {
        scan_symbol();
        if (strcmp((cstritype) symbol.name, "on") == 0) {
#ifdef WITH_COMPILATION_INFO
          if (!option.compilation_info) {
            option.compilation_info = TRUE;
            display_compilation_info();
          } /* if */
          option.linecount_info = TRUE;
#endif
        } else if (strcmp((cstritype) symbol.name, "off") == 0) {
#ifdef WITH_COMPILATION_INFO
          if (option.compilation_info) {
            size_t number;
            for (number = 1; number <= 7 + strlen((cstritype) in_file.name);
                number++) {
              fputc(' ', stdout);
            } /* for */
            fputc('\r', stdout);
            fflush(stdout);
            option.compilation_info = FALSE;
          } /* if */
          option.linecount_info = FALSE;
#endif
        } else {
          err_warning(ILLEGALPRAGMA);
        } /* if */
      } else if (strcmp((cstritype) symbol.name, "trace") == 0) {
        scan_symbol();
        if (symbol.sycategory == STRILITERAL) {
          set_trace2(symbol.strivalue);
        } else {
          err_warning(STRI_EXPECTED);
        } /* if */
      } else if (strcmp((cstritype) symbol.name, "decls") == 0) {
        trace_nodes();
      } else {
        err_warning(ILLEGALPRAGMA);
      } /* if */
    } /* if */
#ifdef TRACE_SCANNER
    printf("END process_pragma\n");
#endif
  } /* process_pragma */



#ifdef ANSI_C

static INLINE void decl_any (nodetype objects)
#else

static INLINE void decl_any (objects)
nodetype objects;
#endif

  {
    objecttype decl_expression;
    objecttype decl_matched;
    objecttype decl_result;
    errinfotype err_info = OKAY_NO_ERROR;

  /* decl_any */
#ifdef TRACE_ANALYZE
    printf("BEGIN decl_any\n");
#endif
    scan_symbol();
    while (symbol.sycategory != STOPSYMBOL) {
      if (current_ident == prog.id_for.dollar) {
        err_info = OKAY_NO_ERROR;
        scan_symbol();
        if (current_ident == prog.id_for.constant) {
          decl_const(objects, &err_info);
        } else if (current_ident == prog.id_for.syntax) {
          decl_syntax();
        } else if (current_ident == prog.id_for.system) {
          system_var();
        } else if (current_ident == prog.id_for.include) {
          include_file();
        } else {
          process_pragma();
          scan_symbol();
          if (current_ident != prog.id_for.semicolon) {
            err_ident(EXPECTED_SYMBOL, prog.id_for.semicolon);
          } else {
            scan_symbol();
          } /* if */
        } /* if */
        if (err_info != OKAY_NO_ERROR) {
          fatal_memory_error(SOURCE_POSITION(2112));
        } /* if */
      } else {
#ifdef TRACE_DECL_ANY
        printf("before parse expression ");
#endif
        decl_expression = pars_infix_expression(SCOL_PRIORITY, FALSE);
#ifdef TRACE_DECL_ANY
        printf("decl_expression ");
        trace1(decl_expression);
        printf("<<<\n");
        fflush(stdout);
#endif
        if (CATEGORY_OF_OBJ(decl_expression) == EXPROBJECT) {
          decl_matched = match_expression(decl_expression);
        } else {
          decl_matched = decl_expression;
        } /* if */
#ifdef TRACE_DECL_ANY
        printf("decl_matched ");
        trace1(decl_matched);
        printf("<<<\n");
        fflush(stdout);
#endif
        if (current_ident != prog.id_for.semicolon) {
          err_ident(EXPECTED_SYMBOL, prog.id_for.semicolon);
        } /* if */
        fail_flag = FALSE;
        decl_result = evaluate(decl_expression);
        if (fail_flag) {
          err_object(EXCEPTION_RAISED, fail_value);
          fail_flag = FALSE;
        } /* if */
        if (current_ident == prog.id_for.semicolon) {
          scan_symbol();
        } /* if */
      } /* if */
    } /* while */
#ifdef TRACE_ANALYZE
    printf("END decl_any\n");
#endif
  } /* decl_any */



#ifdef ANSI_C

static progtype analyze_prog (const_ustritype source_file_name, errinfotype *err_info)
#else

static progtype analyze_prog (source_file_name, err_info)
ustritype source_file_name;
errinfotype *err_info;
#endif

  {
    progrecord prog_backup;
    progtype resultProg;

  /* analyze_prog */
#ifdef TRACE_ANALYZE
    printf("BEGIN analyze_prog\n");
#endif
    if (!ALLOC_RECORD(resultProg, progrecord, count.prog)) {
      *err_info = MEMORY_ERROR;
    } else {
      resultProg->usage_count = 1;
      resultProg->main_object = NULL;
      memcpy(&prog_backup, &prog, sizeof(progrecord));
      init_idents(&prog, err_info);
      init_findid(err_info);
      init_entity(err_info);
      init_sysvar();
      init_declaration_root(&prog, err_info);
      init_stack(&prog, err_info);
      init_symbol(err_info);
      reset_statistic();
      prog.error_count = 0;
      if (*err_info == OKAY_NO_ERROR) {
        set_trace(option.comp_trace_level, -1, option.prot_file_name);
        decl_any(prog.declaration_root);
        if (SYS_MAIN_OBJECT == NULL) {
          prog.error_count++;
          printf("*** System declaration for main missing\n");
        } else if (CATEGORY_OF_OBJ(SYS_MAIN_OBJECT) == FORWARDOBJECT) {
          prog.error_count++;
          printf("*** Declaration for main missing\n");
        } else {
/*          printf("main defined as: ");
          trace1(SYS_MAIN_OBJECT);
          printf("\n"); */
          if (HAS_ENTITY(SYS_MAIN_OBJECT)) {
            if (GET_ENTITY(SYS_MAIN_OBJECT)->owner != NULL) {
              if (GET_ENTITY(SYS_MAIN_OBJECT)->owner->obj != NULL) {
                resultProg->main_object = GET_ENTITY(SYS_MAIN_OBJECT)->owner->obj;
                if ((resultProg->main_object = match_object(resultProg->main_object)) != NULL) {
/*                  printf("main after match_object: ");
                  trace1(resultProg->main_object);
                  printf("\n"); */
                } else {
                  prog.error_count++;
                  printf("*** Main not callobject\n");
                } /* if */
              } else {
                prog.error_count++;
                printf("*** GET_ENTITY(SYS_MAIN_OBJECT)->objects->obj == NULL\n");
              } /* if */
            } else {
              prog.error_count++;
              printf("*** GET_ENTITY(SYS_MAIN_OBJECT)->objects == NULL\n");
            } /* if */
          } else {
            prog.error_count++;
            printf("*** GET_ENTITY(SYS_MAIN_OBJECT) == NULL\n");
          } /* if */
        } /* if */
        /* close_stack(&prog); * can be used when no matching is done during the runtime */
        /* close_declaration_root(&prog); * can be used when no matching is done during the runtime */
        if (option.show_ident_table) {
          write_idents();
        } /* if */
        clean_idents();
        resultProg->source_file_name = source_file_name;
        resultProg->error_count      = prog.error_count;
        memcpy(&resultProg->ident,    &prog.ident, sizeof(idroottype));
        memcpy(&resultProg->id_for,   &prog.id_for, sizeof(findidtype));
        memcpy(&resultProg->sys_var,  &prog.sys_var, sizeof(systype));
        resultProg->declaration_root = prog.declaration_root;
        resultProg->stack_global     = prog.stack_global;
        resultProg->stack_data       = prog.stack_data;
        resultProg->stack_current    = prog.stack_current;
        memcpy(&prog, &prog_backup, sizeof(progrecord));
        close_infile();
        close_symbol();
        if (option.compilation_info || option.linecount_info) {
          show_statistic();
          if (resultProg->error_count >= 1) {
            printf("%6d error%s found\n",
                resultProg->error_count,
                resultProg->error_count > 1 ? "s" : "");
          } /* if */
        } /* if */
        /* trace_list(resultProg->stack_current->local_object_list); */
      } /* if */
    } /* if */
#ifdef TRACE_ANALYZE
    printf("END analyze_prog\n");
#endif
    return resultProg;
  } /* analyze_prog */



#ifdef ANSI_C

progtype analyze (const_ustritype source_file_name)
#else

progtype analyze (source_file_name)
ustritype source_file_name;
#endif

  {
    unsigned int len;
    stritype source_name;
    unsigned int name_len;
    booltype add_extension;
    progtype resultProg;
    errinfotype err_info = OKAY_NO_ERROR;

  /* analyze */
#ifdef TRACE_ANALYZE
    printf("BEGIN analyze\n");
#endif
    in_analyze = TRUE;
    init_analyze();
    resultProg = NULL;
    len = strlen((const_cstritype) source_file_name);
    if (len > 4 && strcmp((const_cstritype) &source_file_name[len - 4], ".sd7") == 0) {
      name_len = len;
      add_extension = FALSE;
    } else {
      name_len = len + 4;
      add_extension = TRUE;
    } /* if */
    if (!ALLOC_STRI(source_name, name_len)) {
      /* fatal_memory_error(SOURCE_POSITION(2113)); */
      err_warning(OUT_OF_HEAP_SPACE);
    } else {
      source_name->size = name_len;
      ustri_expand(source_name->mem, source_file_name, len);
#if PATH_DELIMITER != '/'
      {
        unsigned int pos;

        for (pos = 0; pos < len; pos++) {
          if (source_name->mem[pos] == PATH_DELIMITER) {
            source_name->mem[pos] = (strelemtype) '/';
          } /* if */
        } /* for */
      }
#endif
      if (add_extension) {
        cstri_expand(&source_name->mem[len], ".sd7", 4);
      } /* if */
      open_infile(source_name, &err_info);
      if (err_info == FILE_ERROR && add_extension) {
        err_info = OKAY_NO_ERROR;
        source_name->size = name_len - 4;
        open_infile(source_name, &err_info);
      } /* if */
      if (err_info == FILE_ERROR) {
        err_message(NO_SOURCEFILE, source_name);
        resultProg = NULL;
      } else if (err_info == OKAY_NO_ERROR) {
        scan_byte_order_mark();
        resultProg = analyze_prog(source_file_name, &err_info);
      } /* if */
      if (err_info == MEMORY_ERROR) {
        err_warning(OUT_OF_HEAP_SPACE);
        resultProg = NULL;
      } /* if */
      FREE_STRI(source_name, name_len);
    } /* if */
    in_analyze = FALSE;
#ifdef TRACE_ANALYZE
    printf("END analyze\n");
#endif
    return resultProg;
  } /* analyze */



#ifdef ANSI_C

progtype analyze_string (stritype input_string)
#else

progtype analyze_string (input_string)
stritype input_string;
#endif

  {
    bstritype input_bstri;
    errinfotype err_info = OKAY_NO_ERROR;
    progtype resultProg;

  /* analyze_string */
#ifdef TRACE_ANALYZE
    printf("BEGIN analyze_string\n");
#endif
    in_analyze = TRUE;
    init_analyze();
    resultProg = NULL;
    input_bstri = stri_to_bstri8(input_string);
    if (input_bstri != NULL) {
      open_string(input_bstri, &err_info);
      if (err_info == OKAY_NO_ERROR) {
        resultProg = analyze_prog((const_ustritype) "STRING", &err_info);
        if (err_info == MEMORY_ERROR) {
          err_warning(OUT_OF_HEAP_SPACE);
          resultProg = NULL;
        } /* if */
      } /* if */
      FREE_BSTRI(input_bstri, input_bstri->size);
    } /* if */
    in_analyze = FALSE;
#ifdef TRACE_ANALYZE
    printf("END analyze_string\n");
#endif
    return resultProg;
  } /* analyze_string */

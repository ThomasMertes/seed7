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
/*  Module: Analyzer - Main                                         */
/*  File: seed7/src/analyze.c                                       */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: Main procedure of the analyzing phase.                 */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "version.h"
#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "syvarutl.h"
#include "striutl.h"
#include "traceutl.h"
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
#include "option.h"
#include "runerr.h"

#undef EXTERN
#define EXTERN
#include "analyze.h"

#undef TRACE_ANALYZE
#undef TRACE_DECL_ANY



#ifdef ANSI_C

static INLINE void system_var (nodetype objects)
#else

static INLINE void system_var (objects)
nodetype objects;
#endif

  {
    int index_found;
    objecttype sys_object;

  /* system_var */
#ifdef TRACE_ANALYZE
    printf("BEGIN system_var\n");
#endif
    scan_symbol();
    if (symbol.syclass == STRILITERAL) {
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
    errinfotype err_info = NO_ERROR;

  /* include_file */
#ifdef TRACE_ANALYZE
    printf("BEGIN include_file\n");
#endif
    scan_symbol();
    if (symbol.syclass == STRILITERAL) {
      if (!ALLOC_STRI(include_file_name, symbol.strivalue->size)) {
        err_warning(OUT_OF_HEAP_SPACE);
      } else {
        COUNT_STRI(symbol.strivalue->size);
        include_file_name->size = symbol.strivalue->size;
        memcpy(include_file_name->mem, symbol.strivalue->mem,
            (SIZE_TYPE) symbol.strivalue->size * sizeof(strelemtype));
        scan_symbol();
        if (current_ident != prog.id_for.semicolon) {
          err_ident(EXPECTED_SYMBOL, prog.id_for.semicolon);
        } /* if */
        find_include_file(include_file_name, &err_info);
        if (err_info == FILE_ERROR) {
          err_stri(FILENOTFOUND, include_file_name);
        } else if (err_info != NO_ERROR) {
          err_warning(OUT_OF_HEAP_SPACE);
        } /* if */
        FREE_STRI(include_file_name, include_file_name->size);
        scan_symbol();
      } /* if */
    } else {
      err_warning(STRI_EXPECTED);
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
    int number;
    cstritype str1;

  /* process_pragma */
#ifdef TRACE_SCANNER
    printf("BEGIN process_pragma\n");
#endif
    if (symbol.syclass != NAMESYMBOL) {
      err_warning(NAMEEXPECTED);
    } else {
      if (strcmp((cstritype) symbol.name, "library") == 0) {
        scan_symbol();
        if (symbol.syclass == STRILITERAL) {
          set_search_path(symbol.strivalue);
        } else {
          err_warning(STRI_EXPECTED);
        } /* if */
      } else if (strcmp((cstritype) symbol.name, "message") == 0) {
        scan_symbol();
        if (symbol.syclass == STRILITERAL) {
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
        if (symbol.syclass == STRILITERAL) {
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
    errinfotype err_info = NO_ERROR;

  /* decl_any */
#ifdef TRACE_ANALYZE
    printf("BEGIN decl_any\n");
#endif
    scan_symbol();
    while (symbol.syclass != STOPSYMBOL) {
      if (current_ident == prog.id_for.dollar) {
        err_info = NO_ERROR;
        scan_symbol();
        if (current_ident == prog.id_for.constant) {
          decl_const(objects, TRUE, &err_info);
        } else if (current_ident == prog.id_for.syntax) {
          decl_syntax();
        } else if (current_ident == prog.id_for.system) {
          system_var(objects);
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
        if (err_info != NO_ERROR) {
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
#endif
        if (CLASS_OF_OBJ(decl_expression) == EXPROBJECT) {
          decl_matched = match_expression(decl_expression);
        } else {
          decl_matched = decl_expression;
        } /* if */
#ifdef TRACE_DECL_ANY
        printf("decl_matched ");
        trace1(decl_matched);
        printf("<<<\n");
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

static progtype analyze_prog (ustritype source_file_name, errinfotype *err_info)
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
    if (!ALLOC_RECORD(resultProg, progrecord)) {
      *err_info = MEMORY_ERROR;
    } else {
      COUNT_RECORD(progrecord, count.prog);
      resultProg->usage_count = 1;
      memcpy(&prog_backup, &prog, sizeof(progrecord));
      init_idents(&prog, err_info);
      init_findid(err_info);
      init_entity(err_info);
      init_sysvar();
      init_declaration_root(&prog, err_info);
      init_stack(&prog, err_info);
      init_symbol(err_info);
      reset_statistic();
      if (*err_info == NO_ERROR) {
        set_trace(option.comp_trace_level, -1, option.prot_file_name);
        decl_any(prog.declaration_root);
        if (SYS_MAIN_OBJECT == NULL) {
          printf("*** System declaration for main missing\n");
        } else if (CLASS_OF_OBJ(SYS_MAIN_OBJECT) == FORWARDOBJECT) {
          printf("*** Declaration for main missing\n");
        } else {
/*          printf("main defined as: ");
          trace1(SYS_MAIN_OBJECT);
          printf("\n"); */
          if (SYS_MAIN_OBJECT->entity != NULL) {
            if (SYS_MAIN_OBJECT->entity->owner != NULL) {
              if (SYS_MAIN_OBJECT->entity->owner->obj != NULL) {
                resultProg->main_object = SYS_MAIN_OBJECT->entity->owner->obj;
                if ((resultProg->main_object = match_object(resultProg->main_object)) != NULL) {
/*                  printf("main after match_object: ");
                  trace1(resultProg->main_object);
                  printf("\n"); */
                } else {
                  printf("*** Main not callobject\n");
                } /* if */
              } else {
                printf("SYS_MAIN_OBJECT->entity->objects->obj == NULL\n");
              } /* if */
            } else {
              printf("SYS_MAIN_OBJECT->entity->objects == NULL\n");
            } /* if */
          } else {
            printf("SYS_MAIN_OBJECT->entity == NULL\n");
          } /* if */
        } /* if */
        /* close_stack(&prog); * can be used when no matching is done during the runtime */
        /* close_declaration_root(&prog); * can be used when no matching is done during the runtime */
        if (option.show_ident_table) {
          write_idents();
        } /* if */
        clean_idents();
        resultProg->source_file_name = source_file_name;
        memcpy(&resultProg->ident,    &prog.ident, sizeof(idroottype));
        memcpy(&resultProg->id_for,   &prog.id_for, sizeof(findidtype));
        memcpy(&resultProg->sys_var,  &prog.sys_var, sizeof(systype));
        resultProg->declaration_root = prog.declaration_root;
        resultProg->stack_data       = prog.stack_data;
        resultProg->stack_current    = prog.stack_current;
        memcpy(&prog, &prog_backup, sizeof(progrecord));
        close_infile();
        close_symbol();
        if (option.compilation_info || option.linecount_info) {
          show_statistic();
        } /* if */
        /* trace_list(resultProg->stack_current->local_object_list); */
      } /* if */
    } /* if */
#ifdef TRACE_ANALYZE
    printf("END analyze_prog\n");
#endif
    return(resultProg);
  } /* analyze_prog */



#ifdef ANSI_C

progtype analyze (ustritype source_file_name)
#else

progtype analyze (source_file_name)
ustritype source_file_name;
#endif

  {
    unsigned int len;
    unsigned int pos;
    stritype source_name;
    unsigned int name_len;
    booltype add_extension;
    progtype resultProg;
    ustritype library_environment_variable;
    errinfotype err_info = NO_ERROR;

  /* analyze */
#ifdef TRACE_ANALYZE
    printf("BEGIN analyze\n");
#endif
    resultProg = NULL;
    len = strlen((cstritype) source_file_name);
    pos = len;
    while (pos >= 1 && source_file_name[pos] != '/' &&
        source_file_name[pos] != '\\') {
      pos--;
    } /* while */
    file_path = NULL;
    library_environment_variable = (ustritype) getenv("SEED7_LIBRARY");
    if (library_environment_variable != NULL) {
      set_search_path2(library_environment_variable,
          (memsizetype) strlen(library_environment_variable));
    } else if (source_file_name[pos] == '/' || source_file_name[pos] == '\\') {
      set_search_path2(source_file_name, (memsizetype) pos);
    } /* if */
    if (len > 4 && strcmp(&source_file_name[len - 4], ".sd7") == 0) {
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
      COUNT_STRI(name_len);
      source_name->size = name_len;
      stri_expand(source_name->mem, source_file_name, len);
      if (add_extension) {
        stri_expand(&source_name->mem[len], ".sd7", 4);
      } /* if */
      open_infile(source_name, &err_info);
      if (err_info == NO_ERROR) {
        resultProg = analyze_prog(source_file_name, &err_info);
      } /* if */
      if (err_info == FILE_ERROR) {
        err_message(NO_SOURCEFILE, source_name);
        resultProg = NULL;
      } else if (err_info == MEMORY_ERROR) {
        err_warning(OUT_OF_HEAP_SPACE);
        resultProg = NULL;
      } /* if */
      if (file_path != NULL) {
        FREE_STRI(file_path, file_path->size);
      } /* if */
      FREE_STRI(source_name, name_len);
    } /* if */
#ifdef TRACE_ANALYZE
    printf("END analyze\n");
#endif
    return(resultProg);
  } /* analyze */



#ifdef ANSI_C

progtype analyze_string (stritype input_string)
#else

progtype analyze_string (input_string)
stritype input_string;
#endif

  {
    bstritype input_bstri;
    errinfotype err_info = NO_ERROR;
    progtype resultProg;

  /* analyze_string */
#ifdef TRACE_ANALYZE
    printf("BEGIN analyze_string\n");
#endif
    resultProg = NULL;
    file_path = NULL;
    input_bstri = cp_to_bstri(input_string);
    if (input_bstri != NULL) {
      open_string(input_bstri, &err_info);
      if (err_info == NO_ERROR) {
        resultProg = analyze_prog("STRING", &err_info);
        if (err_info == MEMORY_ERROR) {
          err_warning(OUT_OF_HEAP_SPACE);
          resultProg = NULL;
        } /* if */
      } /* if */
      FREE_BSTRI(input_bstri, input_bstri->size);
    } /* if */
#ifdef TRACE_ANALYZE
    printf("END analyze_string\n");
#endif
    return(resultProg);
  } /* analyze_string */

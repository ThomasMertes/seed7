/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2013  Thomas Mertes                        */
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
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "syvarutl.h"
#include "traceutl.h"
#include "chclsutl.h"
#include "identutl.h"
#include "entutl.h"
#include "listutl.h"
#include "fatal.h"
#include "scanner.h"
#include "parser.h"
#include "symbol.h"
#include "syntax.h"
#include "info.h"
#include "infile.h"
#include "libpath.h"
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
#include "cmd_rtl.h"
#include "str_rtl.h"
#include "ut8_rtl.h"

#undef EXTERN
#define EXTERN
#include "analyze.h"

#undef TRACE_ANALYZE
#undef TRACE_DECL_ANY


/* when the analyzer is used from a compiled program this */
/* flag decides which exception handler should be called: */
boolType interpreter_exception = FALSE;

static boolType analyze_initialized = FALSE;
progRecord prog;



static void init_analyze (void)

  { /* init_analyze */
    if (!analyze_initialized) {
      set_protfile_name(NULL);
      init_chclass();
      init_primitiv();
      init_do_any();
      memset(&prog, 0, sizeof(progRecord)); /* not used, saved in analyze and interpr */
      analyze_initialized = TRUE;
    } /* if */
  } /* init_analyze */



static inline void system_var (void)

  {
    int index_found;
    objectType sys_object;

  /* system_var */
#ifdef TRACE_ANALYZE
    printf("BEGIN system_var\n");
#endif
    scan_symbol();
    if (symbol.sycategory == STRILITERAL) {
      index_found = find_sysvar(symbol.striValue);
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
    /* printf("sys_var[%d] = ", index_found);
        trace1(sys_object);
        printf("\n"); */
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



static inline void include_file (void)

  {
    striType include_file_name;
    errInfoType err_info = OKAY_NO_ERROR;

  /* include_file */
#ifdef TRACE_ANALYZE
    printf("BEGIN include_file\n");
#endif
    scan_symbol();
    if (symbol.sycategory == STRILITERAL) {
      if (!ALLOC_STRI_SIZE_OK(include_file_name, symbol.striValue->size)) {
        err_warning(OUT_OF_HEAP_SPACE);
      } else {
        include_file_name->size = symbol.striValue->size;
        memcpy(include_file_name->mem, symbol.striValue->mem,
            (size_t) symbol.striValue->size * sizeof(strElemType));
        scan_symbol();
        if (current_ident != prog.id_for.semicolon) {
          err_ident(EXPECTED_SYMBOL, prog.id_for.semicolon);
        } /* if */
        if (strChPos(include_file_name, (charType) '\\') != 0) {
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



static void process_pragma (void)

  {
    errInfoType err_info = OKAY_NO_ERROR;

  /* process_pragma */
#ifdef TRACE_SCANNER
    printf("BEGIN process_pragma\n");
#endif
    if (symbol.sycategory != NAMESYMBOL) {
      err_warning(NAMEEXPECTED);
    } else {
      if (strcmp((cstriType) symbol.name, "library") == 0) {
        scan_symbol();
        if (symbol.sycategory == STRILITERAL) {
          if (strChPos(symbol.striValue, (charType) '\\') != 0) {
            err_warning(WRONG_PATH_DELIMITER);
            scan_symbol();
          } else {
            append_to_lib_path(symbol.striValue, &err_info);
            if (err_info != OKAY_NO_ERROR) {
              fatal_memory_error(SOURCE_POSITION(2111));
            } /* if */
          } /* if */
        } else {
          err_warning(STRI_EXPECTED);
        } /* if */
      } else if (strcmp((cstriType) symbol.name, "message") == 0) {
        scan_symbol();
        if (symbol.sycategory == STRILITERAL) {
#ifdef WITH_COMPILATION_INFO
          if (in_file.write_line_numbers) {
            NL_LIN_INFO();
          } /* if */
#endif
          ut8Write(stdout, symbol.striValue);
          putchar('\n');
          fflush(stdout);
          display_compilation_info();
        } else {
          err_warning(STRI_EXPECTED);
        } /* if */
      } else if (strcmp((cstriType) symbol.name, "info") == 0) {
        scan_symbol();
        if (strcmp((cstriType) symbol.name, "on") == 0) {
#ifdef WITH_COMPILATION_INFO
          if (!in_file.write_line_numbers) {
            in_file.write_line_numbers = TRUE;
            display_compilation_info();
          } /* if */
          in_file.write_library_names = TRUE;
#endif
        } else if (strcmp((cstriType) symbol.name, "off") == 0) {
#ifdef WITH_COMPILATION_INFO
          if (in_file.write_line_numbers) {
            size_t number;
            for (number = 1; number <= 7 + strlen((const_cstriType) in_file.name_ustri);
                number++) {
              fputc(' ', stdout);
            } /* for */
            fputc('\r', stdout);
            fflush(stdout);
            in_file.write_line_numbers = FALSE;
          } /* if */
          in_file.write_library_names = FALSE;
#endif
        } else {
          err_warning(ILLEGALPRAGMA);
        } /* if */
      } else if (strcmp((cstriType) symbol.name, "trace") == 0) {
        scan_symbol();
        if (symbol.sycategory == STRILITERAL) {
          mapTraceFlags(symbol.striValue, &prog.option_flags);
          set_trace(prog.option_flags);
        } else {
          err_warning(STRI_EXPECTED);
        } /* if */
      } else if (strcmp((cstriType) symbol.name, "decls") == 0) {
        trace_nodes();
      } else if (strcmp((cstriType) symbol.name, "names") == 0) {
        scan_symbol();
        if (strcmp((cstriType) symbol.name, "unicode") == 0) {
          symbol.unicodeNames = TRUE;
        } else if (strcmp((cstriType) symbol.name, "ascii") == 0) {
          symbol.unicodeNames = FALSE;
        } else {
          err_warning(ILLEGALPRAGMA);
        } /* if */
      } else {
        err_warning(ILLEGALPRAGMA);
      } /* if */
    } /* if */
#ifdef TRACE_SCANNER
    printf("END process_pragma\n");
#endif
  } /* process_pragma */



static inline void decl_any (nodeType objects)

  {
    objectType decl_expression;
    errInfoType err_info = OKAY_NO_ERROR;

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
          match_expression(decl_expression);
        } /* if */
        if (current_ident != prog.id_for.semicolon) {
          err_ident(EXPECTED_SYMBOL, prog.id_for.semicolon);
        } /* if */
        set_fail_flag(FALSE);
        evaluate(decl_expression);
        if (fail_flag) {
          err_object(EXCEPTION_RAISED, fail_value);
          set_fail_flag(FALSE);
        } /* if */
        free_expression(decl_expression);
        if (current_ident == prog.id_for.semicolon) {
          scan_symbol();
        } /* if */
      } /* if */
    } /* while */
#ifdef TRACE_ANALYZE
    printf("END decl_any\n");
#endif
  } /* decl_any */



static striType getProgramName (const const_striType source_file_name)

  {
    memSizeType name_len;
    intType lastSlashPos;
    striType program_name;

  /* getProgramName */
    name_len = source_file_name->size;
    if (name_len > 4 &&
        source_file_name->mem[name_len - 4] == '.' &&
        ((source_file_name->mem[name_len - 3] == 's' &&
          source_file_name->mem[name_len - 2] == 'd' &&
          source_file_name->mem[name_len - 1] == '7') ||
         (source_file_name->mem[name_len - 3] == 's' &&
          source_file_name->mem[name_len - 2] == '7' &&
          source_file_name->mem[name_len - 1] == 'i'))) {
      name_len -= 4;
    } /* if */
    lastSlashPos = strRChPos(source_file_name, (charType) '/');
    name_len -= (memSizeType) lastSlashPos;
    if (ALLOC_STRI_SIZE_OK(program_name, name_len)) {
      program_name->size = name_len;
      memcpy(program_name->mem, &source_file_name->mem[lastSlashPos],
          name_len * sizeof(strElemType));
    } /* if */
    return program_name;
  } /* getProgramName */



static striType getProgramPath (const const_striType source_file_name)

  {
    striType cwd;
    striType program_path;

  /* getProgramPath */
    if (source_file_name->size >= 1 &&
        source_file_name->mem[0] == (charType) '/') {
      program_path = strCreate(source_file_name);
    } else {
      cwd = cmdGetcwd();
      program_path = concat_path(cwd, source_file_name);
      FREE_STRI(cwd, cwd->size);
    } /* if */
    return program_path;
  } /* getProgramPath */



static progType analyze_prog (const const_striType source_file_argument,
    const const_striType source_name, uintType options, const const_rtlArrayType libraryDirs,
    const const_striType prot_file_name, errInfoType *err_info)

  {
    striType source_file_argument_copy;
    traceRecord trace_backup;
    progRecord prog_backup;
    progType resultProg;

  /* analyze_prog */
#ifdef TRACE_ANALYZE
    printf("BEGIN analyze_prog\n");
#endif
    if (!ALLOC_RECORD(resultProg, progRecord, count.prog)) {
      *err_info = MEMORY_ERROR;
    } else if (!ALLOC_STRI_SIZE_OK(source_file_argument_copy, source_file_argument->size)) {
      *err_info = MEMORY_ERROR;
      FREE_RECORD(resultProg, progRecord, count.prog);
      resultProg = NULL;
    } else {
      /* printf("analyze_prog: new progRecord: %lx\n", resultProg); */
      source_file_argument_copy->size = source_file_argument->size;
      memcpy(source_file_argument_copy->mem, source_file_argument->mem,
          source_file_argument->size * sizeof(strElemType));
      resultProg->usage_count = 1;
      resultProg->main_object = NULL;
      resultProg->types = NULL;
      memcpy(&prog_backup, &prog, sizeof(progRecord));
      prog.owningProg = resultProg;
      in_file.owningProg = resultProg;
      init_lib_path(source_file_argument, libraryDirs, err_info);
      init_idents(&prog, err_info);
      init_findid(err_info);
      init_entity(err_info);
      init_sysvar();
      init_declaration_root(&prog, err_info);
      init_stack(&prog, err_info);
      init_symbol(err_info);
      reset_statistic();
      prog.error_count = 0;
      prog.types = NULL;
      prog.literals = NULL;
      if (*err_info == OKAY_NO_ERROR) {
        memcpy(&trace_backup, &trace, sizeof(traceRecord));
        prog.option_flags = options;
        set_trace(prog.option_flags);
        set_protfile_name(prot_file_name);
        decl_any(prog.declaration_root);
        if (SYS_MAIN_OBJECT == NULL) {
          prog.error_count++;
          printf("*** System declaration for main missing\n");
        } else if (CATEGORY_OF_OBJ(SYS_MAIN_OBJECT) != FORWARDOBJECT) {
/*          printf("main defined as: ");
          trace1(SYS_MAIN_OBJECT);
          printf("\n"); */
          if (HAS_ENTITY(SYS_MAIN_OBJECT)) {
            if (GET_ENTITY(SYS_MAIN_OBJECT)->data.owner != NULL) {
              if (GET_ENTITY(SYS_MAIN_OBJECT)->data.owner->obj != NULL) {
                resultProg->main_object = GET_ENTITY(SYS_MAIN_OBJECT)->data.owner->obj;
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
        if (options & SHOW_IDENT_TABLE) {
          write_idents();
        } /* if */
        clean_idents();
        resultProg->owningProg       = resultProg;
        resultProg->arg0             = source_file_argument_copy;
        resultProg->program_name     = getProgramName(source_file_argument);
        resultProg->program_path     = getProgramPath(source_name);
        resultProg->arg_v            = NULL;
        resultProg->option_flags     = prog.option_flags;
        resultProg->error_count      = prog.error_count;
        memcpy(&resultProg->ident,    &prog.ident, sizeof(idRootType));
        memcpy(&resultProg->id_for,   &prog.id_for, sizeof(findIdType));
        memcpy(&resultProg->entity,   &prog.entity, sizeof(entityRootType));
        memcpy(&resultProg->property, &prog.property, sizeof(propertyRootType));
        memcpy(&resultProg->sys_var,  &prog.sys_var, sizeof(sysType));
        resultProg->literals         = prog.literals;
        resultProg->declaration_root = prog.declaration_root;
        resultProg->stack_global     = prog.stack_global;
        resultProg->stack_data       = prog.stack_data;
        resultProg->stack_current    = prog.stack_current;
        memcpy(&prog, &prog_backup, sizeof(progRecord));
        close_infile();
        close_symbol();
        free_lib_path();
        if (options & SHOW_STATISTICS) {
          show_statistic();
          if (resultProg->error_count >= 1) {
            printf("%6d error%s found\n",
                resultProg->error_count,
                resultProg->error_count > 1 ? "s" : "");
          } /* if */
        } /* if */
        /* trace_list(resultProg->stack_current->local_object_list); */
        memcpy(&trace, &trace_backup, sizeof(traceRecord));
      } else {
        memcpy(&prog, &prog_backup, sizeof(progRecord));
        FREE_RECORD(resultProg, progRecord, count.prog);
        FREE_STRI(source_file_argument_copy, source_file_argument_copy->size);
        resultProg = NULL;
      } /* if */
    } /* if */
#ifdef TRACE_ANALYZE
    printf("END analyze_prog --> %lx\n", (unsigned long) resultProg);
#endif
    return resultProg;
  } /* analyze_prog */



progType analyze_file (const const_striType source_file_argument, uintType options,
    const const_rtlArrayType libraryDirs, const const_striType prot_file_name,
    errInfoType *err_info)

  {
    striType source_name;
    memSizeType name_len;
    boolType add_extension;
    progType resultProg;

  /* analyze_file */
#ifdef TRACE_ANALYZE
    printf("BEGIN analyze_file\n");
#endif
    interpreter_exception = TRUE;
    init_analyze();
    resultProg = NULL;
    if (source_file_argument->size > 4 &&
        source_file_argument->mem[source_file_argument->size - 4] == '.' &&
        source_file_argument->mem[source_file_argument->size - 3] == 's' &&
        source_file_argument->mem[source_file_argument->size - 2] == 'd' &&
        source_file_argument->mem[source_file_argument->size - 1] == '7') {
      name_len = source_file_argument->size;
      add_extension = FALSE;
    } else {
      name_len = source_file_argument->size + 4;
      add_extension = TRUE;
    } /* if */
    if (strChPos(source_file_argument, (charType) '\\') != 0) {
      *err_info = RANGE_ERROR;
    } else if (!ALLOC_STRI_CHECK_SIZE(source_name, name_len)) {
      *err_info = MEMORY_ERROR;
    } else if (*err_info == OKAY_NO_ERROR) {
      source_name->size = name_len;
      memcpy(source_name->mem, source_file_argument->mem,
          source_file_argument->size * sizeof(strElemType));
      if (add_extension) {
        memcpy_to_strelem(&source_name->mem[source_file_argument->size],
                          (const_ustriType) ".sd7", 4);
      } /* if */
      open_infile(source_name,
                  (options & WRITE_LIBRARY_NAMES) != 0,
                  (options & WRITE_LINE_NUMBERS) != 0, err_info);
      if (*err_info == FILE_ERROR && add_extension) {
        *err_info = OKAY_NO_ERROR;
        source_name->size = name_len - 4;
        open_infile(source_name,
                    (options & WRITE_LIBRARY_NAMES) != 0,
                    (options & WRITE_LINE_NUMBERS) != 0, err_info);
      } /* if */
#if HAS_SYMBOLIC_LINKS
      source_name = followLink(source_name);
#endif
      if (*err_info == OKAY_NO_ERROR) {
        scan_byte_order_mark();
        resultProg = analyze_prog(source_file_argument, source_name, options,
                                  libraryDirs, prot_file_name, err_info);
      } else if (*err_info == FILE_ERROR) {
        *err_info = OKAY_NO_ERROR;
      } /* if */
      FREE_STRI(source_name, name_len);
    } /* if */
    interpreter_exception = FALSE;
#ifdef TRACE_ANALYZE
    printf("END analyze_file\n");
#endif
    return resultProg;
  } /* analyze_file */



progType analyze (const const_striType source_file_argument, uintType options,
    const const_rtlArrayType libraryDirs, const const_striType prot_file_name)

  {
    errInfoType err_info = OKAY_NO_ERROR;
    progType resultProg;

  /* analyze */
#ifdef TRACE_ANALYZE
    printf("BEGIN analyze\n");
#endif
    resultProg = analyze_file(source_file_argument, options,
                              libraryDirs, prot_file_name, &err_info);
    if (err_info == MEMORY_ERROR) {
      err_warning(OUT_OF_HEAP_SPACE);
    } else if (resultProg == NULL || err_info != OKAY_NO_ERROR) {
      err_message(NO_SOURCEFILE, source_file_argument);
    } /* if */
#ifdef TRACE_ANALYZE
    printf("END analyze\n");
#endif
    return resultProg;
  } /* analyze */



progType analyze_string (const const_striType input_string, uintType options,
    const const_rtlArrayType libraryDirs, const const_striType prot_file_name,
    errInfoType *err_info)

  {
    striType source_file_argument;
    bstriType input_bstri;
    progType resultProg;

  /* analyze_string */
#ifdef TRACE_ANALYZE
    printf("BEGIN analyze_string\n");
#endif
    interpreter_exception = TRUE;
    init_analyze();
    resultProg = NULL;
    source_file_argument = cstri_to_stri("STRING");
    if (source_file_argument == NULL) {
      *err_info = MEMORY_ERROR;
    } else {
      input_bstri = stri_to_bstri8(input_string);
      if (input_bstri == NULL) {
        *err_info = MEMORY_ERROR;
      } else {
        open_string(input_bstri,
                    (options & WRITE_LIBRARY_NAMES) != 0,
                    (options & WRITE_LINE_NUMBERS) != 0, err_info);
        if (*err_info == OKAY_NO_ERROR) {
          resultProg = analyze_prog(source_file_argument, source_file_argument,
                                    options, libraryDirs, prot_file_name, err_info);
        } /* if */
        FREE_BSTRI(input_bstri, input_bstri->size);
      } /* if */
      FREE_STRI(source_file_argument, source_file_argument->size);
    } /* if */
    interpreter_exception = FALSE;
#ifdef TRACE_ANALYZE
    printf("END analyze_string\n");
#endif
    return resultProg;
  } /* analyze_string */

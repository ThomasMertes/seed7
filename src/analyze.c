/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2016, 2018, 2019, 2021  Thomas Mertes      */
/*                2025  Thomas Mertes                               */
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
/*  Changes: 1991 - 1994, 2010, 2012 - 2016, 2018  Thomas Mertes    */
/*           2019, 2021, 2025  Thomas Mertes                        */
/*  Content: Main function of the analysis phase.                   */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "setjmp.h"

#include "common.h"
#include "sigutl.h"
#include "data.h"
#include "data_rtl.h"
#include "os_decls.h"
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
#include "dcllib.h"
#include "primitiv.h"
#include "doany.h"
#include "option.h"
#include "runerr.h"
#include "cmd_rtl.h"
#include "chr_rtl.h"
#include "str_rtl.h"
#include "hsh_rtl.h"
#include "fil_rtl.h"
#include "ut8_rtl.h"
#include "prg_comp.h"

#undef EXTERN
#define EXTERN
#include "analyze.h"

#define TRACE_DECL_ANY 0


/* The long jump position memoryErrorOccurred is  */
/* only used during the analysis phase (parsing). */
extern boolType currentlyAnalyzing;
extern longjmpPosition memoryErrorOccurred;

/* If the analyzer is used from a compiled program this */
/* flag decides which exception handler should be called: */
#if CHECK_STACK
extern boolType interpreter_exception;
#else
boolType interpreter_exception = FALSE;
#endif

static boolType analyze_initialized = FALSE;
progType prog;



static void initAnalyze (void)

  { /* initAnalyze */
    logFunction(printf("initAnalyze\n"););
    if (!analyze_initialized) {
      set_protfile_name(NULL);
      init_chclass();
      init_primitiv();
      init_do_any();
      prog = NULL;
      analyze_initialized = TRUE;
    } /* if */
    logFunction(printf("initAnalyze -->\n"););
  } /* initAnalyze */



static inline void systemVar (void)

  {
    int indexFound;
    objectType sysObject;

  /* systemVar */
    logFunction(printf("systemVar\n"););
    scan_symbol();
    if (symbol.sycategory == STRILITERAL) {
      indexFound = findSysvar(symbol.striValue);
      if (indexFound == -1) {
        err_warning(WRONGSYSTEM);
      } /* if */
    } else {
      err_warning(STRING_EXPECTED);
      indexFound = -1;
    } /* if */
    scan_symbol();
    if (current_ident == prog->id_for.is) {
      scan_symbol();
    } else {
      err_ident(EXPECTED_SYMBOL, prog->id_for.is);
    } /* if */
    sysObject = read_atom();
    /* printf("sys_var[%d] = ", indexFound);
        trace1(sysObject);
        printf("\n"); */
    if (indexFound != -1) {
      if (indexFound >= FIRST_EXCEPTION_SYS_VAR &&
          indexFound <= LAST_EXCEPTION_SYS_VAR &&
          CATEGORY_OF_OBJ(sysObject) != ENUMLITERALOBJECT) {
        err_object(EXCEPTION_EXPECTED, sysObject);
      } else if (indexFound >= FIRST_TYPE_SYS_VAR &&
                 indexFound <= LAST_TYPE_SYS_VAR &&
                 CATEGORY_OF_OBJ(sysObject) != TYPEOBJECT) {
        err_object(TYPE_EXPECTED, sysObject);
      } else {
        prog->sys_var[indexFound] = sysObject;
      } /* if */
    } /* if */
    if (current_ident == prog->id_for.semicolon) {
      scan_symbol();
    } else {
      err_ident(EXPECTED_SYMBOL, prog->id_for.semicolon);
    } /* if */
    logFunction(printf("systemVar -->\n"););
  } /* systemVar */



static boolType includeFile (void)

  {
    striType includeFileName;
    includeResultType includeResult;
    errInfoType err_info = OKAY_NO_ERROR;
    boolType okay = TRUE;

  /* includeFile */
    logFunction(printf("includeFile\n"););
    scan_symbol();
    if (symbol.sycategory == STRILITERAL) {
      if (!ALLOC_STRI_SIZE_OK(includeFileName, symbol.striValue->size)) {
        err_warning(OUT_OF_HEAP_SPACE);
      } else {
        includeFileName->size = symbol.striValue->size;
        memcpy(includeFileName->mem, symbol.striValue->mem,
               (size_t) symbol.striValue->size * sizeof(strElemType));
        scan_symbol();
        if (current_ident != prog->id_for.semicolon) {
          err_ident(EXPECTED_SYMBOL, prog->id_for.semicolon);
        } else {
          if (strChPos(includeFileName, (charType) '\\') != 0) {
            err_stri(WRONG_PATH_DELIMITER, includeFileName);
          } else {
            includeResult = findIncludeFile((rtlHashType) prog->includeFileHash,
                                            includeFileName, &err_info);
            if (unlikely(includeResult == INCLUDE_FAILED)) {
              if (err_info == MEMORY_ERROR) {
                err_warning(OUT_OF_HEAP_SPACE);
              } else {
                /* FILE_ERROR or RANGE_ERROR */
                err_stri(INCLUDE_FILE_NOT_FOUND, includeFileName);
              } /* if */
              if (SYS_MAIN_OBJECT == NULL ||
                  CATEGORY_OF_OBJ(SYS_MAIN_OBJECT) == ILLEGALOBJECT) {
                err_warning(ESSENTIAL_INCLUDE_FAILED);
                okay = FALSE;
              } /* if */
            } else if (includeResult == INCLUDE_SUCCESS) {
              scan_byte_order_mark();
            } /* if */
          } /* if */
          scan_symbol();
        } /* if */
        FREE_STRI(includeFileName);
      } /* if */
    } else {
      err_warning(STRING_EXPECTED);
      if (current_ident != prog->id_for.semicolon) {
        scan_symbol();
      } /* if */
      if (current_ident != prog->id_for.semicolon) {
        err_ident(EXPECTED_SYMBOL, prog->id_for.semicolon);
      } else {
        scan_symbol();
      } /* if */
    } /* if */
    logFunction(printf("includeFile --> %d\n", okay););
    return okay;
  } /* includeFile */



static void processPragma (void)

  {
    errInfoType err_info = OKAY_NO_ERROR;

  /* processPragma */
    logFunction(printf("processPragma\n"););
    if (symbol.sycategory != NAMESYMBOL) {
      err_warning(NAME_EXPECTED);
    } else {
      if (strcmp((cstriType) symbol.name, "library") == 0) {
        scan_symbol();
        if (symbol.sycategory == STRILITERAL) {
          if (strChPos(symbol.striValue, (charType) '\\') != 0) {
            err_warning(WRONG_PATH_DELIMITER);
            scan_symbol();
          } else {
            appendToLibPath(symbol.striValue, &err_info);
            if (unlikely(err_info != OKAY_NO_ERROR)) {
              fatal_memory_error(SOURCE_POSITION(2111));
            } /* if */
          } /* if */
        } else {
          err_warning(STRING_EXPECTED);
        } /* if */
      } else if (strcmp((cstriType) symbol.name, "message") == 0) {
        scan_symbol();
        if (symbol.sycategory == STRILITERAL) {
#if WITH_COMPILATION_INFO
          if (in_file.write_line_numbers) {
            NL_LIN_INFO();
          } /* if */
#endif
          ut8Write(&stdoutFileRecord, symbol.striValue);
          putchar('\n');
          fflush(stdout);
          display_compilation_info();
        } else {
          err_warning(STRING_EXPECTED);
        } /* if */
      } else if (strcmp((cstriType) symbol.name, "info") == 0) {
        scan_symbol();
        if (strcmp((cstriType) symbol.name, "on") == 0) {
#if WITH_COMPILATION_INFO
          if (!in_file.write_line_numbers) {
            in_file.write_line_numbers = TRUE;
            display_compilation_info();
          } /* if */
          in_file.write_library_names = TRUE;
#endif
        } else if (strcmp((cstriType) symbol.name, "off") == 0) {
#if WITH_COMPILATION_INFO
          if (in_file.write_line_numbers) {
            size_t number;
            for (number = 7 + strlen((const_cstriType) in_file.name_ustri);
                number != 0; number--) {
              fputc(' ', stdout);
            } /* for */
            fputc('\r', stdout);
            fflush(stdout);
            in_file.write_line_numbers = FALSE;
          } /* if */
          in_file.write_library_names = FALSE;
#endif
        } else {
          err_cstri(ILLEGALPRAGMA, "info");
        } /* if */
      } else if (strcmp((cstriType) symbol.name, "trace") == 0) {
        scan_symbol();
        if (symbol.sycategory == STRILITERAL) {
          mapTraceFlags(symbol.striValue, &prog->option_flags);
          set_trace(prog->option_flags);
        } else {
          err_warning(STRING_EXPECTED);
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
          err_cstri(ILLEGALPRAGMA, "names");
        } /* if */
      } else {
        err_warning(ILLEGALPRAGMA);
      } /* if */
    } /* if */
    logFunction(printf("processPragma -->\n"););
  } /* processPragma */



static inline void declAny (nodeType objects)

  {
    objectType declExpression;
    objectType match_result;
    errInfoType err_info = OKAY_NO_ERROR;
    boolType okay = TRUE;

  /* declAny */
    logFunction(printf("declAny\n"););
    scan_symbol();
    while (symbol.sycategory != STOPSYMBOL && okay) {
      if (current_ident == prog->id_for.dollar) {
        err_info = OKAY_NO_ERROR;
        scan_symbol();
        if (current_ident == prog->id_for.constant) {
          decl_const(objects, &err_info);
        } else if (current_ident == prog->id_for.syntax) {
          decl_syntax();
        } else if (current_ident == prog->id_for.system) {
          systemVar();
        } else if (current_ident == prog->id_for.include) {
          okay = includeFile();
        } else {
          processPragma();
          scan_symbol();
          if (current_ident != prog->id_for.semicolon) {
            err_ident(EXPECTED_SYMBOL, prog->id_for.semicolon);
          } else {
            scan_symbol();
          } /* if */
        } /* if */
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          err_object(EXCEPTION_RAISED, prog->sys_var[err_info]);
        } /* if */
      } else {
#if TRACE_DECL_ANY
        printf("before parse expression ");
#endif
        declExpression = pars_infix_expression(SCOL_PRIORITY, FALSE);
#if TRACE_DECL_ANY
        printf("declExpression ");
        trace1(declExpression);
        printf("<<<\n");
        fflush(stdout);
#endif
        if (declExpression != NULL) {
          if (CATEGORY_OF_OBJ(declExpression) == EXPROBJECT) {
            match_result = match_expression(declExpression);
          } else {
            err_object(EXPR_EXPECTED, declExpression);
            match_result = NULL;
          } /* if */
          if (current_ident != prog->id_for.semicolon) {
            err_ident(EXPECTED_SYMBOL, prog->id_for.semicolon);
            skip_char(';');
          } /* if */
          if (match_result != NULL &&
              (prog->error_count == 0 ||
               (CATEGORY_OF_OBJ(match_result) == MATCHOBJECT &&
                match_result->value.listValue != NULL &&
                match_result->value.listValue->obj != NULL &&
                CATEGORY_OF_OBJ(match_result->value.listValue->obj) == ACTOBJECT &&
                match_result->value.listValue->obj->value.actValue == dcl_const))) {
            set_fail_flag(FALSE);
            curr_exec_object = NULL;
            evaluate(match_result);
            if (unlikely(fail_flag)) {
              set_fail_flag(FALSE);
              err_object(EXCEPTION_RAISED, fail_value);
            } /* if */
          } /* if */
          free_expression(declExpression);
        } /* if */
        if (current_ident == prog->id_for.semicolon) {
          scan_symbol();
        } /* if */
      } /* if */
    } /* while */
    logFunction(printf("declAny -->\n"););
  } /* declAny */



static striType getProgramName (const const_striType sourceFileArgument)

  {
    memSizeType nameLen;
    intType lastSlashPos;
    striType program_name;

  /* getProgramName */
    logFunction(printf("getProgramName(\"%s\")\n",
                       striAsUnquotedCStri(sourceFileArgument)););
    nameLen = sourceFileArgument->size;
    if (nameLen > 4 &&
        sourceFileArgument->mem[nameLen - 4] == '.' &&
        ((sourceFileArgument->mem[nameLen - 3] == 's' &&
          sourceFileArgument->mem[nameLen - 2] == 'd' &&
          sourceFileArgument->mem[nameLen - 1] == '7') ||
         (sourceFileArgument->mem[nameLen - 3] == 's' &&
          sourceFileArgument->mem[nameLen - 2] == '7' &&
          sourceFileArgument->mem[nameLen - 1] == 'i'))) {
      nameLen -= 4;
    } /* if */
    lastSlashPos = strRChPos(sourceFileArgument, (charType) '/');
    nameLen -= (memSizeType) lastSlashPos;
    if (ALLOC_STRI_SIZE_OK(program_name, nameLen)) {
      program_name->size = nameLen;
      memcpy(program_name->mem, &sourceFileArgument->mem[lastSlashPos],
             nameLen * sizeof(strElemType));
    } /* if */
    logFunction(printf("getProgramName --> \"%s\"\n",
                       striAsUnquotedCStri(program_name)););
    return program_name;
  } /* getProgramName */



/**
 *  Basic function to parse a program.
 *  All program parsing is done via this function.
 *  The function assumes that the infile is already open.
 *  After parsing the infile is closed by this function.
 */
static progType analyzeProg (const const_striType sourceFileArgument,
    const const_striType sourceFilePath, uintType options,
    const const_rtlArrayType libraryDirs, const fileType errorFile,
    errInfoType *err_info)

  {
    boolType backup_interpreter_exception;
    traceRecord traceBackup;
    objectType backup_curr_exec_object;
    listType backup_curr_argument_list;
    progType progBackup;
    progType resultProg;

  /* analyzeProg */
    logFunction(printf("analyzeProg(\"%s\", ",
                       striAsUnquotedCStri(sourceFileArgument));
                printf("\"%s\", 0x" F_X(016) ", *, %s%s%d, %d)\n",
                       striAsUnquotedCStri(sourceFilePath), options,
                       errorFile == NULL ? "NULL " : "",
                       errorFile->cFile == NULL? "NULL_FILE " : "",
                       errorFile != NULL ? safe_fileno(errorFile->cFile) : 0,
                       *err_info););
    if (unlikely(errorFile != NULL &&
                 errorFile != &nullFileRecord &&
                 errorFile->cFile == NULL)) {
      logError(printf("analyzeProg: Attempt to write to closed file.\n"););
      closeInfile();
      *err_info = FILE_ERROR;
      resultProg = NULL;
    } else if (!ALLOC_RECORD(resultProg, progRecord, count.prog)) {
      closeInfile();
      *err_info = MEMORY_ERROR;
    } else {
      memset(resultProg, 0, sizeof(progRecord));
      *err_info = OKAY_NO_ERROR;
      backup_interpreter_exception = interpreter_exception;
      interpreter_exception = TRUE;
      resultProg->usage_count = 1;
      resultProg->main_object = NULL;
      resultProg->types = NULL;
      resultProg->objectNumberMap = (void *) hshEmpty();
      resultProg->nextFreeObjectNumber = 1;
      resultProg->typeNumberMap = (void *) hshEmpty();
      resultProg->nextFreeTypeNumber = 1;
      resultProg->includeFileHash = (void *) initIncludeFileHash();
      resultProg->fileList = in_file.curr_infile;
      resultProg->fileCounter = 1;
      initLibPath(sourceFileArgument, libraryDirs, err_info);
      init_symbol(err_info);
      init_idents(resultProg, err_info);
      init_findid(resultProg, err_info);
      init_entity(resultProg, err_info);
      init_sysvar(resultProg);
      init_declaration_root(resultProg, err_info);
      init_stack(resultProg, err_info);
      reset_statistic();
      resultProg->writeErrors = errorFile != NULL &&
                                errorFile != &nullFileRecord;
      resultProg->errorFile = filCreate(errorFile);
      resultProg->error_count = 0;
      resultProg->types = NULL;
      resultProg->literals = NULL;
      resultProg->arg0         = strCreate(sourceFileArgument);
      resultProg->program_name = getProgramName(sourceFileArgument);
      resultProg->program_path = copy_stri(in_file.path);
      if (resultProg->arg0 == NULL ||
          resultProg->program_name == NULL ||
          resultProg->program_path == NULL) {
        *err_info = MEMORY_ERROR;
      } /* if */
      resultProg->arg_v = NULL;
      backup_curr_exec_object = curr_exec_object;
      backup_curr_argument_list = curr_argument_list;
      memcpy(&traceBackup, &trace, sizeof(traceRecord));
      progBackup = prog;
      if (*err_info == OKAY_NO_ERROR &&
          do_setjmp(memoryErrorOccurred) == 0) {
        currentlyAnalyzing = TRUE;
        resultProg->option_flags = options;
        set_trace(resultProg->option_flags);
        prog = resultProg;
        scan_byte_order_mark();
        declAny(resultProg->declaration_root);
        if (MAIN_OBJECT(resultProg) == NULL ||
            CATEGORY_OF_OBJ(MAIN_OBJECT(resultProg)) == ILLEGALOBJECT) {
          err_warning(SYSTEM_MAIN_MISSING);
        } else {
          resultProg->main_object = MAIN_OBJECT(resultProg);
        } /* if */
        prog = progBackup;
        /* To get a nice list of files with the option -v closeInfile() */
        /* is executed before write_idents() and show_statistic().      */
        closeInfile();
        if (options & SHOW_IDENT_TABLE) {
          write_idents(resultProg);
        } /* if */
        clean_idents(resultProg);
        shutIncludeFileHash((rtlHashType) resultProg->includeFileHash);
        resultProg->includeFileHash = NULL;
        if (options & SHOW_STATISTICS) {
          show_statistic();
          if (resultProg->error_count >= 1) {
            printf("%6d error%s found\n",
                resultProg->error_count,
                resultProg->error_count > 1 ? "s" : "");
          } /* if */
        } /* if */
        /* trace_list(resultProg->stack_current->local_object_list); */
      } else {
        prog = progBackup;
        closeInfile();
        clean_idents(resultProg);
        shutIncludeFileHash((rtlHashType) resultProg->includeFileHash);
        resultProg->includeFileHash = NULL;
        /* heapStatistic(); */
        prgDestr(resultProg);
        resultProg = NULL;
        *err_info = MEMORY_ERROR;
      } /* if */
      currentlyAnalyzing = FALSE;
      curr_exec_object = backup_curr_exec_object;
      curr_argument_list = backup_curr_argument_list;
      memcpy(&trace, &traceBackup, sizeof(traceRecord));
      close_symbol();
      freeLibPath();
      leaveExceptionHandling();
      interpreter_exception = backup_interpreter_exception;
    } /* if */
    logFunction(printf("analyzeProg --> " FMT_U_MEM
                       " (error_count=%u, err_info=%d)\n",
                       (memSizeType) resultProg,
                       resultProg != 0 ? resultProg->error_count : 0,
                       *err_info););
    return resultProg;
  } /* analyzeProg */



progType analyzeFile (const const_striType sourceFileArgument, uintType options,
    const const_rtlArrayType libraryDirs, const fileType errorFile,
    errInfoType *err_info)

  {
    striType sourceFilePath;
    striType absolutePath;
    memSizeType nameLen;
    boolType add_extension;
    boolType isOpen;
    progType resultProg = NULL;

  /* analyzeFile */
    logFunction(printf("analyzeFile(\"%s\", 0x" F_X(016) ", *, %s%s%d, %d)\n",
                       striAsUnquotedCStri(sourceFileArgument), options,
                       errorFile == NULL ? "NULL " : "",
                       errorFile->cFile == NULL? "NULL_FILE " : "",
                       errorFile != NULL ? safe_fileno(errorFile->cFile) : 0,
                       *err_info););
    initAnalyze();
    if (sourceFileArgument->size > STRLEN(".sd7") &&
        sourceFileArgument->mem[sourceFileArgument->size - 4] == '.' &&
        sourceFileArgument->mem[sourceFileArgument->size - 3] == 's' &&
        sourceFileArgument->mem[sourceFileArgument->size - 2] == 'd' &&
        sourceFileArgument->mem[sourceFileArgument->size - 1] == '7') {
      nameLen = sourceFileArgument->size;
      add_extension = FALSE;
    } else {
      nameLen = sourceFileArgument->size + STRLEN(".sd7");
      add_extension = TRUE;
    } /* if */
    if (strChPos(sourceFileArgument, (charType) '\\') != 0) {
      logError(printf("analyzeFile(\"%s\", ...): Backslash in filename.\n",
                      striAsUnquotedCStri(sourceFileArgument)););
      *err_info = RANGE_ERROR;
    } else if (!ALLOC_STRI_CHECK_SIZE(sourceFilePath, nameLen)) {
      *err_info = MEMORY_ERROR;
    } else if (*err_info == OKAY_NO_ERROR) {
      sourceFilePath->size = nameLen;
      memcpy(sourceFilePath->mem, sourceFileArgument->mem,
             sourceFileArgument->size * sizeof(strElemType));
      if (add_extension) {
        memcpy_to_strelem(&sourceFilePath->mem[sourceFileArgument->size],
                          (const_ustriType) ".sd7", STRLEN(".sd7"));
      } /* if */
      if ((absolutePath = getAbsolutePath(sourceFilePath)) == NULL) {
        *err_info = MEMORY_ERROR;
      } else {
        isOpen = openInfile(sourceFilePath, absolutePath, 1, NULL,
                            (options & WRITE_LIBRARY_NAMES) != 0,
                            (options & WRITE_LINE_NUMBERS) != 0, err_info);
        if (!isOpen) {
          FREE_STRI(absolutePath);
          if (add_extension) {
            /* The function below does not raise an exception. */
            sourceFilePath = strHeadAssign(sourceFilePath,
                                           (intType) sourceFileArgument->size);
            if ((absolutePath = getAbsolutePath(sourceFilePath)) == NULL) {
              *err_info = MEMORY_ERROR;
            } else {
              isOpen = openInfile(sourceFilePath, absolutePath, 1, NULL,
                                  (options & WRITE_LIBRARY_NAMES) != 0,
                                  (options & WRITE_LINE_NUMBERS) != 0, err_info);
              if (!isOpen) {
                FREE_STRI(absolutePath);
              } /* if */
            } /* if */
          } /* if */
        } /* if */
#if HAS_SYMBOLIC_LINKS
        if (isOpen) {
          sourceFilePath = followLink(sourceFilePath, err_info);
        } /* if */
#endif
        if (isOpen && sourceFilePath != NULL) {
          resultProg = analyzeProg(sourceFileArgument, sourceFilePath, options,
                                   libraryDirs, errorFile, err_info);
        } else if (*err_info == FILE_ERROR) {
          *err_info = OKAY_NO_ERROR;
        } /* if */
      } /* if */
      if (sourceFilePath != NULL) {
        FREE_STRI(sourceFilePath);
      } /* if */
    } /* if */
    logFunction(printf("analyzeFile --> " FMT_U_MEM
                       " (error_count=%u, err_info=%d)\n",
                       (memSizeType) resultProg,
                       resultProg != 0 ? resultProg->error_count : 0,
                       *err_info););
    return resultProg;
  } /* analyzeFile */



progType analyze (const const_striType sourceFileArgument, uintType options,
    const const_rtlArrayType libraryDirs, const const_striType protFileName)

  {
    fileType errorFile;
    struct striStruct mode_buffer;
    striType mode;
    errInfoType err_info = OKAY_NO_ERROR;
    progType resultProg;

  /* analyze */
    logFunction(printf("analyze(\"%s\", 0x" F_X(016) ", *, ",
                       striAsUnquotedCStri(sourceFileArgument), options);
                printf("\"%s\")\n",
                       striAsUnquotedCStri(protFileName)););
    set_protfile_name(protFileName);
    if (protFileName == NULL) {
      errorFile = &stderrFileRecord;
    } else {
      mode = chrStrMacro('w', mode_buffer);
      errorFile = filOpen(protFileName, mode);
    } /* if */
    if (errorFile == &nullFileRecord) {
      prot_cstri("*** Could not open error file ");
      prot_stri(protFileName);
      prot_nl();
      resultProg = NULL;
    } else {
      resultProg = analyzeFile(sourceFileArgument, options,
                               libraryDirs, errorFile, &err_info);
      if (err_info == MEMORY_ERROR) {
        prot_cstri("*** No more memory");
        prot_nl();
      } else if (resultProg == NULL || err_info != OKAY_NO_ERROR) {
        prot_cstri("*** File ");
        prot_stri(sourceFileArgument);
        prot_cstri(" not found");
        prot_nl();
      } /* if */
      if (protFileName != NULL) {
        filDestr(errorFile);
      } /* if */
    } /* if */
    logFunction(printf("analyze --> " FMT_U_MEM " (error_count=%u)\n",
                       (memSizeType) resultProg,
                       resultProg != 0 ? resultProg->error_count : 0););
    return resultProg;
  } /* analyze */



progType analyzeBString (const bstriType input_bstri, uintType options,
    const const_rtlArrayType libraryDirs, const fileType errorFile,
    errInfoType *err_info)

  {
    striType sourceFileArgument;
    boolType isOpen;
    progType resultProg = NULL;

  /* analyzeBString */
    logFunction(printf("analyzeBString(\"%s\", 0x" F_X(016) ", *, %s%s%d, %d)\n",
                       bstriAsUnquotedCStri(input_bstri), options,
                       errorFile == NULL ? "NULL " : "",
                       errorFile->cFile == NULL? "NULL_FILE " : "",
                       errorFile != NULL ? safe_fileno(errorFile->cFile) : 0,
                       *err_info););
    initAnalyze();
    sourceFileArgument = CSTRI_LITERAL_TO_STRI("STRING");
    if (sourceFileArgument == NULL) {
      *err_info = MEMORY_ERROR;
    } else {
      isOpen = openBString(input_bstri, 1, NULL,
                           (options & WRITE_LIBRARY_NAMES) != 0,
                           (options & WRITE_LINE_NUMBERS) != 0, err_info);
      if (isOpen) {
        resultProg = analyzeProg(sourceFileArgument, sourceFileArgument,
                                 options, libraryDirs, errorFile, err_info);
      } /* if */
      FREE_STRI(sourceFileArgument);
    } /* if */
    logFunction(printf("analyzeBString --> " FMT_U_MEM
                       " (error_count=%u, err_info=%d)\n",
                       (memSizeType) resultProg,
                       resultProg != 0 ? resultProg->error_count : 0,
                       *err_info););
    return resultProg;
  } /* analyzeBString */



progType analyzeString (const const_striType input_string, uintType options,
    const const_rtlArrayType libraryDirs, const fileType errorFile,
    errInfoType *err_info)

  {
    bstriType input_bstri;
    progType resultProg;

  /* analyzeString */
    logFunction(printf("analyzeString(\"%s\", 0x" F_X(016) ", *, %s%s%d, %d)\n",
                       striAsUnquotedCStri(input_string), options,
                       errorFile == NULL ? "NULL " : "",
                       errorFile->cFile == NULL? "NULL_FILE " : "",
                       errorFile != NULL ? safe_fileno(errorFile->cFile) : 0,
                       *err_info););
    input_bstri = stri_to_bstri8(input_string);
    if (input_bstri == NULL) {
      *err_info = MEMORY_ERROR;
      resultProg = NULL;
    } else {
      resultProg = analyzeBString(input_bstri, options, libraryDirs,
                                  errorFile, err_info);
      FREE_BSTRI(input_bstri, input_bstri->size);
    } /* if */
    logFunction(printf("analyzeString --> " FMT_U_MEM
                       " (error_count=%u, err_info=%d)\n",
                       (memSizeType) resultProg,
                       resultProg != 0 ? resultProg->error_count : 0,
                       *err_info););
    return resultProg;
  } /* analyzeString */

/********************************************************************/
/*                                                                  */
/*  chkccomp.c    Check properties of C compiler and runtime.       */
/*  Copyright (C) 2010 - 2024  Thomas Mertes                        */
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
/*  Module: Chkccomp                                                */
/*  File: seed7/src/chkccomp.c                                      */
/*  Changes: 2010 - 2024  Thomas Mertes                             */
/*  Content: Program to Check properties of C compiler and runtime. */
/*                                                                  */
/********************************************************************/

#include "base.h"

/**
 *  The file base.h is NOT part of the Seed7 release. Instead base.h
 *  is generated from the makefile with "make depend" and removed with
 *  "make clean". The file base.h is used for two purposes:
 *   1. It is included in chkccomp.c (see above).
 *   2. Its contents are copied to version.h by chkccomp.
 *  From base.h the following defines are used (for details see: read_me.txt):
 *
 *  PATH_DELIMITER: (mandatory)
 *      Path delimiter character used by the command shell of the operating system.
 *  OBJECT_FILE_EXTENSION: (mandatory)
 *      The extension used by the C compiler for object files.
 *  C_COMPILER: (mandatory)
 *      Contains the command to call the stand-alone C compiler and linker.
 *      C_COMPILER_SCRIPT and TEST_C_COMPILER can be used instead of C_COMPILER.
 *      If the C compiler is called via a script C_COMPILER_SCRIPT is defined
 *      and C_COMPILER is not defined. In that case TEST_C_COMPILER is defined
 *      (in chkccomp.h) and it is used instead of C_COMPILER as command of
 *      the stand-alone C compiler and linker.
 *  LINKER_OPT_OUTPUT_FILE: (mandatory)
 *      Contains the linker option to provide the output filename (e.g.: "-o ").
 *      Usually C compiler and linker are called with C_COMPILER. Without
 *      LINKER_OPT_OUTPUT_FILE the output file name depends on the linker
 *      used. Some use a fixed name like a.out while others use the source
 *      name with a different extension.
 *  OS_STRI_WCHAR: (optional)
 *      Defined if the system calls (os_...) use wide characters (type wchar_t)
 *      for string and path parameters.
 *  QUOTE_WHOLE_SHELL_COMMAND: (optional)
 *      Defined if shell commands, starting with " need to be quoted again.
 *  EXECUTABLE_FILE_EXTENSION: (optional)
 *      The extension which is used by the operating system for executables.
 *  LINKED_PROGRAM_EXTENSION: (optional)
 *      The extension of the file produced by compiling and linking a program.
 *      Normally this is identical to the EXECUTABLE_FILE_EXTENSION, but in case
 *      of Emscripten this is independent from the EXECUTABLE_FILE_EXTENSION.
 *  INTERPRETER_FOR_LINKED_PROGRAM: (optional)
 *      Defines an interpreter that is used if compiler and linker create
 *      a file that must be interpreted.
 *  CC_OPT_OPTIMIZE_1: (optional)
 *      C compiler option to optimize with a level of -O1.
 *  CC_OPT_OPTIMIZE_2: (optional)
 *      C compiler option to optimize with a level of -O2.
 *  CC_OPT_OPTIMIZE_3: (optional)
 *      C compiler option to optimize with a level of -O3.
 *  CC_OPT_TRAP_OVERFLOW: (optional)
 *      Contains a C compiler option that triggers the generation of code to
 *      raise OVERFLOW_SIGNAL in case there is an integer overflow.
 *  CC_OPT_VERSION_INFO: (optional)
 *      C compiler option to write the C compiler version information.
 *  CC_FLAGS: (optional)
 *      Contains C compiler flags, which should be used when C programs are
 *      compiled.
 *  CC_NO_OPT_OUTPUT_FILE: (optional)
 *      Defined, if compiling and linking with one command cannot use -o.
 *  CC_ERROR_FILEDES: (optional)
 *      File descriptor to which the C compiler writes errors.
 *  CC_VERSION_INFO_FILEDES: (optional)
 *      File descriptor to which the C compiler writes its version info.
 *  LINKER: (optional)
 *      Defined if C_COMPILER does just invoke the stand-alone C compiler.
 *      In that case LINKER contains the command to call the stand-alone linker.
 *  SYSTEM_LIBS: (optional)
 *      Contains system libraries for the stand-alone linker.
 *  SYSTEM_MATH_LIBS: (optional)
 *      Contains system mathematic libraries for the stand-alone linker.
 *  INT64TYPE_NO_SUFFIX_BUT_CAST: (optional)
 *      Defined if 64-bit integer literals do not use a suffix.
 *  TURN_OFF_FP_EXCEPTIONS (optional)
 *      Use the function _control87() to turn off floating point exceptions.
 *  INT_DIV_BY_ZERO_POPUP: (optional)
 *      Defined if an integer division by zero may trigger a popup window.
 *      Consequently chkccomp.c defines CHECK_INT_DIV_BY_ZERO, to avoid the
 *      popup.
 *  DO_SIGFPE_WITH_DIV_BY_ZERO: (optional)
 *      TRUE, if SIGFPE should be raised with an integer division by zero.
 *      If it is FALSE raise(SIGFPE) can be called instead. Under Windows
 *      it is necessary to trigger SIGFPE this way, to assure that the debugger
 *      can catch it.
 *  LIMITED_CSTRI_LITERAL_LEN: (optional)
 *      = 0: The C compiler does not limit the length of string literals.
 *      != 0: An array of characters should be used instead of a string literal.
 *      Some compilers (cl) run out of heap space when compiling chkbig.sd7
 *      with string literals. To avoid this problem the makefile (mk_msvc.mak)
 *      defines LIMITED_CSTRI_LITERAL_LEN.
 *  USE_ALTERNATE_LOCALTIME_R: (optional)
 *      Defined if the function alternate_localtime_r() should be used
 *      instead of localtime().
 *  FORMAT_LL_TRIGGERS_WARNINGS: (optional)
 *      Defined if the printf format ll triggers a warning (and there is an
 *      alternate format).
 *  FILENO_WORKS_FOR_NULL: (optional)
 *      TRUE, if the fileno() function works for NULL and returns -1.
 *  STAT_MISSING: (optional)
 *      Defined if the function stat() is missing.
 *  REDIRECT_FILEDES_1: (optional)
 *      Symbol to redirect stdout in shell commands (executed with system()).
 *  REDIRECT_FILEDES_2: (optional)
 *      Symbol to redirect stderr in shell commands (executed with system()).
 */

#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "stddef.h"
#include "time.h"
#include "ctype.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "errno.h"

#include "chkccomp.h"

/**
 *  The file chkccomp.h is NOT part of the Seed7 release. Instead chkccomp.h
 *  is generated from the makefile with "make depend" and removed with
 *  "make clean". In chkccomp.h the following macros might be defined:
 *
 *  TEST_C_COMPILER:
 *      If TEST_C_COMPILER is defined it is used instead of C_COMPILER
 *      as command of the stand-alone C compiler and linker.
 *  LIST_DIRECTORY_CONTENTS:
 *      Either "ls" or "dir".
 *      E.g.: #define LIST_DIRECTORY_CONTENTS "ls"
 *            #define LIST_DIRECTORY_CONTENTS "dir"
 *  CC_FLAGS: (optional)
 *      Contains C compiler flags, which should be used when C programs are
 *      compiled. This is defined in chkccomp.h if 32- and 64-bit flags differ.
 *  CC_FLAGS64: (optional)
 *      Contains additional C compiler flags, which should be used when
 *      C programs are compiled with a 64-bit C compiler.
 *  CC_OPT_LINK_TIME_OPTIMIZATION: (optional)
 *      Contains the compiler option for link time optimization (e.g.: "-flto").
 *  LINKER_OPT_STATIC_LINKING: (optional)
 *      Contains the linker option to force static linking (e.g.: "-static").
 *  LINKER_OPT_DYN_LINK_LIBS: (optional)
 *      Contains the linker option to add the dynamic linking library.
 *      It might be added to the SYSTEM_DRAW_LIBS, SYSTEM_CONSOLE_LIBS or
 *      SYSTEM_DATABASE_LIBS settings.
 *  POTENTIAL_PARTIAL_LINKING_OPTIONS: (optional)
 *      A comma separated list of potential partial/incremental linking options.
 *      Defined if partial/incremental linking is possibly supported.
 *      This list is tested to find an option that supports partial linking.
 *      A working option is written to "version.h" and "macros" under the
 *      name LINKER_OPT_PARTIAL_LINKING. Usually LINKER_OPT_PARTIAL_LINKING
 *      is "-r" or "-r -nostdlib". In case partial works the source code
 *      can be compiled with the options -c and LINKER_OPT_PARTIAL_LINKING.
 *      This option produces a relocatable object as output. This is
 *      also known as partial linking. The tool objcopy is used also.
 *      Objcopy is used with the option -L symbolname which converts
 *      a global or weak symbol called symbolname into a local symbol.
 *      This way the symbol is not visible externally.
 *  ERROR_REDIRECTING_FAILS: (optional)
 *      Defined if redirection with 2> fails.
 *  USE_GMP: (optional)
 *      Defines which library is used to implement bigInteger arithmetic.
 *      Set to 1 if the GNU Multiple Precision Arithmetic Library (GMP)
 *      should be used. Set to 0 if the build-in big_rtl.c library should
 *      be used.
 *  SYSTEM_CONSOLE_LIBS: (optional)
 *      Options to link system console libraries to a program.
 *  SYSTEM_DRAW_LIBS: (optional)
 *      Options to link system graphic libraries to a program.
 *  ALLOW_REPLACEMENT_OF_SYSTEM_HEADERS: (optional)
 *      Defined if X11 or ncurses header files can be replaced by header
 *      files provided by Seed7.
 *  PRINTF_MAXIMUM_FLOAT_PRECISION: (optional)
 *      Precision up to which writing a float with printf (using format %e or
 *      %f) will always work ok.
 *  The macros described can be defined in a makefile and they are only used
 *  in chkccomp.c. These macros are not used in the Seed7 Interpreter (s7) or
 *  in the Seed7 Runtime Library.
 */


#ifndef LINKED_PROGRAM_EXTENSION
#ifdef EXECUTABLE_FILE_EXTENSION
#define LINKED_PROGRAM_EXTENSION EXECUTABLE_FILE_EXTENSION
#else
#define LINKED_PROGRAM_EXTENSION ""
#endif
#endif

#ifndef ARCHIVER
#define ARCHIVER "ar"
#endif

#ifndef ARCHIVER_OPT_REPLACE
#define ARCHIVER_OPT_REPLACE "r "
#endif

#ifndef LIBRARY_FILE_EXTENSION
#define LIBRARY_FILE_EXTENSION ".a"
#endif

#define UNIX_LIBRARIES 1
#define MACOS_LIBRARIES 2
#define WINDOWS_LIBRARIES 3

#ifdef _WIN32
#define LIBRARY_TYPE WINDOWS_LIBRARIES
#elif __APPLE__
#define LIBRARY_TYPE MACOS_LIBRARIES
#else
#define LIBRARY_TYPE UNIX_LIBRARIES
#endif

#ifndef CC_OPT_OPTIMIZE_1
#define CC_OPT_OPTIMIZE_1 "-O1"
#endif

#ifndef CC_OPT_OPTIMIZE_2
#define CC_OPT_OPTIMIZE_2 "-O2"
#endif

#ifndef CC_OPT_OPTIMIZE_3
#define CC_OPT_OPTIMIZE_3 "-O3"
#endif

#ifndef CC_OPT_TRAP_OVERFLOW
#define CC_OPT_TRAP_OVERFLOW ""
#endif

#ifndef CC_OPT_VERSION_INFO
#define CC_OPT_VERSION_INFO ""
#endif

#ifndef CC_FLAGS
#define CC_FLAGS ""
#endif

#ifndef LINKER_OPT_DYN_LINK_LIBS
#define LINKER_OPT_DYN_LINK_LIBS ""
#endif

#ifndef S_ISREG
#define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
#endif

#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif

#ifndef REDIRECT_FILEDES_1
#define REDIRECT_FILEDES_1 ">"
#endif

#ifndef REDIRECT_FILEDES_2
#define REDIRECT_FILEDES_2 "2>"
#endif

#ifndef FORMAT_LL_TRIGGERS_WARNINGS
#define FORMAT_LL_TRIGGERS_WARNINGS 0
#endif

#ifndef SYSTEM_LIBS
#define SYSTEM_LIBS ""
#endif

#ifndef SYSTEM_MATH_LIBS
#define SYSTEM_MATH_LIBS ""
#endif

#ifndef MYSQL_LIBRARY_PATH
#define MYSQL_LIBRARY_PATH ""
#endif

#ifndef SQLITE_LIBRARY_PATH
#define SQLITE_LIBRARY_PATH ""
#endif

#ifndef POSTGRESQL_LIBRARY_PATH
#define POSTGRESQL_LIBRARY_PATH ""
#endif

#ifndef ODBC_LIBRARY_PATH
#define ODBC_LIBRARY_PATH ""
#endif

#ifndef OCI_LIBRARY_PATH
#define OCI_LIBRARY_PATH ""
#endif

#ifndef FIRE_LIBRARY_PATH
#define FIRE_LIBRARY_PATH ""
#endif

#ifndef DB2_LIBRARY_PATH
#define DB2_LIBRARY_PATH ""
#endif

#ifndef SQL_SERVER_LIBRARY_PATH
#define SQL_SERVER_LIBRARY_PATH ""
#endif

#ifndef TDS_LIBRARY_PATH
#define TDS_LIBRARY_PATH ""
#endif

#ifndef INFORMIX_LIBRARY_PATH
#define INFORMIX_LIBRARY_PATH ""
#endif

#define NAME_SIZE    1024
#define PATH_SIZE    4096
#define COMMAND_SIZE 1024
#define BUFFER_SIZE  4096

static int testNumber = 0;
static char c_compiler[COMMAND_SIZE];
static const char *nullDevice = NULL;
static FILE *logFile = NULL;
static int buildTimeLinking = 0;
static unsigned long removeReattempts = 0;
static unsigned long filePresentAfterDelay = 0;
static unsigned long numberOfSuccessfulTestsAfterRestart = 0;

static char buildDirectory[PATH_SIZE];
static char s7LibDir[PATH_SIZE];
static char seed7Library[PATH_SIZE];

static int sizeof_int;
static int sizeof_long;
static int sizeof_long_long;
static int sizeof_pointer;
static int sizeof_float;
static int sizeof_double;

static const char *int16TypeStri = NULL;
static const char *uint16TypeStri = NULL;
static const char *int32TypeStri = NULL;
static const char *uint32TypeStri = NULL;
static const char *int32TypeSuffix = "";
static const char *int32TypeFormat = NULL;
static const char *int64TypeStri = NULL;
static const char *uint64TypeStri = NULL;
static const char *int64TypeSuffix = "";
static const char *int64TypeFormat = NULL;
static const char *int128TypeStri = NULL;
static const char *uint128TypeStri = NULL;

static const char *makeDirDefinition = NULL;
static const char *removeDirDefinition = NULL;

static int supportsPartialLinking = 0;



static void prepareCompileCommand (void)

  {
    int mapAbsolutePathToDriveLetters = 0;
    int pos;
    int quote_command = 0;
    size_t len;

  /* prepareCompileCommand */
#ifdef TEST_C_COMPILER
    strcpy(c_compiler, TEST_C_COMPILER);
#else
    strcpy(c_compiler, C_COMPILER);
#endif
    {
      char *searchPath;

      searchPath = getenv("PATH");
      if (searchPath != NULL &&
          isalpha(searchPath[0]) && searchPath[1] == ':') {
        mapAbsolutePathToDriveLetters = 1;
      } /* if */
    }
    if (mapAbsolutePathToDriveLetters) {
      if (c_compiler[0] == '/' && isalpha(c_compiler[1]) &&
          c_compiler[2] == '/') {
        c_compiler[0] = c_compiler[1];
        c_compiler[1] = ':';
      } /* if */
    } /* if */
    for (pos = 0; c_compiler[pos] != '\0'; pos++) {
      if (c_compiler[pos] == '/') {
        c_compiler[pos] = PATH_DELIMITER;
      } else if (c_compiler[pos] == ' ') {
        quote_command = 1;
      } /* if */
    } /* for */
    if (quote_command) {
      len = strlen(c_compiler);
      memmove(&c_compiler[1], c_compiler, len);
      c_compiler[0] = '\"';
      c_compiler[len + 1] = '\"';
      c_compiler[len + 2] = '\0';
    } /* if */
  } /* prepareCompileCommand */



#ifdef STAT_MISSING
static int fileExists (const char *fileName)

  {
    FILE *aFile;
    int exists;

  /* fileExists */
    aFile = fopen(fileName, "r");
    exists = aFile != NULL;
    if (exists) {
      fclose(aFile);
    } /* if */
    return exists;
  } /* fileExists */


#define fileIsRegular fileExists
#define fileIsDir fileExists

#else



static int fileIsRegular (const char *fileName)

  {
    struct stat stat_buf;

  /* fileIsRegular */
    return stat(fileName, &stat_buf) == 0 && S_ISREG(stat_buf.st_mode);
  } /* fileIsRegular */



static int fileIsDir (const char *fileName)

  {
    struct stat stat_buf;

  /* fileIsDir */
    return stat(fileName, &stat_buf) == 0 && S_ISDIR(stat_buf.st_mode);
  } /* fileIsDir */
#endif



static void doRemove (const char *fileName)

  {

  /* doRemove */
    fprintf(logFile, "#");
    fflush(logFile);
    if (fileIsRegular(fileName)) {
      if (remove(fileName) != 0) {
#if defined OS_STRI_WCHAR && defined _WIN32
        /* This workaround is necessary for windows. */
        time_t start_time;
        char command[COMMAND_SIZE];

        sprintf(command, "DEL %s > nul 2>&1", fileName);
        start_time = time(NULL);
        while (time(NULL) < start_time + 20 &&
               fileIsRegular(fileName)) {
          removeReattempts++;
          if (remove(fileName) != 0) {
            removeReattempts++;
            system(command);
          } /* if */
        } /* while */
        if (fileIsRegular(fileName)) {
          fprintf(logFile, "\n *** Cannot remove %s\n", fileName);
        } /* if */
#else
        fprintf(logFile, "\n *** Cannot remove %s\n", fileName);
#endif
      } /* if */
    } /* if */
    fprintf(logFile, "\b");
    fflush(logFile);
  } /* doRemove */



static void copyFile (const char *sourceName, const char *destName)

  {
    FILE *source;
    FILE *dest;
    char buffer[BUFFER_SIZE];
    size_t len;

  /* copyFile */
    if (sourceName != NULL && destName != NULL) {
      if ((source = fopen(sourceName, "r")) != NULL) {
        if ((dest = fopen(destName, "w")) != NULL) {
          while ((len = fread(buffer, 1, 1024, source)) != 0) {
            fwrite(buffer, 1, len, dest);
          } /* while */
          fclose(dest);
        } /* if */
        fclose(source);
      } /* if */
    } /* if */
  } /* copyFile */



static void appendFile (const char *sourceName, const char *destName)

  {
    FILE *source;
    FILE *dest;
    char buffer[BUFFER_SIZE];
    size_t len;

  /* appendFile */
    if (sourceName != NULL && destName != NULL) {
      if ((source = fopen(sourceName, "r")) != NULL) {
        if ((dest = fopen(destName, "a")) != NULL) {
          while ((len = fread(buffer, 1, 1024, source)) != 0) {
            fwrite(buffer, 1, len, dest);
          } /* while */
          fclose(dest);
        } /* if */
        fclose(source);
      } /* if */
    } /* if */
  } /* appendFile */



/**
 *  Removes quotes from quotations in a string with newline separated values.
 *  This function is used to write library options to the file version.h.
 *  The Seed7 compiler needs unquoted library options if the C linker is
 *  invoked without the use of a shell (that would interpret the quotations).
 *  The unquoted library options might contain spaces but this is not a problem.
 *  If text is "one\ntwo\n\"three four\"\n\"five  six\""
 *  after removeQuotesFromQuotations(text) the text buffer will contain:
 *  "one\ntwo\nthree four\nfive  six"
 */
static void removeQuotesFromQuotations (char *text)

  {
    char *source;
    char *dest;
    char *nlPos;
    int inQuotation = 0;

  /* removeQuotesFromQuotations */
    source = text;
    dest = text;
    if (*source == '\"') {
      nlPos = strchr(&source[1], '\n');
      if (nlPos == NULL) {
        nlPos = &source[strlen(source)];
      } /* if */
      if (nlPos >= &source[2] && nlPos[-1] == '\"') {
        source++;
        inQuotation = 1;
      } else {
        *dest = '\"';
        dest++;
        source++;
      } /* if */
    } /* if */
    while (*source != '\0') {
      if (*source == '\n') {
        *dest = '\n';
        dest++;
        if (source[1] == '\"') {
          nlPos = strchr(&source[2], '\n');
          if (nlPos == NULL) {
            nlPos = &source[strlen(source)];
          } /* if */
          if (nlPos >= &source[3] && nlPos[-1] == '\"') {
            source += 2;
            inQuotation = 1;
          } else {
            *dest = '\"';
            dest++;
            source += 2;
          } /* if */
        } else {
          source++;
        } /* if */
      } else if (*source == '\"') {
        if (source[1] == '\n' || source[1] == '\0') {
          if (inQuotation) {
            source++;
            inQuotation = 0;
          } else {
            *dest = '\"';
            dest++;
            source++;
          } /* if */
        } else {
          *dest = '\"';
          dest++;
          source++;
        } /* if */
      } else {
        *dest = *source;
        dest++;
        source++;
      } /* if */
    } /* while */
    *dest = '\0';
  } /* removeQuotesFromQuotations */



/**
 *  Replace unquoted spaces with newlines.
 *  Unquoted consecutive spaces are replaced with one newline.
 *  This function is used to convert library options that come from chkccomp.h.
 *  Makefiles write library options to chkccomp.h and use them also when they
 *  invoke the C linker (because of that the quoting of paths with spaces is needed).
 *  E.g.: replaceUnquotedSpacesWithNl(dest, "one  two \"three four\"  \"five  six\"")
 *        assigns "one\ntwo\n\"three four\"\n\"five  six\"" to dest.
 */
static void replaceUnquotedSpacesWithNl (char *dest, const char *source)

  {
    const char *quotePos;
    const char *spacePos;
    size_t len;

  /* replaceUnquotedSpacesWithNl */
    while (*source == ' ') {
      source++;
    } /* while */
    while (*source != '\0') {
      if (*source == '\"') {
        quotePos = source;
        do {
          quotePos = strchr(&quotePos[1], '\"');
        } while (quotePos != NULL && quotePos[1] != ' ' && quotePos[1] != '\0');
        if (quotePos != NULL) {
          memcpy(dest, source, (quotePos - source) + 1);
          dest += (quotePos - source) + 1;
          source = &quotePos[1];
          while (*source == ' ') {
            source++;
          } /* while */
          if (quotePos[1] == ' ' && *source != '\0') {
            *dest = '\n';
            dest++;
          } /* if */
        } else {
          len = strlen(&source[1]);
          memcpy(dest, &source[1], len);
          dest += len;
          source = &source[1 + len];
        } /* if */
      } else {
        while (*source == ' ') {
          source++;
        } /* while */
        if (*source != '\0') {
          spacePos = strchr(source, ' ');
          if (spacePos != NULL) {
            memcpy(dest, source, spacePos - source);
            dest += spacePos - source;
            source = &spacePos[1];
            while (*source == ' ') {
              source++;
            } /* while */
            if (*source != '\0') {
              *dest = '\n';
              dest++;
            } /* if */
          } else {
            len = strlen(source);
            memcpy(dest, source, len);
            dest += len;
            source = &source[len];
          } /* if */
        } /* if */
      } /* if */
    } /* while */
    *dest = '\0';
  } /* replaceUnquotedSpacesWithNl */



/**
 *  Replace all newlines with spaces.
 *  This function is used just before the C compiler is invoked.
 *  It is assumed that paths with spaces in them are quoted.
 */
static void replaceNLBySpace (char *text)

  { /* replaceNLBySpace */
    while (*text != '\0') {
      if (*text == '\n') {
        *text = ' ';
      } /* if */
      text++;
    } /* while */
  } /* replaceNLBySpace */



static void escapeString (FILE *versionFile, const char *text)

  { /* escapeString */
    while (*text != '\0') {
      if (*text == '\"' || *text == '\\') {
        putc('\\', versionFile);
        putc(*text, versionFile);
      } else if (*text == '\n') {
        fputs("\\n", versionFile);
      } else {
        putc(*text, versionFile);
      } /* if */
      text++;
    } /* while */
  } /* escapeString */



static void determineCompilerVersion (FILE *versionFile)

  {
    char command[BUFFER_SIZE];
    int cc_version_info_filedes = 0;
    FILE *aFile;
    int ch;

  /* determineCompilerVersion */
#ifdef CC_VERSION_INFO_FILEDES
    cc_version_info_filedes = CC_VERSION_INFO_FILEDES;
#else
    /* Use heuristic to determine CC_VERSION_INFO_FILEDES. */
#ifdef ERROR_REDIRECTING_FAILS
    sprintf(command, "%s %s %scc_vers1.txt",
            c_compiler, CC_OPT_VERSION_INFO,
            REDIRECT_FILEDES_1);
#else
    sprintf(command, "%s %s %scc_vers1.txt %scc_vers2.txt",
            c_compiler, CC_OPT_VERSION_INFO,
            REDIRECT_FILEDES_1, REDIRECT_FILEDES_2);
#endif
    aFile = fopen("cc_vers1.txt", "r");
    if (aFile != NULL) {
      ch = getc(aFile);
      fclose(aFile);
      if (ch != EOF) {
        cc_version_info_filedes = 1;
      } /* if */
    } /* if */
    if (cc_version_info_filedes == 0) {
      aFile = fopen("cc_vers2.txt", "r");
      if (aFile != NULL) {
        ch = getc(aFile);
        fclose(aFile);
        if (ch != EOF) {
          cc_version_info_filedes = 2;
        } /* if */
      } /* if */
    } /* if */
    doRemove("cc_vers1.txt");
    doRemove("cc_vers2.txt");
    fprintf(versionFile, "#define CC_VERSION_INFO_FILEDES %d\n", cc_version_info_filedes);
#endif
    if (cc_version_info_filedes == 1) {
      if (nullDevice != NULL) {
#ifdef ERROR_REDIRECTING_FAILS
        sprintf(command, "%s %s %scc_vers.txt",
                c_compiler, CC_OPT_VERSION_INFO, REDIRECT_FILEDES_1);
#else
        sprintf(command, "%s %s %scc_vers.txt %s%s",
                c_compiler, CC_OPT_VERSION_INFO, REDIRECT_FILEDES_1,
                REDIRECT_FILEDES_2, nullDevice);
#endif
      } else {
        sprintf(command, "%s %s %scc_vers.txt",
                c_compiler, CC_OPT_VERSION_INFO, REDIRECT_FILEDES_1);
      } /* if */
    } else if (cc_version_info_filedes == 2) {
      if (nullDevice != NULL) {
        sprintf(command, "%s %s %scc_vers.txt %s%s",
                c_compiler, CC_OPT_VERSION_INFO, REDIRECT_FILEDES_2,
                REDIRECT_FILEDES_1, nullDevice);
      } else {
        sprintf(command, "%s %s %scc_vers.txt",
                c_compiler, CC_OPT_VERSION_INFO, REDIRECT_FILEDES_2);
      } /* if */
    } /* if */
    system(command);
    aFile = fopen("cc_vers.txt", "r");
    if (aFile != NULL) {
      fprintf(versionFile, "#define C_COMPILER_VERSION \"");
      do {
        ch = getc(aFile);
      } while (ch == 10 || ch == 13);
      for (; ch != EOF && ch != 10 && ch != 13; ch = getc(aFile)) {
        if (ch >= ' ' && ch <= '~') {
          if (ch == '\"' || ch == '\'' || ch == '\\') {
            putc('\\', versionFile);
          } /* if */
          putc(ch, versionFile);
        } else {
          fprintf(versionFile, "\\%3o", ch);
        } /* if */
      } /* for */
      fputs("\"\n", versionFile);
      fclose(aFile);
      doRemove("cc_vers.txt");
    } /* if */
  } /* determineCompilerVersion */



static void cleanUpCompilation (const char *programName, int testNumberToClean)

  {
    char baseFileName[NAME_SIZE];
    char fileName[NAME_SIZE + 6];

  /* cleanUpCompilation */
    if (testNumberToClean == 0) {
      strcpy(baseFileName, programName);
    } else {
      sprintf(baseFileName, "%s%d", programName, testNumberToClean);
    } /* if */
    sprintf(fileName, "%s.c", baseFileName);
    doRemove(fileName);
    sprintf(fileName, "%s.cerrs", baseFileName);
    doRemove(fileName);
    sprintf(fileName, "%s.lerrs", baseFileName);
    doRemove(fileName);
    sprintf(fileName, "%s%s", baseFileName, OBJECT_FILE_EXTENSION);
    doRemove(fileName);
    sprintf(fileName, "%s%s", baseFileName, LINKED_PROGRAM_EXTENSION);
    doRemove(fileName);
    sprintf(fileName, "%s.out", baseFileName);
    doRemove(fileName);
    sprintf(fileName, "%s.err", baseFileName);
    doRemove(fileName);
#ifdef REMOVE_WASM
    sprintf(fileName, "%s.wasm", baseFileName);
    doRemove(fileName);
#endif
  } /* cleanUpCompilation */



static int fileIsPresentPossiblyAfterDelay (const char *fileName)

  {
    int fileIsPresent;

  /* fileIsPresentPossiblyAfterDelay */
    fileIsPresent = fileIsRegular(fileName);
#if defined OS_STRI_WCHAR && defined _WIN32
    if (!fileIsPresent) {
      /* Maybe the file shows up after executing a shell command. */
      char command[COMMAND_SIZE];

      sprintf(command, "%s %s > nul 2>&1", LIST_DIRECTORY_CONTENTS, fileName);
      system(command);
      fileIsPresent = fileIsRegular(fileName);
      if (fileIsPresent) {
        filePresentAfterDelay++;
      } /* if */
    } /* if */
#endif
    return fileIsPresent;
  } /* fileIsPresentPossiblyAfterDelay */



static int doCompile (const char *compilerOptions, int testNumberToCompile)

  {
    char command[COMMAND_SIZE + NAME_SIZE];
    int len;
    char fileName[NAME_SIZE];
    int returncode;
    int okay = 0;

  /* doCompile */
    fprintf(logFile, "*");
    fflush(logFile);
    sprintf(command, "%s %s %s -c ctest%d.c",
            c_compiler, compilerOptions, CC_FLAGS, testNumberToCompile);
    replaceNLBySpace(command);
#ifdef CC_ERROR_FILEDES
    /* A missing CC_ERROR_FILEDES or an CC_ERROR_FILEDES of zero means: Do not redirect. */
    if (CC_ERROR_FILEDES == 1) {
      sprintf(&command[strlen(command)], " %sctest%d.cerrs %s%s",
              REDIRECT_FILEDES_1, testNumberToCompile, REDIRECT_FILEDES_2, nullDevice);
    } else if (CC_ERROR_FILEDES == 2) {
      sprintf(&command[strlen(command)], " %sctest%d.cerrs %s%s",
              REDIRECT_FILEDES_2, testNumberToCompile, REDIRECT_FILEDES_1, nullDevice);
    } /* if */
#endif
#ifdef QUOTE_WHOLE_SHELL_COMMAND
    if (command[0] == '\"') {
      len = strlen(command);
      memmove(&command[1], command, len);
      command[0] = '\"';
      command[len + 1] = '\"';
      command[len + 2] = '\0';
    } /* if */
#endif
    /* fprintf(logFile, "command: %s\n", command); */
    returncode = system(command);
    sprintf(fileName, "ctest%d%s", testNumberToCompile, OBJECT_FILE_EXTENSION);
    if (fileIsPresentPossiblyAfterDelay(fileName)) {
      if (returncode == 0) {
        okay = 1;
      } else {
        /* fprintf(logFile, "\n *** The compiler %s fails, but creates an executable.\n", c_compiler); */
      } /* if */
    } else {
      /* fprintf(logFile, "\n *** The compiler %s produces no executable: %s\n", c_compiler, fileName); */
    } /* if */
#ifdef DEBUG_CHKCCOMP
    fprintf(logFile, "doCompile command: %s\n", command);
    fprintf(logFile, "returncode: %d\n", returncode);
    if (returncode == -1) {
      fprintf(logFile, "errno: %d\nerror: %s\n", errno, strerror(errno));
    } /* if */
    fprintf(logFile, "okay: %d\n", okay);
#endif
    fprintf(logFile, "\b.");
    fflush(logFile);
    return okay;
  } /* doCompile */



static int compileWithOptionsOk (const char *content, const char *compilerOptions)

  {
    char fileName[NAME_SIZE];
    FILE *testFile;
    int okay = 0;

  /* compileWithOptionsOk */
    /* fprintf(logFile, "compileWithOptionsOk(%s)\n", content); */
    cleanUpCompilation("ctest", testNumber);
    testNumber++;
    cleanUpCompilation("ctest", testNumber);
    sprintf(fileName, "ctest%d.c", testNumber);
    testFile = fopen(fileName, "w");
    if (testFile != NULL) {
      fprintf(testFile, "%s", content);
      fclose(testFile);
      okay = doCompile(compilerOptions, testNumber);
    } /* if */
#ifdef DEBUG_CHKCCOMP
    fprintf(logFile, "content: %s\n", content);
#endif
    /* fprintf(logFile, "compileWithOptionsOk --> %d\n", okay); */
    return okay;
  } /* compileWithOptionsOk */



static int doLink (const char *objectOrLibraryName, const char *linkerOptions)

  {
    char command[COMMAND_SIZE + NAME_SIZE];
    int len;
    char fileName[NAME_SIZE];
    int returncode;
    int okay = 0;

  /* doLink */
    fprintf(logFile, "*");
    fflush(logFile);
#ifdef LINKER
    sprintf(command, "%s %s %s %sctest%d%s",
            LINKER, objectOrLibraryName, linkerOptions,
            LINKER_OPT_OUTPUT_FILE, testNumber, LINKED_PROGRAM_EXTENSION);
#else
    sprintf(command, "%s %s %s",
            c_compiler, objectOrLibraryName, linkerOptions);
    replaceNLBySpace(command);
#if defined LINKER_OPT_OUTPUT_FILE && !defined CC_NO_OPT_OUTPUT_FILE
    sprintf(&command[strlen(command)], " %sctest%d%s",
            LINKER_OPT_OUTPUT_FILE, testNumber, LINKED_PROGRAM_EXTENSION);
#endif
#endif
#ifdef CC_ERROR_FILEDES
    /* A missing CC_ERROR_FILEDES or an CC_ERROR_FILEDES of zero means: Do not redirect. */
    if (CC_ERROR_FILEDES == 1) {
      sprintf(&command[strlen(command)], " %sctest%d.lerrs %s%s",
              REDIRECT_FILEDES_1, testNumber, REDIRECT_FILEDES_2, nullDevice);
    } else if (CC_ERROR_FILEDES == 2) {
      sprintf(&command[strlen(command)], " %sctest%d.lerrs %s%s",
              REDIRECT_FILEDES_2, testNumber, REDIRECT_FILEDES_1, nullDevice);
    } /* if */
#endif
#ifdef QUOTE_WHOLE_SHELL_COMMAND
    if (command[0] == '\"') {
      len = strlen(command);
      memmove(&command[1], command, len);
      command[0] = '\"';
      command[len + 1] = '\"';
      command[len + 2] = '\0';
    } /* if */
#endif
    /* fprintf(logFile, "command: %s\n", command); */
    returncode = system(command);
    /* fprintf(logFile, "returncode: %d\n", returncode); */
    sprintf(fileName, "ctest%d%s", testNumber, LINKED_PROGRAM_EXTENSION);
    /* fprintf(logFile, "fileName: \"%s\"\n", fileName); */
    if (fileIsPresentPossiblyAfterDelay(fileName)) {
      if (returncode == 0) {
        okay = 1;
      } else {
        /* fprintf(logFile, "\n *** The compiler %s fails, but creates an executable.\n", c_compiler); */
      } /* if */
    } else {
      /* fprintf(logFile, "\n *** The compiler %s produces no executable: %s\n", c_compiler, fileName); */
    } /* if */
#ifdef DEBUG_CHKCCOMP
    fprintf(logFile, "doLink command: %s\n", command);
    fprintf(logFile, "returncode: %d\n", returncode);
    if (returncode == -1) {
      fprintf(logFile, "errno: %d\nerror: %s\n", errno, strerror(errno));
    } /* if */
    fprintf(logFile, "okay: %d\n", okay);
#endif
    fprintf(logFile, "\b.");
    fflush(logFile);
    return okay;
  } /* doLink */



static int doCompileAndLink (const char *programName, const char *compilerOptions,
    const char *linkerOptions, int testNumberToCompileAndLink)

  {
    char command[COMMAND_SIZE + 2 * NAME_SIZE];
    int len;
    char baseFileName[NAME_SIZE];
    char fileName[NAME_SIZE];
    int returncode;
    int okay = 0;

  /* doCompileAndLink */
    fprintf(logFile, "*");
    fflush(logFile);
    if (testNumberToCompileAndLink == 0) {
      strcpy(baseFileName, programName);
    } else {
      sprintf(baseFileName, "%s%d", programName, testNumberToCompileAndLink);
    } /* if */
#ifdef LINKER
    sprintf(command, "%s %s %s -c %s.c",
            c_compiler, compilerOptions, CC_FLAGS, baseFileName);
#else
    sprintf(command, "%s %s %s %s.c %s",
            c_compiler, compilerOptions, CC_FLAGS, baseFileName, linkerOptions);
#endif
    replaceNLBySpace(command);
#if !defined LINKER && defined LINKER_OPT_OUTPUT_FILE && !defined CC_NO_OPT_OUTPUT_FILE
    sprintf(&command[strlen(command)], " %s%s%s",
            LINKER_OPT_OUTPUT_FILE, baseFileName, LINKED_PROGRAM_EXTENSION);
#endif
#ifdef CC_ERROR_FILEDES
    /* A missing CC_ERROR_FILEDES or an CC_ERROR_FILEDES of zero means: Do not redirect. */
    if (CC_ERROR_FILEDES == 1) {
      sprintf(&command[strlen(command)], " %s%s.cerrs %s%s",
              REDIRECT_FILEDES_1, baseFileName, REDIRECT_FILEDES_2, nullDevice);
    } else if (CC_ERROR_FILEDES == 2) {
      sprintf(&command[strlen(command)], " %s%s.cerrs %s%s",
              REDIRECT_FILEDES_2, baseFileName, REDIRECT_FILEDES_1, nullDevice);
    } /* if */
#endif
#ifdef QUOTE_WHOLE_SHELL_COMMAND
    if (command[0] == '\"') {
      len = strlen(command);
      memmove(&command[1], command, len);
      command[0] = '\"';
      command[len + 1] = '\"';
      command[len + 2] = '\0';
    } /* if */
#endif
    /* fprintf(logFile, "command: %s\n", command); */
    returncode = system(command);
#ifdef LINKER
    if (returncode == 0) {
      /* fprintf(logFile, "returncode: %d\n", returncode); */
      sprintf(command, "%s %s%s %s %s%s%s",
              LINKER, baseFileName, OBJECT_FILE_EXTENSION, linkerOptions,
              LINKER_OPT_OUTPUT_FILE, baseFileName, LINKED_PROGRAM_EXTENSION);
      /* fprintf(logFile, "command: %s\n", command); */
      returncode = system(command);
    } /* if */
#endif
    sprintf(fileName, "%s%s", baseFileName, LINKED_PROGRAM_EXTENSION);
    if (fileIsPresentPossiblyAfterDelay(fileName)) {
      if (returncode == 0) {
        okay = 1;
      } else {
        /* fprintf(logFile, "\n *** The compiler %s fails, but creates an executable.\n", c_compiler); */
      } /* if */
    } else {
      /* fprintf(logFile, "\n *** The compiler %s produces no executable: %s\n", c_compiler, fileName); */
    } /* if */
#ifdef DEBUG_CHKCCOMP
    fprintf(logFile, "doCompileAndLink command: %s\n", command);
    fprintf(logFile, "returncode: %d\n", returncode);
    if (returncode == -1) {
      fprintf(logFile, "errno: %d\nerror: %s\n", errno, strerror(errno));
    } /* if */
    fprintf(logFile, "okay: %d\n", okay);
#endif
    fprintf(logFile, "\b.");
    fflush(logFile);
    return okay;
  } /* doCompileAndLink */



static int doCompileAndLinkContent (const char *programName, const char *content,
    const char *compilerOptions, const char *linkerOptions, int testNumberToCompileAndLink)

  {
    char fileName[NAME_SIZE];
    FILE *testFile;
    int okay = 0;

  /* doCompileAndLinkContent */
    if (testNumberToCompileAndLink == 0) {
      sprintf(fileName, "%s.c", programName);
    } else {
      sprintf(fileName, "%s%d.c", programName, testNumberToCompileAndLink);
    } /* if */
    testFile = fopen(fileName, "w");
    if (testFile != NULL) {
      fprintf(testFile, "%s", content);
      fclose(testFile);
      okay = doCompileAndLink(programName, compilerOptions, linkerOptions, testNumberToCompileAndLink);
    } /* if */
#ifdef DEBUG_CHKCCOMP
    fprintf(logFile, "content: %s\n", content);
#endif
    return okay;
  } /* doCompileAndLinkContent */



static int compileAndLinkWithOptionsOk (const char *content, const char *compilerOptions,
    const char *linkerOptions)

  {
    int okay = 0;

  /* compileAndLinkWithOptionsOk */
    /* fprintf(logFile, "compileAndLinkWithOptionsOk(%s)\n", content); */
    cleanUpCompilation("ctest", testNumber);
    testNumber++;
    cleanUpCompilation("ctest", testNumber);
    okay = doCompileAndLinkContent("ctest", content, compilerOptions, linkerOptions, testNumber);
    /* fprintf(logFile, "compileAndLinkWithOptionsOk --> %d\n", okay); */
    return okay;
  } /* compileAndLinkWithOptionsOk */



static void showErrorsForTool (const char *tool, const char *extension)

  {
    char fileName[NAME_SIZE];
    FILE *errorFile;
    int ch;

  /* showErrorsForTool */
    sprintf(fileName, "ctest%d%s", testNumber, extension);
    errorFile = fopen(fileName, "r");
    if (errorFile != NULL) {
      ch = getc(errorFile);
      if (ch != EOF) {
        fprintf(logFile, "\n%s errors:\n", tool);
        do {
          fputc(ch, logFile);
          ch = getc(errorFile);
        } while (ch != EOF);
        fprintf(logFile, "\n");
      } /* if */
      fclose(errorFile);
    } /* if */
  } /* showErrorsForTool */



static void showErrors (void)

  { /* showErrors */
    showErrorsForTool("Compiler", ".cerrs");
    showErrorsForTool("Linker", ".lerrs");
  } /* showErrors */



static int assertCompAndLnkWithOptions (const char *content, const char *compilerOptions,
    const char *linkerOptions)

  {
    int okay;

  /* assertCompAndLnkWithOptions */
    okay = compileAndLinkWithOptionsOk(content, compilerOptions, linkerOptions);
    if (!okay) {
      fprintf(logFile, "\n **** Compile and link failed for:\n%s\n", content);
      showErrors();
    } /* if */
    return okay;
  } /* assertCompAndLnkWithOptions */



static int compileAndLinkOk (const char *content)

  { /* compileAndLinkOk */
    return compileAndLinkWithOptionsOk(content, "", "");
  } /* compileAndLinkOk */



static int assertCompAndLnk (const char *content)

  {
    int okay;

  /* assertCompAndLnk */
    okay = compileAndLinkOk(content);
    if (!okay) {
      fprintf(logFile, "\n **** Compile and link failed for:\n%s\n", content);
      showErrors();
    } /* if */
    return okay;
  } /* assertCompAndLnk */



static void prepareDoSleep (void)

  { /* prepareDoSleep */
#if defined UNIX_DO_SLEEP
    cleanUpCompilation("do_sleep", 0);
    if (!doCompileAndLinkContent("do_sleep",
                                 "#include <unistd.h>\n"
                                 "#include <stdio.h>\n"
                                 "int main(int argc, char *argv[]) {\n"
                                 "int seconds;\n"
                                 "if (argc == 2 && sscanf(argv[1], \"%d\", &seconds) == 1) {\n"
                                 "  sleep(seconds);\n"
                                 "} return 0;}\n",
                                 "", "", 0)) {
      fprintf(logFile, "\n **** Preparing do_sleep failed.\n");
    } /* if */
#elif defined _WIN32
    cleanUpCompilation("do_sleep", 0);
    if (!doCompileAndLinkContent("do_sleep",
                                 "#include <windows.h>\n"
                                 "#include <stdio.h>\n"
                                 "int main(int argc, char *argv[]) {\n"
                                 "int seconds;\n"
                                 "if (argc == 2 && sscanf(argv[1], \"%d\", &seconds) == 1) {\n"
                                 "  Sleep(1000 * seconds);\n"
                                 "} return 0;}\n",
                                 "", "", 0)) {
      fprintf(logFile, "\n **** Preparing do_sleep failed.\n");
    } /* if */
#endif
  }  /* prepareDoSleep */



static void removeDoSleep (void)

  { /* removeDoSleep */
#if defined UNIX_DO_SLEEP || defined _WIN32
    cleanUpCompilation("do_sleep", 0);
#endif
  } /* removeDoSleep */



static void doSleep (unsigned int seconds)

  {
    char command[COMMAND_SIZE];

  /* doSleep */
    /* printf("doSleep(%u)\n", seconds); */
#if defined UNIX_DO_SLEEP || defined _WIN32
#ifdef INTERPRETER_FOR_LINKED_PROGRAM
    sprintf(command, "%s .%cdo_sleep%s %d",
            INTERPRETER_FOR_LINKED_PROGRAM, PATH_DELIMITER,
            LINKED_PROGRAM_EXTENSION, seconds);
#else
    sprintf(command, ".%cdo_sleep%s %d", PATH_DELIMITER,
            LINKED_PROGRAM_EXTENSION, seconds);
#endif
#else
    sprintf(command, "sleep %d", seconds);
#endif
    system(command);
  }  /* doSleep */



static int runTest (int checkNumericValue)

  {
    char command[COMMAND_SIZE];
    char outputFileName[NAME_SIZE];
    char sourceFileName[NAME_SIZE];
    char backupFileName[NAME_SIZE];
    time_t startTime;
    int returncode;
    FILE *outFile;
    int readFailed = 0;
    int errorOccurred = 0;
    int repeatCount = 0;
    int result = -1;

  /* runTest */
    fprintf(logFile, "+");
    fflush(logFile);
#ifdef ERROR_REDIRECTING_FAILS
#ifdef INTERPRETER_FOR_LINKED_PROGRAM
    sprintf(command, "%s .%cctest%d%s %sctest%d.out",
            INTERPRETER_FOR_LINKED_PROGRAM, PATH_DELIMITER, testNumber,
            LINKED_PROGRAM_EXTENSION, REDIRECT_FILEDES_1, testNumber);
#else
    sprintf(command, ".%cctest%d%s %sctest%d.out", PATH_DELIMITER,
            testNumber, LINKED_PROGRAM_EXTENSION, REDIRECT_FILEDES_1,
            testNumber);
#endif
#else
#ifdef INTERPRETER_FOR_LINKED_PROGRAM
    sprintf(command, "%s .%cctest%d%s %sctest%d.out %sctest%d.err",
            INTERPRETER_FOR_LINKED_PROGRAM, PATH_DELIMITER, testNumber,
            LINKED_PROGRAM_EXTENSION, REDIRECT_FILEDES_1, testNumber,
            REDIRECT_FILEDES_2, testNumber);
#else
    sprintf(command, ".%cctest%d%s %sctest%d.out %sctest%d.err",
            PATH_DELIMITER, testNumber, LINKED_PROGRAM_EXTENSION,
            REDIRECT_FILEDES_1, testNumber, REDIRECT_FILEDES_2, testNumber);
#endif
#endif
    sprintf(outputFileName, "ctest%d.out", testNumber);
    startTime = time(NULL);
    do {
      returncode = system(command);
      if (returncode != -1) {
        if (fileIsPresentPossiblyAfterDelay(outputFileName)) {
          outFile = fopen(outputFileName, "r");
          if (outFile != NULL) {
            readFailed = fscanf(outFile, "%d", &result) != 1;
            fclose(outFile);
            if (checkNumericValue && readFailed) {
              /* This workaround is necessary for windows. Some  */
              /* antivirus software causes an empty output file. */
              /* The test program is restarted to get a result.  */
              doSleep(1);
              repeatCount++;
            } /* if */
          } else {
            fprintf(logFile, "\n *** Cannot open \"%s\".\n ", outputFileName);
            errorOccurred = 1;
          } /* if */
        } else {
          fprintf(logFile, "\n *** File \"%s\" missing.\n ", outputFileName);
          errorOccurred = 1;
        } /* if */
      } else {
        fprintf(logFile, "\n *** system(\"%s\") failed with errno: %d.\n ", command, errno);
        errorOccurred = 1;
      } /* if */
    } while (checkNumericValue && readFailed && !errorOccurred &&
             time(NULL) < startTime + 5);
    if (checkNumericValue && repeatCount != 0) {
      if (readFailed) {
        fprintf(logFile, "\n *** No numeric result in \"%s\".\n", outputFileName);
#ifndef ERROR_REDIRECTING_FAILS
        showErrorsForTool("Run", ".err");
#endif
        sprintf(sourceFileName, "ctest%d.c", testNumber);
        sprintf(backupFileName, "ctest%d.cbak", testNumber);
        copyFile(sourceFileName, backupFileName);
      } else {
        numberOfSuccessfulTestsAfterRestart++;
      } /* if */
    } /* if */
    fprintf(logFile, "\b");
    /* printf(" runTest: %d\n", result); */
    fflush(logFile);
    return result;
  } /* runTest */



static int doTest (void)

  { /* doTest */
    return runTest(1);
  } /* doTest */



static int doTestNoResultCheck (void)

  { /* doTestNoResultCheck */
    return runTest(0);
  } /* doTestNoResultCheck */



static int expectTestResult (const char *content, int expected)

  {
    int testResult;
    int okay = 0;

  /* expectTestResult */
    if (compileAndLinkOk(content)) {
      testResult = doTest();
      okay = testResult == expected;
    } else {
      fprintf(logFile, "\n *** Unable to compile test program:\n%s\n", content);
    } /* if */
    return okay;
  } /* expectTestResult */



static void testOutputToBuffer (char *buffer, size_t bufferSize)

  {
    char command[COMMAND_SIZE];
    char outputFileName[NAME_SIZE];
    char sourceFileName[NAME_SIZE];
    char backupFileName[NAME_SIZE];
    time_t startTime;
    int returncode;
    FILE *outFile;
    int ch;
    int pos = 0;
    int readFailed = 0;
    int errorOccurred = 0;
    int repeatCount = 0;

  /* testOutputToBuffer */
    fprintf(logFile, ">");
    fflush(logFile);
#ifdef ERROR_REDIRECTING_FAILS
#ifdef INTERPRETER_FOR_LINKED_PROGRAM
    sprintf(command, "%s .%cctest%d%s %sctest%d.out",
            INTERPRETER_FOR_LINKED_PROGRAM, PATH_DELIMITER, testNumber,
            LINKED_PROGRAM_EXTENSION, REDIRECT_FILEDES_1, testNumber);
#else
    sprintf(command, ".%cctest%d%s %sctest%d.out", PATH_DELIMITER,
            testNumber, LINKED_PROGRAM_EXTENSION, REDIRECT_FILEDES_1,
            testNumber);
#endif
#else
#ifdef INTERPRETER_FOR_LINKED_PROGRAM
    sprintf(command, "%s .%cctest%d%s %sctest%d.out %sctest%d.err",
            INTERPRETER_FOR_LINKED_PROGRAM, PATH_DELIMITER, testNumber,
            LINKED_PROGRAM_EXTENSION, REDIRECT_FILEDES_1, testNumber,
            REDIRECT_FILEDES_2, testNumber);
#else
    sprintf(command, ".%cctest%d%s %sctest%d.out %sctest%d.err",
            PATH_DELIMITER, testNumber, LINKED_PROGRAM_EXTENSION,
            REDIRECT_FILEDES_1, testNumber, REDIRECT_FILEDES_2, testNumber);
#endif
#endif
    sprintf(outputFileName, "ctest%d.out", testNumber);
    startTime = time(NULL);
    do {
      returncode = system(command);
      if (returncode != -1) {
        if (fileIsPresentPossiblyAfterDelay(outputFileName)) {
          outFile = fopen(outputFileName, "r");
          if (outFile != NULL) {
            ch = getc(outFile);
            readFailed = ch == EOF;
            if (readFailed) {
              /* This workaround is necessary for windows. Some  */
              /* antivirus software causes an empty output file. */
              /* The test program is restarted to get a result.  */
              fclose(outFile);
              doSleep(1);
              repeatCount++;
            } else {
              while (ch != EOF && ch != '\n' && pos < bufferSize - 1) {
                buffer[pos] = ch;
                pos++;
                ch = getc(outFile);
              } /* while */
              fclose(outFile);
              buffer[pos] = '\0';
            } /* if */
          } else {
            fprintf(logFile, "\n *** Cannot open \"%s\".\n ", outputFileName);
            errorOccurred = 1;
          } /* if */
        } else {
          fprintf(logFile, "\n *** File \"%s\" missing.\n ", outputFileName);
          errorOccurred = 1;
        } /* if */
      } else {
        fprintf(logFile, "\n *** system(\"%s\") failed with errno: %d.\n ", command, errno);
        errorOccurred = 1;
      } /* if */
    } while (readFailed && !errorOccurred && time(NULL) < startTime + 5);
    if (repeatCount != 0) {
      if (readFailed) {
        fprintf(logFile, "\n *** Cannot read data from \"%s\".\n", outputFileName);
#ifndef ERROR_REDIRECTING_FAILS
        showErrorsForTool("Run", ".err");
#endif
        sprintf(sourceFileName, "ctest%d.c", testNumber);
        sprintf(backupFileName, "ctest%d.cbak", testNumber);
        copyFile(sourceFileName, backupFileName);
      } else {
        numberOfSuccessfulTestsAfterRestart++;
      } /* if */
    } /* if */
    fprintf(logFile, "\b");
    fflush(logFile);
  } /* testOutputToBuffer */




static void testOutputToVersionFile (FILE *versionFile)

  {
    char command[COMMAND_SIZE];
    char outputFileName[NAME_SIZE];
    char sourceFileName[NAME_SIZE];
    char backupFileName[NAME_SIZE];
    time_t startTime;
    int returncode;
    FILE *outFile;
    int ch;
    int readFailed = 0;
    int errorOccurred = 0;
    int repeatCount = 0;

  /* testOutputToVersionFile */
    fprintf(logFile, ">");
    fflush(logFile);
#ifdef ERROR_REDIRECTING_FAILS
#ifdef INTERPRETER_FOR_LINKED_PROGRAM
    sprintf(command, "%s .%cctest%d%s %sctest%d.out",
            INTERPRETER_FOR_LINKED_PROGRAM, PATH_DELIMITER, testNumber,
            LINKED_PROGRAM_EXTENSION, REDIRECT_FILEDES_1, testNumber);
#else
    sprintf(command, ".%cctest%d%s %sctest%d.out", PATH_DELIMITER,
            testNumber, LINKED_PROGRAM_EXTENSION, REDIRECT_FILEDES_1,
            testNumber);
#endif
#else
#ifdef INTERPRETER_FOR_LINKED_PROGRAM
    sprintf(command, "%s .%cctest%d%s %sctest%d.out %sctest%d.err",
            INTERPRETER_FOR_LINKED_PROGRAM, PATH_DELIMITER, testNumber,
            LINKED_PROGRAM_EXTENSION, REDIRECT_FILEDES_1, testNumber,
            REDIRECT_FILEDES_2, testNumber);
#else
    sprintf(command, ".%cctest%d%s %sctest%d.out %sctest%d.err",
            PATH_DELIMITER, testNumber, LINKED_PROGRAM_EXTENSION,
            REDIRECT_FILEDES_1, testNumber, REDIRECT_FILEDES_2, testNumber);
#endif
#endif
    sprintf(outputFileName, "ctest%d.out", testNumber);
    startTime = time(NULL);
    do {
      returncode = system(command);
      if (returncode != -1) {
        if (fileIsPresentPossiblyAfterDelay(outputFileName)) {
          outFile = fopen(outputFileName, "r");
          if (outFile != NULL) {
            ch = getc(outFile);
            readFailed = ch == EOF;
            if (readFailed) {
              /* This workaround is necessary for windows. Some  */
              /* antivirus software causes an empty output file. */
              /* The test program is restarted to get a result.  */
              fclose(outFile);
              doSleep(1);
              repeatCount++;
            } else {
              do {
                putc(ch, versionFile);
              } while ((ch = getc(outFile)) != EOF);
              fclose(outFile);
            } /* if */
          } else {
            fprintf(logFile, "\n *** Cannot open \"%s\".\n ", outputFileName);
            errorOccurred = 1;
          } /* if */
        } else {
          fprintf(logFile, "\n *** File \"%s\" missing.\n ", outputFileName);
          errorOccurred = 1;
        } /* if */
      } else {
        fprintf(logFile, "\n *** system(\"%s\") failed with errno: %d.\n ", command, errno);
        errorOccurred = 1;
      } /* if */
    } while (readFailed && !errorOccurred && time(NULL) < startTime + 5);
    if (repeatCount != 0) {
      if (readFailed) {
        fprintf(logFile, "\n *** Cannot read data from \"%s\".\n", outputFileName);
#ifndef ERROR_REDIRECTING_FAILS
        showErrorsForTool("Run", ".err");
#endif
        sprintf(sourceFileName, "ctest%d.c", testNumber);
        sprintf(backupFileName, "ctest%d.cbak", testNumber);
        copyFile(sourceFileName, backupFileName);
      } else {
        numberOfSuccessfulTestsAfterRestart++;
      } /* if */
    } /* if */
    fprintf(logFile, "\b");
    fflush(logFile);
  } /* testOutputToVersionFile */



static int isNullDevice (const char *nullDeviceName, int eofChar)

  {
    FILE *aFile;
    int fileIsEmpty;
    int fileIsNullDevice = 0;

  /* isNullDevice */
    aFile = fopen(nullDeviceName, "r");
    if (aFile != NULL) {
      /* The file exists. Now check if the file is empty. */
      fileIsEmpty = getc(aFile) == eofChar;
      fclose(aFile);
      if (fileIsEmpty) {
        /* Reading from a null device returns always EOF. */
        aFile = fopen(nullDeviceName, "r+");
        if (aFile != NULL) {
          /* Writing of 'X' to a null device should be ignored. */
          putc('X', aFile);
          fclose(aFile);
          aFile = fopen(nullDeviceName, "r");
          if (aFile != NULL) {
            /* Check if the file is still empty. */
            fileIsEmpty = getc(aFile) == eofChar;
            fclose(aFile);
            if (fileIsEmpty) {
              /* Everything written to the null device is ignored. */
              fileIsNullDevice = 1;
            } else {
              /* The file is not empty, so it is not a null device. */
              aFile = fopen(nullDeviceName, "w");
              if (aFile != NULL) {
                /* Make sure that the file is empty, as it was before. */
                fclose(aFile);
              } /* if */
            } /* if */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    return fileIsNullDevice;
  } /* isNullDevice */



static void initializeNullDevice (void)

  { /* initializeNullDevice */
    if (isNullDevice("/dev/null", EOF)) {
      nullDevice = "/dev/null";
    } else if (isNullDevice("NUL:", EOF)) {
      nullDevice = "NUL:";
    } else if (isNullDevice("NUL", EOF)) {
      nullDevice = "NUL";
    } else if (isNullDevice("NUL:", 0)) {
      nullDevice = "NUL:";
    } else if (isNullDevice("NUL", 0)) {
      nullDevice = "NUL";
    } /* if */
    if (nullDevice == NULL) {
      fputs("\n **** Unable to determine a null device for chkccomp.\n", logFile);
      nullDevice = "null_device";
    } /* if */
  } /* initializeNullDevice */



static int checkIfNullDevice (const char *nullDeviceName, const char *eofName)

  {
    char buffer[BUFFER_SIZE];
    int fileIsNullDevice = 0;

  /* checkIfNullDevice */
    sprintf(buffer,
            "#include <stdio.h>\n"
            "int main(int argc, char *argv[]) {\n"
            "FILE *aFile;\n"
            "int fileIsEmpty;\n"
            "char *nullDeviceName = \"%s\";\n"
            "int eofChar = %s;\n"
            "int fileIsNullDevice = 0;\n"
            "aFile = fopen(nullDeviceName, \"r\");\n"
            "if (aFile != NULL) {\n"
            "  fileIsEmpty = getc(aFile) == eofChar;\n"
            "  fclose(aFile);\n"
            "  if (fileIsEmpty) {\n"
            "    aFile = fopen(nullDeviceName, \"r+\");\n"
            "    if (aFile != NULL) {\n"
            "      putc('X', aFile);\n"
            "      fclose(aFile);\n"
            "      aFile = fopen(nullDeviceName, \"r\");\n"
            "      if (aFile != NULL) {\n"
            "        fileIsEmpty = getc(aFile) == eofChar;\n"
            "        fclose(aFile);\n"
            "        if (fileIsEmpty) {\n"
            "          fileIsNullDevice = 1;\n"
            "        } else {\n"
            "          aFile = fopen(nullDeviceName, \"w\");\n"
            "          if (aFile != NULL) {\n"
            "            fclose(aFile);\n"
            "          }\n"
            "        }\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "}\n"
            "printf(\"%%d\\n\", fileIsNullDevice);\n"
            "return 0;}\n",
            nullDeviceName, eofName);
    /* printf("%s\n", buffer); */
    if (assertCompAndLnk(buffer)) {
      fileIsNullDevice = doTest() == 1;
    } /* if */
    return fileIsNullDevice;
  } /* checkIfNullDevice */



static void determineNullDevice (FILE *versionFile)

  {
    const char *nullDeviceName = NULL;

  /* determineNullDevice */
    if (checkIfNullDevice("/dev/null", "EOF")) {
      nullDeviceName = "/dev/null";
    } else if (checkIfNullDevice("NUL:", "EOF")) {
      nullDeviceName = "NUL:";
    } else if (checkIfNullDevice("NUL", "EOF")) {
      nullDeviceName = "NUL";
    } else if (checkIfNullDevice("NUL:", "0")) {
      nullDeviceName = "NUL:";
    } else if (checkIfNullDevice("NUL", "0")) {
      nullDeviceName = "NUL";
    } /* if */
    if (nullDeviceName == NULL) {
      fputs("\n **** Unable to determine a null device.\n", logFile);
      nullDeviceName = "null_device";
    } /* if */
    fprintf(versionFile, "#define NULL_DEVICE \"%s\"\n", nullDeviceName);
  } /* determineNullDevice */



static void determineCallingConventions (FILE *versionFile)

  {
    int has_cdecl;
    int has_stdcall;

  /* determineCallingConventions */
    has_cdecl = compileAndLinkOk("#include <stdio.h>\n"
                                 "int __cdecl aFunction (void) { return 1; }\n"
                                 "int main (int argc, char *argv[]) {\n"
                                 "printf(\"%d\\n\", aFunction());\n"
                                 "return 0; }\n") &&
                doTest() == 1;
    has_stdcall = compileAndLinkOk("#include <stdio.h>\n"
                                   "int __stdcall aFunction (void) { return 1; }\n"
                                   "int main (int argc, char *argv[]) {\n"
                                   "printf(\"%d\\n\", aFunction());\n"
                                   "return 0; }\n") &&
                  doTest() == 1;
    fprintf(versionFile, "#define HAS_CDECL %d\n", has_cdecl);
    fprintf(versionFile, "#define HAS_STDCALL %d\n", has_stdcall);
  } /* determineCallingConventions */



static void checkSignal (FILE *versionFile)

  {
    int has_signal = 0;
    int has_sigaction = 0;
    int signal_resets_handler = 0;

  /* checkSignal */
    if (compileAndLinkOk("#include <stdio.h>\n#include <signal.h>\n"
                         "volatile int res=4;\n"
                         "void handleSig1(int sig){res = 1;}\n"
                         "void handleSig2(int sig){res = 2;}\n"
                         "int main (int argc, char *argv[]){\n"
                         "if (signal(SIGINT, handleSig2) == SIG_ERR ||\n"
                         "signal(SIGINT, handleSig1) != handleSig2){\n"
                         "puts(\"3\");"
                         "}else if (raise(SIGINT) != 0){puts(\"5\");\n"
                         "}else{printf(\"%d\\n\",res);}\n"
                         "return 0;}\n") && doTest() == 1) {
      has_signal = 1;
    } /* if */
    fprintf(versionFile, "#define HAS_SIGNAL %d\n", has_signal);
    if (compileAndLinkOk("#include <stdio.h>\n#include <signal.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "printf(\"%d\\n\",(int)sizeof(struct sigaction));\n"
                         "return 0;}\n") &&
        compileAndLinkOk("#include <stdio.h>\n#include <signal.h>\n"
                         "volatile int res=4;\n"
                         "void handleSig(int sig){res=1;}\n"
                         "int main(int argc, char *argv[]){\n"
                         "struct sigaction sigAct;\n"
                         "sigAct.sa_handler = handleSig;\n"
                         "sigemptyset(&sigAct.sa_mask);\n"
                         "sigAct.sa_flags = SA_RESTART;\n"
                         "if (sigaction(SIGINT, &sigAct, NULL) == -1){\n"
                         "puts(\"3\");"
                         "}else if (raise(SIGINT) != 0){puts(\"5\");\n"
                         "}else{printf(\"%d\\n\",res);}\n"
                         "return 0;}\n") && doTest() == 1) {
      has_sigaction = 1;
    } /* if */
    fprintf(versionFile, "#define HAS_SIGACTION %d\n", has_sigaction);
    if (!has_sigaction) {
      if (compileAndLinkOk("#include <stdio.h>\n#include <signal.h>\n"
                           "volatile int res=4;\n"
                           "void handleSig1(int sig)\n"
                           "{if (signal(SIGINT, handleSig1) == SIG_DFL) res = 1; else res = 6;}\n"
                           "int main (int argc, char *argv[]){\n"
                           "if (signal(SIGINT, handleSig1) == SIG_ERR){\n"
                           "puts(\"3\");"
                           "}else if (raise(SIGINT) != 0){puts(\"5\");\n"
                           "}else{printf(\"%d\\n\",res);}\n"
                           "return 0;}\n") && doTest() == 1) {
        signal_resets_handler = 1;
      } /* if */
    } /* if */
    fprintf(versionFile, "#define SIGNAL_RESETS_HANDLER %d\n", signal_resets_handler);
  } /* checkSignal */



static void writeMacroDefs (FILE *versionFile)

  {
    char macroDefs[BUFFER_SIZE];

  /* writeMacroDefs */
    if (compileAndLinkOk("static inline int test(int a){return 2*a;}\n"
                         "int main(int argc,char *argv[]){return test(argc);}\n")) {
      /* The C compiler accepts the definition of inline functions. */
    } else if (compileAndLinkOk("static __inline int test(int a){return 2*a;}\n"
                                "int main(int argc,char *argv[]){return test(argc);}\n")) {
      fputs("#define inline __inline\n", versionFile);
    } else if (compileAndLinkOk("static __inline__ int test(int a){return 2*a;}\n"
                                "int main(int argc,char *argv[]){return test(argc);}\n")) {
      fputs("#define inline __inline__\n", versionFile);
    } else {
      fputs("#define inline\n", versionFile);
    } /* if */
    if (!compileAndLinkOk("int test (int *restrict ptrA, int *restrict ptrB, int *restrict ptrC)\n"
                          "{*ptrA += *ptrC; *ptrB += *ptrC; return *ptrA + *ptrB;}\n"
                          "int main(int argc,char *argv[])\n"
                          "{int a=1, b=2, c=3; return test(&a, &b, &c);}\n")) {
      fputs("#define restrict\n", versionFile);
    } /* if */
    macroDefs[0] = '\0';
    if (compileAndLinkOk("#include <stdio.h>\nint main(int argc,char *argv[])\n"
                         "{if(__builtin_expect(1,1))puts(\"1\");else puts(\"0\");\n"
                         "return 0;}\n") && doTest() == 1) {
      fputs("#define likely(x)   __builtin_expect((x),1)\n", versionFile);
      fputs("#define unlikely(x) __builtin_expect((x),0)\n", versionFile);
      strcat(macroDefs, "#define likely(x)   __builtin_expect((x),1)\\n");
      strcat(macroDefs, "#define unlikely(x) __builtin_expect((x),0)\\n");
    } else {
      fputs("#define likely(x) (x)\n", versionFile);
      fputs("#define unlikely(x) (x)\n", versionFile);
      strcat(macroDefs, "#define likely(x) (x)\\n");
      strcat(macroDefs, "#define unlikely(x) (x)\\n");
    } /* if */
    if (compileAndLinkOk("#include <stdlib.h>\n"
                         "__attribute__ ((noreturn)) void fatal (void) {exit(1);}\n"
                         "int main(int argc,char *argv[])\n"
                         "{return 0;}\n")) {
      fputs("#define NORETURN __attribute__ ((noreturn))\n", versionFile);
      strcat(macroDefs, "#define NORETURN __attribute__ ((noreturn))\\n");
    } else if (compileAndLinkOk("#include <stdlib.h>\n"
                                "__declspec(noreturn) void fatal (void) {exit(1);}\n"
                                "int main(int argc,char *argv[])\n"
                                "{return 0;}\n")) {
      fputs("#define NORETURN __declspec(noreturn)\n", versionFile);
      strcat(macroDefs, "#define NORETURN __declspec(noreturn)\\n");
    } else {
      fputs("#define NORETURN\n", versionFile);
      strcat(macroDefs, "#define NORETURN\\n");
    } /* if */
    fprintf(versionFile, "#define MACRO_DEFS \"%s\"\n", macroDefs);
  } /* writeMacroDefs */



static void checkPopen (FILE *versionFile)

  {
    const char *popen = NULL;
    int binary_mode_supported;
    const char *binary_mode = "";
    char buffer[BUFFER_SIZE];
    char fileName[NAME_SIZE];

  /* checkPopen */
    if (compileAndLinkOk("#include <stdio.h>\n"
                         "int main(int argc, char *argv[])\n"
                         "{FILE *aFile; aFile=popen(\""
                         LIST_DIRECTORY_CONTENTS
                         "\", \"r\");\n"
                         "printf(\"%d\\n\", aFile != NULL); return 0;}\n") && doTest() == 1) {
      popen = "popen";
    } else if (compileAndLinkOk("#include <stdio.h>\n"
                                "int main(int argc, char *argv[])\n"
                                "{FILE *aFile; aFile=_popen(\""
                                LIST_DIRECTORY_CONTENTS
                                "\", \"r\");\n"
                                "printf(\"%d\\n\", aFile != NULL); return 0;}\n") && doTest() == 1) {
      popen = "_popen";
    } /* if */
    if (popen != NULL) {
      sprintf(buffer, "#include <stdio.h>\n"
                      "int main(int argc, char *argv[])\n"
                      "{FILE *aFile; aFile=%s(\""
                      LIST_DIRECTORY_CONTENTS
                      "\", \"r\");\n"
                      "printf(\"%%d\\n\", aFile != NULL &&\n"
                      "       fileno(aFile) != -1); return 0;}\n", popen);
      if (compileAndLinkOk(buffer) && doTest() != 1) {
        fprintf(logFile, "\n *** fileno() does not work on a file opened with %s().\n",
                popen);
        popen = NULL;
      } /* if */
    } /* if */
    fprintf(versionFile, "#define HAS_POPEN %d\n", popen != NULL);
    if (popen != NULL) {
      sprintf(buffer, "#include <stdio.h>\n"
                      "int main(int argc, char *argv[])\n"
                      "{FILE *aFile; aFile=%s(\""
                      LIST_DIRECTORY_CONTENTS
                      "\", \"rb\");\n"
                      "printf(\"%%d\\n\", aFile != NULL); return 0;}\n", popen);
      if (assertCompAndLnk(buffer)) {
        binary_mode_supported = doTest() == 1;
        fprintf(versionFile, "#define POPEN_SUPPORTS_BINARY_MODE %d\n", binary_mode_supported);
        if (binary_mode_supported) {
          binary_mode = "b";
        } /* if */
      } /* if */
      sprintf(buffer, "#include <stdio.h>\n"
                      "int main(int argc, char *argv[])\n"
                      "{FILE *aFile; aFile=%s(\""
                      LIST_DIRECTORY_CONTENTS
                      "\", \"rt\");\n"
                      "printf(\"%%d\\n\", aFile != NULL); return 0;}\n", popen);
      if (assertCompAndLnk(buffer)) {
        fprintf(versionFile, "#define POPEN_SUPPORTS_TEXT_MODE %d\n", doTest() == 1);
      } /* if */
      sprintf(buffer, "#include <stdio.h>\n#include <windows.h>\n"
                      "int main(int argc, char *argv[])\n"
                      "{FILE *aFile;\n"
                      "SetErrorMode(SEM_NOGPFAULTERRORBOX);\n"
                      "aFile=%s(\""
                      LIST_DIRECTORY_CONTENTS
                      "\", \"re\");\n"
                      "printf(\"%%d\\n\", aFile != NULL); return 0;}\n", popen);
      if (compileAndLinkOk(buffer)) {
        fprintf(versionFile, "#define POPEN_SUPPORTS_CLOEXEC_MODE %d\n", doTestNoResultCheck() == 1);
      } else {
        sprintf(buffer, "#include <stdio.h>\n"
                        "int main(int argc, char *argv[])\n"
                        "{FILE *aFile; aFile=%s(\""
                        LIST_DIRECTORY_CONTENTS
                        "\", \"re\");\n"
                        "printf(\"%%d\\n\", aFile != NULL); return 0;}\n", popen);
        if (assertCompAndLnk(buffer)) {
          fprintf(versionFile, "#define POPEN_SUPPORTS_CLOEXEC_MODE %d\n", doTest() == 1);
        } /* if */
      } /* if */
      sprintf(buffer, "#include <stdio.h>\nint main(int argc, char *argv[])\n"
                      "{FILE *aFile; aFile=%s(\""
                      LIST_DIRECTORY_CONTENTS
                      "\", \"r\");\n"
                      "printf(\"%%d\\n\", ftell(aFile) != -1); return 0;}\n", popen);
      if (assertCompAndLnk(buffer)) {
        fprintf(versionFile, "#define FTELL_SUCCEEDS_FOR_PIPE %d\n", doTest() == 1);
      } /* if */
      if (assertCompAndLnk("#include <stdio.h>\n"
                           "int main(int argc, char *argv[])\n"
                           "{printf(\"x\\n\"); return 0;}\n")) {
        sprintf(fileName, "ctest%d%s", testNumber, LINKED_PROGRAM_EXTENSION);
        if (rename(fileName, "ctest_a" LINKED_PROGRAM_EXTENSION) == 0) {
          sprintf(buffer, "#include <stdio.h>\n#include <string.h>\n"
                          "int main(int argc, char *argv[])\n"
                          "{char buffer[5]; FILE *aFile; aFile=%s(\""
                          ".%s%cctest_a%s"
                          "\", \"r%s\");\n"
                          "if (aFile != NULL && fgets(buffer, 4, aFile) != NULL)\n"
                          "printf(\"%%d\\n\", memcmp(buffer, \"x\\r\\n\", 3) == 0);\n"
                          "else printf(\"0\\n\"); return 0;}\n",
                          popen, PATH_DELIMITER == '\\' ? "\\" : "", PATH_DELIMITER,
                          LINKED_PROGRAM_EXTENSION, binary_mode);
          if (assertCompAndLnk(buffer)) {
            fprintf(versionFile, "#define STDOUT_IS_IN_TEXT_MODE %d\n", doTest() == 1);
          } /* if */
          doRemove("ctest_a" LINKED_PROGRAM_EXTENSION);
        } else {
          fprintf(logFile, "\n *** Unable to rename %s to ctest_a%s\n",
                  fileName, LINKED_PROGRAM_EXTENSION);
        } /* if */
      } /* if */
    } /* if */
  } /* checkPopen */



static void checkSystemResult (FILE *versionFile)

  {
    int testResult;
    char buffer[BUFFER_SIZE];

  /* checkSystemResult */
    cleanUpCompilation("ctest_b", 0);
    /* Some anti virus software considers an empty program dangerous.  */
    /* To avoid these false positives the test below writes something. */
    if (!doCompileAndLinkContent("ctest_b",
                                 "#include <stdio.h>\n"
                                 "int main (int argc, char *argv[])\n"
                                 "{ printf(\"Test program returning 0.\\n\");\n"
                                 "return 0; }\n",
                                 "", "", 0)) {
      fprintf(logFile, "\n **** Compiling ctest_b failed.\n");
    } else {
#ifdef INTERPRETER_FOR_LINKED_PROGRAM
      sprintf(buffer, "#include <stdio.h>\n#include <stdlib.h>\n"
                      "int main(int argc, char *argv[])\n"
                      "{char buffer[5]; int retVal; retVal=system(\""
                      "%s .%s%cctest_b%s>ctest_b.out"
                      "\");\n"
                      "printf(\"%%d\\n\", retVal); return 0;}\n",
                      INTERPRETER_FOR_LINKED_PROGRAM,
                      PATH_DELIMITER == '\\' ? "\\" : "", PATH_DELIMITER,
                      LINKED_PROGRAM_EXTENSION);
#else
      sprintf(buffer, "#include <stdio.h>\n#include <stdlib.h>\n"
                      "int main(int argc, char *argv[])\n"
                      "{char buffer[5]; int retVal; retVal=system(\""
                      ".%s%cctest_b%s>ctest_b.out"
                      "\");\n"
                      "printf(\"%%d\\n\", retVal); return 0;}\n",
                      PATH_DELIMITER == '\\' ? "\\" : "", PATH_DELIMITER,
                      LINKED_PROGRAM_EXTENSION);
#endif
      if (assertCompAndLnk(buffer)) {
        testResult = doTest();
        fprintf(versionFile, "#define SYSTEM_RESULT_FOR_RETURN_0 %d\n", testResult);
        if (testResult != 0) {
          fprintf(logFile, "\n *** System result for return 0 is %d\n", testResult);
        } /* if */
      } /* if */
      cleanUpCompilation("ctest_b", 0);
    } /* if */
  } /* checkSystemResult */



static int getSizeof (const char *typeName)

  {
    char buffer[BUFFER_SIZE];
    int computedSize = -1;

  /* getSizeof */
    /* fprintf(logFile, "getSizeof(%s)\n", typeName); */
    sprintf(buffer, "#include <stdio.h>\n"
                    "#include <stddef.h>\n"
                    "#include <time.h>\n"
                    "#include <sys/types.h>\n"
                    "#include \"tst_vers.h\"\n"
                    "int main(int argc, char *argv[])\n"
                    "{printf(\"%%d\\n\",(int)sizeof(%s));return 0;}\n",
                    typeName);
    /* printf("getSizeof(%s):\n%s\n", typeName, buffer); */
    if (compileAndLinkOk(buffer)) {
      computedSize = doTest();
      if (computedSize == -1) {
        fprintf(logFile, "\n *** Unable to determine sizeof(%s).\n", typeName);
      } /* if */
    } /* if */
    return computedSize;
  } /* getSizeof */



static int isSignedType (const char *typeName)

  {
    char buffer[BUFFER_SIZE];
    int isSigned = -1;

  /* isSignedType */
    /* fprintf(logFile, "isSignedType(%s)\n", typeName); */
    sprintf(buffer, "#include <stdio.h>\n"
                    "#include <stddef.h>\n"
                    "#include <time.h>\n"
                    "#include <sys/types.h>\n"
                    "#include \"tst_vers.h\"\n"
                    "int main(int argc, char *argv[])"
                    "{printf(\"%%d\\n\",(%s)-1<0);return 0;}\n",
                    typeName);
    if (compileAndLinkOk(buffer)) {
      isSigned = doTest();
    } /* if */
    if (isSigned == -1) {
      fprintf(logFile, "\n *** Unable to determine if %s is signed.\n", typeName);
    } /* if */
    return isSigned == 1;
  } /* isSignedType */



static void numericSizes (FILE *versionFile)

  {
    int char_bit;
    int sizeof_char;
    int sizeof_short;
    int sizeof_int64;
    int sizeof_gid_t;
    int sizeof_uid_t;

  /* numericSizes */
    fprintf(logFile, "Numeric sizes: ");
    fflush(stdout);
    if (compileAndLinkOk("#include <stdio.h>\n#include <limits.h>\n"
                         "int main(int argc, char *argv[])"
                         "{printf(\"%d\\n\",CHAR_BIT);return 0;}\n")) {
      char_bit = doTest();
    } else {
      fputs("#define CHAR_BIT 8\n", versionFile);
      char_bit = 8;
    } /* if */
    sizeof_char      = getSizeof("char");
    sizeof_short     = getSizeof("short");
    sizeof_int       = getSizeof("int");
    sizeof_long      = getSizeof("long");
    sizeof_long_long = getSizeof("long long");
    sizeof_int64     = getSizeof("__int64");
    sizeof_pointer   = getSizeof("char *");
    sizeof_float     = getSizeof("float");
    sizeof_double    = getSizeof("double");
    sizeof_gid_t     = getSizeof("gid_t");
    sizeof_uid_t     = getSizeof("uid_t");

    fprintf(versionFile, "#define CHAR_SIZE %d\n",        char_bit * sizeof_char);
    fprintf(versionFile, "#define SHORT_SIZE %d\n",       char_bit * sizeof_short);
    fprintf(versionFile, "#define INT_SIZE %d\n",         char_bit * sizeof_int);
    fprintf(versionFile, "#define LONG_SIZE %d\n",        char_bit * sizeof_long);
    if (sizeof_long_long != -1) {
      fprintf(versionFile, "#define LONG_LONG_SIZE %d\n", char_bit * sizeof_long_long);
    } /* if */
    if (sizeof_int64 != -1) {
      fprintf(versionFile, "#define INT64_SIZE %d\n",     char_bit * sizeof_int64);
    } /* if */
    fprintf(versionFile, "#define POINTER_SIZE %d\n",     char_bit * sizeof_pointer);
    fprintf(versionFile, "#define FLOAT_SIZE %d\n",       char_bit * sizeof_float);
    fprintf(versionFile, "#define DOUBLE_SIZE %d\n",      char_bit * sizeof_double);
    fprintf(versionFile, "#define WCHAR_T_SIZE %d\n",     char_bit * getSizeof("wchar_t"));
    fprintf(versionFile, "#define TIME_T_SIZE %d\n",      char_bit * getSizeof("time_t"));
    fprintf(versionFile, "#define TIME_T_SIGNED %d\n", isSignedType("time_t"));
    if (sizeof_gid_t > 0) {
      fprintf(versionFile, "#define GID_T_SIZE %d\n",     char_bit * sizeof_gid_t);
      fprintf(versionFile, "#define GID_T_SIGNED %d\n", isSignedType("gid_t"));
    } /* if */
    if (sizeof_uid_t > 0) {
      fprintf(versionFile, "#define UID_T_SIZE %d\n",     char_bit * sizeof_uid_t);
      fprintf(versionFile, "#define UID_T_SIGNED %d\n", isSignedType("uid_t"));
    } /* if */
    fprintf(versionFile, "#define SIZE_T_SIGNED %d\n", isSignedType("size_t"));
    fprintf(versionFile, "#define CHAR_SIGNED %d\n",   isSignedType("char"));
    /* The expression to check for BOOLTYPE below has been chosen, */
    /* because lcc-win32 fails with boolean expressions of this kind. */
    if (compileAndLinkOk("#include <stdio.h>\nint main(int argc, char *argv[])"
                         "{int x = 0;\n"
                         "_Bool flag = argc != -1 ? x++, (_Bool) 1:(_Bool) 0;\n"
                         "return 0;}\n")) {
      fputs("#define BOOLTYPE _Bool\n", versionFile);
      fputs("#define BOOLTYPE_STRI \"_Bool\"\n", versionFile);
    } else if (compileAndLinkOk("#include <stdio.h>\nint main(int argc, char *argv[])"
                         "{int x = 0;\n"
                         "bool flag = argc != -1 ? x++, (bool) 1:(bool) 0;\n"
                         "return 0;}\n")) {
      fputs("#define BOOLTYPE bool\n", versionFile);
      fputs("#define BOOLTYPE_STRI \"bool\"\n", versionFile);
    } else {
      fputs("#define BOOLTYPE int\n", versionFile);
      fputs("#define BOOLTYPE_STRI \"int\"\n", versionFile);
    } /* if */
    if (sizeof_char == 1) {
      fputs("#define INT8TYPE signed char\n", versionFile);
      fputs("#define INT8TYPE_STRI \"signed char\"\n", versionFile);
      fputs("#define UINT8TYPE unsigned char\n", versionFile);
      fputs("#define UINT8TYPE_STRI \"unsigned char\"\n", versionFile);
    } /* if */
    if (sizeof_short == 2) {
      int16TypeStri = "short int";
      uint16TypeStri = "unsigned short int";
    } else if (sizeof_int == 2) {
      int16TypeStri = "int";
      uint16TypeStri = "unsigned int";
    } /* if */
    if (int16TypeStri != NULL) {
      fprintf(versionFile, "#define INT16TYPE %s\n", int16TypeStri);
      fprintf(versionFile, "#define INT16TYPE_STRI \"%s\"\n", int16TypeStri);
      fprintf(versionFile, "#define UINT16TYPE %s\n", uint16TypeStri);
      fprintf(versionFile, "#define UINT16TYPE_STRI \"%s\"\n", uint16TypeStri);
    } /* if */
    if (sizeof_int == 4) {
      int32TypeStri = "int";
      uint32TypeStri = "unsigned int";
      int32TypeSuffix = "";
      int32TypeFormat = "";
    } else if (sizeof_long == 4) {
      int32TypeStri = "long";
      uint32TypeStri = "unsigned long";
      int32TypeSuffix = "L";
      int32TypeFormat = "l";
    } /* if */
    if (int32TypeStri != NULL) {
      fprintf(versionFile, "#define INT32TYPE %s\n", int32TypeStri);
      fprintf(versionFile, "#define INT32TYPE_STRI \"%s\"\n", int32TypeStri);
      fprintf(versionFile, "#define UINT32TYPE %s\n", uint32TypeStri);
      fprintf(versionFile, "#define UINT32TYPE_STRI \"%s\"\n", uint32TypeStri);
      if (int32TypeSuffix[0] == '\0') {
        fprintf(versionFile, "#define INT32_SUFFIX(num) num\n");
      } else {
        fprintf(versionFile, "#define INT32_SUFFIX(num) num ## %s\n", int32TypeSuffix);
      } /* if */
      fprintf(versionFile, "#define UINT32_SUFFIX(num) num ## U%s\n", int32TypeSuffix);
      fprintf(versionFile, "#define INT32TYPE_LITERAL_SUFFIX \"%s\"\n", int32TypeSuffix);
      fprintf(versionFile, "#define INT32TYPE_FORMAT_LENGTH_MODIFIER \"%s\"\n", int32TypeFormat);
    } /* if */
    if (sizeof_long == 8) {
      int64TypeStri = "long";
      uint64TypeStri = "unsigned long";
      if (expectTestResult("#include <stdio.h>\nint main(int argc, char *argv[])"
                           "{long n=12345678L;printf(\"%d\\n\",(int)sizeof(1L));return 0;}\n", 8)) {
        int64TypeSuffix = "L";
      } /* if */
      int64TypeFormat = "l";
    } else if (sizeof_long_long == 8) {
      /* The type long long is defined and it is a 64-bit type */
      int64TypeStri = "long long";
      uint64TypeStri = "unsigned long long";
      if (compileAndLinkOk("#include <stdio.h>\nint main(int argc, char *argv[])"
                           "{long long n=12345678LL;printf(\"%d\\n\",sizeof(1LL));\n"
                           "return 0;}\n") && doTest() == 8) {
        int64TypeSuffix = "LL";
      } /* if */
      if (!FORMAT_LL_TRIGGERS_WARNINGS &&
          compileAndLinkOk("#include <stdio.h>\n#include <string.h>\n"
                           "int main(int argc, char *argv[])\n"
                           "{char b[99]; sprintf(b, \"A%lldB\", (long long) 1 << 32);\n"
                           "printf(\"%d\\n\", strcmp(b,\"A4294967296B\")==0);\n"
                           "return 0;}\n") && doTest() == 1) {
        int64TypeFormat = "ll";
      } else if (compileAndLinkOk("#include <stdio.h>\n#include <string.h>\n"
                                  "int main(int argc, char *argv[])\n"
                                  "{char b[99]; sprintf(b, \"A%LdB\", (long long) 1 << 32);\n"
                                  "printf(\"%d\\n\", strcmp(b,\"A4294967296B\")==0);\n"
                                  "return 0;}\n") && doTest() == 1) {
        int64TypeFormat = "L";
      } else if (compileAndLinkOk("#include <stdio.h>\n#include <string.h>\n"
                                  "int main(int argc, char *argv[])\n"
                                  "{char b[99]; sprintf(b, \"A%I64dB\", (long long) 1 << 32);\n"
                                  "printf(\"%d\\n\", strcmp(b,\"A4294967296B\")==0);\n"
                                  "return 0;}\n") && doTest() == 1) {
        int64TypeFormat = "I64";
      } /* if */
    } else if (sizeof_int64 == 8) {
      /* The type __int64 is defined and it is a 64-bit type */
      int64TypeStri = "__int64";
      uint64TypeStri = "unsigned __int64";
      if (compileAndLinkOk("#include <stdio.h>\nint main(int argc, char *argv[])"
                           "{__int64 n=12345678LL;printf(\"%d\\n\",sizeof(1LL));\n"
                           "return 0;}\n") && doTest() == 8) {
        int64TypeSuffix = "LL";
      } /* if */
      if (compileAndLinkOk("#include <stdio.h>\n#include <string.h>\n"
                           "int main(int argc, char *argv[])\n"
                           "{char b[99]; sprintf(b, \"A%lldB\", (__int64) 1 << 32);\n"
                           "printf(\"%d\\n\", strcmp(b,\"A4294967296B\")==0);\n"
                           "return 0;}\n") && doTest() == 1) {
        int64TypeFormat = "ll";
      } else if (compileAndLinkOk("#include <stdio.h>\n#include <string.h>\n"
                                  "int main(int argc, char *argv[])\n"
                                  "{char b[99]; sprintf(b, \"A%LdB\", (__int64) 1 << 32);\n"
                                  "printf(\"%d\\n\", strcmp(b,\"A4294967296B\")==0);\n"
                                  "return 0;}\n") && doTest() == 1) {
        int64TypeFormat = "L";
      } else if (compileAndLinkOk("#include <stdio.h>\n#include <string.h>\n"
                                  "int main(int argc, char *argv[])\n"
                                  "{char b[99]; sprintf(b, \"A%I64dB\", (__int64) 1 << 32);\n"
                                  "printf(\"%d\\n\", strcmp(b,\"A4294967296B\")==0);\n"
                                  "return 0;}\n") && doTest() == 1) {
        int64TypeFormat = "I64";
      } /* if */
    } /* if */
    if (int64TypeStri != NULL) {
      fprintf(versionFile, "#define INT64TYPE %s\n", int64TypeStri);
      fprintf(versionFile, "#define INT64TYPE_STRI \"%s\"\n", int64TypeStri);
      fprintf(versionFile, "#define UINT64TYPE %s\n", uint64TypeStri);
      fprintf(versionFile, "#define UINT64TYPE_STRI \"%s\"\n", uint64TypeStri);
#ifdef INT64TYPE_NO_SUFFIX_BUT_CAST
      fprintf(versionFile, "#define INT64_SUFFIX(num)  ((int64Type) num)\n");
      fprintf(versionFile, "#define UINT64_SUFFIX(num) ((uint64Type) num ## U)\n");
      fprintf(versionFile, "#define INT64TYPE_LITERAL_SUFFIX \"\"\n");
#else
      if (int64TypeSuffix[0] == '\0') {
        fprintf(versionFile, "#define INT64_SUFFIX(num) ((%s) num)\n", int64TypeStri);
      } else {
        fprintf(versionFile, "#define INT64_SUFFIX(num) num ## %s\n", int64TypeSuffix);
      } /* if */
      fprintf(versionFile, "#define UINT64_SUFFIX(num) num ## U%s\n", int64TypeSuffix);
      fprintf(versionFile, "#define INT64TYPE_LITERAL_SUFFIX \"%s\"\n", int64TypeSuffix);
#endif
      fprintf(versionFile, "#define INT64TYPE_FORMAT_LENGTH_MODIFIER \"%s\"\n", int64TypeFormat);
    } /* if */
    if (compileAndLinkOk("#include <stdio.h>\n#include <time.h>\n"
                         "int main(int argc, char *argv[])\n"
                         "{__int128 a = (__int128) time(NULL) * (__int128) clock();\n"
                         "if (sizeof(__int128)==sizeof(unsigned __int128))\n"
                         "printf(\"%d\\n\",sizeof(__int128));\n"
                         "else printf(\"0\\n\");return 0;}\n") && doTest() == 16) {
      /* The type __int128 is defined and it is a 128-bit type */
      int128TypeStri = "__int128";
      uint128TypeStri = "unsigned __int128";
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <time.h>\n"
                                "int main(int argc, char *argv[])\n"
                                "{__int128 a = (__int128) time(NULL) * (__int128) clock();\n"
                                "if (sizeof(__int128_t)==sizeof(__uint128_t))\n"
                                "printf(\"%d\\n\",sizeof(__int128_t));\n"
                                "else printf(\"0\\n\");return 0;}\n") && doTest() == 16) {
      /* The type __int128_t is defined and it is a 128-bit type */
      int128TypeStri = "__int128_t";
      uint128TypeStri = "__uint128_t";
    } /* if */
    if (int128TypeStri != NULL && uint128TypeStri != NULL) {
      fprintf(versionFile, "#define INT128TYPE %s\n", int128TypeStri);
      fprintf(versionFile, "#define INT128TYPE_STRI \"%s\"\n", int128TypeStri);
      fprintf(versionFile, "#define UINT128TYPE %s\n", uint128TypeStri);
      fprintf(versionFile, "#define UINT128TYPE_STRI \"%s\"\n", uint128TypeStri);
    } /* if */
    fprintf(versionFile, "#define HAS_LLABS %d\n",
            sizeof_long_long != -1 &&
            compileAndLinkOk("#include <stdio.h>\n#include <stdlib.h>\n"
                             "int main(int argc, char *argv[])\n"
                             "{long long a = (long long) -123;\n"
                             "printf(\"%d\\n\", llabs(a) == (long long) 123);\n"
                             "return 0;}\n") &&
            doTest() == 1);
    fprintf(versionFile, "#define HAS_ABS64 %d\n",
            sizeof_int64 != -1 &&
            compileAndLinkOk("#include <stdio.h>\n#include <stdlib.h>\n"
                             "int main(int argc, char *argv[])\n"
                             "{__int64 a = (__int64) -123;\n"
                             "printf(\"%d\\n\", _abs64(a) == (__int64) 123);\n"
                             "return 0;}\n") &&
            doTest() == 1);
    fprintf(versionFile, "#define INTPTR_T_DEFINED %d\n",
            compileAndLinkOk("#include <stdint.h>\nint main(int argc, char *argv[])"
                             "{intptr_t intptr = (intptr_t) &argc;return 0;}\n"));
    fprintf(logFile, " determined\n");
  } /* numericSizes */



static int checkIntDivisionOneByZero (const char *compilerOptions,
    const char *linkerOptions)

  {
    int check_int_div_one_by_zero;

  /* checkIntDivisionOneByZero */
    check_int_div_one_by_zero =
        !compileAndLinkWithOptionsOk("#include<stdio.h>\n"
                                     "int main(int argc,char *argv[]){"
                                     "printf(\"%d\\n\", 1/0);return 0;}\n",
                                     compilerOptions, linkerOptions) ||
        !compileAndLinkWithOptionsOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                                     "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                                     "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                                     "int main(int argc,char *argv[]){\n"
                                     "signal(SIGFPE,handleSigfpe);\n"
                                     "signal(SIGILL,handleSigill);\n"
                                     "printf(\"%d\\n\",1/0==0);return 0;}\n",
                                     compilerOptions, linkerOptions) || doTest() != 2 ||
        !compileAndLinkWithOptionsOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                                     "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                                     "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                                     "int main(int argc,char *argv[]){\n"
                                     "int zero=0;\n"
                                     "signal(SIGFPE,handleSigfpe);\n"
                                     "signal(SIGILL,handleSigill);\n"
                                     "printf(\"%d\\n\",1/zero==0);return 0;}\n",
                                     compilerOptions, linkerOptions) || doTest() != 2 ||
        !compileAndLinkWithOptionsOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                                     "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                                     "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                                     "int main(int argc,char *argv[]){\n"
                                     "int one=1;\n"
                                     "signal(SIGFPE,handleSigfpe);\n"
                                     "signal(SIGILL,handleSigill);\n"
                                     "printf(\"%d\\n\",one/0==0);return 0;}\n",
                                     compilerOptions, linkerOptions) || doTest() != 2 ||
        !compileAndLinkWithOptionsOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                                     "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                                     "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                                     "int main(int argc,char *argv[]){\n"
                                     "int one=1;\n"
                                     "int zero=0;\n"
                                     "signal(SIGFPE,handleSigfpe);\n"
                                     "signal(SIGILL,handleSigill);\n"
                                     "printf(\"%d\\n\",one/zero==0);return 0;}\n",
                                     compilerOptions, linkerOptions) || doTest() != 2;
    /* fprintf(logFile, "checkIntDivisionOneByZero --> %d\n", check_int_div_one_by_zero); */
    return check_int_div_one_by_zero;
  } /* checkIntDivisionOneByZero */



static int checkIntDivisionZeroByZero (const char *compilerOptions,
    const char *linkerOptions)

  {
    int check_int_div_zero_by_zero;

  /* checkIntDivisionZeroByZero */
    check_int_div_zero_by_zero =
        !compileAndLinkWithOptionsOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                                     "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                                     "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                                     "int main(int argc,char *argv[]){\n"
                                     "int zero=0;\n"
                                     "signal(SIGFPE,handleSigfpe);\n"
                                     "signal(SIGILL,handleSigill);\n"
                                     "printf(\"%d\\n\",zero/0==0);return 0;}\n",
                                     compilerOptions, linkerOptions) || doTest() != 2 ||
        !compileAndLinkWithOptionsOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                                     "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                                     "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                                     "int main(int argc,char *argv[]){\n"
                                     "int zero1=0;\n"
                                     "int zero2=0;\n"
                                     "signal(SIGFPE,handleSigfpe);\n"
                                     "signal(SIGILL,handleSigill);\n"
                                     "printf(\"%d\\n\",zero1/zero2==0);return 0;}\n",
                                     compilerOptions, linkerOptions) || doTest() != 2;
    /* fprintf(logFile, "checkIntDivisionZeroByZero --> %d\n", check_int_div_zero_by_zero); */
    return check_int_div_zero_by_zero;
  } /* checkIntDivisionZeroByZero */



static int checkIntRemainderByZero (const char *compilerOptions,
    const char *linkerOptions)

  {
    int check_int_rem_by_zero;

  /* checkIntRemainderByZero */
    check_int_rem_by_zero =
        !compileAndLinkWithOptionsOk("#include<stdio.h>\n"
                                     "int main(int argc,char *argv[]){"
                                     "printf(\"%d\\n\", 1%0);return 0;}\n",
                                     compilerOptions, linkerOptions) ||
        !compileAndLinkWithOptionsOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                                     "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                                     "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                                     "int main(int argc,char *argv[]){\n"
                                     "signal(SIGFPE,handleSigfpe);\n"
                                     "signal(SIGILL,handleSigill);\n"
                                     "printf(\"%d\\n\",1%0==0);return 0;}\n",
                                     compilerOptions, linkerOptions) || doTest() != 2 ||
        !compileAndLinkWithOptionsOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                                     "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                                     "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                                     "int main(int argc,char *argv[]){\n"
                                     "int zero=0;\n"
                                     "signal(SIGFPE,handleSigfpe);\n"
                                     "signal(SIGILL,handleSigill);\n"
                                     "printf(\"%d\\n\",1%zero==0);return 0;}\n",
                                     compilerOptions, linkerOptions) || doTest() != 2 ||
        !compileAndLinkWithOptionsOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                                     "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                                     "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                                     "int main(int argc,char *argv[]){\n"
                                     "int one=1;\n"
                                     "signal(SIGFPE,handleSigfpe);\n"
                                     "signal(SIGILL,handleSigill);\n"
                                     "printf(\"%d\\n\",one%0==0);return 0;}\n",
                                     compilerOptions, linkerOptions) || doTest() != 2 ||
        !compileAndLinkWithOptionsOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                                     "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                                     "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                                     "int main(int argc,char *argv[]){\n"
                                     "int one=1;\n"
                                     "int zero=0;\n"
                                     "signal(SIGFPE,handleSigfpe);\n"
                                     "signal(SIGILL,handleSigill);\n"
                                     "printf(\"%d\\n\",one%zero==0);return 0;}\n",
                                     compilerOptions, linkerOptions) || doTest() != 2 ||
        !compileAndLinkWithOptionsOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                                     "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                                     "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                                     "int main(int argc,char *argv[]){\n"
                                     "int zero=0;\n"
                                     "signal(SIGFPE,handleSigfpe);\n"
                                     "signal(SIGILL,handleSigill);\n"
                                     "printf(\"%d\\n\",zero%0==0);return 0;}\n",
                                     compilerOptions, linkerOptions) || doTest() != 2 ||
        !compileAndLinkWithOptionsOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                                     "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                                     "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                                     "int main(int argc,char *argv[]){\n"
                                     "int zero1=0;\n"
                                     "int zero2=0;\n"
                                     "signal(SIGFPE,handleSigfpe);\n"
                                     "signal(SIGILL,handleSigill);\n"
                                     "printf(\"%d\\n\",zero1%zero2==0);return 0;}\n",
                                     compilerOptions, linkerOptions) || doTest() != 2;
    /* fprintf(logFile, "checkIntRemainderByZero --> %d\n", check_int_rem_by_zero); */
    return check_int_rem_by_zero;
  } /* checkIntRemainderByZero */



static void checkIntDivisions (FILE *versionFile)

  {
    int check_int_div_by_zero;
    int check_int_div_zero_by_zero;
    int check_int_rem_by_zero;
    int check_int_rem_zero_by_zero;

  /* checkIntDivisions */
#ifdef INT_DIV_BY_ZERO_POPUP
    check_int_div_by_zero = 1;
    check_int_div_zero_by_zero = 1;
    check_int_rem_by_zero = 1;
    check_int_rem_zero_by_zero = 1;
#ifndef DO_SIGFPE_WITH_DIV_BY_ZERO
    /* If 1/0 triggers SIGFPE this expression can be used to trigger SIGFPE. */
    fprintf(versionFile, "#define DO_SIGFPE_WITH_DIV_BY_ZERO %d\n", !check_int_div_by_zero);
#endif
#else
    check_int_div_by_zero = checkIntDivisionOneByZero("", "");
    if (!check_int_div_by_zero) {
      check_int_div_by_zero = checkIntDivisionOneByZero(CC_OPT_OPTIMIZE_1, "");
      if (!check_int_div_by_zero) {
        check_int_div_by_zero = checkIntDivisionOneByZero(CC_OPT_OPTIMIZE_2, "");
        if (!check_int_div_by_zero) {
          check_int_div_by_zero = checkIntDivisionOneByZero(CC_OPT_OPTIMIZE_3, "");
        } /* if */
      } /* if */
    } /* if */
#ifndef DO_SIGFPE_WITH_DIV_BY_ZERO
    /* If 1/0 triggers SIGFPE this expression can be used to trigger SIGFPE. */
    fprintf(versionFile, "#define DO_SIGFPE_WITH_DIV_BY_ZERO %d\n", !check_int_div_by_zero);
#endif
    if (!check_int_div_by_zero) {
      check_int_div_by_zero = checkIntDivisionZeroByZero("", "");
      if (!check_int_div_by_zero) {
        check_int_div_by_zero = checkIntDivisionZeroByZero(CC_OPT_OPTIMIZE_1, "");
        if (!check_int_div_by_zero) {
          check_int_div_by_zero = checkIntDivisionZeroByZero(CC_OPT_OPTIMIZE_2, "");
          if (!check_int_div_by_zero) {
            check_int_div_by_zero = checkIntDivisionZeroByZero(CC_OPT_OPTIMIZE_3, "");
          } /* if */
        } /* if */
      } /* if */
    } /* if */

    /* Determine if the C compiler does some "optimizations" with 0/0, */
    /* because it is "undefined behavior" that should not happen.      */
    check_int_div_zero_by_zero =
        !compileAndLinkOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                          "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                          "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                          "int main(int argc,char *argv[]){\n"
                          "signal(SIGFPE,handleSigfpe);\n"
                          "signal(SIGILL,handleSigill);\n"
                          "printf(\"%d\\n\",0/0==0);return 0;}\n") || doTest() != 2 ||
        !compileAndLinkOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                          "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                          "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                          "int main(int argc,char *argv[]){\n"
                          "int zero=0;\n"
                          "signal(SIGFPE,handleSigfpe);\n"
                          "signal(SIGILL,handleSigill);\n"
                          "printf(\"%d\\n\",0/zero==0);return 0;}\n") || doTest() != 2;

    check_int_rem_by_zero = checkIntRemainderByZero("", "");
    if (!check_int_rem_by_zero) {
      check_int_rem_by_zero = checkIntRemainderByZero(CC_OPT_OPTIMIZE_1, "");
      if (!check_int_rem_by_zero) {
        check_int_rem_by_zero = checkIntRemainderByZero(CC_OPT_OPTIMIZE_2, "");
        if (!check_int_rem_by_zero) {
          check_int_rem_by_zero = checkIntRemainderByZero(CC_OPT_OPTIMIZE_3, "");
        } /* if */
      } /* if */
    } /* if */

    /* Determine if the C compiler does some "optimizations" with 0%0, */
    /* because it is "undefined behavior" that should not happen.      */
    check_int_rem_zero_by_zero =
        !compileAndLinkOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                          "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                          "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                          "int main(int argc,char *argv[]){\n"
                          "signal(SIGFPE,handleSigfpe);\n"
                          "signal(SIGILL,handleSigill);\n"
                          "printf(\"%d\\n\",0%0==0);return 0;}\n") || doTest() != 2 ||
        !compileAndLinkOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                          "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                          "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                          "int main(int argc,char *argv[]){\n"
                          "int zero=0;\n"
                          "signal(SIGFPE,handleSigfpe);\n"
                          "signal(SIGILL,handleSigill);\n"
                          "printf(\"%d\\n\",0%zero==0);return 0;}\n") || doTest() != 2;
#endif
    fprintf(versionFile, "#define CHECK_INT_DIV_BY_ZERO %d\n", check_int_div_by_zero);
    fprintf(versionFile, "#define CHECK_INT_DIV_ZERO_BY_ZERO %d\n", check_int_div_zero_by_zero);
    fprintf(versionFile, "#define CHECK_INT_REM_BY_ZERO %d\n", check_int_rem_by_zero);
    fprintf(versionFile, "#define CHECK_INT_REM_ZERO_BY_ZERO %d\n", check_int_rem_zero_by_zero);
  } /* checkIntDivisions */



static void checkOverflowSignal (FILE *versionFile)

  {
    char buffer[BUFFER_SIZE];

  /* checkOverflowSignal */
    sprintf(buffer, "#include<stdlib.h>\n#include<stdio.h>\n#include<limits.h>\n"
                    "#include<signal.h>\n"
                    "void handleSigill(int sig){puts(\"2\");exit(0);}\n"
                    "void handleSigabrt(int sig){puts(\"3\");exit(0);}\n"
                    "void handleSigtrap(int sig){puts(\"4\");exit(0);}\n"
                    "int main(int argc,char *argv[]){\n"
                    "%s a=0x7fffffffffffffff,b=1,c=2;\n"
                    "signal(SIGILL,handleSigill);\n"
                    "signal(SIGABRT,handleSigabrt);\n"
                    "#ifdef SIGTRAP\n"
                    "signal(SIGTRAP,handleSigtrap);\n"
                    "#endif\n"
                    "printf(\"%%d\\n\",a+b==0x8000000000000000 && a*c== -2);return 0;}\n",
                    int64TypeStri);
    if (compileAndLinkWithOptionsOk(buffer, CC_OPT_TRAP_OVERFLOW, "")) {
      switch (doTest()) {
        case 2:
          fputs("#define OVERFLOW_SIGNAL SIGILL\n", versionFile);
          fputs("#define OVERFLOW_SIGNAL_STR \"SIGILL\"\n", versionFile);
          break;
        case 3:
          fputs("#define OVERFLOW_SIGNAL SIGABRT\n", versionFile);
          fputs("#define OVERFLOW_SIGNAL_STR \"SIGABRT\"\n", versionFile);
          break;
        case 4:
          fputs("#define OVERFLOW_SIGNAL SIGTRAP\n", versionFile);
          fputs("#define OVERFLOW_SIGNAL_STR \"SIGTRAP\"\n", versionFile);
          break;
        default:
          fputs("#define OVERFLOW_SIGNAL 0\n", versionFile);
          fputs("#define OVERFLOW_SIGNAL_STR \"\"\n", versionFile);
          break;
      } /* switch */
    } else {
      fputs("#define INT_MULT64_COMPILE_ERROR\n", versionFile);
      fputs("#define OVERFLOW_SIGNAL 0\n", versionFile);
      fputs("#define OVERFLOW_SIGNAL_STR \"\"\n", versionFile);
    } /* if */
  } /* checkOverflowSignal */



static int timeLShiftOverflowCheck (const char *lshift5)

  {
    char buffer[BUFFER_SIZE];
    int clockCycles = 0;

  /* timeLShiftOverflowCheck */
    sprintf(buffer, "#include<stdio.h>\n#include<time.h>\n"
                    "#define FMT_D \"%%%sd\"\n"
                    "typedef %s intType;\n"
                    "typedef %s uintType;\n"
                    "%s"
                    "int main(int argc,char *argv[]){\n"
                    "intType number = -12345678;\n"
                    "intType increment = 1;\n"
                    "uintType sum = 0;\n"
                    "clock_t startClock;\n"
                    "int count;\n"
                    "if (argc >= 3) {\n"
                    "sscanf(argv[1], FMT_D, &number);\n"
                    "sscanf(argv[2], FMT_D, &increment);}\n"
                    "startClock = clock();\n"
                    "for (count = 0; count < 100000000; count++) {\n"
                    "sum += (uintType) lshift5(number) ^ (uintType) number;\n"
                    "number += increment;}\n"
                    "printf(\"%%lu \" FMT_D \"\\n\",\n"
                    "(unsigned long) (clock() - startClock),sum);\n"
                    "return 0;}\n",
                    int64TypeFormat, int64TypeStri, uint64TypeStri, lshift5);
    /* printf("%s\n", buffer); */
    if (compileAndLinkWithOptionsOk(buffer, CC_OPT_OPTIMIZE_3, "")) {
      clockCycles = doTest();
    } /* if */
    return clockCycles;
  } /* timeLShiftOverflowCheck */



static void checkIntDivisionOverflow (FILE *versionFile)

  {
    char buffer[BUFFER_SIZE];

  /* checkIntDivisionOverflow */
#if defined INT_DIV_OVERFLOW_INFINITE_LOOP
    fputs("#define INT_DIV_OVERFLOW 0\n", versionFile);
    fputs("#define INT_REM_OVERFLOW 0\n", versionFile);
#elif defined INT_DIV_BY_ZERO_POPUP
    fputs("#define INT_DIV_OVERFLOW 1\n", versionFile);
    fputs("#define INT_REM_OVERFLOW 1\n", versionFile);
#else
    sprintf(buffer,
            "#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
            "typedef %s int64Type;\n"
            "void handleSigfpe(int sig){puts(\"4\");exit(0);}\n"
            "void handleSigill(int sig){puts(\"5\");exit(0);}\n"
            "void handleSigabrt(int sig){puts(\"6\");exit(0);}\n"
            "void handleSigtrap(int sig){puts(\"7\");exit(0);}\n"
            "int main(int argc,char *argv[]){\n"
            "int64Type minusOne=-1;\n"
            "int64Type minIntValue = -9223372036854775807-1;\n"
            "signal(SIGFPE,handleSigfpe);\n"
            "signal(SIGILL,handleSigill);\n"
            "signal(SIGABRT,handleSigabrt);\n"
            "#ifdef SIGTRAP\n"
            "signal(SIGTRAP,handleSigtrap);\n"
            "#endif\n"
            "printf(\"%%d\\n\", minIntValue / minusOne == minIntValue ? 2 : 3);\n"
            "return 0;}\n",
            int64TypeStri);
    if (compileAndLinkOk(buffer)) {
      fprintf(versionFile, "#define INT_DIV_OVERFLOW %d\n", doTest());
    } /* if */
    sprintf(buffer,
            "#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
            "typedef %s int64Type;\n"
            "void handleSigfpe(int sig){puts(\"4\");exit(0);}\n"
            "void handleSigill(int sig){puts(\"5\");exit(0);}\n"
            "void handleSigabrt(int sig){puts(\"6\");exit(0);}\n"
            "void handleSigtrap(int sig){puts(\"7\");exit(0);}\n"
            "int main(int argc,char *argv[]){\n"
            "int64Type minusOne=-1;\n"
            "int64Type minIntValue = -9223372036854775807-1;\n"
            "signal(SIGFPE,handleSigfpe);\n"
            "signal(SIGILL,handleSigill);\n"
            "signal(SIGABRT,handleSigabrt);\n"
            "#ifdef SIGTRAP\n"
            "signal(SIGTRAP,handleSigtrap);\n"
            "#endif\n"
            "printf(\"%%d\\n\", minIntValue %% minusOne == 0 ? 2 : 3);\n"
            "return 0;}\n",
            int64TypeStri);
    if (compileAndLinkOk(buffer)) {
      fprintf(versionFile, "#define INT_REM_OVERFLOW %d\n", doTest());
    } /* if */
#endif
  } /* checkIntDivisionOverflow */



static const char *determine_os_isnan_definition (const char *computeValues,
    const char *os_isnan_definition)

  {
    char buffer[BUFFER_SIZE];
    const char *macro_definition = NULL;

  /* determine_os_isnan_definition */
    sprintf(buffer,
            "#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
            "%s\n"
            "int main(int argc,char *argv[]){\n"
            "%s"
            "printf(\"%%d\\n\",\n"
            "       os_isnan(floatNanValue1) &&\n"
            "       os_isnan(floatNanValue2) &&\n"
            "       os_isnan(doubleNanValue1) &&\n"
            "       os_isnan(doubleNanValue2) &&\n"
            "       !os_isnan(floatPlusInf) &&\n"
            "       !os_isnan(floatMinusInf) &&\n"
            "       !os_isnan(floatNegativeZero) &&\n"
            "       !os_isnan(doublePlusInf) &&\n"
            "       !os_isnan(doubleMinusInf) &&\n"
            "       !os_isnan(doubleNegativeZero) &&\n"
            "       !os_isnan(0.0) &&\n"
            "       !os_isnan(10.0) &&\n"
            "       !os_isnan(100.0) &&\n"
            "       !os_isnan(1000.0) &&\n"
            "       !os_isnan(10000.0) &&\n"
            "       !os_isnan(100000.0) &&\n"
            "       !os_isnan(1000000.0) &&\n"
            "       !os_isnan(10000000.0) &&\n"
            "       !os_isnan(100000000.0) &&\n"
            "       !os_isnan(1000000000.0) &&\n"
            "       !os_isnan(9007199254740992.0) &&\n"
            "       !os_isnan(9007199254740993.0));\n"
            "return 0;}\n",
            os_isnan_definition, computeValues);
    /* printf("%s\n", buffer); */
    if (compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS " " SYSTEM_MATH_LIBS) && doTest() == 1) {
      macro_definition = os_isnan_definition;
    } /* if */
    return macro_definition;
  } /* determine_os_isnan_definition */



static void defineTransferUnions (char * buffer)

  { /* defineTransferUnions */
    strcat(buffer,
           "union {\n"
           "  ");
    switch (sizeof_float) {
      case 2: strcat(buffer, uint16TypeStri); break;
      case 4: strcat(buffer, uint32TypeStri); break;
      case 8: strcat(buffer, uint64TypeStri); break;
    } /* switch */
    strcat(buffer,
           " i;\n"
           "  float f;\n"
           "} fltTransfer;\n");
    strcat(buffer,
           "union {\n"
           "  ");
    switch (sizeof_double) {
      case 2: strcat(buffer, uint16TypeStri); break;
      case 4: strcat(buffer, uint32TypeStri); break;
      case 8: strcat(buffer, uint64TypeStri); break;
    } /* switch */
    strcat(buffer,
           " i;\n"
           "  double f;\n"
           "} dblTransfer;\n");
  } /* defineTransferUnions */



static void numericProperties (FILE *versionFile)

  {
    int testResult;
    char buffer[11150];
    char computeValues[BUFFER_SIZE];
    const char *builtin_add_overflow = "nonexistent_function";
    const char *builtin_mul_overflow = "nonexistent_function";
    int has_log2;
    const char *os_isnan_definition = NULL;

  /* numericProperties */
    fprintf(logFile, "Numeric properties: ");
    fflush(stdout);
    if (assertCompAndLnk("#include <stdio.h>\nint main(int argc,char *argv[])"
                         "{long num=-1;printf(\"%d\\n\",num>>1==(long)-1);return 0;}\n")) {
      fprintf(versionFile, "#define RSHIFT_DOES_SIGN_EXTEND %d\n", doTest());
    } /* if */
    if (assertCompAndLnk("#include <stdio.h>\nint main(int argc,char *argv[])"
                         "{long num=-1;printf(\"%d\\n\",~num==(long)0);return 0;}\n")) {
      fprintf(versionFile, "#define TWOS_COMPLEMENT_INTTYPE %d\n", doTest());
    } /* if */
    if (assertCompAndLnk("#include <stdio.h>\nint main(int argc,char *argv[])"
                         "{long num=-1;printf(\"%d\\n\",~num==(long)1);return 0;}\n")) {
      fprintf(versionFile, "#define ONES_COMPLEMENT_INTTYPE %d\n", doTest());
    } /* if */
    if (assertCompAndLnk("#include <stdio.h>\nint main(int argc,char *argv[])"
                         "{long num=1;printf(\"%d\\n\",((char*)&num)[0]==1);return 0;}\n")) {
      fprintf(versionFile, "#define LITTLE_ENDIAN_INTTYPE %d\n", doTest());
    } /* if */
    if (assertCompAndLnk("#include <stdio.h>\nint main(int argc,char *argv[])"
                         "{long num=1;printf(\"%d\\n\",((char*)&num)[sizeof(long) - 1]==1);\n"
                         "return 0;}\n")) {
      fprintf(versionFile, "#define BIG_ENDIAN_INTTYPE %d\n", doTest());
    } /* if */
    checkIntDivisions(versionFile);
    checkOverflowSignal(versionFile);
    if (sizeof_int == 8) {
      builtin_add_overflow = "__builtin_sadd_overflow";
      builtin_mul_overflow = "__builtin_smul_overflow";
    } else if (sizeof_long == 8) {
      builtin_add_overflow = "__builtin_saddl_overflow";
      builtin_mul_overflow = "__builtin_smull_overflow";
    } else if (sizeof_long_long == 8) {
      builtin_add_overflow = "__builtin_saddll_overflow";
      builtin_mul_overflow = "__builtin_smulll_overflow";
    } /* if */
    sprintf(buffer, "#include<stdlib.h>\n#include<stdio.h>\n#include<limits.h>\n"
                    "#include<signal.h>\n"
                    "void handleSigill(int sig){puts(\"2\");exit(0);}\n"
                    "void handleSigabrt(int sig){puts(\"3\");exit(0);}\n"
                    "int main(int argc,char *argv[]){\n"
                    "%s a=0x7fffffffffffffff,b=1,c=2;\n"
                    "signal(SIGILL,handleSigill);\nsignal(SIGABRT,handleSigabrt);\n"
                    "printf(\"%%d\\n\",%s(a, b, &c) != 0);return 0;}\n",
                    int64TypeStri, builtin_add_overflow);
    fprintf(versionFile, "#define HAS_BUILTIN_OVERFLOW_OPERATIONS %d\n",
            compileAndLinkOk(buffer) && doTest() == 1);
    sprintf(buffer, "intType lshift5 (intType number)\n"
                    "{intType result;\n"
                    "if (%s(number, (intType) 32, &result)) {\n"
                    "printf(\"overflow\\n\"); return 0;}\n"
                    "return result;}\n",
                    builtin_mul_overflow);
    fprintf(versionFile, "#define LSHIFT_BUILTIN_MUL_TIME %d\n",
            timeLShiftOverflowCheck(buffer));
    sprintf(buffer, "intType lshift5 (intType number)\n"
                    "{if ((uintType) number + (uintType) 288230376151711744 > (uintType) 576460752303423487) {\n"
                    "printf(\"overflow\\n\"); return 0;}\n"
                    "return number << 5;}\n");
    fprintf(versionFile, "#define LSHIFT_TWO_ARG_CHECKS_TIME %d\n",
            timeLShiftOverflowCheck(buffer));
    sprintf(buffer, "intType lshift5 (intType number)\n"
                    "{if (number > (intType) 288230376151711743) {\n"
                    "printf(\"overflow\\n\"); return 0;}\n"
                    "return number << 5;}\n");
    fprintf(versionFile, "#define LSHIFT_ONE_ARG_CHECK_TIME %d\n",
            timeLShiftOverflowCheck(buffer));
    sprintf(buffer, "intType lshift5 (intType number)\n"
                    "{intType result;\n"
                    "result = (intType) ((uintType) number << 5);\n"
                    "if (result >> 5 != number) {\n"
                    "printf(\"overflow\\n\"); return 0;}\n"
                    "return result;}\n");
    fprintf(versionFile, "#define LSHIFT_RESULT_CHECK_TIME %d\n",
            timeLShiftOverflowCheck(buffer));
    sprintf(buffer, "intType lshift5 (intType number)\n"
                    "{return number << 5;}\n");
    fprintf(versionFile, "#define LSHIFT_NO_CHECK_TIME %d\n",
            timeLShiftOverflowCheck(buffer));
    checkIntDivisionOverflow(versionFile);
    if (assertCompAndLnk("#include<stdio.h>\n#include<string.h>\n"
                         "int main(int argc,char *argv[]){\n"
                         "char buffer[1024];\n"
                         "int rounding = 0;\n"
                         "sprintf(buffer,\n"
                         "    \"%1.0f %1.0f %1.0f %1.1f %1.1f %1.2f %1.2f\"\n"
                         "    \" %1.0f %1.0f %1.0f %1.1f %1.1f %1.2f %1.2f\",\n"
                         "    0.5, 1.5, 2.5, 1.25, 1.75, 0.125, 0.375,\n"
                         "    -0.5, -1.5, -2.5, -1.25, -1.75, -0.125, -0.375);\n"
                         "if (strcmp(buffer, \"0 2 2 1.2 1.8 0.12 0.38\"\n"
                         "           \" 0 -2 -2 -1.2 -1.8 -0.12 -0.38\") == 0 ||\n"
                         "    strcmp(buffer, \"0 2 2 1.2 1.8 0.12 0.38\"\n"
                         "           \" -0 -2 -2 -1.2 -1.8 -0.12 -0.38\") == 0) {\n"
                         "  rounding = 1;\n"
                         "} else if (strcmp(buffer, \"1 2 3 1.3 1.8 0.13 0.38\"\n"
                         "                  \" -1 -2 -3 -1.3 -1.8 -0.13 -0.38\") == 0) {\n"
                         "  rounding = 2;\n"
                         "} else if (strcmp(buffer, \"1 2 3 1.3 1.8 0.13 0.38\"\n"
                         "                  \" 0 -1 -2 -1.2 -1.7 -0.12 -0.37\") == 0 ||\n"
                         "           strcmp(buffer, \"1 2 3 1.3 1.8 0.13 0.38\"\n"
                         "                  \" -0 -1 -2 -1.2 -1.7 -0.12 -0.37\") == 0) {\n"
                         "  rounding = 3;\n"
                         "}\n"
                         "printf(\"%d\\n\", rounding);\n"
                         "return 0;}\n")) {
      switch (doTest()) {
        case 1:
          fputs("#define ROUND_HALF_TO_EVEN 1\n", versionFile);
          fputs("#define PRINTF_ROUNDING ROUND_HALF_TO_EVEN\n", versionFile);
          break;
        case 2:
          fputs("#define ROUND_HALF_AWAY_FROM_ZERO 2\n", versionFile);
          fputs("#define PRINTF_ROUNDING ROUND_HALF_AWAY_FROM_ZERO\n", versionFile);
          break;
        case 3:
          fputs("#define ROUND_HALF_UP 3\n", versionFile);
          fputs("#define PRINTF_ROUNDING ROUND_HALF_UP\n", versionFile);
          break;
        default:
          fputs("#define ROUND_HALF_NOT_RECOGNIZED -1\n", versionFile);
          fputs("#define PRINTF_ROUNDING ROUND_HALF_NOT_RECOGNIZED\n", versionFile);
          fprintf(logFile, "\n *** Rounding of half values not recognized.\n");
          break;
      } /* switch */
    } /* if */
    if (assertCompAndLnk("#include<stdio.h>\n#include<string.h>\n#include<float.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{int precision;double number;char buffer[128];\n"
                         "precision = 3;number = 0.123456789;\n"
                         "sprintf(buffer, \"%1.*f\", precision, number);\n"
                         "printf(\"%d\\n\",strcmp(buffer,\"0.123\")==0);return 0;}\n")) {
      fprintf(versionFile, "#define PRINTF_SUPPORTS_VARIABLE_FORMATS %d\n", doTest());
    } /* if */
    if (assertCompAndLnk("#include<stdio.h>\n#include<string.h>\n#include<float.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{int minExp10;char buffer[128];\n"
                         "sprintf(buffer, \"%1.10e\", DBL_MIN);\n"
                         "sscanf(strchr(buffer,'e') + 1, \"%d\", &minExp10);\n"
                         "printf(\"%d\\n\",minExp10);return 0;}\n")) {
      fprintf(versionFile, "#define DOUBLE_MIN_EXP10 %d\n", doTest());
    } /* if */
    if (assertCompAndLnk("#include<stdio.h>\n#include<string.h>\n#include<float.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{int maxExp10;char buffer[128];\n"
                         "sprintf(buffer, \"%1.10e\", DBL_MAX);\n"
                         "sscanf(strchr(buffer,'e') + 1, \"%d\", &maxExp10);\n"
                         "printf(\"%d\\n\",maxExp10);return 0;}\n")) {
      fprintf(versionFile, "#define DOUBLE_MAX_EXP10 %d\n", doTest());
    } /* if */
    if (assertCompAndLnk("#include<stdio.h>\n#include<string.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{char buffer[128];\n"
                         "sprintf(buffer, \"%1.14e\", 1.12345678901234);\n"
                         "printf(\"%d\\n\",(int)strlen(buffer)-18);return 0;}\n")) {
      fprintf(versionFile, "#define MIN_PRINTED_EXPONENT_DIGITS %u\n", doTest());
    } /* if */
    if (assertCompAndLnk("#include<stdio.h>\n#include<string.h>\n#include<float.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{int maxExp10;char buffer[128];\n"
                         "sprintf(buffer, \"%1.10e\", DBL_MAX);\n"
                         "sscanf(strchr(buffer,'e') + 1, \"%d\", &maxExp10);\n"
                         "sprintf(buffer, \"%d\", maxExp10);\n"
                         "printf(\"%d\\n\",(int)strlen(buffer));return 0;}\n")) {
      fprintf(versionFile, "#define MAX_PRINTED_EXPONENT_DIGITS %u\n", doTest());
    } /* if */
    if (assertCompAndLnk("#include<stdio.h>\n#include<float.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{printf(\"%d\\n\",FLT_DIG);return 0;}\n")) {
      testResult = doTest();
      fprintf(versionFile, "#define FMT_E_FLT \"%%1.%de\"\n", testResult - 1);
      fprintf(versionFile, "#define FMT_E_FLT_PRECISION %d\n", testResult - 1);
      fprintf(versionFile, "#define FLOAT_STR_LARGE_NUMBER 1.0e%d\n", testResult);
    } /* if */
    if (assertCompAndLnk("#include<stdio.h>\n#include<float.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{printf(\"%d\\n\",DBL_DIG);return 0;}\n")) {
      testResult = doTest();
      fprintf(versionFile, "#define FMT_E_DBL \"%%1.%de\"\n", testResult - 1);
      fprintf(versionFile, "#define FMT_E_DBL_PRECISION %d\n", testResult - 1);
      fprintf(versionFile, "#define DOUBLE_STR_LARGE_NUMBER 1.0e%d\n", testResult);
    } /* if */
    strcpy(buffer, "#include<stdio.h>\n"
                   "int main(int argc,char *argv[]){\n"
                   "int okay;\n");
    defineTransferUnions(buffer);
    strcat(buffer, "dblTransfer.f = 1.0/0.0;\n"
                   "okay = dblTransfer.i == 0x7ff0000000000000;\n"
                   "dblTransfer.f = -1.0/0.0;\n"
                   "okay &= dblTransfer.i == 0xfff0000000000000;\n"
                   "printf(\"%d\\n\", okay);return 0;}\n");
    fprintf(versionFile, "#define FLOAT_ZERO_DIV_ERROR %d\n",
            !compileAndLinkOk("#include<stdio.h>\n"
                              "int main(int argc,char *argv[]){"
                              "printf(\"%f\", 1.0/0.0);return 0;}\n") ||
            !compileAndLinkOk("#include<stdlib.h>\n#include<stdio.h>\n"
                              "#include<float.h>\n#include<signal.h>\n"
                              "void handleSig(int sig){puts(\"2\");exit(0);}\n"
                              "int main(int argc,char *argv[]){\n"
#ifdef TURN_OFF_FP_EXCEPTIONS
                              "_control87(MCW_EM, MCW_EM);\n"
#endif
                              "signal(SIGFPE,handleSig);\nsignal(SIGILL,handleSig);\n"
                              "signal(SIGINT,handleSig);\n"
                              "printf(\"%d\\n\",1.0/0.0==0.0);return 0;}\n") ||
            doTest() == 2 ||
            !compileAndLinkOk("#include<stdio.h>\n"
                              "#define POSITIVE_INFINITY ( 1.0 / 0.0)\n"
                              "#define NEGATIVE_INFINITY (-1.0 / 0.0)\n"
                              "int isLess (double number1, double number2)\n"
                              "  {return number1 < number2;}\n"
                              "int isGreater (double number1, double number2)\n"
                              "  {return number1 > number2;}\n"
                              "int main(int argc,char *argv[]){\n"
                              "int check1, check2, check3, check4;\n"
                              "check1 = -(POSITIVE_INFINITY) < -1.0E37;\n"
                              "check2 = isLess(-(POSITIVE_INFINITY), -1.0E37);\n"
                              "check3 = -(NEGATIVE_INFINITY) > 1.0E37;\n"
                              "check4 = isGreater(-(NEGATIVE_INFINITY), 1.0E37);\n"
                              "printf(\"%d\\n\", check1 == check2 && check3 == check4);\n"
                              "return 0;}\n") ||
            doTest() != 1 ||
            !compileAndLinkOk(buffer) ||
            doTest() != 1);
    if (assertCompAndLnk("#include<stdio.h>\n#include<float.h>\n"
                         "int main(int argc,char *argv[]){\n"
                         "printf(\"%d\\n\",\n"
                         "       1.0 == (double) 1 &&\n"
                         "       10.0 == (double) 10 &&\n"
                         "       100.0 == (double) 100 &&\n"
                         "       1000.0 == (double) 1000 &&\n"
                         "       10000.0 == (double) 10000 &&\n"
                         "       100000.0 == (double) 100000 &&\n"
                         "       1000000.0 == (double) 1000000 &&\n"
                         "       10000000.0 == (double) 10000000 &&\n"
                         "       100000000.0 == (double) 100000000 &&\n"
                         "       1000000000.0 == (double) 1000000000);\n"
                         "return 0;}\n")) {
      fprintf(versionFile, "#define CAST_INT_TO_FLOAT_OKAY %d\n", doTest());
    } /* if */
    fprintf(versionFile, "#define HAS_LOG1P %d\n",
        compileAndLinkWithOptionsOk("#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
                                    "int main(int argc,char *argv[]){\n"
                                    "float num1 = 0.0;\n"
                                    "double num2 = 0.0;\n"
                                    "printf(\"%d\\n\",\n"
                                    "       log1p(num1) == 0.0 &&\n"
                                    "       log1p(num2) == 0.0);\n"
                                    "return 0;}\n",
                                    "", SYSTEM_LIBS " " SYSTEM_MATH_LIBS) && doTest() == 1);
    has_log2 =
        compileAndLinkWithOptionsOk("#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
                                    "int main(int argc,char *argv[]){\n"
                                    "float num1 = 2.0;\n"
                                    "double num2 = 2.0;\n"
                                    "printf(\"%d\\n\",\n"
                                    "       log2(num1) == 1.0 &&\n"
                                    "       log2(num2) == 1.0);\n"
                                    "return 0;}\n",
                                    "", SYSTEM_LIBS " " SYSTEM_MATH_LIBS) && doTest() == 1;
    fprintf(versionFile, "#define HAS_LOG2 %d\n", has_log2);
    fprintf(versionFile, "#define HAS_CBRT %d\n",
        compileAndLinkWithOptionsOk("#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
                                    "int main(int argc,char *argv[]){\n"
                                    "double zero = 0.0;\n"
                                    "printf(\"%d\\n\",\n"
                                    "       cbrt( zero) == 0.0 &&\n"
                                    "       cbrt(  0.0) == 0.0 &&\n"
                                    "       cbrt(  1.0) == 1.0 &&\n"
                                    "       cbrt(  8.0) == 2.0 &&\n"
                                    "       cbrt( 27.0) == 3.0 &&\n"
                                    "       cbrt( 64.0) == 4.0 &&\n"
                                    "       cbrt(125.0) == 5.0); return 0;}\n",
                                    "", SYSTEM_LIBS " " SYSTEM_MATH_LIBS) && doTest() == 1);
    strcpy(computeValues,
           "float floatZero = 0.0;\n"
           "float floatNegativeZero;\n"
           "float floatNanValue1;\n"
           "float floatNanValue2;\n"
           "float floatPlusInf;\n"
           "float floatMinusInf;\n"
           "double doubleZero = 0.0;\n"
           "double doubleNegativeZero;\n"
           "double doubleNanValue1;\n"
           "double doubleNanValue2;\n"
           "double doublePlusInf;\n"
           "double doubleMinusInf;\n");
    if (!compileAndLinkOk("#include<stdlib.h>\n#include<stdio.h>\n#include<float.h>\n#include<signal.h>\n"
                          "void handleSig(int sig){puts(\"2\");exit(0);}\n"
                          "int main(int argc,char *argv[]){\n"
                          "float zero1=0.0;\n"
                          "float zero2=0.0;\n"
#ifdef TURN_OFF_FP_EXCEPTIONS
                          "_control87(MCW_EM, MCW_EM);\n"
#endif
                          "signal(SIGFPE,handleSig);\nsignal(SIGILL,handleSig);\n"
                          "signal(SIGINT,handleSig);\n"
                          "printf(\"%d\\n\", 1.0/zero1!=0.0 && 0.0/zero1!=0.0/zero2);return 0;}\n") ||
                          doTest() != 1) {
      fputs("#define CHECK_FLOAT_DIV_BY_ZERO 1\n", versionFile);
      fputs("#define USE_NEGATIVE_ZERO_BITPATTERN 1\n", versionFile);
      defineTransferUnions(computeValues);
#ifdef TURN_OFF_FP_EXCEPTIONS
      strcat(computeValues,
             "_control87(MCW_EM, MCW_EM);\n");
#endif
      strcat(computeValues,
             "fltTransfer.i = 0xffc00000;\n"
             "floatNanValue1 = fltTransfer.f;\n"
             "fltTransfer.i = 0x7f800000;\n"
             "floatPlusInf = fltTransfer.f;\n"
             "fltTransfer.i = 0xff800000;\n"
             "floatMinusInf = fltTransfer.f;\n"
             "fltTransfer.i = 0x80000000;\n"
             "floatNegativeZero = fltTransfer.f;\n"
             "floatNanValue2 = floatNanValue1;\n");
      strcat(computeValues,
             "dblTransfer.i = 0xfff8000000000000;\n"
             "doubleNanValue1 = dblTransfer.f;\n"
             "dblTransfer.i = 0x7ff0000000000000;\n"
             "doublePlusInf = dblTransfer.f;\n"
             "dblTransfer.i = 0xfff0000000000000;\n"
             "doubleMinusInf = dblTransfer.f;\n"
             "dblTransfer.i = 0x8000000000000000;\n"
             "doubleNegativeZero = dblTransfer.f;\n"
             "doubleNanValue2 = doubleNanValue1;\n");
    } else {
      strcat(computeValues,
#ifdef TURN_OFF_FP_EXCEPTIONS
             "_control87(MCW_EM, MCW_EM);\n"
#endif
             "floatNanValue1 = 0.0 / floatZero;\n"
             "floatNanValue2 = 0.0 / floatZero;\n"
             "floatPlusInf = 1.0 / floatZero;\n"
             "floatMinusInf = -floatPlusInf;\n"
             "floatNegativeZero = -1.0 / floatPlusInf;\n"
             "doubleNanValue1 = 0.0 / doubleZero;\n"
             "doubleNanValue2 = 0.0 / doubleZero;\n"
             "doublePlusInf = 1.0 / doubleZero;\n"
             "doubleMinusInf = -doublePlusInf;\n"
             "doubleNegativeZero = -1.0 / doublePlusInf;\n");
      sprintf(buffer,
              "#include<stdio.h>\n#include<string.h>\n#include<float.h>\n"
              "int main(int argc,char *argv[]){\n"
              "float minusZero;\n"
              "%s"
              "printf(\"#define CHECK_FLOAT_DIV_BY_ZERO %%d\\n\",\n"
              "    floatPlusInf == floatMinusInf ||\n"
              "    -1.0 / floatZero != floatMinusInf ||\n"
              "     1.0 / floatNegativeZero != floatMinusInf);\n"
              "minusZero = -floatZero;\n"
              "printf(\"#define USE_NEGATIVE_ZERO_BITPATTERN %%d\\n\",\n"
              "    memcmp(&floatNegativeZero, &minusZero, sizeof(float)) != 0);\n"
              "return 0;}\n", computeValues);
      if (assertCompAndLnk(buffer)) {
        testOutputToVersionFile(versionFile);
      } /* if */
    } /* if */
    os_isnan_definition = determine_os_isnan_definition(computeValues, "#define os_isnan isnan");
    if (os_isnan_definition == NULL) {
      os_isnan_definition = determine_os_isnan_definition(computeValues, "#define os_isnan _isnan");
      if (os_isnan_definition == NULL) {
        os_isnan_definition = determine_os_isnan_definition(computeValues, "#define os_isnan(x) (x != x)");
      } /* if */
    } /* if */
    if (os_isnan_definition == NULL) {
      fprintf(logFile, "\n *** Unable to determine isnan() macro.\n");
    } else {
      fprintf(versionFile, "%s\n", os_isnan_definition);
      fprintf(versionFile, "#define OS_ISNAN_DEFINITION \"%s\\n\"\n", os_isnan_definition);
    } /* if */
    sprintf(buffer,
            "#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
            "%s\n"
            "float floatFunc (const float number) { return number; }\n"
            "double doubleFunc (const double number) { return number; }\n"
            "int main(int argc,char *argv[]){\n"
            "%s\n"
            "printf(\"%%d\\n\",\n"
            "    !(floatNanValue1 == floatNanValue2 ||\n"
            "      floatNanValue1 <  floatNanValue2 ||\n"
            "      floatNanValue1 >  floatNanValue2 ||\n"
            "      floatNanValue1 <= floatNanValue2 ||\n"
            "      floatNanValue1 >= floatNanValue2 ||\n"
            "      floatNanValue1 == 0.0 ||\n"
            "      floatNanValue1 <  0.0 ||\n"
            "      floatNanValue1 >  0.0 ||\n"
            "      floatNanValue1 <= 0.0 ||\n"
            "      floatNanValue1 >= 0.0 ||\n"
            "      0.0 == floatNanValue2 ||\n"
            "      0.0 <  floatNanValue2 ||\n"
            "      0.0 >  floatNanValue2 ||\n"
            "      0.0 <= floatNanValue2 ||\n"
            "      0.0 >= floatNanValue2 ||\n"
            "      floatFunc(floatNanValue1) == 0.0 ||\n"
            "      floatFunc(floatNanValue1) <  0.0 ||\n"
            "      floatFunc(floatNanValue1) >  0.0 ||\n"
            "      floatFunc(floatNanValue1) <= 0.0 ||\n"
            "      floatFunc(floatNanValue1) >= 0.0 ||\n"
            "      0.0 == floatFunc(floatNanValue2) ||\n"
            "      0.0 <  floatFunc(floatNanValue2) ||\n"
            "      0.0 >  floatFunc(floatNanValue2) ||\n"
            "      0.0 <= floatFunc(floatNanValue2) ||\n"
            "      0.0 >= floatFunc(floatNanValue2) ||\n"
            "      doubleNanValue1 == doubleNanValue2 ||\n"
            "      doubleNanValue1 <  doubleNanValue2 ||\n"
            "      doubleNanValue1 >  doubleNanValue2 ||\n"
            "      doubleNanValue1 <= doubleNanValue2 ||\n"
            "      doubleNanValue1 >= doubleNanValue2 ||\n"
            "      doubleNanValue1 == 0.0 ||\n"
            "      doubleNanValue1 <  0.0 ||\n"
            "      doubleNanValue1 >  0.0 ||\n"
            "      doubleNanValue1 <= 0.0 ||\n"
            "      doubleNanValue1 >= 0.0 ||\n"
            "      0.0 == doubleNanValue2 ||\n"
            "      0.0 <  doubleNanValue2 ||\n"
            "      0.0 >  doubleNanValue2 ||\n"
            "      0.0 <= doubleNanValue2 ||\n"
            "      0.0 >= doubleNanValue2 ||\n"
            "      doubleFunc(doubleNanValue1) == 0.0 ||\n"
            "      doubleFunc(doubleNanValue1) <  0.0 ||\n"
            "      doubleFunc(doubleNanValue1) >  0.0 ||\n"
            "      doubleFunc(doubleNanValue1) >= 0.0 ||\n"
            "      doubleFunc(doubleNanValue1) >= 0.0 ||\n"
            "      0.0 == doubleFunc(doubleNanValue2) ||\n"
            "      0.0 <  doubleFunc(doubleNanValue2) ||\n"
            "      0.0 >  doubleFunc(doubleNanValue2) ||\n"
            "      0.0 <= doubleFunc(doubleNanValue2) ||\n"
            "      0.0 >= doubleFunc(doubleNanValue2)));\n"
            "return 0;}\n",
            os_isnan_definition, computeValues);
    fprintf(versionFile, "#define FLOAT_NAN_COMPARISON_OKAY %d\n",
        compileAndLinkWithOptionsOk(buffer, "", "") && doTest() == 1);
    sprintf(buffer,
            "#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
            "%s\n"
            "int main(int argc,char *argv[]){\n"
            "%s\n"
            "printf(\"%%d\\n\",\n"
            "       0.0 == -0.0  &&\n"
            "    !( 0.0 != -0.0) &&\n"
            "    !( 0.0 <  -0.0) &&\n"
            "    !( 0.0 >  -0.0) &&\n"
            "       0.0 >= -0.0  &&\n"
            "       0.0 <= -0.0  &&\n"
            "      -0.0 ==  0.0  &&\n"
            "    !(-0.0 !=  0.0) &&\n"
            "    !(-0.0 <   0.0) &&\n"
            "    !(-0.0 >   0.0) &&\n"
            "      -0.0 >=  0.0  &&\n"
            "      -0.0 <=  0.0  &&\n"
            "       floatZero == floatNegativeZero  &&\n"
            "    !( floatZero != floatNegativeZero) &&\n"
            "    !( floatZero <  floatNegativeZero) &&\n"
            "    !( floatZero >  floatNegativeZero) &&\n"
            "       floatZero >= floatNegativeZero  &&\n"
            "       floatZero <= floatNegativeZero  &&\n"
            "      floatNegativeZero ==  floatZero  &&\n"
            "    !(floatNegativeZero !=  floatZero) &&\n"
            "    !(floatNegativeZero <   floatZero) &&\n"
            "    !(floatNegativeZero >   floatZero) &&\n"
            "      floatNegativeZero >=  floatZero  &&\n"
            "      floatNegativeZero <=  floatZero  &&\n"
            "       doubleZero == doubleNegativeZero  &&\n"
            "    !( doubleZero != doubleNegativeZero) &&\n"
            "    !( doubleZero <  doubleNegativeZero) &&\n"
            "    !( doubleZero >  doubleNegativeZero) &&\n"
            "       doubleZero >= doubleNegativeZero  &&\n"
            "       doubleZero <= doubleNegativeZero  &&\n"
            "      doubleNegativeZero ==  doubleZero  &&\n"
            "    !(doubleNegativeZero !=  doubleZero) &&\n"
            "    !(doubleNegativeZero <   doubleZero) &&\n"
            "    !(doubleNegativeZero >   doubleZero) &&\n"
            "      doubleNegativeZero >=  doubleZero  &&\n"
            "      doubleNegativeZero <=  doubleZero);\n"
            "return 0;}\n",
            os_isnan_definition, computeValues);
    fprintf(versionFile, "#define FLOAT_ZERO_COMPARISON_OKAY %d\n",
        compileAndLinkWithOptionsOk(buffer, "", "") && doTest() == 1);
    sprintf(buffer,
            "#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
            "%s\n"
            "int main(int argc,char *argv[]){\n"
            "%s\n"
            "printf(\"%%d\\n\",\n"
            "       exp(0.0) == 1.0 &&\n"
            "       exp(floatZero) == 1.0 &&\n"
            "       exp(floatNegativeZero) == 1.0 &&\n"
            "       exp(floatMinusInf) == 0.0 &&\n"
            "       exp(doubleMinusInf) == 0.0 &&\n"
            "       exp(floatPlusInf) == floatPlusInf &&\n"
            "       exp(doublePlusInf) == doublePlusInf &&\n"
            "       os_isnan(exp(floatNanValue1)) &&\n"
            "       os_isnan(exp(doubleNanValue1)));\n"
            "return 0;}\n",
            os_isnan_definition, computeValues);
    fprintf(versionFile, "#define HAS_EXP %d\n",
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS " " SYSTEM_MATH_LIBS) && doTest() == 1);
    sprintf(buffer,
            "#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
            "%s\n"
            "int main(int argc,char *argv[]){\n"
            "%s\n"
            "printf(\"%%d\\n\",\n"
            "       exp2(-5.0) == 0.03125 &&\n"
            "       exp2(-4.0) == 0.0625 &&\n"
            "       exp2(-3.0) == 0.125 &&\n"
            "       exp2(-2.0) == 0.25 &&\n"
            "       exp2(-1.0) == 0.5 &&\n"
            "       exp2( 0.0) == 1.0 &&\n"
            "       exp2( 1.0) == 2.0 &&\n"
            "       exp2( 2.0) == 4.0 &&\n"
            "       exp2( 3.0) == 8.0 &&\n"
            "       exp2( 4.0) == 16.0 &&\n"
            "       exp2( 5.0) == 32.0 &&\n"
            "       exp2(10.0) == 1024.0 &&\n"
            "       exp2(20.0) == 1048576.0 &&\n"
            "       exp2(30.0) == 1073741824.0 &&\n"
            "       exp2(40.0) == 1099511627776.0 &&\n"
            "       exp2(50.0) == 1125899906842624.0 &&\n"
            "       exp2(60.0) == 1152921504606846976.0 &&\n"
            "       exp2(62.0) == 4611686018427387904.0 &&\n"
            "       exp2(floatZero) == 1.0 &&\n"
            "       exp2(floatNegativeZero) == 1.0 &&\n"
            "       exp2(floatMinusInf) == 0.0 &&\n"
            "       exp2(doubleMinusInf) == 0.0 &&\n"
            "       exp2(floatPlusInf) == floatPlusInf &&\n"
            "       exp2(doublePlusInf) == doublePlusInf &&\n"
            "       os_isnan(exp2(floatNanValue1)) &&\n"
            "       os_isnan(exp2(doubleNanValue1)));\n"
            "return 0;}\n",
            os_isnan_definition, computeValues);
    fprintf(versionFile, "#define HAS_EXP2 %d\n",
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS " " SYSTEM_MATH_LIBS) && doTest() == 1);
    sprintf(buffer,
            "#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
            "%s\n"
            "int main(int argc,char *argv[]){\n"
            "%s\n"
            "printf(\"%%d\\n\",\n"
            "       exp10(0.0) == 1.0 &&\n"
            "       exp10(floatZero) == 1.0 &&\n"
            "       exp10(floatNegativeZero) == 1.0 &&\n"
            "       exp10(floatMinusInf) == 0.0 &&\n"
            "       exp10(doubleMinusInf) == 0.0 &&\n"
            "       exp10(floatPlusInf) == floatPlusInf &&\n"
            "       exp10(doublePlusInf) == doublePlusInf &&\n"
            "       os_isnan(exp10(floatNanValue1)) &&\n"
            "       os_isnan(exp10(doubleNanValue1)));\n"
            "return 0;}\n",
            os_isnan_definition, computeValues);
    fprintf(versionFile, "#define HAS_EXP10 %d\n",
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS " " SYSTEM_MATH_LIBS) && doTest() == 1);
    sprintf(buffer,
            "#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
            "%s\n"
            "int main(int argc,char *argv[]){\n"
            "%s\n"
            "printf(\"%%d\\n\",\n"
            "       expm1(0.0) == 0.0 &&\n"
            "       expm1(floatZero) == 0.0 &&\n"
            "       expm1(floatNegativeZero) == 0.0 &&\n"
            "       expm1(floatMinusInf) == -1.0 &&\n"
            "       expm1(doubleMinusInf) == -1.0 &&\n"
            "       expm1(floatPlusInf) == floatPlusInf &&\n"
            "       expm1(doublePlusInf) == doublePlusInf &&\n"
            "       os_isnan(expm1(floatNanValue1)) &&\n"
            "       os_isnan(expm1(doubleNanValue1)));\n"
            "return 0;}\n",
            os_isnan_definition, computeValues);
    fprintf(versionFile, "#define HAS_EXPM1 %d\n",
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS " " SYSTEM_MATH_LIBS) && doTest() == 1);
    sprintf(buffer,
            "#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
            "%s\n"
            "int main(int argc,char *argv[]){\n"
            "%s\n"
            "printf(\"%%d\\n\",\n"
            "       os_isnan(sqrt(floatNanValue1)) &&\n"
            "       os_isnan(sqrt(doubleNanValue1)));\n"
            "return 0;}\n",
            os_isnan_definition, computeValues);
    fprintf(versionFile, "#define SQRT_OF_NAN_OKAY %d\n",
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS " " SYSTEM_MATH_LIBS) && doTest() == 1);
    sprintf(buffer,
            "#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
            "%s\n"
            "int main(int argc,char *argv[]){\n"
            "float floatMinusOne = -1.0;\n"
            "float floatMinusTwo = -2.0;\n"
            "double doubleMinusOne = -1.0;\n"
            "double doubleMinusTwo = -2.0;\n"
            "%s\n"
            "printf(\"%%d\\n\",\n"
            "       os_isnan(sqrt(doubleMinusInf)) &&\n"
            "       os_isnan(sqrt(floatMinusInf)) &&\n"
            "       os_isnan(sqrt(-1.7976931348623157e308)) &&\n"
            "       os_isnan(sqrt(-3.4028234664e38)) &&\n"
            "       os_isnan(sqrt(-2.0)) &&\n"
            "       os_isnan(sqrt(-1.5)) &&\n"
            "       os_isnan(sqrt(-1.0)) &&\n"
            "       os_isnan(sqrt(-0.5)) &&\n"
            "       os_isnan(sqrt(floatMinusTwo)) &&\n"
            "       os_isnan(sqrt(doubleMinusTwo)) &&\n"
            "       os_isnan(sqrt(floatMinusOne)) &&\n"
            "       os_isnan(sqrt(doubleMinusOne)));\n"
            "return 0;}\n",
            os_isnan_definition, computeValues);
    fprintf(versionFile, "#define SQRT_OF_NEGATIVE_OKAY %d\n",
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS " " SYSTEM_MATH_LIBS) && doTest() == 1);
    sprintf(buffer,
            "#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
            "%s\n"
            "int main(int argc,char *argv[]){\n"
            "%s\n"
            "printf(\"%%d\\n\",\n"
            "       os_isnan(exp(floatNanValue1)) &&\n"
            "       os_isnan(exp(doubleNanValue1)));\n"
            "return 0;}\n",
            os_isnan_definition, computeValues);
    fprintf(versionFile, "#define EXP_OF_NAN_OKAY %d\n",
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS " " SYSTEM_MATH_LIBS) && doTest() == 1);
    sprintf(buffer,
            "#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
            "%s\n"
            "int main(int argc,char *argv[]){\n"
            "%s\n"
            "printf(\"%%d\\n\",\n"
            "       os_isnan(ldexp(doubleNanValue1, -1000)) &&\n"
            "       os_isnan(ldexp(doubleNanValue1,  0)) &&\n"
            "       os_isnan(ldexp(doubleNanValue1, 1000)));\n"
            "return 0;}\n",
            os_isnan_definition, computeValues);
    fprintf(versionFile, "#define LDEXP_OF_NAN_OKAY %d\n",
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS " " SYSTEM_MATH_LIBS) && doTest() == 1);
    sprintf(buffer,
            "#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
            "int main(int argc,char *argv[]){\n"
            "%s\n"
            "printf(\"%%d\\n\",\n"
            "       ldexp(doubleMinusInf, -1000) ==  doubleMinusInf &&\n"
            "       ldexp(doubleMinusInf,     0) ==  doubleMinusInf &&\n"
            "       ldexp(doubleMinusInf,  1000) ==  doubleMinusInf &&\n"
            "       ldexp(doublePlusInf,  -1000) ==  doublePlusInf &&\n"
            "       ldexp(doublePlusInf,      0) ==  doublePlusInf &&\n"
            "       ldexp(doublePlusInf,   1000) ==  doublePlusInf);\n"
            "return 0;}\n",
            computeValues);
    fprintf(versionFile, "#define LDEXP_OF_INFINITY_OKAY %d\n",
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS " " SYSTEM_MATH_LIBS) && doTest() == 1);
    sprintf(buffer,
            "#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
            "%s\n"
            "int main(int argc,char *argv[]){\n"
            "%s\n"
            "printf(\"%%d\\n\",\n"
            "       ldexp(5.777494836628130267e-309,  1071) == 1.461722466756107500e+014 &&\n"
            "       ldexp(2.546209045782271622e-309,  2047) == 4.114292659568633900e+307 &&\n"
            "       ldexp(6.009742445482107613e+258, -1882) == 1.739418242816761100e-308 &&\n"
            "       ldexp(3.094739191357472818e+93,  -1334) == 8.252961878480251800e-309);\n"
            "return 0;}\n",
            os_isnan_definition, computeValues);
    fprintf(versionFile, "#define LDEXP_SUBNORMAL_OKAY %d\n",
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS " " SYSTEM_MATH_LIBS) && doTest() == 1);
    sprintf(buffer,
            "#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
            "%s\n"
            "int main(int argc,char *argv[]){\n"
            "int exponent1 = 999999;\n"
            "int exponent2 = 999999;\n"
            "int exponent3 = 999999;\n"
            "int exponent4 = 999999;\n"
            "int exponent5 = 999999;\n"
            "int exponent6 = 999999;\n"
            "int frexp_okay = 1;\n"
            "%s\n"
            "if (frexp(floatMinusInf,  &exponent1) != doubleMinusInf ||\n"
            "    frexp(doubleMinusInf, &exponent2) != doubleMinusInf ||\n"
            "    frexp(floatPlusInf,   &exponent3) != doublePlusInf  ||\n"
            "    frexp(doublePlusInf,  &exponent4) != doublePlusInf  ||\n"
            "    !os_isnan(frexp(floatNanValue1,  &exponent5)) ||\n"
            "    !os_isnan(frexp(doubleNanValue1, &exponent6))) {\n"
            "  frexp_okay = 0;\n"
            "}\n"
            "if (exponent1 != 0 || exponent2 != 0 ||\n"
            "    exponent3 != 0 || exponent4 != 0 ||\n"
            "    exponent5 != 0 || exponent6 != 0) {\n"
            "  frexp_okay = 0;\n"
            "}\n"
            "printf(\"%%d\\n\", frexp_okay);\n"
            "return 0;}\n",
            os_isnan_definition, computeValues);
    fprintf(versionFile, "#define FREXP_INFINITY_NAN_OKAY %d\n",
            compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS " " SYSTEM_MATH_LIBS) ? doTest() : 0);
    sprintf(buffer,
            "#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
            "int main(int argc,char *argv[]){\n"
            "int exponent1 = 999999;\n"
            "int exponent2 = 999999;\n"
            "int frexp_okay = 1;\n"
            "if (frexp(5.0e-324,                &exponent1) != 0.5 ||\n"
            "    frexp(2.2250738585072009e-308, &exponent2) != 0.9999999999999998) {\n"
            "  frexp_okay = 0;\n"
            "}\n"
            "if (exponent1 != -1073 || exponent2 != -1022) {\n"
            "  frexp_okay = 0;\n"
            "}\n"
            "printf(\"%%d\\n\", frexp_okay);\n"
            "return 0;}\n");
    fprintf(versionFile, "#define FREXP_SUBNORMAL_OKAY %d\n",
            compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS " " SYSTEM_MATH_LIBS) ? doTest() : 0);
    sprintf(buffer,
            "#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
            "%s\n"
            "int main(int argc,char *argv[]){\n"
            "%s\n"
            "printf(\"%%d\\n\",\n"
            "       os_isnan(fmod(doubleNanValue1, -13.25)) &&\n"
            "       os_isnan(fmod(doubleNanValue1, 13.25)) &&\n"
            "       os_isnan(fmod(doubleNanValue1, doubleMinusInf)) &&\n"
            "       os_isnan(fmod(doubleNanValue1, doublePlusInf)) &&\n"
            "       os_isnan(fmod(doubleNanValue1, doubleNanValue1)));\n"
            "return 0;}\n",
            os_isnan_definition, computeValues);
    fprintf(versionFile, "#define FMOD_DIVIDEND_NAN_OKAY %d\n",
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS " " SYSTEM_MATH_LIBS) && doTest() == 1);
    sprintf(buffer,
            "#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
            "%s\n"
            "int main(int argc,char *argv[]){\n"
            "%s\n"
            "printf(\"%%d\\n\",\n"
            "       os_isnan(fmod(-31.125, doubleNanValue1)) &&\n"
            "       os_isnan(fmod(31.125, doubleNanValue1)) &&\n"
            "       os_isnan(fmod(doubleMinusInf, doubleNanValue1)) &&\n"
            "       os_isnan(fmod(doublePlusInf, doubleNanValue1)) &&\n"
            "       os_isnan(fmod(doubleNanValue1, doubleNanValue1)));\n"
            "return 0;}\n",
            os_isnan_definition, computeValues);
    fprintf(versionFile, "#define FMOD_DIVISOR_NAN_OKAY %d\n",
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS " " SYSTEM_MATH_LIBS) && doTest() == 1);
    sprintf(buffer,
            "#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
            "%s\n"
            "int main(int argc,char *argv[]){\n"
            "%s\n"
            "printf(\"%%d\\n\",\n"
            "       os_isnan(fmod(doubleMinusInf, -13.25)) &&\n"
            "       os_isnan(fmod(doublePlusInf, -13.25)) &&\n"
            "       os_isnan(fmod(doubleMinusInf, 13.25)) &&\n"
            "       os_isnan(fmod(doublePlusInf, 13.25)) &&\n"
            "       os_isnan(fmod(doubleMinusInf, doubleMinusInf)) &&\n"
            "       os_isnan(fmod(doubleMinusInf, doublePlusInf)) &&\n"
            "       os_isnan(fmod(doublePlusInf, doubleMinusInf)) &&\n"
            "       os_isnan(fmod(doublePlusInf, doublePlusInf)));\n"
            "return 0;}\n",
            os_isnan_definition, computeValues);
    fprintf(versionFile, "#define FMOD_DIVIDEND_INFINITY_OKAY %d\n",
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS " " SYSTEM_MATH_LIBS) && doTest() == 1);
    sprintf(buffer,
            "#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
            "%s\n"
            "int main(int argc,char *argv[]){\n"
            "%s\n"
            "printf(\"%%d\\n\",\n"
            "       fmod(-31.125, doubleMinusInf) == -31.125 &&\n"
            "       fmod(-31.125, doublePlusInf) == -31.125 &&\n"
            "       fmod(31.125, doubleMinusInf) == 31.125 &&\n"
            "       fmod(31.125, doublePlusInf) == 31.125 &&\n"
            "       os_isnan(fmod(doubleMinusInf, doubleMinusInf)) &&\n"
            "       os_isnan(fmod(doubleMinusInf, doublePlusInf)) &&\n"
            "       os_isnan(fmod(doublePlusInf, doubleMinusInf)) &&\n"
            "       os_isnan(fmod(doublePlusInf, doublePlusInf)));\n"
            "return 0;}\n",
            os_isnan_definition, computeValues);
    fprintf(versionFile, "#define FMOD_DIVISOR_INFINITY_OKAY %d\n",
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS " " SYSTEM_MATH_LIBS) && doTest() == 1);
    sprintf(buffer,
            "#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
            "%s\n"
            "int main(int argc,char *argv[]){\n"
            "%s\n"
            "printf(\"%%d\\n\",\n"
            "       os_isnan(fmod(-31.125, 0.0)) &&\n"
            "       os_isnan(fmod(0.0, 0.0)) &&\n"
            "       os_isnan(fmod(31.125, 0.0)) &&\n"
            "       os_isnan(fmod(doubleMinusInf, 0.0)) &&\n"
            "       os_isnan(fmod(doublePlusInf, 0.0)));\n"
            "return 0;}\n",
            os_isnan_definition, computeValues);
    fprintf(versionFile, "#define FMOD_DIVISOR_ZERO_OKAY %d\n",
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS " " SYSTEM_MATH_LIBS) && doTest() == 1);
    sprintf(buffer,
            "#include<stdio.h>\n#include<string.h>\n"
            "#include<float.h>\n#include<math.h>\n"
            "%s\n"
            "int doubleCompare (double num1, double num2){\n"
            "return memcmp(&num1, &num2, sizeof(double));}\n"
            "double getMaxOddFloat(int exponent){\n"
            "double base = (double) FLT_RADIX;\n"
            "double power;\n"
            "for (power=1.0; exponent>0; exponent--) power*=base;\n"
            "if ((FLT_RADIX & 1) == 0) power = floor(power-1.0);\n"
            "return power;}\n"
            "int main(int argc,char *argv[]){\n"
            "float floatOne = 1.0;\n"
            "float floatTwo = 2.0;\n"
            "double doubleOne = 1.0;\n"
            "double doubleTwo = 2.0;\n"
            "double doubleMinusOne = -1.0;\n"
            "double doubleMinusTwo = 2.0;\n"
            "%s"
            "printf(\"#define FLOAT_ZERO_TIMES_INFINITE_OKAY %%d\\n\",\n"
            "    os_isnan(0.0 * floatPlusInf) &&\n"
            "    os_isnan(0.0 * floatMinusInf) &&\n"
            "    os_isnan(floatPlusInf * 0.0) &&\n"
            "    os_isnan(floatMinusInf * 0.0) &&\n"
            "    os_isnan(0.0 * doublePlusInf) &&\n"
            "    os_isnan(0.0 * doubleMinusInf) &&\n"
            "    os_isnan(doublePlusInf * 0.0) &&\n"
            "    os_isnan(doubleMinusInf * 0.0));\n"
            "printf(\"#define NAN_MULTIPLICATION_OKAY %%d\\n\",\n"
            "    os_isnan(floatNanValue1 * 0.0) &&\n"
            "    os_isnan(floatNanValue1 * 1.0) &&\n"
            "    os_isnan(floatNanValue1 * floatOne) &&\n"
            "    os_isnan(0.0 * floatNanValue1) &&\n"
            "    os_isnan(1.0 * floatNanValue1) &&\n"
            "    os_isnan(floatOne * floatNanValue1) &&\n"
            "    os_isnan(floatNanValue1 * floatNanValue2) &&\n"
            "    os_isnan(doubleNanValue1 * 0.0) &&\n"
            "    os_isnan(doubleNanValue1 * 1.0) &&\n"
            "    os_isnan(doubleNanValue1 * doubleOne) &&\n"
            "    os_isnan(0.0 * doubleNanValue1) &&\n"
            "    os_isnan(1.0 * doubleNanValue1) &&\n"
            "    os_isnan(doubleOne * doubleNanValue1) &&\n"
            "    os_isnan(doubleNanValue1 * doubleNanValue2));\n"
            "printf(\"#define NAN_DIVISION_OKAY %%d\\n\",\n"
            "    os_isnan(floatNanValue1 / 1.0) &&\n"
            "    os_isnan(floatNanValue1 / floatOne) &&\n"
            "    os_isnan(1.0 / floatNanValue1) &&\n"
            "    os_isnan(floatOne / floatNanValue1) &&\n"
            "    os_isnan(floatMinusInf / floatNanValue1) &&\n"
            "    os_isnan(floatPlusInf / floatNanValue1) &&\n"
            "    os_isnan(floatNanValue1 / floatNanValue2) &&\n"
            "    os_isnan(doubleNanValue1 / 1.0) &&\n"
            "    os_isnan(doubleNanValue1 / doubleOne) &&\n"
            "    os_isnan(1.0 / doubleNanValue1) &&\n"
            "    os_isnan(doubleOne / doubleNanValue1) &&\n"
            "    os_isnan(doubleMinusInf / doubleNanValue1) &&\n"
            "    os_isnan(doublePlusInf / doubleNanValue1) &&\n"
            "    os_isnan(doubleNanValue1 / doubleNanValue2));\n"
            "printf(\"#define MAX_ODD_FLOAT %%0.1f\\n\", getMaxOddFloat(FLT_MANT_DIG));\n"
            "printf(\"#define MAX_ODD_DOUBLE %%0.1f\\n\", getMaxOddFloat(DBL_MANT_DIG));\n"
            "printf(\"#define POW_OF_NAN_OKAY %%d\\n\",\n"
            "    os_isnan(pow(floatNanValue1, floatMinusInf)) &&\n"
            "    os_isnan(pow(floatNanValue1, -1.0E37)) &&\n"
            "    os_isnan(pow(floatNanValue1, -1.0)) &&\n"
            "    os_isnan(pow(floatNanValue1, -0.5)) &&\n"
            "    pow(floatNanValue1, 0.0) == 1.0 &&\n"
            "    pow(floatNanValue1, floatZero) == 1.0 &&\n"
            "    os_isnan(pow(floatNanValue1, 0.5)) &&\n"
            "    os_isnan(pow(floatNanValue1, 1.0)) &&\n"
            "    os_isnan(pow(floatNanValue1, 1.0E37)) &&\n"
            "    os_isnan(pow(floatNanValue1, floatPlusInf)) &&\n"
            "    os_isnan(pow(doubleNanValue1, doubleMinusInf)) &&\n"
            "    os_isnan(pow(doubleNanValue1, -1.0E37)) &&\n"
            "    os_isnan(pow(doubleNanValue1, -1.0)) &&\n"
            "    os_isnan(pow(doubleNanValue1, -0.5)) &&\n"
            "    pow(doubleNanValue1, 0.0) == 1.0 &&\n"
            "    pow(doubleNanValue1, doubleZero) == 1.0 &&\n"
            "    os_isnan(pow(doubleNanValue1, 0.5)) &&\n"
            "    os_isnan(pow(doubleNanValue1, 1.0)) &&\n"
            "    os_isnan(pow(doubleNanValue1, 1.0E37)) &&\n"
            "    os_isnan(pow(doubleNanValue1, doublePlusInf)));\n"
            "printf(\"#define POW_OF_ZERO_OKAY %%d\\n\",\n"
            "    pow(floatZero, -1.0) == floatPlusInf &&\n"
            "    pow(floatZero, -2.0) == floatPlusInf &&\n"
            "    pow(floatNegativeZero, -1.0) == floatMinusInf &&\n"
            "    pow(floatNegativeZero, -2.0) == floatPlusInf &&\n"
            "    pow(floatNegativeZero, -getMaxOddFloat(FLT_MANT_DIG)) == floatMinusInf &&\n"
            "    pow(doubleZero, -1.0) == doublePlusInf &&\n"
            "    pow(doubleZero, -2.0) == doublePlusInf &&\n"
            "    pow(doubleZero, doubleMinusOne) == doublePlusInf &&\n"
            "    pow(doubleZero, doubleMinusTwo) == doublePlusInf &&\n"
            "    pow(doubleNegativeZero, -1.0) == doubleMinusInf &&\n"
            "    pow(doubleNegativeZero, -2.0) == doublePlusInf &&\n"
            "    pow(doubleNegativeZero, -getMaxOddFloat(DBL_MANT_DIG)) == doubleMinusInf);\n"
            "printf(\"#define POW_OF_NEGATIVE_OKAY %%d\\n\",\n"
            "    os_isnan(pow(-2.0, -1.5)) &&\n"
            "    os_isnan(pow(-1.5, -0.5)) &&\n"
            "    os_isnan(pow(-1.0, 0.5)) &&\n"
            "    os_isnan(pow(-0.5, 1.5)));\n"
            "printf(\"#define POW_OF_ONE_OKAY %%d\\n\",\n"
            "    pow(1.0, floatNanValue1) == 1.0 &&\n"
            "    pow(1.0, doubleNanValue1) == 1.0 &&\n"
            "    pow(floatOne, floatNanValue1) == 1.0 &&\n"
            "    pow(doubleOne, doubleNanValue1) == 1.0);\n"
            "printf(\"#define POW_EXP_NAN_OKAY %%d\\n\",\n"
            "    os_isnan(pow(doubleMinusInf, doubleNanValue1)) &&\n"
            "    os_isnan(pow(-1.0, doubleNanValue1)) &&\n"
            "    os_isnan(pow(0.0, doubleNanValue1)) &&\n"
            "    os_isnan(pow(doubleZero, doubleNanValue1)) &&\n"
            "    os_isnan(pow(2.0, doubleNanValue1)) &&\n"
            "    os_isnan(pow(doubleTwo, doubleNanValue1)) &&\n"
            "    os_isnan(pow(10.0, doubleNanValue1)) &&\n"
            "    os_isnan(pow(doublePlusInf, doubleNanValue1)));\n"
            "printf(\"#define POW_EXP_MINUS_INFINITY_OKAY %%d\\n\",\n"
            "    pow(2.0, floatMinusInf) == 0.0 &&\n"
            "    pow(floatTwo, floatMinusInf) == 0.0 &&\n"
            "    pow(0.9, floatMinusInf) == floatPlusInf &&\n"
            "    pow(2.0, doubleMinusInf) == 0.0 &&\n"
            "    pow(doubleTwo, doubleMinusInf) == 0.0 &&\n"
            "    pow(0.9, doubleMinusInf) == doublePlusInf);\n"
            "printf(\"#define POW_UNDERFLOW_WITH_SIGN %%d\\n\",\n"
            "    doubleCompare(pow(-2.0, -2147483649.0), doubleNegativeZero) == 0 &&\n"
            "    doubleCompare(pow(-doubleTwo, -2147483649.0), doubleNegativeZero) == 0);\n"
            "printf(\"#define LOG_OF_NAN_OKAY %%d\\n\",\n"
            "    os_isnan(log(floatNanValue1)) &&\n"
            "    os_isnan(log(doubleNanValue1)));\n"
            "printf(\"#define LOG_OF_ZERO_OKAY %%d\\n\",\n"
            "    log(floatZero) == floatMinusInf &&\n"
            "    log(floatNegativeZero) == floatMinusInf &&\n"
            "    log(doubleZero) == doubleMinusInf &&\n"
            "    log(doubleNegativeZero) == doubleMinusInf);\n"
            "printf(\"#define LOG_OF_NEGATIVE_OKAY %%d\\n\",\n"
            "    os_isnan(log(floatMinusInf)) &&\n"
            "    os_isnan(log(doubleMinusInf)) &&\n"
            "    os_isnan(log(-1.0)));\n"
            "printf(\"#define LOG10_OF_NAN_OKAY %%d\\n\",\n"
            "    os_isnan(log10(floatNanValue1)) &&\n"
            "    os_isnan(log10(doubleNanValue1)));\n"
            "printf(\"#define LOG10_OF_ZERO_OKAY %%d\\n\",\n"
            "    log10(floatZero) == floatMinusInf &&\n"
            "    log10(floatNegativeZero) == floatMinusInf &&\n"
            "    log10(doubleZero) == doubleMinusInf &&\n"
            "    log10(doubleNegativeZero) == doubleMinusInf);\n"
            "printf(\"#define LOG10_OF_NEGATIVE_OKAY %%d\\n\",\n"
            "    os_isnan(log10(floatMinusInf)) &&\n"
            "    os_isnan(log10(doubleMinusInf)) &&\n"
            "    os_isnan(log10(-1.0)));\n"
            "#if %d\n"
            "printf(\"#define LOG2_OF_NAN_OKAY %%d\\n\",\n"
            "    os_isnan(log2(floatNanValue1)) &&\n"
            "    os_isnan(log2(doubleNanValue1)));\n"
            "printf(\"#define LOG2_OF_ZERO_OKAY %%d\\n\",\n"
            "    log2(floatZero) == floatMinusInf &&\n"
            "    log2(floatNegativeZero) == floatMinusInf &&\n"
            "    log2(doubleZero) == doubleMinusInf &&\n"
            "    log2(doubleNegativeZero) == doubleMinusInf);\n"
            "printf(\"#define LOG2_OF_NEGATIVE_OKAY %%d\\n\",\n"
            "    os_isnan(log2(floatMinusInf)) &&\n"
            "    os_isnan(log2(doubleMinusInf)) &&\n"
            "    os_isnan(log2(-1.0)));\n"
            "#else\n"
            "fputs(\"#define LOG2_OF_NAN_OKAY 0\\n\", stdout);\n"
            "fputs(\"#define LOG2_OF_ZERO_OKAY 0\\n\", stdout);\n"
            "fputs(\"#define LOG2_OF_NEGATIVE_OKAY 0\\n\", stdout);\n"
            "#endif\n"
            "{ char buffer[1024]; sprintf(buffer, \"%%1.1f\", floatNegativeZero);\n"
            "printf(\"#define PRINTS_NEGATIVE_ZERO %%d\\n\", buffer[0] == '-'); }\n"
            "return 0;}\n", os_isnan_definition, computeValues, has_log2);
    if (assertCompAndLnkWithOptions(buffer, "", SYSTEM_LIBS " " SYSTEM_MATH_LIBS)) {
      testOutputToVersionFile(versionFile);
    } /* if */
    if (compileAndLinkOk("#include<float.h>\n#include<math.h>\nint main(int argc,char *argv[])"
                          "{float f=0.0; isinf(f); return 0;}\n")) {
      fputs("#define os_isinf isinf\n", versionFile);
    } else {
      if (compileAndLinkOk("#include<float.h>\n#include<math.h>\nint main(int argc,char *argv[])"
                           "{float f=0.0; _isinf(f); return 0;}\n")) {
        fputs("#define os_isinf _isinf\n", versionFile);
      } else {
        sprintf(buffer,
                "#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
                "int main(int argc,char *argv[]){\n"
                "%s"
                "printf(\"%%d\\n\", fabs(floatPlusInf) > DBL_MAX && fabs(floatMinusInf) > DBL_MAX);\n"
                "return 0;}\n", computeValues);
        if (assertCompAndLnk(buffer)) {
          if (doTest() == 1) {
            fputs("#define os_isinf(x) (fabs(x) > DBL_MAX)\n", versionFile);
          } /* if */
        } /* if */
      } /* if */
    } /* if */
    if (compileAndLinkOk("#include<stdlib.h>\n#include<stdio.h>\n#include<float.h>\n#include<signal.h>\n"
                         "void handleSig(int sig){puts(\"2\");exit(0);}\n"
                         "int main(int argc,char *argv[]){\n"
#ifdef TURN_OFF_FP_EXCEPTIONS
                         "_control87(MCW_EM, MCW_EM);\n"
#endif
                         "signal(SIGFPE,handleSig);\nsignal(SIGILL,handleSig);\n"
                         "signal(SIGINT,handleSig);\n"
                         "printf(\"%d\\n\",(int) 1.0E37);return 0;}\n")) {
      testResult = doTest();
      if ((sizeof(int) == 4 && (long) testResult == 2147483647L) ||
          (sizeof(int) == 2 && testResult == 32767)) {
        fputs("#define FLOAT_TO_INT_OVERFLOW_SATURATES\n", versionFile);
      } else if (testResult == 2) {
        fputs("#define FLOAT_TO_INT_OVERFLOW_SIGNALS\n", versionFile);
      } else if (testResult == 0) {
        fputs("#define FLOAT_TO_INT_OVERFLOW_ZERO\n", versionFile);
      } else {
        fprintf(versionFile, "#define FLOAT_TO_INT_OVERFLOW_GARBAGE %d\n", testResult);
      } /* if */
    } /* if */
    strcpy(buffer,
           "#include<stdio.h>\n#include<float.h>\n"
           "double dblPower(double base, int exponent){\n"
           "double power;\n"
           "for(power=1.0;exponent>0;exponent--)power*=base;\n"
           "return power;}\n"
           "int main(int argc,char *argv[]){\n"
           "int floatRadixFactor;\n"
           "double power;\n");
    defineTransferUnions(buffer);
    strcat(buffer,
#ifdef TURN_OFF_FP_EXCEPTIONS
           "_control87(MCW_EM, MCW_EM);\n"
#endif
           "if (FLT_RADIX == 2) floatRadixFactor = 1;\n"
           "else if (FLT_RADIX == 4) floatRadixFactor = 2;\n"
           "else if (FLT_RADIX == 8) floatRadixFactor = 3;\n"
           "else if (FLT_RADIX == 16) floatRadixFactor = 4;\n"
           "power = dblPower((double) FLT_RADIX, FLT_MANT_DIG);\n"
           "printf(\"#define INT_RANGE_IN_FLOAT_MAX %0.0f\\n\", power);\n"
           "printf(\"#define FLOAT_MANTISSA_FACTOR %0.1f\\n\", power);\n"
           "printf(\"#define FLOAT_MANTISSA_SHIFT %u\\n\", "
                   "FLT_MANT_DIG * floatRadixFactor);\n"
           "fltTransfer.f = 1.0;\n"
           "printf(\"#define FLOAT_EXPONENT_OFFSET %d\\n\", "
                   "fltTransfer.i >> (FLT_MANT_DIG * floatRadixFactor - 1));\n"
           "power = dblPower((double) FLT_RADIX, DBL_MANT_DIG);\n"
           "printf(\"#define INT_RANGE_IN_DOUBLE_MAX %0.0f\\n\", power);\n"
           "printf(\"#define DOUBLE_MANTISSA_FACTOR %0.1f\\n\", power);\n"
           "printf(\"#define DOUBLE_MANTISSA_SHIFT %u\\n\", "
                   "DBL_MANT_DIG * floatRadixFactor);\n"
           "dblTransfer.f = 1.0;\n"
           "printf(\"#define DOUBLE_EXPONENT_OFFSET %d\\n\", "
                   "dblTransfer.i >> (DBL_MANT_DIG * floatRadixFactor - 1));\n"
           "return 0;}\n");
    if (assertCompAndLnk(buffer)) {
      testOutputToVersionFile(versionFile);
    } /* if */
    sprintf(buffer,
            "#include<stdio.h>\n#include<float.h>\n"
            "typedef %s int64Type;\n"
            "double minFltValues[] = { -9223372036854773760.0,\n"
            "  -9223372036854774784.0, -9223372036854775808.0 };\n"
            "double maxFltValues[] = { 9223372036854773760.0,\n"
            "   9223372036854774784.0, 9223372036854775808.0 };\n"
            "int64Type minIntValues[] = { -9223372036854773760,\n"
            "  -9223372036854774784, -9223372036854775807-1 };\n"
            "int64Type maxIntValues[] = { 9223372036854773760,\n"
            "   9223372036854774784, 9223372036854775807 };\n"
            "int main(int argc,char *argv[]){\n"
            "int64Type minimumTruncArgument;\n"
            "int64Type maximumTruncArgument;\n"
            "int pos;\n"
#ifdef TURN_OFF_FP_EXCEPTIONS
            "_control87(MCW_EM, MCW_EM);\n"
#endif
            "for (pos = 0; pos < 3; pos++) {\n"
            "  if ((int64Type) minFltValues[pos] == minIntValues[pos]) {\n"
            "    minimumTruncArgument = minIntValues[pos];\n"
            "  }\n"
            "}\n"
            "printf(\"#define MINIMUM_TRUNC_ARGUMENT %%%sd\\n\",\n"
            "       minimumTruncArgument);\n"
            "for (pos = 0; pos < 3; pos++) {\n"
            "  if ((int64Type) maxFltValues[pos] == maxIntValues[pos]) {\n"
            "    maximumTruncArgument = maxIntValues[pos];\n"
            "  }\n"
            "}\n"
            "printf(\"#define MAXIMUM_TRUNC_ARGUMENT %%%sd\\n\",\n"
            "       maximumTruncArgument);\n"
            "return 0;}\n",
            int64TypeStri, int64TypeFormat, int64TypeFormat);
    if (assertCompAndLnk(buffer)) {
      testOutputToVersionFile(versionFile);
    } /* if */
#ifdef LIMIT_PRINTF_MAXIMUM_FLOAT_PRECISION
    fprintf(versionFile, "#define LIMIT_FMT_E_MAXIMUM_FLOAT_PRECISION \"%d\"\n",
            LIMIT_PRINTF_MAXIMUM_FLOAT_PRECISION);
    fprintf(versionFile, "#define PRINTF_FMT_E_MAXIMUM_FLOAT_PRECISION %d\n",
            LIMIT_PRINTF_MAXIMUM_FLOAT_PRECISION);
    fprintf(versionFile, "#define LIMIT_FMT_F_MAXIMUM_FLOAT_PRECISION \"%d\"\n",
            LIMIT_PRINTF_MAXIMUM_FLOAT_PRECISION);
    fprintf(versionFile, "#define PRINTF_FMT_F_MAXIMUM_FLOAT_PRECISION %d\n",
            LIMIT_PRINTF_MAXIMUM_FLOAT_PRECISION);
#else
    if (assertCompAndLnk("#include<stdio.h>\n#include<string.h>\n"
                         "char buffer[100010];\n"
                         "int main(int argc,char *argv[]){\n"
                         "sprintf(buffer, \"%1.100000e\", 1.0);\n"
                         "printf(\"%lu\\n\", (unsigned long) (strchr(buffer, 'e') - buffer));\n"
                         "return 0;}\n")) {
      testResult = doTest();
      if (testResult == -1) {
        /* The test program crashed. Assume a low precision limit. */
        fputs("#define LIMIT_FMT_E_MAXIMUM_FLOAT_PRECISION \"100\"\n", versionFile);
        fputs("#define PRINTF_FMT_E_MAXIMUM_FLOAT_PRECISION 100\n", versionFile);
      } else if (testResult >= 2 && testResult < 100002) {
        testResult -= 2;
#ifdef PRINTF_MAXIMUM_FLOAT_PRECISION
        fprintf(versionFile, "/* PRINTF_FMT_E_MAXIMUM_FLOAT_PRECISION %d */\n", testResult);
        testResult = PRINTF_MAXIMUM_FLOAT_PRECISION;
#endif
        fprintf(versionFile, "#define PRINTF_FMT_E_MAXIMUM_FLOAT_PRECISION %d\n", testResult);
      } /* if */
    } /* if */
    if (assertCompAndLnk("#include<stdio.h>\n#include<string.h>\n"
                         "char buffer[100010];\n"
                         "int main(int argc,char *argv[]){\n"
                         "sprintf(buffer, \"%1.100000f\", 1.0);\n"
                         "printf(\"%lu\\n\", (unsigned long) strlen(buffer));\n"
                         "return 0;}\n")) {
      testResult = doTest();
      if (testResult == -1) {
        /* The test program crashed. Assume a low precision limit. */
        fputs("#define LIMIT_FMT_F_MAXIMUM_FLOAT_PRECISION \"100\"\n", versionFile);
        fputs("#define PRINTF_FMT_F_MAXIMUM_FLOAT_PRECISION 100\n", versionFile);
      } else if (testResult >= 2 && testResult < 100002) {
        testResult -= 2;
#ifdef PRINTF_MAXIMUM_FLOAT_PRECISION
        fprintf(versionFile, "/* PRINTF_FMT_F_MAXIMUM_FLOAT_PRECISION %d */\n", testResult);
        testResult = PRINTF_MAXIMUM_FLOAT_PRECISION;
#endif
        fprintf(versionFile, "#define PRINTF_FMT_F_MAXIMUM_FLOAT_PRECISION %d\n", testResult);
      } /* if */
    } /* if */
#endif
    if (assertCompAndLnk("#include<stdio.h>\n#include<stdlib.h>\n"
                         "int main(int argc,char *argv[]){\n"
                         "printf(\"%d\\n\", strtod(\"0x123\", NULL) != 0.0);\n"
                         "return 0;}\n")) {
      fprintf(versionFile, "#define STRTOD_ACCEPTS_HEX_NUMBERS %d\n", doTest());
    } /* if */
    if (assertCompAndLnk("#include<stdio.h>\n#include<stdlib.h>\n"
                         "int main(int argc,char *argv[]){\n"
                         "printf(\"%d\\n\", atof(\"0x123\") != 0.0);\n"
                         "return 0;}\n")) {
      fprintf(versionFile, "#define ATOF_ACCEPTS_HEX_NUMBERS %d\n", doTest());
    } /* if */
    if (assertCompAndLnk("#include<stdio.h>\n#include<stdlib.h>\n#include <float.h>\n"
                         "int main(int argc,char *argv[]){\n"
                         "double num = DBL_MIN; char buffer[1024];\n"
                         "sprintf(buffer, \"%1.20e\", num / 2.0);\n"
                         "printf(\"%d\\n\", strtod(buffer, NULL) != 0.0);\n"
                         "return 0;}\n")) {
      fprintf(versionFile, "#define STRTOD_ACCEPTS_DENORMAL_NUMBERS %d\n", doTest());
    } /* if */
    if (assertCompAndLnk("#include<stdio.h>\n#include<stdlib.h>\n#include <float.h>\n"
                         "int main(int argc,char *argv[]){\n"
                         "double num = DBL_MIN; char buffer[1024];\n"
                         "sprintf(buffer, \"%1.20e\", num / 2.0);\n"
                         "printf(\"%d\\n\", atof(buffer) != 0.0);\n"
                         "return 0;}\n")) {
      fprintf(versionFile, "#define ATOF_ACCEPTS_DENORMAL_NUMBERS %d\n", doTest());
    } /* if */
    if (assertCompAndLnk("#include<stdio.h>\n#include<stdlib.h>\n"
                         "#include<string.h>\n#include<float.h>\n"
                         "int main(int argc,char *argv[]){\n"
                         "char buffer[1024];\n"
                         "char *next_ch;\n"
                         "int accepts_plus, accepts_minus;\n"
                         "strcpy(buffer, \"+\");\n"
                         "accepts_plus = strtod(buffer, &next_ch) == 0.0 &&\n"
                         "               next_ch == &buffer[1];\n"
                         "strcpy(buffer, \"-\");\n"
                         "accepts_minus = strtod(buffer, &next_ch) == 0.0 &&\n"
                         "                next_ch == &buffer[1];\n"
                         "printf(\"%d\\n\", accepts_plus || accepts_minus);\n"
                         "return 0;}\n")) {
      fprintf(versionFile, "#define STRTOD_ACCEPTS_SIGN_WITHOUT_DIGITS %d\n", doTest());
    } /* if */
    sprintf(buffer,
            "#include<stdio.h>\n#include<stdlib.h>\n#include<float.h>\n#include<math.h>\n"
            "int main(int argc,char *argv[]){\n"
            "%s\n"
            "printf(\"%%d\\n\", strtod(\"Inf\", NULL) == doublePlusInf &&\n"
            "               strtod(\"+Inf\", NULL) == doublePlusInf &&\n"
            "               strtod(\"-Inf\", NULL) == doubleMinusInf &&\n"
            "               strtod(\"INF\", NULL) == doublePlusInf &&\n"
            "               strtod(\"iNf\", NULL) == doublePlusInf);\n"
            "return 0;}\n", computeValues);
    if (assertCompAndLnk(buffer)) {
      fprintf(versionFile, "#define STRTOD_ACCEPTS_INF %d\n", doTest());
    } /* if */
    sprintf(buffer,
            "#include<stdio.h>\n#include<stdlib.h>\n#include<string.h>\n"
            "#include<float.h>\n#include<math.h>\n"
            "int main(int argc,char *argv[]){\n"
            "char buffer[1024];\n"
            "char *next_ch;\n"
            "%s\n"
            "strcpy(buffer, \"Infinity\");\n"
            "printf(\"%%d\\n\", strtod(buffer, &next_ch) == doublePlusInf &&\n"
            "               next_ch == &buffer[8] &&\n"
            "               strtod(\"Infinity\", NULL) == doublePlusInf &&\n"
            "               strtod(\"+Infinity\", NULL) == doublePlusInf &&\n"
            "               strtod(\"-Infinity\", NULL) == doubleMinusInf &&\n"
            "               strtod(\"INFINITY\", NULL) == doublePlusInf &&\n"
            "               strtod(\"iNfInItY\", NULL) == doublePlusInf);\n"
            "return 0;}\n", computeValues);
    if (assertCompAndLnk(buffer)) {
      fprintf(versionFile, "#define STRTOD_ACCEPTS_INFINITY %d\n", doTest());
    } /* if */
    sprintf(buffer,
            "#include<stdio.h>\n#include<stdlib.h>\n#include<float.h>\n#include<math.h>\n"
            "%s\n"
            "int main(int argc,char *argv[]){\n"
            "%s\n"
            "printf(\"%%d\\n\", os_isnan(strtod(\"NaN\", NULL)) &&\n"
            "               os_isnan(strtod(\"+NaN\", NULL)) &&\n"
            "               os_isnan(strtod(\"-NaN\", NULL)) &&\n"
            "               os_isnan(strtod(\"NAN\", NULL)) &&\n"
            "               os_isnan(strtod(\"nAn\", NULL)));\n"
            "return 0;}\n", os_isnan_definition, computeValues);
    if (assertCompAndLnk(buffer)) {
      fprintf(versionFile, "#define STRTOD_ACCEPTS_NAN %d\n", doTest());
    } /* if */
    fprintf(logFile, " determined\n");
  } /* numericProperties */



static void determineMallocProperties (FILE *versionFile)

  {
    int alignment = -1;
    int mallocOf0ReturnsNull = -1;

  /* determineMallocProperties */
    if (compileAndLinkOk("#include <stdio.h>\n#include <stdlib.h>\n"
                         "static const int alignmentTable[] = {\n"
                         "    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,\n"
                         "    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,\n"
                         "    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,\n"
                         "    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,\n"
                         "  };\n"
                         "int main(int argc, char *argv[])"
                         "{\n"
                         "  int count;\n"
                         "  unsigned long malloc_result;\n"
                         "  int alignment;\n"
                         "  int minAlignment = 7;\n"
                         "  for (count = 1; count <= 64; count++) {\n"
                         "    malloc_result = (unsigned long) (size_t) malloc(count);\n"
                         "    alignment = alignmentTable[malloc_result & 0x3f];\n"
                         "    if (alignment < minAlignment) {\n"
                         "      minAlignment = alignment;\n"
                         "    }\n"
                         "  }\n"
                         "  printf(\"%d\\n\", minAlignment);\n"
                         "  return 0;"
                         "}\n")) {
      alignment = doTest();
    } /* if */
    if (alignment == -1) {
      fprintf(logFile, "\n *** Unable to determine malloc alignment.\n");
    } else {
      fprintf(versionFile, "#define MALLOC_ALIGNMENT %d\n", alignment);
    } /* if */
    if (compileAndLinkOk("#include <stdio.h>\n#include <stdlib.h>\n"
                         "int main(int argc, char *argv[])"
                         "{\n"
                         "  void *malloc_result;\n"
                         "  malloc_result = malloc(0);\n"
                         "  printf(\"%d\\n\", malloc_result == NULL);\n"
                         "  return 0;"
                         "}\n")) {
      mallocOf0ReturnsNull = doTest();
    } /* if */
    if (mallocOf0ReturnsNull == -1) {
      fprintf(logFile, "\n *** Unable to determine result of malloc(0).\n");
    } else {
      fprintf(versionFile, "#define MALLOC_OF_0_RETURNS_NULL %d\n", mallocOf0ReturnsNull);
    } /* if */
  } /* determineMallocProperties */



static void checkForLimitedStringLiteralLength (FILE *versionFile)

  {
    const char *programStart = "#include <stdio.h>\n#include <string.h>\n"
                               "int main(int argc, char *argv[]) {\n"
                               "char ch = '1';\n"
                               "char *chPtr;\n"
                               "char *stri =\n";
    const char *line = "\"12345678901234567890123456789012345678901234567890"
                       "12345678901234567890123456789012345678901234567890\"\n";
    const char *programEnd = ";\n"
                             "for (chPtr = stri; *chPtr != '\\0'; chPtr++) {\n"
                             "  if (*chPtr != ch) {\n"
                             "    printf(\"%u\\n\", (unsigned int) (chPtr - stri));\n"
                             "    return 0;\n"
                             "  }\n"
                             "  ch = ch == '9' ? '0' : ch + 1;\n"
                             "}\n"
                             "printf(\"%u\\n\", strlen(stri));\n"
                             "return 0;\n"
                             "}\n";
    /* The string literal length is repeatCount * charsInLine. */
    /* The definitions below correspond to a string literal length of 100000. */
    const size_t repeatCount = 1000;
    const size_t charsInLine = 100;
    size_t lineLength;
    size_t totalLength;
    size_t count;
    char *buffer;
    char *bufPos;
    int testResult;

  /* checkForLimitedStringLiteralLength */
#ifndef LIMITED_CSTRI_LITERAL_LEN
    lineLength = strlen(line);
    totalLength = strlen(programStart) + lineLength * repeatCount + strlen(programEnd);
    buffer = (char *) malloc((totalLength + 1) * sizeof(char));
    strcpy(buffer, programStart);
    bufPos = &buffer[strlen(buffer)];
    for (count = 1; count <= repeatCount; count++) {
      strcpy(bufPos, line);
      bufPos += lineLength;
    } /* for */
    strcpy(bufPos, programEnd);
    /* printf("\n%s\n", buffer); */
    /* Some C compilers limit the maximum string literal length.   */
    /* There are limits of 2,048 bytes and 16,384 (16K) bytes.     */
    /* Some C compilers accept long string literals, but at        */
    /* run-time the string does not contain the correct data.      */
    /* If LIMITED_CSTRI_LITERAL_LEN is 0 a string literal with     */
    /* repeatCount * charsInLine characters is accepted and works. */
    if (compileAndLinkOk(buffer)) {
      testResult = doTest();
      if (testResult == (int) (repeatCount * charsInLine)) {
        /* At run-time the string literal has the correct value. */
        fputs("#define LIMITED_CSTRI_LITERAL_LEN 0\n", versionFile);
      } else if (testResult != 0 ) {
        /* At run-time the string literal is correct up to a position. */
        fprintf(versionFile, "#define LIMITED_CSTRI_LITERAL_LEN %d\n",
                testResult);
      } else {
        /* At run-time the whole string literal is wrong. */
        fputs("#define LIMITED_CSTRI_LITERAL_LEN -2\n", versionFile);
      } /* if */
    } else {
      /* If the compiler prohibits long string literals. */
      fputs("#define LIMITED_CSTRI_LITERAL_LEN -1\n", versionFile);
    } /* if */
    free(buffer);
#endif
  } /* checkForLimitedStringLiteralLength */



static void checkForLimitedArrayLiteralLength (FILE *versionFile)

  {
    const char *programStart = "#include <stdio.h>\n#include <string.h>\n"
                               "char stri[] = {\n";
    const char *line = "'1','2','3','4','5','6','7','8','9','0',"
                       "'1','2','3','4','5','6','7','8','9','0',\n";
    const char *programEnd = "};\n"
                             "unsigned int doTest (char *stri) {\n"
                             "char ch = '1';\n"
                             "char *chPtr;\n"
                             "for (chPtr = stri; *chPtr != '\\0'; chPtr++) {\n"
                             "  if (*chPtr != ch) {\n"
                             "    return (unsigned int) (chPtr - stri);\n"
                             "  }\n"
                             "  ch = ch == '9' ? '0' : ch + 1;\n"
                             "}\n"
                             "return (unsigned int) strlen(stri);\n"
                             "}\n"
                             "int main(int argc, char *argv[]) {\n"
                             "printf(\"%u\\n\", doTest(stri));\n"
                             "return 0;\n"
                             "}\n";
    /* The array literal length is repeatCount * elementsInLine. */
    /* The definitions below correspond to an array literal length of 100000. */
    const size_t repeatCount = 5000;
    const size_t elementsInLine = 20;
    size_t lineLength;
    size_t totalLength;
    size_t count;
    char *buffer;
    char *bufPos;
    int testResult;

  /* checkForLimitedArrayLiteralLength */
#ifndef LIMITED_ARRAY_LITERAL_LEN
    lineLength = strlen(line);
    totalLength = strlen(programStart) + lineLength * repeatCount + strlen(programEnd);
    buffer = (char *) malloc((totalLength + 1) * sizeof(char));
    strcpy(buffer, programStart);
    bufPos = &buffer[strlen(buffer)];
    for (count = 1; count <= repeatCount; count++) {
      strcpy(bufPos, line);
      bufPos += lineLength;
    } /* for */
    strcpy(bufPos, programEnd);
    /* printf("\n%s\n", buffer); */
    /* Some C compilers limit the maximum array literal length.     */
    /* If LIMITED_ARRAY_LITERAL_LEN is 0 an array literal with      */
    /* repeatCount * elementsInLine elements is accepted and works. */
    if (compileAndLinkOk(buffer)) {
      testResult = doTest();
      if (testResult == (int) (repeatCount * elementsInLine)) {
        /* At run-time the array literal has the correct value. */
        fputs("#define LIMITED_ARRAY_LITERAL_LEN 0\n", versionFile);
      } else if (testResult != 0 ) {
        /* At run-time the array literal is correct up to a position. */
        fprintf(versionFile, "#define LIMITED_ARRAY_LITERAL_LEN %d\n",
                testResult);
      } else {
        /* At run-time the whole array literal is wrong. */
        fputs("#define LIMITED_ARRAY_LITERAL_LEN -2\n", versionFile);
      } /* if */
    } else {
      /* If the compiler prohibits long array literals. */
      fputs("#define LIMITED_ARRAY_LITERAL_LEN -1\n", versionFile);
    } /* if */
    free(buffer);
#endif
  } /* checkForLimitedArrayLiteralLength */



static void checkForSwitchWithInt64Type (FILE *versionFile)

  {
    char buffer[BUFFER_SIZE];

  /* checkForSwitchWithInt64Type */
    sprintf(buffer, "#include<stdio.h>\n"
                    "int main(int argc,char *argv[]){\n"
                    "%s a=1;\n"
                    "switch (a) {\n"
                    "case 1: printf(\"1\\n\");\n"
                    "}return 0;}\n",
                    int64TypeStri);
    fprintf(versionFile, "#define SWITCH_WORKS_FOR_INT64TYPE %d\n",
            compileAndLinkOk(buffer));
  } /* checkForSwitchWithInt64Type */



static void determineStackDirection (FILE *versionFile)

  {
    int stackGrowsUpward;

  /* determineStackDirection */
    if (compileAndLinkOk("#include <stdio.h>\n"
                         "char *stack_base;\n"
                         "void subFunc()\n"
                         "{char localVar;\n"
                         "printf(\"%d\\n\",stack_base<&localVar);return;}\n"
                         "int main(int argc, char *argv[])\n"
                         "{char mainVar;stack_base=&mainVar;\n"
                         "subFunc();return 0;}\n") &&
        (stackGrowsUpward = doTest()) != -1) {
      fprintf(versionFile, "#define STACK_GROWS_UPWARD %d\n", stackGrowsUpward);
    } /* if */
  } /* determineStackDirection */



static void determineLanguageProperties (FILE *versionFile)

  { /* determineLanguageProperties */
    fprintf(versionFile, "#define STMT_BLOCK_IN_PARENTHESES_OK %d\n",
            compileAndLinkOk("#include <stdio.h>\n"
                             "int main (int argc, char *argv[]){\n"
                             "int j = ({int x = 3; x+5;});\n"
                             "printf(\"%d\\n\", j == 8);\n"
                             "return 0;}\n") &&
            doTest() == 1);
  } /* determineLanguageProperties */



static void determinePreprocessorProperties (FILE *versionFile)

  { /* determinePreprocessorProperties */
    if (assertCompAndLnk("#include <stdio.h>\n#include <string.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "printf(\"%d\\n\", strcmp(\"\?\?(\", \"[\") == 0);\n"
                         "return 0;}\n")) {
      fprintf(versionFile, "#define TRIGRAPH_SEQUENCES_ARE_REPLACED %d\n", doTest() == 1);
    } /* if */
    fprintf(versionFile, "#define DIGRAPH_SEQUENCES_ARE_REPLACED %d\n",
        compileAndLinkOk("%:include <stdio.h>\n"
                         "int main (int argc, char *argv<::>)\n"
                         "<%printf(\"1\\n\");return 0;%>\n"));
    fprintf(versionFile, "#define STRINGIFY_WORKS %d\n",
            compileAndLinkOk("#include <stdio.h>\n#include <string.h>\n"
                             "#define STRINGIFY(s) STRINGIFY_HELPER(s)\n"
                             "#define STRINGIFY_HELPER(s) #s\n"
                             "#define TEST1 1\n"
                             "#define TEST2 2 + 3\n"
                             "#define TEST3 TEST1 + TEST2\n"
                             "int main(int argc, char *argv[]){\n"
                             "printf(\"%d\\n\",\n"
                             "       strcmp(\"0\", STRINGIFY(0)) == 0 &&\n"
                             "       strcmp(\"1\", STRINGIFY(TEST1)) == 0 &&\n"
                             "       strcmp(\"2 + 3\", STRINGIFY(TEST2)) == 0 &&\n"
                             "       strcmp(\"1 + 2 + 3\", STRINGIFY(TEST3)) == 0);\n"
                             "return 0;}\n") &&
            doTest() == 1);
    fprintf(versionFile, "#define DIRECTIVES_IN_MACRO_ARGUMENTS_OK %d\n",
            compileAndLinkOk("#include <stdio.h>\n"
                             "#define X(A) A\n"
                             "int x = X(\n"
                             "#line 3 \"test.c\"\n"
                             "0\n"
                             ");\n"
                             "int main(int argc, char *argv[]) {\n"
                             "return 0;}\n"));
  } /* determinePreprocessorProperties */



static void localtimeProperties (FILE *versionFile)

  { /* localtimeProperties */
#ifdef USE_ALTERNATE_LOCALTIME_R
    fputs("#define USE_LOCALTIME_R\n", versionFile);
    fprintf(versionFile, "#define LOCALTIME_WORKS_SIGNED %d\n", isSignedType("time_t"));
#else
    if (compileAndLinkOk("#include<time.h>\nint main(int argc,char *argv[])\n"
                         "{time_t ts;struct tm res;struct tm*lt;\n"
                         "lt=localtime_r(&ts,&res);return 0;}\n")) {
      fputs("#define USE_LOCALTIME_R\n", versionFile);
      if (assertCompAndLnk("#include<stdio.h>\n#include<time.h>\n"
                           "int main(int argc,char *argv[])"
                           "{time_t ts=-2147483647-1;struct tm res;struct tm*lt;\n"
                           "lt=localtime_r(&ts,&res);\n"
                           "printf(\"%d\\n\",lt!=NULL&&lt->tm_year==1);return 0;}\n")) {
        fprintf(versionFile, "#define LOCALTIME_WORKS_SIGNED %d\n", doTest() == 1);
      } /* if */
    } else if (compileAndLinkOk("#include<stdio.h>\n#include<time.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{time_t ts=0;struct tm res;int retval;\n"
                                "retval=localtime_s(&res,&ts);\n"
                                "printf(\"%d\\n\",retval==0);return 0;}\n") && doTest() == 1) {
      fputs("#define USE_LOCALTIME_S\n", versionFile);
      if (assertCompAndLnk("#include<stdio.h>\n#include<time.h>\n"
                           "int main(int argc,char *argv[])"
                           "{time_t ts=-2147483647-1;struct tm res;\n"
                           "printf(\"%d\\n\",localtime_s(&res,&ts)==0&&res.tm_year==1);return 0;}\n")) {
        fprintf(versionFile, "#define LOCALTIME_WORKS_SIGNED %d\n", doTest() == 1);
      } /* if */
    } else if (assertCompAndLnk("#include<stdio.h>\n#include<time.h>\n"
                                "int main(int argc,char *argv[])"
                                "{time_t ts=-2147483647-1;struct tm*lt;\n"
                                "lt = localtime(&ts);\n"
                                "printf(\"%d\\n\",lt!=NULL&&lt->tm_year==1);return 0;}\n")) {
      fprintf(versionFile, "#define LOCALTIME_WORKS_SIGNED %d\n", doTest() == 1);
    } /* if */
#endif
    if (compileAndLinkOk("#include<time.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{time_t ts;struct tm res;struct tm*lt;\n"
                         "lt=gmtime_r(&ts,&res);return 0;}\n")) {
      fputs("#define HAS_GMTIME_R\n", versionFile);
    } else if (compileAndLinkOk("#include<stdio.h>\n#include<time.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{time_t ts=0;struct tm res;int retval;\n"
                                "retval=gmtime_s(&res,&ts);\n"
                                "printf(\"%d\\n\",retval==0);return 0;}\n") && doTest() == 1) {
      fputs("#define HAS_GMTIME_S\n", versionFile);
    } /* if */
  } /* localtimeProperties */



static const char *defineMakeDir (void)

  {
    const char *makeDirDef = NULL;

  /* defineMakeDir */
    if (compileAndLinkOk("#include <stdio.h>\n#include <direct.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{mkdir(\"tmp_empty_dir1\");return 0;}\n")) {
      makeDirDef = "#include <direct.h>\n"
                   "#define makeDir(path,mode) mkdir(path)\n";
    } else if (compileAndLinkOk("#include <stdio.h>\n"
                                "#include <sys/stat.h>\n#include <sys/types.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{mkdir(\"tmp_empty_dir1\");return 0;}\n")) {
      makeDirDef = "#include <sys/stat.h>\n#include <sys/types.h>\n"
                   "#define makeDir(path,mode) mkdir(path)\n";
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <direct.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{_mkdir(\"tmp_empty_dir1\");return 0;}\n")) {
      makeDirDef = "#include <direct.h>\n"
                   "#define makeDir(path,mode) _mkdir(path)\n";
    } else if (compileAndLinkOk("#include <stdio.h>\n"
                                "#include <sys/stat.h>\n#include <sys/types.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{_mkdir(\"tmp_empty_dir1\");return 0;}\n")) {
      makeDirDef = "#include <sys/stat.h>\n#include <sys/types.h>\n"
                   "#define makeDir(path,mode) _mkdir(path)\n";
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <direct.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{mkdir(\"tmp_empty_dir1\", 0755);return 0;}\n")) {
      makeDirDef = "#include <direct.h>\n"
                   "#define makeDir(path,mode) mkdir(path,mode)\n";
    } else if (compileAndLinkOk("#include <stdio.h>\n"
                                "#include <sys/stat.h>\n#include <sys/types.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{mkdir(\"tmp_empty_dir1\", 0755);return 0;}\n")) {
      makeDirDef = "#include <sys/stat.h>\n#include <sys/types.h>\n"
                   "#define makeDir(path,mode) mkdir(path,mode)\n";
    } /* if */
    /* fprintf(logFile, "defineMakeDir -->\n%s\n", makeDirDef); */
    return makeDirDef;
  } /* defineMakeDir */



static const char *defineRemoveDir (void)

  {
    const char *removeDirDef = NULL;

  /* defineRemoveDir */
    if (compileAndLinkOk("#include <stdio.h>\n#include <direct.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{rmdir(\"tmp_empty_dir1\");return 0;}\n")) {
      removeDirDef = "#include <direct.h>\n"
                     "#define removeDir(path) rmdir(path)\n";
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <unistd.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{rmdir(\"tmp_empty_dir1\");return 0;}\n")) {
      removeDirDef = "#include <unistd.h>\n"
                     "#define removeDir(path) rmdir(path)\n";
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <direct.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{_rmdir(\"tmp_empty_dir1\");return 0;}\n")) {
      removeDirDef = "#include <direct.h>\n"
                     "#define removeDir(path) _rmdir(path)\n";
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <unistd.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{_rmdir(\"tmp_empty_dir1\");return 0;}\n")) {
      removeDirDef = "#include <unistd.h>\n"
                     "#define removeDir(path) _rmdir(path)\n";
    } /* if */
    /* fprintf(logFile, "defineRemoveDir -->\n%s\n", removeDirDef); */
    return removeDirDef;
  } /* defineRemoveDir */



static void checkRemoveDir (const char *makeDirDef, FILE *versionFile)

  {
    char buffer[BUFFER_SIZE];

  /* checkRemoveDir */
    sprintf(buffer, "#include <stdio.h>\n#include <unistd.h>\n%s"
                    "int main(int argc,char *argv[])\n"
                    "{int rmFail=0;makeDir(\"tmp_empty_dir1\",0755);\n"
                    "if(remove(\"tmp_empty_dir1\")!=0){rmFail=1;rmdir(\"tmp_empty_dir1\");}\n"
                    "printf(\"%%d\\n\",rmFail);return 0;}\n",
                    makeDirDef);
    if (compileAndLinkOk(buffer)) {
      fprintf(versionFile, "#define REMOVE_FAILS_FOR_EMPTY_DIRS %d\n", doTest() == 1);
    } else {
      sprintf(buffer, "#include <stdio.h>\n#include <direct.h>\n%s"
                      "int main(int argc,char *argv[])\n"
                      "{int rmFail=0;makeDir(\"tmp_empty_dir1\",0755);\n"
                      "if(remove(\"tmp_empty_dir1\")!=0){rmFail=1;rmdir(\"tmp_empty_dir1\");}\n"
                      "printf(\"%%d\\n\",rmFail);return 0;}\n",
                      makeDirDef);
      if (compileAndLinkOk(buffer)) {
        fprintf(versionFile, "#define REMOVE_FAILS_FOR_EMPTY_DIRS %d\n", doTest() == 1);
      } /* if */
    } /* if */
  } /* checkRemoveDir */



static void determineGetaddrlimit (FILE *versionFile)

  {
    int has_getrlimit;

  /* determineGetaddrlimit */
    /* In FreeBSD it is necessary to include <sys/types.h> before <sys/resource.h> */
    has_getrlimit = compileAndLinkOk("#include <stdio.h>\n"
                                     "#include <sys/types.h>\n#include <sys/resource.h>\n"
                                     "int main(int argc, char *argv[]){\n"
                                     "struct rlimit rlim;\n"
                                     "printf(\"%d\\n\", getrlimit(RLIMIT_STACK, &rlim) == 0);\n"
                                     "return 0;}\n") && doTest() == 1;
    fprintf(versionFile, "#define HAS_GETRLIMIT %d\n", has_getrlimit);
    if (has_getrlimit) {
      if (assertCompAndLnk("#include <stdio.h>\n"
                           "#include <sys/types.h>\n#include <sys/resource.h>\n"
                           "int main(int argc, char *argv[]){\n"
                           "struct rlimit rlim;\n"
                           "if (getrlimit(RLIMIT_STACK, &rlim) == 0) {\n"
                           "  if (rlim.rlim_cur == RLIM_INFINITY) {\n"
                           "    printf(\"0\\n\");\n"
                           "  } else {\n"
                           "    printf(\"%d\\n\", (int) (rlim.rlim_cur / 1024));\n"
                           "  }\n"
                           "} else {\n"
                           "  printf(\"-1\\n\");\n"
                           "}\n"
                           "return 0;}\n")) {
        fprintf(versionFile, "#define SOFT_STACK_LIMIT %lu\n", (unsigned long) doTest() * 1024);
      } /* if */
      if (assertCompAndLnk("#include <stdio.h>\n"
                           "#include <sys/types.h>\n#include <sys/resource.h>\n"
                           "int main(int argc, char *argv[]){\n"
                           "struct rlimit rlim;\n"
                           "if (getrlimit(RLIMIT_STACK, &rlim) == 0) {\n"
                           "  if (rlim.rlim_max == RLIM_INFINITY) {\n"
                           "    printf(\"0\\n\");\n"
                           "  } else {\n"
                           "    printf(\"%d\\n\", (int) (rlim.rlim_max / 1024));\n"
                           "  }\n"
                           "} else {\n"
                           "  printf(\"-1\\n\");\n"
                           "}\n"
                           "return 0;}\n")) {
        fprintf(versionFile, "#define HARD_STACK_LIMIT %lu\n", (unsigned long) doTest() * 1024);
      } /* if */
    } /* if */
  } /* determineGetaddrlimit */



static void determineGrpAndPwFunctions (FILE *versionFile)

  {
    char group0Name[NAME_SIZE];
    char user0Name[NAME_SIZE];
    char buffer[BUFFER_SIZE];

  /* determineGrpAndPwFunctions */
    fprintf(versionFile, "#define HAS_GETGRGID_R %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <sys/types.h>\n"
                         "#include <grp.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "int funcRes; struct group grp;\n"
                         "char buffer[2048]; struct group *grpResult;\n"
                         "funcRes = getgrgid_r((gid_t) 0, &grp,\n"
                         "buffer, sizeof(buffer), &grpResult);\n"
                         "printf(\"%d\\n\", funcRes==0 && grpResult==&grp);\n"
                         "return 0;}\n") && doTest() == 1);
    fprintf(versionFile, "#define HAS_GETGRGID %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <sys/types.h>\n"
                         "#include <grp.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "struct group *grpResult;\n"
                         "grpResult = getgrgid((gid_t) 0);\n"
                         "printf(\"%d\\n\", grpResult!=NULL);\n"
                         "return 0;}\n") && doTest() == 1);
    group0Name[0] = '\0';
    if (compileAndLinkOk("#include <stdio.h>\n#include <sys/types.h>\n"
                         "#include <grp.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "struct group *grpResult;\n"
                         "grpResult = getgrgid((gid_t) 0);\n"
                         "if (grpResult!=NULL) {\n"
                         "  printf(\"%s\", grpResult->gr_name);\n"
                         "}\n"
                         "printf(\"\\n\"); return 0;}\n")) {
      testOutputToBuffer(group0Name, sizeof(group0Name));
    } /* if */
    fprintf(versionFile, "#define GROUP_0_NAME \"%s\"\n", group0Name);
    sprintf(buffer, "#include <stdio.h>\n#include <sys/types.h>\n"
                    "#include <grp.h>\n"
                    "int main(int argc, char *argv[]){\n"
                    "int funcRes; struct group grp;\n"
                    "char buffer[2048]; struct group *grpResult;\n"
                    "funcRes = getgrnam_r(\"%s\", &grp,\n"
                    "buffer, sizeof(buffer), &grpResult);\n"
                    "printf(\"%%d\\n\", funcRes==0 && grpResult==&grp);\n"
                    "return 0;}\n", group0Name);
    fprintf(versionFile, "#define HAS_GETGRNAM_R %d\n",
            compileAndLinkOk(buffer) && doTest() == 1);
    sprintf(buffer, "#include <stdio.h>\n#include <sys/types.h>\n"
                    "#include <grp.h>\n"
                    "int main(int argc, char *argv[]){\n"
                    "struct group *grpResult;\n"
                    "grpResult = getgrnam(\"%s\");\n"
                    "printf(\"%%d\\n\", grpResult!=NULL);\n"
                    "return 0;}\n", group0Name);
    fprintf(versionFile, "#define HAS_GETGRNAM %d\n",
            compileAndLinkOk(buffer) && doTest() == 1);
    fprintf(versionFile, "#define HAS_GETPWUID_R %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <sys/types.h>\n"
                         "#include <pwd.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "int funcRes; struct passwd pwd;\n"
                         "char buffer[2048]; struct passwd *pwdResult;\n"
                         "funcRes = getpwuid_r((uid_t) 0, &pwd,\n"
                         "    buffer, sizeof(buffer), &pwdResult);\n"
                         "printf(\"%d\\n\", funcRes==0 && pwdResult==&pwd &&\n"
                         "    pwdResult->pw_uid != -1);\n"
                         "return 0;}\n") && doTest() == 1);
    fprintf(versionFile, "#define HAS_GETPWUID %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <sys/types.h>\n"
                         "#include <pwd.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "struct passwd *pwdResult;\n"
                         "pwdResult = getpwuid((uid_t) 0);\n"
                         "printf(\"%d\\n\", pwdResult!=NULL && pwdResult->pw_uid != -1);\n"
                         "return 0;}\n") && doTest() == 1);
    user0Name[0] = '\0';
    if (compileAndLinkOk("#include <stdio.h>\n#include <sys/types.h>\n"
                         "#include <pwd.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "struct passwd *pwdResult;\n"
                         "pwdResult = getpwuid((uid_t) 0);\n"
                         "if (pwdResult!=NULL) {\n"
                         "  printf(\"%s\", pwdResult->pw_name);\n"
                         "}\n"
                         "printf(\"\\n\"); return 0;}\n")) {
      testOutputToBuffer(user0Name, sizeof(user0Name));
    } /* if */
    fprintf(versionFile, "#define USER_0_NAME \"%s\"\n", user0Name);
    sprintf(buffer, "#include <stdio.h>\n#include <sys/types.h>\n"
                    "#include <pwd.h>\n"
                    "int main(int argc, char *argv[]){\n"
                    "int funcRes; struct passwd pwd;\n"
                    "char buffer[2048]; struct passwd *pwdResult;\n"
                    "funcRes = getpwnam_r(\"%s\", &pwd,\n"
                    "    buffer, sizeof(buffer), &pwdResult);\n"
                    "printf(\"%%d\\n\", funcRes==0 && pwdResult==&pwd &&\n"
                    "    pwdResult->pw_uid != -1);\n"
                    "return 0;}\n", user0Name);
    fprintf(versionFile, "#define HAS_GETPWNAM_R %d\n",
            compileAndLinkOk(buffer) && doTest() == 1);
    sprintf(buffer, "#include <stdio.h>\n#include <sys/types.h>\n"
                    "#include <pwd.h>\n"
                    "int main(int argc, char *argv[]){\n"
                    "struct passwd *pwdResult;\n"
                    "pwdResult = getpwnam(\"%s\");\n"
                    "printf(\"%%d\\n\", pwdResult!=NULL && pwdResult->pw_uid != -1);\n"
                    "return 0;}\n", user0Name);
    fprintf(versionFile, "#define HAS_GETPWNAM %d\n",
            compileAndLinkOk(buffer) && doTest() == 1);
  } /* determineGrpAndPwFunctions */



static void determineSocketLib (FILE *versionFile)

  { /* determineSocketLib */
    if (compileAndLinkOk("#include <sys/types.h>\n"
                         "#include <sys/socket.h>\n"
                         "#include <netdb.h>\n"
                         "#include <netinet/in.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{int sock = socket(AF_INET, SOCK_STREAM, 0);\n"
                         "return 0;}\n")) {
      fputs("#define UNIX_SOCKETS 1\n", versionFile);
      fputs("#define SOCKET_LIB UNIX_SOCKETS\n", versionFile);
    } else if (compileAndLinkWithOptionsOk("#include <winsock2.h>\n"
                                           "int main(int argc,char *argv[])\n"
                                           "{unsigned int sock;\n"
                                           "sock = socket(AF_INET, SOCK_STREAM, 0);\n"
                                           "closesocket(sock); return 0;}\n",
                                           "", SYSTEM_LIBS)) {
      fputs("#define WINSOCK_SOCKETS 2\n", versionFile);
      fputs("#define SOCKET_LIB WINSOCK_SOCKETS\n", versionFile);
    } else {
      fputs("#define NO_SOCKETS -1\n", versionFile);
      fputs("#define SOCKET_LIB NO_SOCKETS\n", versionFile);
    } /* if */
    fprintf(versionFile, "#define HAS_GETADDRINFO %d\n",
        compileAndLinkOk("#include <sys/types.h>\n"
                         "#include <sys/socket.h>\n"
                         "#include <netdb.h>\n"
                         "int main(int argc,char *argv[]){\n"
                         "struct addrinfo *res;\n"
                         "struct addrinfo hints;\n"
                         "getaddrinfo(\"localhost\", \"80\", &hints, &res);\n"
                         "return 0;}\n") ||
        compileAndLinkWithOptionsOk("#include <winsock2.h>\n"
                                    "#include <ws2tcpip.h>\n"
                                    "int main(int argc,char *argv[]){\n"
                                    "struct addrinfo *res;\n"
                                    "struct addrinfo hints;\n"
                                    "getaddrinfo(\"localhost\", \"80\", &hints, &res);\n"
                                    "return 0;}\n",
                                    "", SYSTEM_LIBS));
  } /* determineSocketLib */



static void determineOsDirAccess (FILE *versionFile)

  {
    const char *directory_lib = NULL;
    int lib_number;

  /* determineOsDirAccess */
    if (compileAndLinkOk("#include <stdio.h>\n#include <windows.h>\n"
                         "#include <direct.h>\n"
                         "int main (int argc, char *argv[]) {\n"
                         "HANDLE dirHandle;\n"
                         "WIN32_FIND_DATAW findData;\n"
                         "dirHandle = FindFirstFileW(L\"./*\", &findData);\n"
                         "if (dirHandle != INVALID_HANDLE_VALUE) {\n"
                         "  FindNextFileW(dirHandle, &findData);\n"
                         "  FindClose(dirHandle);\n"
                         "} printf(\"%d\\n\", dirHandle != INVALID_HANDLE_VALUE);\n"
                         "return 0;}\n")) {
      /* This is checked first, because DIRWIN_DIRECTORY */
      /* should be used, even if opendir() is available. */
      directory_lib = "DIRWIN_DIRECTORY";
      lib_number = 4;
      fputs("#define os_DIR WDIR\n", versionFile);
      fputs("#define os_dirent_struct struct wdirent\n", versionFile);
      fputs("#define os_opendir wopendir\n", versionFile);
      fputs("#define os_readdir wreaddir\n", versionFile);
      fputs("#define os_closedir wclosedir\n", versionFile);
      if (!compileAndLinkOk("#include <stdio.h>\n#include <windows.h>\n"
                            "int main (int argc, char *argv[]) {\n"
                            "HANDLE dirHandle;\n"
                            "WIN32_FIND_DATAW findData;\n"
                            "dirHandle = FindFirstFileW(L\"./*\", &findData);\n"
                            "if (dirHandle != INVALID_HANDLE_VALUE) {\n"
                            "  FindNextFileW(dirHandle, &findData);\n"
                            "  FindClose(dirHandle);\n"
                            "} printf(\"%d\\n\", dirHandle != INVALID_HANDLE_VALUE);\n"
                            "return 0;}\n")) {
        fputs("define OS_OPENDIR_INCLUDE_DIRECT_H\n", versionFile);
      } /* if */
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <dirent.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{DIR *directory; struct dirent *dirEntry;\n"
                                "printf(\"%d\\n\", (directory = opendir(\".\")) != NULL &&\n"
                                "(dirEntry = readdir(directory)) != NULL &&\n"
                                "closedir(directory) == 0);\n"
                                "return 0;}\n")) {
      directory_lib = "DIRENT_DIRECTORY";
      lib_number = 1;
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <direct.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{DIR *directory; struct dirent *dirEntry;\n"
                                "printf(\"%d\\n\", (directory = opendir(\".\")) != NULL &&\n"
                                "(dirEntry = readdir(directory)) != NULL &&\n"
                                "closedir(directory) == 0);\n"
                                "return 0;}\n")) {
      directory_lib = "DIRECT_DIRECTORY";
      lib_number = 2;
    } /* if */
    if (directory_lib != NULL) {
      fprintf(versionFile, "#define %s %d\n", directory_lib, lib_number);
      fprintf(versionFile, "#define DIR_LIB %s\n", directory_lib);
    } else {
      fprintf(logFile, "\n *** Cannot define DIR_LIB.\n");
      fputs("#define NO_DIRECTORY -1\n", versionFile);
      fputs("#define DIR_LIB NO_DIRECTORY\n", versionFile);
    } /* if */
  } /* determineOsDirAccess */



static void determineFseekFunctions (FILE *versionFile, const char *fileno)

  {
    int sizeof_off_t;
    int os_off_t_size;
    const char *os_off_t_stri = NULL;
    const char *os_fseek_stri = NULL;
    const char *os_ftell_stri = NULL;
    char buffer[BUFFER_SIZE];

  /* determineFseekFunctions */
#ifndef os_fseek
    if (compileAndLinkOk("#include <stdio.h>\n#include <string.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{FILE *aFile;\n"
                         "int fseek_result = -1;\n"
                         "off_t ftell_result = -1;\n"
                         "char buffer1[10];\n"
                         "char buffer2[10];\n"
                         "aFile = fopen(\"tst_vers.h\", \"rb\");\n"
                         "if (aFile != NULL) {\n"
                         "  fread(buffer1, 1, 10, aFile);\n"
                         "  fseek_result = fseeko(aFile, (off_t) 0, SEEK_SET);\n"
                         "  fread(buffer2, 1, 10, aFile);\n"
                         "  ftell_result = ftello(aFile);\n"
                         "  fclose(aFile);\n"
                         "}\n"
                         "printf(\"%d\\n\", fseek_result == 0 && ftell_result == 10 &&\n"
                         "       memcmp(buffer1, buffer2, 10) == 0);\n"
                         "return 0;}\n") && doTest() == 1) {
      if (compileAndLinkOk("#include <stdio.h>\n"
                           "int main(int argc,char *argv[])\n"
                           "{printf(\"%d\\n\", sizeof(off_t));\n"
                           "return 0;}\n")) {
        sizeof_off_t = doTest();
        if (sizeof_off_t == 4) {
          if (compileAndLinkOk("#define _FILE_OFFSET_BITS 64\n"
                               "#include <stdio.h>\n"
                               "int main(int argc,char *argv[])\n"
                               "{printf(\"%d\\n\", sizeof(off_t));\n"
                               "return 0;}\n")) {
            sizeof_off_t = doTest();
            if (sizeof_off_t == 8) {
              fputs("#define _FILE_OFFSET_BITS 64\n", versionFile);
            } else {
              sizeof_off_t = 4;
            } /* if */
          } /* if */
        } /* if */
        if (sizeof_off_t == 8) {
          os_off_t_size = 64;
          os_off_t_stri = "off_t";
          os_fseek_stri = "fseeko";
          os_ftell_stri = "ftello";
        } /* if */
      } /* if */
    } /* if */
    if (os_fseek_stri == NULL &&
        compileAndLinkOk("#include <stdio.h>\n#include <string.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{FILE *aFile;\n"
                         "int fseek_result = -1;\n"
                         "off64_t ftell_result = -1;\n"
                         "char buffer1[10];\n"
                         "char buffer2[10];\n"
                         "aFile = fopen(\"tst_vers.h\", \"rb\");\n"
                         "if (aFile != NULL) {\n"
                         "  fread(buffer1, 1, 10, aFile);\n"
                         "  fseek_result = fseeko64(aFile, (off64_t) 0, SEEK_SET);\n"
                         "  fread(buffer2, 1, 10, aFile);\n"
                         "  ftell_result = ftello64(aFile);\n"
                         "  fclose(aFile);\n"
                         "}\n"
                         "printf(\"%d\\n\", fseek_result == 0 && ftell_result == 10 &&\n"
                         "       memcmp(buffer1, buffer2, 10) == 0);\n"
                         "return 0;}\n") && doTest() == 1) {
      os_off_t_size = 64;
      os_off_t_stri = "off64_t";
      os_fseek_stri = "fseeko64";
      os_ftell_stri = "ftello64";
    } /* if */
    if (os_fseek_stri == NULL &&
        compileAndLinkOk("#include <stdio.h>\n#include <string.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{FILE *aFile;\n"
                         "int fseek_result = -1;\n"
                         "__int64 ftell_result = -1;\n"
                         "char buffer1[10];\n"
                         "char buffer2[10];\n"
                         "aFile = fopen(\"tst_vers.h\", \"rb\");\n"
                         "if (aFile != NULL) {\n"
                         "  fread(buffer1, 1, 10, aFile);\n"
                         "  fseek_result = _fseeki64(aFile, (__int64) 0, SEEK_SET);\n"
                         "  fread(buffer2, 1, 10, aFile);\n"
                         "  ftell_result = _ftelli64(aFile);\n"
                         "  fclose(aFile);\n"
                         "}\n"
                         "printf(\"%d\\n\", fseek_result == 0 && ftell_result == 10 &&\n"
                         "       memcmp(buffer1, buffer2, 10) == 0);\n"
                         "return 0;}\n") && doTest() == 1) {
      os_off_t_size = 64;
      os_off_t_stri = "__int64";
      os_fseek_stri = "_fseeki64";
      os_ftell_stri = "_ftelli64";
    } /* if */
    if (os_fseek_stri == NULL) {
      sprintf(buffer,
              "#include <stdio.h>\n#include <string.h>\n"
              "int main(int argc,char *argv[])\n"
              "{FILE *aFile;\n"
              "int fseek_result = -1;\n"
              "%s ftell_result = -1;\n"
              "char buffer1[10];\n"
              "char buffer2[10];\n"
              "aFile = fopen(\"tst_vers.h\", \"rb\");\n"
              "if (aFile != NULL) {\n"
              "  fread(buffer1, 1, 10, aFile);\n"
              "  fseek_result = _fseeki64(aFile, (%s) 0, SEEK_SET);\n"
              "  fread(buffer2, 1, 10, aFile);\n"
              "  ftell_result = _ftelli64(aFile);\n"
              "  fclose(aFile);\n"
              "}\n"
              "printf(\"%%d\\n\", fseek_result == 0 && ftell_result == 10 &&\n"
              "       memcmp(buffer1, buffer2, 10) == 0);\n"
              "return 0;}\n", int64TypeStri, int64TypeStri);
      if (compileAndLinkOk(buffer) && doTest() == 1) {
        os_off_t_size = 64;
        os_off_t_stri = int64TypeStri;
        os_fseek_stri = "_fseeki64";
        os_ftell_stri = "_ftelli64";
      } /* if */
    } /* if */
    if (os_fseek_stri == NULL &&
        compileAndLinkOk("#include <stdio.h>\n#include <string.h>\n"
                         "extern int __cdecl _fseeki64(FILE *, __int64, int);\n"
                         "extern __int64 __cdecl _ftelli64(FILE *);\n"
                         "int main(int argc,char *argv[])\n"
                         "{FILE *aFile;\n"
                         "int fseek_result = -1;\n"
                         "__int64 ftell_result = -1;\n"
                         "char buffer1[10];\n"
                         "char buffer2[10];\n"
                         "aFile = fopen(\"tst_vers.h\", \"rb\");\n"
                         "if (aFile != NULL) {\n"
                         "  fread(buffer1, 1, 10, aFile);\n"
                         "  fseek_result = _fseeki64(aFile, (__int64) 0, SEEK_SET);\n"
                         "  fread(buffer2, 1, 10, aFile);\n"
                         "  ftell_result = _ftelli64(aFile);\n"
                         "  fclose(aFile);\n"
                         "}\n"
                         "printf(\"%d\\n\", fseek_result == 0 && ftell_result == 10 &&\n"
                         "       memcmp(buffer1, buffer2, 10) == 0);\n"
                         "return 0;}\n") && doTest() == 1) {
      fputs("#define DEFINE_FSEEKI64_PROTOTYPE\n", versionFile);
      fputs("#define DEFINE_FTELLI64_PROTOTYPE\n", versionFile);
      os_off_t_size = 64;
      os_off_t_stri = "__int64";
      os_fseek_stri = "_fseeki64";
      os_ftell_stri = "_ftelli64";
    } /* if */
    if (os_fseek_stri == NULL &&
        compileAndLinkOk("#include <stdio.h>\n#include <string.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{FILE *aFile;\n"
                         "int fseek_result = -1;\n"
                         "__int64 ftell_result = -1;\n"
                         "fpos_t pos;\n"
                         "char buffer1[10];\n"
                         "char buffer2[10];\n"
                         "aFile = fopen(\"tst_vers.h\", \"rb\");\n"
                         "if (aFile != NULL) {\n"
                         "  fread(buffer1, 1, 10, aFile);\n"
                         "  fseek_result = _fseeki64(aFile, (__int64) 0, SEEK_SET);\n"
                         "  fread(buffer2, 1, 10, aFile);\n"
                         "  if (fgetpos(aFile, &pos) == 0) {\n"
                         "    memcpy(&ftell_result, &pos, sizeof(ftell_result));\n"
                         "  } else {\n"
                         "    ftell_result = -1;\n"
                         "  }\n"
                         "  fclose(aFile);\n"
                         "}\n"
                         "printf(\"%d\\n\", fseek_result == 0 && ftell_result == 10 &&\n"
                         "       memcmp(buffer1, buffer2, 10) == 0 &&\n"
                         "       sizeof(fpos_t) == sizeof(__int64));\n"
                         "return 0;}\n") && doTest() == 1) {
      fputs("#define DEFINE_FTELLI64_EXT 1\n", versionFile);
      os_off_t_size = 64;
      os_off_t_stri = "__int64";
      os_fseek_stri = "_fseeki64";
      os_ftell_stri = "ftelli64Ext";
    } /* if */
    if (os_fseek_stri == NULL) {
      sprintf(buffer,
              "#include <stdio.h>\n#include <string.h>\n"
              "#include <io.h>\n"
              "int main(int argc,char *argv[])\n"
              "{FILE *aFile;\n"
              "int fseek_result = -1;\n"
              "__int64 ftell_result = -1;\n"
              "fpos_t pos;\n"
              "char buffer1[10];\n"
              "char buffer2[10];\n"
              "aFile = fopen(\"tst_vers.h\", \"rb\");\n"
              "if (aFile != NULL) {\n"
              "  fread(buffer1, 1, 10, aFile);\n"
              "  fseek_result = _fseeki64(aFile, (__int64) 0, SEEK_SET);\n"
              "  fread(buffer2, 1, 10, aFile);\n"
              "  if (fgetpos(aFile, &pos) == 0 && fsetpos(aFile, &pos) == 0) {\n"
              "    ftell_result = _telli64(%s(aFile));\n"
              "  } else {\n"
              "    ftell_result = -1;\n"
              "  }\n"
              "  fclose(aFile);\n"
              "}\n"
              "printf(\"%%d\\n\", fseek_result == 0 && ftell_result == 10 &&\n"
              "       memcmp(buffer1, buffer2, 10) == 0);\n"
              "return 0;}\n", fileno);
      if (compileAndLinkOk(buffer) && doTest() == 1) {
        fputs("#define DEFINE_FTELLI64_EXT 2\n", versionFile);
        os_off_t_size = 64;
        os_off_t_stri = "__int64";
        os_fseek_stri = "_fseeki64";
        os_ftell_stri = "ftelli64Ext";
      } /* if */
    } /* if */
    if (os_fseek_stri == NULL) {
      sprintf(buffer,
              "#include <stdio.h>\n#include <string.h>\n"
              "int main(int argc,char *argv[])\n"
              "{FILE *aFile;\n"
              "int fseek_result = -1;\n"
              "%s ftell_result = -1;\n"
              "char buffer1[10];\n"
              "char buffer2[10];\n"
              "aFile = fopen(\"tst_vers.h\", \"rb\");\n"
              "if (aFile != NULL) {\n"
              "  fread(buffer1, 1, 10, aFile);\n"
              "  fseek_result = _fseek64(aFile, (%s) 0, SEEK_SET);\n"
              "  fread(buffer2, 1, 10, aFile);\n"
              "  ftell_result = _ftell64(aFile);\n"
              "  fclose(aFile);\n"
              "}\n"
              "printf(\"%%d\\n\", fseek_result == 0 && ftell_result == 10 &&\n"
              "       memcmp(buffer1, buffer2, 10) == 0);\n"
              "return 0;}\n", int64TypeStri, int64TypeStri);
      if (compileAndLinkOk(buffer) && doTest() == 1) {
        os_off_t_size = 64;
        os_off_t_stri = int64TypeStri;
        os_fseek_stri = "_fseek64";
        os_ftell_stri = "_ftell64";
      } /* if */
    } /* if */
    if (os_fseek_stri == NULL &&
        compileAndLinkOk("#include <stdio.h>\n#include <string.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{FILE *aFile;\n"
                         "int fseek_result = -1;\n"
                         "long ftell_result = -1;\n"
                         "char buffer1[10];\n"
                         "char buffer2[10];\n"
                         "aFile = fopen(\"ctstfile.txt\", \"wb\");\n"
                         "if (aFile != NULL) {\n"
                         "  fputs(\"abcdefghijklmnopqrstuvwxyz\\n\", aFile);\n"
                         "  fclose(aFile);\n"
                         "}\n"
                         "aFile = fopen(\"ctstfile.txt\", \"rb\");\n"
                         "if (aFile != NULL) {\n"
                         "  fread(buffer1, 1, 10, aFile);\n"
                         "  fseek_result = fseek(aFile, (long) 0, SEEK_SET);\n"
                         "  fread(buffer2, 1, 10, aFile);\n"
                         "  ftell_result = ftell(aFile);\n"
                         "  fclose(aFile);\n"
                         "}\n"
                         "printf(\"%d\\n\", fseek_result == 0 && ftell_result == 10 &&\n"
                         "       memcmp(buffer1, buffer2, 10) == 0);\n"
                         "return 0;}\n") && doTest() == 1) {
      if (compileAndLinkOk("#include <stdio.h>\n"
                           "int main (int argc, char *argv[])\n"
                           "{printf(\"%d\\n\", sizeof(ftell(NULL)));\n"
                           "return 0;}\n") &&
          (sizeof_off_t = doTest()) >= 1) {
        /* The classic fseek() and ftell() functions work for long, */
        /* but some strange implementations use a different type. */
        if (sizeof_off_t == sizeof_long) {
          os_off_t_size = 8 * sizeof_long;
          os_off_t_stri = "long";
        } else if (sizeof_off_t == 2) {
          os_off_t_size = 16;
          os_off_t_stri = int16TypeStri;
        } else if (sizeof_off_t == 4) {
          os_off_t_size = 32;
          os_off_t_stri = int32TypeStri;
        } else if (sizeof_off_t == 8) {
          os_off_t_size = 64;
          os_off_t_stri = int64TypeStri;
        } else if (sizeof_off_t == 16) {
          os_off_t_size = 128;
          os_off_t_stri = int128TypeStri;
        } else {
          fprintf(logFile, "\n *** sizeof(ftell(NULL)) is %d.\n", sizeof_off_t);
        } /* if */
      } else {
        os_off_t_size = 8 * sizeof_long;
        os_off_t_stri = "long";
      } /* if */
      if (os_off_t_stri != NULL) {
        os_fseek_stri = "fseek";
        os_ftell_stri = "ftell";
      } /* if */
    } /* if */
    if (os_fseek_stri != NULL) {
      fprintf(versionFile, "#define OS_OFF_T_SIZE %d\n", os_off_t_size);
      fprintf(versionFile, "#define os_off_t %s\n", os_off_t_stri);
      fprintf(versionFile, "#define os_fseek %s\n", os_fseek_stri);
      fprintf(versionFile, "#define os_ftell %s\n", os_ftell_stri);
      sprintf(buffer,
              "#include <stdio.h>\n#include <string.h>\n"
              "int main(int argc,char *argv[])\n"
              "{FILE *aFile;\n"
              "int eofResult = -1;\n"
              "aFile = fopen(\"tst_vers.h\", \"rb\");\n"
              "if (aFile != NULL) {\n"
              "  while (getc(aFile) != EOF) ;\n"
              "  if (%s(aFile, (%s) 0, SEEK_SET) == 0) {\n"
              "    eofResult = feof(aFile);\n"
              "  }\n"
              "  fclose(aFile);\n"
              "}\n"
              "printf(\"%%d\\n\", eofResult == 0);\n"
              "return 0;}\n", os_fseek_stri, os_off_t_stri);
      fprintf(versionFile, "#define OS_FSEEK_RESETS_EOF_FLAG %d\n",
              compileAndLinkOk(buffer) && doTest() == 1);
    } else {
      fprintf(logFile, "\n *** Cannot define os_fseek and os_ftell.\n");
    } /* if */
    doRemove("ctstfile.txt");
#else
      fprintf(versionFile, "#define OS_OFF_T_SIZE %d\n", 8 * getSizeof("os_off_t"));
#endif
  } /* determineFseekFunctions */



static void determineLseekFunction (FILE *versionFile)

  { /* determineLseekFunction */
    if (compileAndLinkOk("#include <stdio.h>\n#include <io.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{printf(\"%d\\n\", lseeki64(0, 0, SEEK_CUR) != -1);\n"
                         "return 0;}\n")) {
      fputs("#define LSEEK_INCLUDE_IO_H\n", versionFile);
      fprintf(versionFile, "#define os_lseek lseeki64\n");
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <io.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{printf(\"%d\\n\", _lseeki64(0, 0, SEEK_CUR) != -1);\n"
                         "return 0;}\n")) {
      fputs("#define LSEEK_INCLUDE_IO_H\n", versionFile);
      fprintf(versionFile, "#define os_lseek _lseeki64\n");
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <io.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{printf(\"%d\\n\", lseek(0, 0, SEEK_CUR) != -1);\n"
                         "return 0;}\n")) {
      fputs("#define LSEEK_INCLUDE_IO_H\n", versionFile);
      fprintf(versionFile, "#define os_lseek lseek\n");
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <io.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{printf(\"%d\\n\", _lseek(0, 0, SEEK_CUR) != -1);\n"
                         "return 0;}\n")) {
      fputs("#define LSEEK_INCLUDE_IO_H\n", versionFile);
      fprintf(versionFile, "#define os_lseek _lseek\n");
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <sys/types.h>\n"
                         "#include <unistd.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{printf(\"%d\\n\", lseek(0, 0, SEEK_CUR) != -1);\n"
                         "return 0;}\n")) {
      fprintf(versionFile, "#define os_lseek lseek\n");
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <sys/types.h>\n"
                         "#include <unistd.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{printf(\"%d\\n\", _lseek(0, 0, SEEK_CUR) != -1);\n"
                         "return 0;}\n")) {
      fprintf(versionFile, "#define os_lseek _lseek\n");
    } else {
      fprintf(logFile, "\n *** Cannot define os_lseek.\n");
    } /* if */
  } /* determineLseekFunction */



static void determineFtruncate (FILE *versionFile, const char *fileno)

  {
    char buffer[BUFFER_SIZE];
    const char *os_ftruncate_stri = NULL;
    const char *ftruncate_size_in_bits;
    char size_buffer[10];

  /* determineFtruncate */
    sprintf(buffer, "#include <stdio.h>\n#include <unistd.h>\n"
                    "int main(int argc, char *argv[])\n"
                    "{FILE *aFile; int file_no; char buffer[5];\n"
                    "char *stri = NULL; int func_res = -1;\n"
                    "aFile = fopen(\"ctstfile.txt\", \"w+\");\n"
                    "if (aFile != NULL) {\n"
                    "  file_no = %s(aFile);\n"
                    "  if (file_no != -1) {\n"
                    "    fprintf(aFile, \"abcd\");\n"
                    "    fflush(aFile);\n"
                    "    func_res = ftruncate(file_no, 1);\n"
                    "    fseek(aFile, 0, SEEK_SET);\n"
                    "    stri = fgets(buffer, 4, aFile);\n"
                    "  }\n"
                    "  fclose(aFile);\n"
                    "}\n"
                    "printf(\"%%d\\n\", func_res == 0 && stri != NULL &&\n"
                    "       stri[0] == 'a' && stri[1] == '\\0' && sizeof(off_t) == 8);\n"
                    "return 0;}\n", fileno);
    if (compileAndLinkOk(buffer) && doTest() == 1) {
      os_ftruncate_stri = "ftruncate";
      ftruncate_size_in_bits = "OS_OFF_T_SIZE";
    } /* if */
    doRemove("ctstfile.txt");
    if (os_ftruncate_stri == NULL) {
      sprintf(buffer, "#include <stdio.h>\n#include <io.h>\n"
                      "int main(int argc, char *argv[])\n"
                      "{FILE *aFile; int file_no; char buffer[5];\n"
                      "char *stri = NULL; int func_res = -1;\n"
                      "aFile = fopen(\"ctstfile.txt\", \"w+\");\n"
                      "if (aFile != NULL) {\n"
                      "  file_no = %s(aFile);\n"
                      "  if (file_no != -1) {\n"
                      "    fprintf(aFile, \"abcd\");\n"
                      "    fflush(aFile);\n"
                      "    func_res = _chsize_s(file_no, 1);\n"
                      "    fseek(aFile, 0, SEEK_SET);\n"
                      "    stri = fgets(buffer, 4, aFile);\n"
                      "  }\n"
                      "  fclose(aFile);\n"
                      "}\n"
                      "printf(\"%%d\\n\", func_res == 0 && stri != NULL &&\n"
                      "       stri[0] == 'a' && stri[1] == '\\0');\n"
                      "return 0;}\n", fileno);
      if (compileAndLinkOk(buffer) && doTest() == 1) {
        os_ftruncate_stri = "_chsize_s";
        ftruncate_size_in_bits = "64";
        fputs("#define FTRUNCATE_INCLUDE_IO_H\n", versionFile);
      } /* if */
      doRemove("ctstfile.txt");
    } /* if */
    if (os_ftruncate_stri == NULL) {
      sprintf(buffer, "#include <stdio.h>\n"
                      "extern int _chsize_s(int fd, %s size);\n"
                      "int main(int argc, char *argv[])\n"
                      "{FILE *aFile; int file_no; char buffer[5];\n"
                      "char *stri = NULL; int func_res = -1;\n"
                      "aFile = fopen(\"ctstfile.txt\", \"w+\");\n"
                      "if (aFile != NULL) {\n"
                      "  file_no = %s(aFile);\n"
                      "  if (file_no != -1) {\n"
                      "    fprintf(aFile, \"abcd\");\n"
                      "    fflush(aFile);\n"
                      "    func_res = _chsize_s(file_no, 1);\n"
                      "    fseek(aFile, 0, SEEK_SET);\n"
                      "    stri = fgets(buffer, 4, aFile);\n"
                      "  }\n"
                      "  fclose(aFile);\n"
                      "}\n"
                      "printf(\"%%d\\n\", func_res == 0 && stri != NULL &&\n"
                      "       stri[0] == 'a' && stri[1] == '\\0');\n"
                      "return 0;}\n", int64TypeStri, fileno);
      if (compileAndLinkOk(buffer) && doTest() == 1) {
        os_ftruncate_stri = "_chsize_s";
        ftruncate_size_in_bits = "64";
        fputs("#define DEFINE_CHSIZE_S_PROTOTYPE\n", versionFile);
      } /* if */
      doRemove("ctstfile.txt");
    } /* if */
    if (os_ftruncate_stri == NULL) {
      sprintf(buffer, "#include <stdio.h>\n#include <unistd.h>\n"
                      "int main(int argc, char *argv[])\n"
                      "{FILE *aFile; int file_no; char buffer[5];\n"
                      "char *stri = NULL; int func_res = -1;\n"
                      "aFile = fopen(\"ctstfile.txt\", \"w+\");\n"
                      "if (aFile != NULL) {\n"
                      "  file_no = %s(aFile);\n"
                      "  if (file_no != -1) {\n"
                      "    fprintf(aFile, \"abcd\");\n"
                      "    fflush(aFile);\n"
                      "    func_res = ftruncate(file_no, 1);\n"
                      "    fseek(aFile, 0, SEEK_SET);\n"
                      "    stri = fgets(buffer, 4, aFile);\n"
                      "  }\n"
                      "  fclose(aFile);\n"
                      "}\n"
                      "printf(\"%%d\\n\", func_res == 0 && stri != NULL &&\n"
                      "       stri[0] == 'a' && stri[1] == '\\0' && sizeof(off_t) == 4);\n"
                      "return 0;}\n", fileno);
      if (compileAndLinkOk(buffer) && doTest() == 1) {
        os_ftruncate_stri = "ftruncate";
        ftruncate_size_in_bits = "32";
      } /* if */
      doRemove("ctstfile.txt");
    } /* if */
    if (os_ftruncate_stri == NULL) {
      sprintf(buffer, "#include <stdio.h>\n#include <io.h>\n"
                      "int main(int argc, char *argv[])\n"
                      "{FILE *aFile; int file_no; char buffer[5];\n"
                      "char *stri = NULL; int func_res = -1;\n"
                      "aFile = fopen(\"ctstfile.txt\", \"w+\");\n"
                      "if (aFile != NULL) {\n"
                      "  file_no = %s(aFile);\n"
                      "  if (file_no != -1) {\n"
                      "    fprintf(aFile, \"abcd\");\n"
                      "    fflush(aFile);\n"
                      "    func_res = _chsize(file_no, 1);\n"
                      "    fseek(aFile, 0, SEEK_SET);\n"
                      "    stri = fgets(buffer, 4, aFile);\n"
                      "  }\n"
                      "  fclose(aFile);\n"
                      "}\n"
                      "printf(\"%%d\\n\", func_res == 0 && stri != NULL &&\n"
                      "       stri[0] == 'a' && stri[1] == '\\0');\n"
                      "return 0;}\n", fileno);
      if (compileAndLinkOk(buffer) && doTest() == 1) {
        os_ftruncate_stri = "_chsize";
        sprintf(size_buffer, "%d", sizeof_long * 8);
        ftruncate_size_in_bits = size_buffer;
        fputs("#define FTRUNCATE_INCLUDE_IO_H\n", versionFile);
      } /* if */
      doRemove("ctstfile.txt");
    } /* if */
    if (os_ftruncate_stri == NULL) {
      sprintf(buffer, "#include <stdio.h>\n#include <io.h>\n"
                      "int main(int argc, char *argv[])\n"
                      "{FILE *aFile; int file_no; char buffer[5];\n"
                      "char *stri = NULL; int func_res = -1;\n"
                      "aFile = fopen(\"ctstfile.txt\", \"w+\");\n"
                      "if (aFile != NULL) {\n"
                      "  file_no = %s(aFile);\n"
                      "  if (file_no != -1) {\n"
                      "    fprintf(aFile, \"abcd\");\n"
                      "    fflush(aFile);\n"
                      "    func_res = chsize(file_no, 1);\n"
                      "    fseek(aFile, 0, SEEK_SET);\n"
                      "    stri = fgets(buffer, 4, aFile);\n"
                      "  }\n"
                      "  fclose(aFile);\n"
                      "}\n"
                      "printf(\"%%d\\n\", func_res == 0 && stri != NULL &&\n"
                      "       stri[0] == 'a' && stri[1] == '\\0');\n"
                      "return 0;}\n", fileno);
      if (compileAndLinkOk(buffer) && doTest() == 1) {
        os_ftruncate_stri = "chsize";
        sprintf(size_buffer, "%d", sizeof_long * 8);
        ftruncate_size_in_bits = size_buffer;
        fputs("#define FTRUNCATE_INCLUDE_IO_H\n", versionFile);
      } /* if */
      doRemove("ctstfile.txt");
    } /* if */
    if (os_ftruncate_stri == NULL) {
      sprintf(buffer, "#include <stdio.h>\n"
                      "extern int _chsize(int fd, long size);\n"
                      "int main(int argc, char *argv[])\n"
                      "{FILE *aFile; int file_no; char buffer[5];\n"
                      "char *stri = NULL; int func_res = -1;\n"
                      "aFile = fopen(\"ctstfile.txt\", \"w+\");\n"
                      "if (aFile != NULL) {\n"
                      "  file_no = %s(aFile);\n"
                      "  if (file_no != -1) {\n"
                      "    fprintf(aFile, \"abcd\");\n"
                      "    fflush(aFile);\n"
                      "    func_res = _chsize(file_no, 1);\n"
                      "    fseek(aFile, 0, SEEK_SET);\n"
                      "    stri = fgets(buffer, 4, aFile);\n"
                      "  }\n"
                      "  fclose(aFile);\n"
                      "}\n"
                      "printf(\"%%d\\n\", func_res == 0 && stri != NULL &&\n"
                      "       stri[0] == 'a' && stri[1] == '\\0');\n"
                      "return 0;}\n", fileno);
      if (compileAndLinkOk(buffer) && doTest() == 1) {
        os_ftruncate_stri = "_chsize";
        sprintf(size_buffer, "%d", sizeof_long * 8);
        ftruncate_size_in_bits = size_buffer;
        fputs("#define DEFINE_CHSIZE_PROTOTYPE\n", versionFile);
      } /* if */
      doRemove("ctstfile.txt");
    } /* if */
    if (os_ftruncate_stri != NULL) {
      fprintf(versionFile, "#define os_ftruncate %s\n", os_ftruncate_stri);
      fprintf(versionFile, "#define FTRUNCATE_SIZE %s\n", ftruncate_size_in_bits);
    } /* if */
  } /* determineFtruncate */



/**
 *  Determine values for DECLARE_OS_ENVIRON, USE_GET_ENVIRONMENT,
 *  INITIALIZE_OS_ENVIRON, DEFINE_WGETENV, DEFINE_WSETENV, os_environ.
 *  os_getenv, os_setenv, os_putenv and DELETE_PUTENV_ARGUMENT.
 */
static void determineEnvironDefines (FILE *versionFile)

  {
    char buffer[3 * BUFFER_SIZE];
    char getenv_definition[BUFFER_SIZE];
    char setenv_definition[BUFFER_SIZE];
    const char *os_environ_stri = NULL;
    int declare_os_environ = 0;
    int use_get_environment = 0;
    int initialize_os_environ = 0;
    int define_wgetenv = 0;
    int define_wsetenv = 0;
    int define_wunsetenv = 0;
    int test_result;
    int putenv_can_remove_keys = 0;
    int getenv_is_case_sensitive = -1;
    const char *os_getenv_stri = NULL;
    const char *os_putenv_stri = NULL;
    const char *os_setenv_stri = NULL;
    const char *os_unsetenv_stri = NULL;

  /* determineEnvironDefines */
    buffer[0] = '\0';
#ifdef EMULATE_ENVIRONMENT
    os_environ_stri = "environ7";
    declare_os_environ = 1;
#else
#ifdef OS_STRI_WCHAR
    if (compileAndLinkOk("#include <stdio.h>\n#include <stdlib.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{printf(\"%d\\n\", _wenviron != NULL);return 0;}\n")) {
      strcat(buffer, "#include <stdlib.h>\n");
      os_environ_stri = "_wenviron";
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <unistd.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{printf(\"%d\\n\", _wenviron != NULL);return 0;}\n")) {
      strcat(buffer, "#include <unistd.h>\n");
      os_environ_stri = "_wenviron";
    } else if (compileAndLinkOk("#include <stdio.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{extern wchar_t **_wenviron;\n"
                                "printf(\"%d\\n\", _wenviron != NULL);return 0;}\n")) {
      declare_os_environ = 1;
      os_environ_stri = "_wenviron";
#else
    if (compileAndLinkOk("#include <stdio.h>\n#include <stdlib.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{printf(\"%d\\n\", environ != NULL);return 0;}\n")) {
      strcat(buffer, "#include <stdlib.h>\n");
      os_environ_stri = "environ";
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <unistd.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{printf(\"%d\\n\", environ != NULL);return 0;}\n")) {
      strcat(buffer, "#include <unistd.h>\n");
      os_environ_stri = "environ";
    } else if (compileAndLinkOk("#include <stdio.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{extern char **environ;\n"
                                "printf(\"%d\\n\", environ != NULL);return 0;}\n")) {
      declare_os_environ = 1;
      os_environ_stri = "environ";
#endif
    } else {
      use_get_environment = 1;
    } /* if */
    if (!use_get_environment) {
      strcat(buffer, "#include <stdio.h>\n");
      strcat(buffer, "#include \"tst_vers.h\"\n");
#ifdef OS_STRI_WCHAR
      strcat(buffer, "typedef wchar_t *os_striType;\n");
#else
      strcat(buffer, "typedef char *os_striType;\n");
#endif
      if (declare_os_environ) {
        strcat(buffer, "extern os_striType *");
        strcat(buffer, os_environ_stri);
        strcat(buffer, ";\n");
      } /* if */
#ifdef USE_WMAIN
      strcat(buffer, "int wmain(int argc,wchar_t *argv[])");
#else
      strcat(buffer, "int main(int argc,char *argv[])");
#endif
      strcat(buffer, "{printf(\"%d\\n\",");
      strcat(buffer, os_environ_stri);
      strcat(buffer, "==(os_striType *)0);return 0;}\n");
      if (!compileAndLinkOk(buffer) || doTest() == 1) {
        initialize_os_environ = 1;
      } /* if */
    } /* if */
#endif
    if (os_environ_stri != NULL) {
      fprintf(versionFile, "#define os_environ %s\n", os_environ_stri);
    } /* if */
    fprintf(versionFile, "#define DECLARE_OS_ENVIRON %d\n", declare_os_environ);
    fprintf(versionFile, "#define USE_GET_ENVIRONMENT %d\n", use_get_environment);
    fprintf(versionFile, "#define INITIALIZE_OS_ENVIRON %d\n", initialize_os_environ);
#ifdef EMULATE_ENVIRONMENT
    os_getenv_stri = "getenv7";
#elif defined OS_STRI_WCHAR
    if (compileAndLinkOk("#include <stdio.h>\n#include <stdlib.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{printf(\"%d\\n\",\n"
                         "  _wgetenv(L\"PATH\") != NULL ||\n"
                         "  _wgetenv(L\"HOME\") != NULL ||\n"
                         "  _wgetenv(L\"TERM\") != NULL ||\n"
                         "  _wgetenv(L\"LOGNAME\") != NULL ||\n"
                         "  _wgetenv(L\"PROMPT\") != NULL ||\n"
                         "  _wgetenv(L\"HOMEPATH\") != NULL ||\n"
                         "  _wgetenv(L\"USERNAME\") != NULL ||\n"
                         "  _wgetenv(L\"TEMP\") != NULL ||\n"
                         "  _wgetenv(L\"TMP\") != NULL ||\n"
                         "  _wgetenv(L\"OS\") != NULL);\n"
                         "return 0;}\n") && doTest() == 1) {
      os_getenv_stri = "_wgetenv";
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <stdlib.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{printf(\"%d\\n\",\n"
                         "  wgetenv(L\"PATH\") != NULL ||\n"
                         "  wgetenv(L\"HOME\") != NULL ||\n"
                         "  wgetenv(L\"TERM\") != NULL ||\n"
                         "  wgetenv(L\"LOGNAME\") != NULL ||\n"
                         "  wgetenv(L\"PROMPT\") != NULL ||\n"
                         "  wgetenv(L\"HOMEPATH\") != NULL ||\n"
                         "  wgetenv(L\"USERNAME\") != NULL ||\n"
                         "  wgetenv(L\"TEMP\") != NULL ||\n"
                         "  wgetenv(L\"TMP\") != NULL ||\n"
                         "  wgetenv(L\"OS\") != NULL);\n"
                         "return 0;}\n") && doTest() == 1) {
      os_getenv_stri = "wgetenv";
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <stdlib.h>\n"
                                "#include <windows.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{wchar_t buf[4096];\n"
                                "printf(\"%d\\n\",\n"
                                "  GetEnvironmentVariableW(L\"PATH\", buf, 4096) != 0 ||\n"
                                "  GetEnvironmentVariableW(L\"HOME\", buf, 4096) != 0 ||\n"
                                "  GetEnvironmentVariableW(L\"TERM\", buf, 4096) != 0 ||\n"
                                "  GetEnvironmentVariableW(L\"LOGNAME\", buf, 4096) != 0 ||\n"
                                "  GetEnvironmentVariableW(L\"PROMPT\", buf, 4096) != 0 ||\n"
                                "  GetEnvironmentVariableW(L\"HOMEPATH\", buf, 4096) != 0 ||\n"
                                "  GetEnvironmentVariableW(L\"USERNAME\", buf, 4096) != 0 ||\n"
                                "  GetEnvironmentVariableW(L\"TEMP\", buf, 4096) != 0 ||\n"
                                "  GetEnvironmentVariableW(L\"TMP\", buf, 4096) != 0 ||\n"
                                "  GetEnvironmentVariableW(L\"OS\", buf, 4096) != 0);\n"
                                "return 0;}\n") && doTest() == 1) {
      define_wgetenv = 1;
      os_getenv_stri = "wgetenv";
    } else {
      fprintf(logFile, "\nAssume that wgetenv() should be defined.\n");
      define_wgetenv = 1;
      os_getenv_stri = "wgetenv";
    } /* if */
#else
    if (compileAndLinkOk("#include <stdio.h>\n#include <stdlib.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{printf(\"%d\\n\",\n"
                         "  getenv(\"PATH\") != NULL ||\n"
                         "  getenv(\"HOME\") != NULL ||\n"
                         "  getenv(\"TERM\") != NULL ||\n"
                         "  getenv(\"LOGNAME\") != NULL ||\n"
                         "  getenv(\"PROMPT\") != NULL ||\n"
                         "  getenv(\"HOMEPATH\") != NULL ||\n"
                         "  getenv(\"USERNAME\") != NULL ||\n"
                         "  getenv(\"TEMP\") != NULL ||\n"
                         "  getenv(\"TMP\") != NULL ||\n"
                         "  getenv(\"OS\") != NULL);\n"
                         "return 0;}\n") && doTest() == 1) {
      os_getenv_stri = "getenv";
    } /* if */
#endif
#ifdef EMULATE_ENVIRONMENT
#ifdef EMULATE_NODE_ENVIRONMENT
    os_setenv_stri = "setenvForNodeJs";
    os_unsetenv_stri = "unsetenvForNodeJs";
#else
    os_setenv_stri = "setenv7";
    os_unsetenv_stri = "unsetenv7";
#endif
    getenv_is_case_sensitive = 1;
#elif defined OS_STRI_WCHAR
    if (compileAndLinkOk("#include <stdio.h>\n#include <stdlib.h>\n"
                         "#include <string.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{printf(\"%d\\n\",\n"
                         "    (_wsetenv(L\"AsDfGhJkL\", L\"asdfghjkl\", 1) == 0 &&\n"
                         "     getenv(\"AsDfGhJkL\") != NULL &&\n"
                         "     strcmp(getenv(\"AsDfGhJkL\"), \"asdfghjkl\") == 0) +\n"
                         "    (getenv(\"ASDFGHJKL\") != NULL &&\n"
                         "     strcmp(getenv(\"ASDFGHJKL\"), \"asdfghjkl\") == 0));\n"
                         "return 0;}\n") && (test_result = doTest()) >= 1) {
      os_setenv_stri = "_wsetenv";
      getenv_is_case_sensitive = test_result == 1;
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <stdlib.h>\n"
                                "#include <string.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{printf(\"%d\\n\",\n"
                                "    (wsetenv(L\"AsDfGhJkL\", L\"asdfghjkl\", 1) == 0 &&\n"
                                "     getenv(\"AsDfGhJkL\") != NULL &&\n"
                                "     strcmp(getenv(\"AsDfGhJkL\"), \"asdfghjkl\") == 0) +\n"
                                "    (getenv(\"ASDFGHJKL\") != NULL &&\n"
                                "     strcmp(getenv(\"ASDFGHJKL\"), \"asdfghjkl\") == 0));\n"
                                "return 0;}\n") && (test_result = doTest()) >= 1) {
      os_setenv_stri = "wsetenv";
      getenv_is_case_sensitive = test_result == 1;
    } /* if */
    if (os_setenv_stri != NULL) {
      sprintf(buffer, "#include <stdio.h>\n#include <stdlib.h>\n"
                      "#include <string.h>\n"
                      "int main(int argc,char *argv[])\n"
                      "{printf(\"%%d\\n\",\n"
                      "    (%s(L\"AsDfGhJkL\", L\"asdfghjkl\", 1) == 0 &&\n"
                      "     getenv(\"AsDfGhJkL\") != NULL &&\n"
                      "     _wunsetenv(L\"AsDfGhJkL\") == 0 &&\n"
                      "     getenv(\"AsDfGhJkL\") == NULL));\n"
                      "return 0;}\n", os_setenv_stri);
      if (compileAndLinkOk(buffer) && doTest() == 1) {
        os_unsetenv_stri = "_wunsetenv";
      } else {
        sprintf(buffer, "#include <stdio.h>\n#include <stdlib.h>\n"
                        "#include <string.h>\n"
                        "int main(int argc,char *argv[])\n"
                        "{printf(\"%%d\\n\",\n"
                        "    (%s(L\"AsDfGhJkL\", L\"asdfghjkl\", 1) == 0 &&\n"
                        "     getenv(\"AsDfGhJkL\") != NULL &&\n"
                        "     wunsetenv(L\"AsDfGhJkL\") == 0 &&\n"
                        "     getenv(\"AsDfGhJkL\") == NULL));\n"
                        "return 0;}\n", os_setenv_stri);
        if (compileAndLinkOk(buffer) && doTest() == 1) {
          os_unsetenv_stri = "wunsetenv";
        } /* if */
      } /* if */
    } /* if */
    if (os_setenv_stri != NULL && os_unsetenv_stri != NULL) {
      /* No need to consider putenv(). */
    } else if (!define_wgetenv &&
               compileAndLinkOk("#include <stdio.h>\n#include <stdlib.h>\n"
                                "#include <string.h>\n"
                                "wchar_t *key_value = L\"AsDfGhJkL=asdfghjkl\";\n"
                                "int main(int argc,char *argv[])\n"
                                "{printf(\"%d\\n\",\n"
                                "    (_wputenv(key_value) == 0 &&\n"
                                "     getenv(\"AsDfGhJkL\") != NULL &&\n"
                                "     strcmp(getenv(\"AsDfGhJkL\"), \"asdfghjkl\") == 0) +\n"
                                "    (getenv(\"ASDFGHJKL\") != NULL &&\n"
                                "     strcmp(getenv(\"ASDFGHJKL\"), \"asdfghjkl\") == 0));\n"
                                "return 0;}\n") && (test_result = doTest()) >= 1) {
      os_putenv_stri = "_wputenv";
      getenv_is_case_sensitive = test_result == 1;
    } else if (!define_wgetenv &&
               compileAndLinkOk("#include <stdio.h>\n#include <stdlib.h>\n"
                                "#include <string.h>\n"
                                "wchar_t *key_value = L\"AsDfGhJkL=asdfghjkl\";\n"
                                "int main(int argc,char *argv[])\n"
                                "{printf(\"%d\\n\",\n"
                                "    (wputenv(key_value) == 0 &&\n"
                                "     getenv(\"AsDfGhJkL\") != NULL &&\n"
                                "     strcmp(getenv(\"AsDfGhJkL\"), \"asdfghjkl\") == 0) +\n"
                                "    (getenv(\"ASDFGHJKL\") != NULL &&\n"
                                "     strcmp(getenv(\"ASDFGHJKL\"), \"asdfghjkl\") == 0));\n"
                                "return 0;}\n") && (test_result = doTest()) >= 1) {
      os_putenv_stri = "wputenv";
      getenv_is_case_sensitive = test_result == 1;
    } else if (!define_wgetenv &&
               compileAndLinkOk("#include <stdio.h>\n#include <stdlib.h>\n"
                                "#include <string.h>\n#include <windows.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{printf(\"%d\\n\",\n"
                                "    (SetEnvironmentVariableW(L\"AsDfGhJkL\", L\"asdfghjkl\") != 0 &&\n"
                                "     getenv(\"AsDfGhJkL\") != NULL &&\n"
                                "     strcmp(getenv(\"AsDfGhJkL\"), \"asdfghjkl\") == 0) +\n"
                                "    (getenv(\"ASDFGHJKL\") != NULL &&\n"
                                "     strcmp(getenv(\"ASDFGHJKL\"), \"asdfghjkl\") == 0));\n"
                                "return 0;}\n") && (test_result = doTest()) >= 1) {
      define_wsetenv = 1;
      os_setenv_stri = "wsetenv";
      getenv_is_case_sensitive = test_result == 1;
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <stdlib.h>\n"
                                "#include <string.h>\n#include <windows.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{char buf1[10], buf2[10];\n"
                                "printf(\"%d\\n\",\n"
                                "    (SetEnvironmentVariableW(L\"AsDfGhJkL\", L\"asdfghjkl\") != 0 &&\n"
                                "     GetEnvironmentVariable(\"AsDfGhJkL\", buf1, 10) == 9 &&\n"
                                "     strcmp(buf1, \"asdfghjkl\") == 0) +\n"
                                "    (GetEnvironmentVariable(\"ASDFGHJKL\", buf2, 10) == 9 &&\n"
                                "     strcmp(buf2, \"asdfghjkl\") == 0));\n"
                                "return 0;}\n") && (test_result = doTest()) >= 1) {
      define_wsetenv = 1;
      os_setenv_stri = "wsetenv";
      getenv_is_case_sensitive = test_result == 1;
    } else {
      fprintf(logFile, "\nAssume that wsetenv() should be defined.\n");
      define_wsetenv = 1;
      os_setenv_stri = "wsetenv";
    } /* if */
    if (os_putenv_stri != NULL) {
      sprintf(buffer, "#include <stdio.h>\n#include <stdlib.h>\n"
                      "#include <string.h>\n"
                      "wchar_t *key_value = L\"AsDfGhJkL=asdfghjkl\";\n"
                      "wchar_t *key_no_value = L\"AsDfGhJkL=\";\n"
                      "wchar_t *key_missing = L\"QqAaWwSsEeDdRrFfTtGg=\";\n"
                      "int main(int argc,char *argv[])\n"
                      "{printf(\"%%d\\n\",\n"
                      "    %s(key_value) == 0 &&\n"
                      "    getenv(\"AsDfGhJkL\") != NULL &&\n"
                      "    %s(key_no_value) == 0 &&\n"
                      "    getenv(\"AsDfGhJkL\") == NULL &&\n"
                      "    %s(key_missing) == 0);\n"
                      "return 0;}\n", os_putenv_stri, os_putenv_stri, os_putenv_stri);
      if (compileAndLinkOk(buffer) && doTest() == 1) {
        putenv_can_remove_keys = 1;
      } /* if */
    } else if (define_wsetenv) {
      if (os_getenv_stri != NULL && !define_wgetenv) {
        sprintf(buffer, "#include <stdio.h>\n#include <stdlib.h>\n"
                        "#include <string.h>\n#include <windows.h>\n"
                        "int main(int argc,char *argv[])\n"
                        "{printf(\"%%d\\n\",\n"
                        "    SetEnvironmentVariableW(L\"AsDfGhJkL\", L\"asdfghjkl\") != 0 &&\n"
                        "    %s(L\"AsDfGhJkL\") != NULL &&\n"
                        "    SetEnvironmentVariableW(L\"AsDfGhJkL\", NULL) != 0 &&\n"
                        "    %s(L\"AsDfGhJkL\") == NULL);\n"
                        "return 0;}\n", os_getenv_stri, os_getenv_stri);
        if (compileAndLinkOk(buffer) && doTest() == 1) {
          define_wunsetenv = 1;
          os_unsetenv_stri = "wunsetenv";
        } /* if */
      } /* if */
      if (!define_wunsetenv &&
          compileAndLinkOk("#include <stdio.h>\n#include <stdlib.h>\n"
                           "#include <string.h>\n#include <windows.h>\n"
                           "int main(int argc,char *argv[])\n"
                           "{wchar_t buf1[10], buf2[10];\n"
                           "printf(\"%d\\n\",\n"
                           "    SetEnvironmentVariableW(L\"AsDfGhJkL\", L\"asdfghjkl\") != 0 &&\n"
                           "    GetEnvironmentVariableW(L\"AsDfGhJkL\", buf1, 10) == 9 &&\n"
                           "    SetEnvironmentVariableW(L\"AsDfGhJkL\", NULL) != 0 &&\n"
                           "    GetEnvironmentVariableW(L\"AsDfGhJkL\", buf2, 10) == 0);\n"
                           "return 0;}\n") && doTest() == 1) {
        /* Either define_wgetenv is already active or os_getenv_stri() */
        /* is not able to receive the correct result after */
        /* SetEnvironmentVariableW() has been called. */
        define_wgetenv = 1;
        os_getenv_stri = "wgetenv";
        define_wunsetenv = 1;
        os_unsetenv_stri = "wunsetenv";
      } /* if */
    } /* if */
#else
    if (compileAndLinkOk("#include <stdio.h>\n#include <stdlib.h>\n"
                         "#include <string.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{printf(\"%d\\n\",\n"
                         "    (setenv(\"AsDfGhJkL\", \"asdfghjkl\", 1) == 0 &&\n"
                         "     getenv(\"AsDfGhJkL\") != NULL &&\n"
                         "     strcmp(getenv(\"AsDfGhJkL\"), \"asdfghjkl\") == 0) +\n"
                         "    (getenv(\"ASDFGHJKL\") != NULL &&\n"
                         "     strcmp(getenv(\"ASDFGHJKL\"), \"asdfghjkl\") == 0));\n"
                         "return 0;}\n") && (test_result = doTest()) >= 1) {
      os_setenv_stri = "setenv";
      getenv_is_case_sensitive = test_result == 1;
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <stdlib.h>\n"
                                "#include <string.h>\n"
                                "char *key_value = \"AsDfGhJkL=asdfghjkl\";\n"
                                "int main(int argc,char *argv[])\n"
                                "{printf(\"%d\\n\",\n"
                                "    (putenv(key_value) == 0 &&\n"
                                "     getenv(\"AsDfGhJkL\") != NULL &&\n"
                                "     strcmp(getenv(\"AsDfGhJkL\"), \"asdfghjkl\") == 0) +\n"
                                "    (getenv(\"ASDFGHJKL\") != NULL &&\n"
                                "     strcmp(getenv(\"ASDFGHJKL\"), \"asdfghjkl\") == 0));\n"
                                "return 0;}\n") && (test_result = doTest()) >= 1) {
      os_putenv_stri = "putenv";
      getenv_is_case_sensitive = test_result == 1;
    } /* if */
    if (os_setenv_stri != NULL) {
      sprintf(buffer, "#include <stdio.h>\n#include <stdlib.h>\n"
                      "#include <string.h>\n"
                      "int main(int argc,char *argv[])\n"
                      "{printf(\"%%d\\n\",\n"
                      "    (%s(\"AsDfGhJkL\", \"asdfghjkl\", 1) == 0 &&\n"
                      "     getenv(\"AsDfGhJkL\") != NULL &&\n"
                      "     unsetenv(\"AsDfGhJkL\") == 0 &&\n"
                      "     getenv(\"AsDfGhJkL\") == NULL));\n"
                      "return 0;}\n", os_setenv_stri);
      if (compileAndLinkOk(buffer) && doTest() == 1) {
        os_unsetenv_stri = "unsetenv";
      } /* if */
    } /* if */
#endif
    if (define_wgetenv) {
      fputs("#define DEFINE_WGETENV\n", versionFile);
    } /* if */
    if (os_getenv_stri != NULL) {
      fprintf(versionFile, "#define os_getenv %s\n", os_getenv_stri);
    } else {
      fprintf(logFile, "\n *** Cannot define os_getenv.\n");
    } /* if */
    if (getenv_is_case_sensitive != -1) {
      fprintf(versionFile, "#define GETENV_IS_CASE_SENSITIVE %d\n",
              getenv_is_case_sensitive);
    } /* if */
    if (os_setenv_stri != NULL) {
      if (define_wsetenv) {
        fputs("#define DEFINE_WSETENV\n", versionFile);
      } /* if */
      fprintf(versionFile, "#define os_setenv %s\n", os_setenv_stri);
#ifndef EMULATE_ENVIRONMENT
      if (define_wgetenv) {
        sprintf(getenv_definition,
                "#ifndef WINDOWS_H_INCLUDED\n"
                "#include <windows.h>\n"
                "#define WINDOWS_H_INCLUDED\n"
                "#endif\n"
                "wchar_t *wgetenv (wchar_t *name) {\n"
                "  size_t value_size;\n"
                "  wchar_t *value;\n"
                "  value_size = GetEnvironmentVariableW(name, NULL, 0);\n"
                "  if (value_size == 0) {\n"
                "    value = NULL;\n"
                "  } else {\n"
                "    if ((value = malloc(value_size * sizeof(wchar_t))) != NULL) {\n"
                "      if (GetEnvironmentVariableW(name, value, value_size) != value_size - 1) {\n"
                "        free(value);\n"
                "        value = NULL;\n"
                "      } /* if */\n"
                "    } /* if */\n"
                "  } /* if */\n"
                "  return value;\n"
                "}\n");
      } else {
        getenv_definition[0] = '\0';
      } /* if */
      if (define_wsetenv) {
        sprintf(setenv_definition,
                "#ifndef WINDOWS_H_INCLUDED\n"
                "#include <windows.h>\n"
                "#define WINDOWS_H_INCLUDED\n"
                "#endif\n"
                "int wsetenv (wchar_t *name, wchar_t *value, int overwrite) {\n"
                "return !SetEnvironmentVariableW(name, value);\n"
                "}\n");
      } else {
        setenv_definition[0] = '\0';
      } /* if */
      sprintf(buffer, "#include <stdio.h>\n"
                      "#include <stdlib.h>\n"
                      "#include <string.h>\n"
                      "#include <wchar.h>\n"
                      "%s\n"
                      "%s\n"
                      "int main(int argc,char *argv[]){\n"
                      "int setenv_result;\n"
                      "int setenv_okay;\n"
                      "int getenv_okay = 0;\n"
#ifdef OS_STRI_WCHAR
                      "wchar_t *key = L\"ASDF=ghjkl\";\n"
                      "wchar_t *value = L\"asdfghjkl\";\n"
                      "wchar_t *getenv_result;\n"
#else
                      "char *key = \"ASDF=ghjkl\";\n"
                      "char *value = \"asdfghjkl\";\n"
                      "char *getenv_result;\n"
#endif
                      "setenv_result = %s(key, value, 1);\n"
                      "setenv_okay = setenv_result == 0;\n"
                      "getenv_result = %s(key);\n"
                      "getenv_okay = getenv_result != NULL;\n"
                      "if (setenv_okay && getenv_okay) {\n"
#ifdef OS_STRI_WCHAR
                      "  getenv_okay = wcscmp(getenv_result, value);\n"
#else
                      "  getenv_okay = strcmp(getenv_result, value);\n"
#endif
                      "}\n"
                      "printf(\"%%d\\n\", setenv_okay + 2 * getenv_okay);\n"
                      "return 0;}\n",
                      getenv_definition, setenv_definition,
                      os_setenv_stri, os_getenv_stri);
      if (assertCompAndLnk(buffer) && (test_result = doTest()) >= 0) {
        fprintf(versionFile, "#define SETENV_ALLOWS_KEY_WITH_EQUALS_SIGN %d\n",
                test_result & 1);
        fprintf(versionFile, "#define GETENV_ALLOWS_KEY_WITH_EQUALS_SIGN %d\n",
                test_result >> 1);
      } /* if */
#endif
    } /* if */
    if ((os_setenv_stri == NULL || os_unsetenv_stri == NULL) &&
        os_putenv_stri != NULL && os_getenv_stri != NULL && !define_wgetenv) {
      fprintf(versionFile, "#define os_putenv %s\n", os_putenv_stri);
      sprintf(buffer, "#include <stdio.h>\n"
                      "#include <stdlib.h>\n"
                      "#include <string.h>\n"
                      "#include <wchar.h>\n"
                      "int main(int argc,char *argv[]){\n"
#ifdef OS_STRI_WCHAR
                      "wchar_t *key_value = L\"ASDFGHJKL=asdfghjkl\";\n"
                      "wchar_t *value;\n"
                      "%s(key_value);\n"
                      "value = %s(L\"ASDFGHJKL\");\n"
                      "printf(\"%%d\\n\", value != NULL &&\n"
                      "  wcschr(key_value, '=') + 1 != value);\n"
#else
                      "char *key_value = \"ASDFGHJKL=asdfghjkl\";\n"
                      "char *value;\n"
                      "%s(key_value);\n"
                      "value = %s(\"ASDFGHJKL\");\n"
                      "printf(\"%%d\\n\", value != NULL &&\n"
                      "  strchr(key_value, '=') + 1 != value);\n"
#endif
                      "return 0;}\n", os_putenv_stri, os_getenv_stri);
      if (assertCompAndLnk(buffer)) {
        /* There are two possible implementations of putenv():  */
        /* 1. Putenv inserts its argument into the environment. */
        /*    In this case the argument of putenv() must not be */
        /*    changed afterwards.                               */
        /* 2. Putenv inserts a copy of its argument into the    */
        /*    environment. In this case it is okay to delete    */
        /*    the argument of putenv() afterwards.              */
        fprintf(versionFile, "#define DELETE_PUTENV_ARGUMENT %d\n", doTest());
      } /* if */
      fprintf(versionFile, "#define PUTENV_CAN_REMOVE_KEYS %d\n",
              putenv_can_remove_keys);
    } else if (os_setenv_stri == NULL) {
      fprintf(logFile, "\n *** Cannot define os_setenv or os_putenv.\n");
    } /* if */
    if (os_unsetenv_stri != NULL) {
      if (define_wunsetenv) {
        fputs("#define DEFINE_WUNSETENV\n", versionFile);
      } /* if */
      fprintf(versionFile, "#define os_unsetenv %s\n", os_unsetenv_stri);
    } /* if */
  } /* determineEnvironDefines */



#ifdef OS_STRI_WCHAR
static void determineOsUtime (FILE *versionFile)

  {
    const char *utime_include = NULL;
    const char *os_utimbuf_struct_stri = NULL;
    const char *os_utime_stri = NULL;
    char buffer[BUFFER_SIZE];

  /* determineOsUtime */
    if (compileAndLinkOk("#include <stdio.h>\n#include <utime.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{struct _utimbuf buf; buf.actime = 0, buf.modtime = 0;\n"
                         "printf(\"%d\\n\", &buf != NULL); return 0;}\n")) {
      utime_include = "utime.h";
      os_utimbuf_struct_stri = "struct _utimbuf";
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <utime.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{struct utimbuf buf; buf.actime = 0, buf.modtime = 0;\n"
                         "printf(\"%d\\n\", &buf != NULL); return 0;}\n")) {
      utime_include = "utime.h";
      os_utimbuf_struct_stri = "struct utimbuf";
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <sys/utime.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{struct _utimbuf buf; buf.actime = 0, buf.modtime = 0;\n"
                         "printf(\"%d\\n\", &buf != NULL); return 0;}\n")) {
      utime_include = "sys/utime.h";
      os_utimbuf_struct_stri = "struct _utimbuf";
      fputs("#define INCLUDE_SYS_UTIME\n", versionFile);
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <sys/utime.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{struct utimbuf buf; buf.actime = 0, buf.modtime = 0;\n"
                         "printf(\"%d\\n\", &buf != NULL); return 0;}\n")) {
      utime_include = "sys/utime.h";
      os_utimbuf_struct_stri = "struct utimbuf";
      fputs("#define INCLUDE_SYS_UTIME\n", versionFile);
    } /* if */
    if (utime_include == NULL || os_utimbuf_struct_stri == NULL) {
      fprintf(logFile, "\n *** Cannot find utime.h include and os_utimbuf_struct.\n");
    } else {
      fprintf(versionFile, "#define os_utimbuf_struct %s\n", os_utimbuf_struct_stri);
      sprintf(buffer, "#include <stdio.h>\n#include <%s>\n"
                      "int main(int argc,char *argv[])\n"
                      "{%s buf; buf.actime = 0, buf.modtime = 0;\n"
                      "printf(\"%%d\\n\", _wutime(L\"testfile\", &buf) != -1);return 0;}\n",
                      utime_include, os_utimbuf_struct_stri);
      if (compileAndLinkOk(buffer)) {
        os_utime_stri = "_wutime";
      } else {
        sprintf(buffer, "#include <stdio.h>\n#include <%s>\n"
                        "int main(int argc,char *argv[])\n"
                        "{%s buf; buf.actime = 0, buf.modtime = 0;\n"
                        "printf(\"%%d\\n\", wutime(L\"testfile\", &buf) != -1);return 0;}\n",
                        utime_include, os_utimbuf_struct_stri);
        if (compileAndLinkOk(buffer)) {
          os_utime_stri = "wutime";
        } /* if */
      } /* if */
      if (os_utime_stri != NULL) {
        sprintf(buffer,
                "#include <stdio.h>\n#include <%s>\n#include <errno.h>\n%s\n%s"
                "int main(int argc,char *argv[])"
                "{%s utime_buf;makeDir(\"tmp_empty_dir2\",0755);\n"
                "utime_buf.actime=1234567890;utime_buf.modtime=1234567890;\n"
                "printf(\"%%d\\n\",utime(\"tmp_empty_dir2\",&utime_buf)!=0&&errno==EACCES);\n"
                "if(remove(\"tmp_empty_dir2\")!=0)removeDir(\"tmp_empty_dir2\");return 0;}\n",
                utime_include, makeDirDefinition, removeDirDefinition, os_utimbuf_struct_stri);
        if (compileAndLinkOk(buffer) && doTest() == 1) {
          fputs("#define USE_ALTERNATE_UTIME\n", versionFile);
          fprintf(versionFile, "#define os_utime_orig %s\n", os_utime_stri);
          fputs("#define os_utime alternate_utime\n", versionFile);
        } else {
          fprintf(versionFile, "#define os_utime %s\n", os_utime_stri);
        } /* if */
      } else {
        fputs("#define USE_ALTERNATE_UTIME\n", versionFile);
        fputs("#define os_utime alternate_utime\n", versionFile);
      } /* if */
    } /* if */
  } /* determineOsUtime */



static void determineStatFunctions (FILE *versionFile)

  {
    char buffer[BUFFER_SIZE];
    char *os_stat_struct_stri = NULL;
    char *os_fstat_func = NULL;
    char *os_fstat_struct_stri = NULL;
    char *os_fstat_orig_func = NULL;
    char *os_fstat_struct_orig_stri = NULL;
    int has_struct_stati64 = 0;

  /* determineStatFunctions */
#ifndef os_lstat
    fputs("#define DEFINE_LSTATI64_EXT\n", versionFile);
    fputs("#define os_lstat lstati64Ext\n", versionFile);
#endif
#ifndef os_stat
    fputs("#define DEFINE_WSTATI64_EXT\n", versionFile);
    fputs("#define os_stat wstati64Ext\n", versionFile);
#endif
#ifndef os_stat_struct
    if (compileAndLinkWithOptionsOk("#include <sys/types.h>\n"
                                    "#include <sys/stat.h>\n"
                                    "int main(int argc,char *argv[])\n"
                                    "{struct _stati64 statData;\n"
                                    "return 0;}\n", "", SYSTEM_LIBS)) {
      has_struct_stati64 = 1;
      os_stat_struct_stri = "struct _stati64";
    } else if (compileAndLinkWithOptionsOk("#include <sys/types.h>\n"
                                           "#include <sys/stat.h>\n"
                                           "int main(int argc,char *argv[])\n"
                                           "{struct stati64 statData;\n"
                                           "return 0;}\n", "", SYSTEM_LIBS)) {
      has_struct_stati64 = 1;
      os_stat_struct_stri = "struct stati64";
    } else {
      fputs("#define DEFINE_STRUCT_STATI64_EXT\n", versionFile);
      os_stat_struct_stri = "struct stati64Ext";
    } /* if */
    if (has_struct_stati64) {
      sprintf(buffer, "#include <stdio.h>\n"
                      "#include <sys/types.h>\n"
                      "#include <sys/stat.h>\n"
                      "int main(int argc,char *argv[])\n"
                      "{%s statData;\n"
                      "printf(\"%%d\\n\",\n"
                      "    _wstati64(L\".\", &statData) == 0);\n"
                      "return 0;}\n",
                      os_stat_struct_stri);
      if (compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS)) {
        fputs("#define os_stat_orig _wstati64\n", versionFile);
      } else {
        sprintf(buffer, "#include <stdio.h>\n"
                        "#include <sys/types.h>\n"
                        "#include <sys/stat.h>\n"
                        "int main(int argc,char *argv[])\n"
                        "{%s statData;\n"
                        "printf(\"%%d\\n\",\n"
                        "    wstati64(L\".\", &statData) == 0);\n"
                        "return 0;}\n",
                        os_stat_struct_stri);
        if (compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS)) {
          fputs("#define os_stat_orig wstati64\n", versionFile);
        } /* if */
      } /* if */
    } /* if */
    fprintf(versionFile, "#define os_stat_struct %s\n", os_stat_struct_stri);
#endif
#ifndef os_fstat
    if (has_struct_stati64) {
      sprintf(buffer, "#include <stdio.h>\n"
                      "#include <sys/types.h>\n#include <sys/stat.h>\n"
                      "int main(int argc,char *argv[])\n"
                      "{%s statData;\n"
                      "printf(\"%%d\\n\",\n"
                      "    _fstati64(0, &statData) == 0);\n"
                      "return 0;}\n",
                      os_stat_struct_stri);
      if (compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS)) {
        os_fstat_func ="_fstati64";
        os_fstat_struct_stri = os_stat_struct_stri;
      } else {
        sprintf(buffer, "#include <stdio.h>\n"
                        "#include <sys/types.h>\n#include <sys/stat.h>\n"
                        "int main(int argc,char *argv[])\n"
                        "{%s statData;\n"
                        "printf(\"%%d\\n\",\n"
                        "    fstati64(0, &statData) == 0);\n"
                        "return 0;}\n",
                        os_stat_struct_stri);
        if (compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS)) {
          os_fstat_func ="fstati64";
          os_fstat_struct_stri = os_stat_struct_stri;
        } /* if */
      } /* if */
    } /* if */
    if (os_fstat_struct_stri == NULL) {
      if (compileAndLinkWithOptionsOk("#include <stdio.h>\n"
                                      "#include <sys/types.h>\n"
                                      "#include <sys/stat.h>\n"
                                      "int main(int argc,char *argv[])\n"
                                      "{struct _stat statData;\n"
                                      "printf(\"%d\\n\",\n"
                                      "    _fstat(0, &statData) == 0);\n"
                                      "return 0;}\n", "", SYSTEM_LIBS)) {
        os_fstat_orig_func = "_fstat";
        os_fstat_struct_orig_stri = "struct _stat";
        if (!has_struct_stati64) {
          os_fstat_func ="_fstat";
          os_fstat_struct_stri = "struct _stat";
        } /* if */
      } else if (compileAndLinkWithOptionsOk("#include <stdio.h>\n"
                                             "#include <sys/types.h>\n"
                                             "#include <sys/stat.h>\n"
                                             "int main(int argc,char *argv[])\n"
                                             "{struct stat statData;\n"
                                             "printf(\"%d\\n\",\n"
                                             "    _fstat(0, &statData) == 0);\n"
                                             "return 0;}\n", "", SYSTEM_LIBS)) {
        os_fstat_orig_func = "_fstat";
        os_fstat_struct_orig_stri = "struct stat";
        if (!has_struct_stati64) {
          os_fstat_func ="_fstat";
          os_fstat_struct_stri = "struct stat";
        } /* if */
      } else if (compileAndLinkWithOptionsOk("#include <stdio.h>\n"
                                             "#include <sys/types.h>\n"
                                             "#include <sys/stat.h>\n"
                                             "int main(int argc,char *argv[])\n"
                                             "{struct _stat statData;\n"
                                             "printf(\"%d\\n\",\n"
                                             "    fstat(0, &statData) == 0);\n"
                                             "return 0;}\n", "", SYSTEM_LIBS)) {
        os_fstat_orig_func = "fstat";
        os_fstat_struct_orig_stri = "struct _stat";
        if (!has_struct_stati64) {
          os_fstat_func ="fstat";
          os_fstat_struct_stri = "struct _stat";
        } /* if */
      } else if (compileAndLinkWithOptionsOk("#include <stdio.h>\n"
                                             "#include <sys/types.h>\n"
                                             "#include <sys/stat.h>\n"
                                             "int main(int argc,char *argv[])\n"
                                             "{struct stat statData;\n"
                                             "printf(\"%d\\n\",\n"
                                             "    fstat(0, &statData) == 0);\n"
                                             "return 0;}\n", "", SYSTEM_LIBS)) {
        os_fstat_orig_func = "fstat";
        os_fstat_struct_orig_stri = "struct stat";
        if (!has_struct_stati64) {
          os_fstat_func ="fstat";
          os_fstat_struct_stri = "struct stat";
        } /* if */
      } /* if */
    } /* if */
    if (os_fstat_struct_stri == NULL) {
      if (os_fstat_orig_func != NULL && os_fstat_struct_orig_stri != NULL) {
        fprintf(versionFile, "#define os_fstat_orig %s\n", os_fstat_orig_func);
        fprintf(versionFile, "#define os_fstat_struct_orig %s\n", os_fstat_struct_orig_stri);
      } /* if */
      os_fstat_struct_stri = os_stat_struct_stri;
      fputs("#define DEFINE_FSTATI64_EXT\n", versionFile);
      os_fstat_func ="fstati64Ext";
    } else if (os_fstat_orig_func != NULL && os_fstat_struct_orig_stri != NULL) {
      sprintf(buffer, "#include <stdio.h>\n"
                      "#include <sys/types.h>\n#include <sys/stat.h>\n"
                      "int main(int argc,char *argv[])\n"
                      "{%s statData;\n"
                      "printf(\"%%d\\n\", sizeof(statData.st_size));\n"
                      "return 0;}\n",
                      os_fstat_struct_orig_stri);
      if (compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS) ||
          doTest() != 8) {
        fprintf(versionFile, "#define os_fstat_orig %s\n", os_fstat_orig_func);
        fprintf(versionFile, "#define os_fstat_struct_orig %s\n", os_fstat_struct_orig_stri);
        os_fstat_struct_stri = os_stat_struct_stri;
        fputs("#define DEFINE_FSTATI64_EXT\n", versionFile);
        os_fstat_func ="fstati64Ext";
      } /* if */
    } /* if */
    if (os_fstat_func != NULL) {
      fprintf(versionFile, "#define os_fstat %s\n", os_fstat_func);
    } /* if */
    if (os_stat_struct_stri != NULL && os_fstat_struct_stri != NULL &&
        strcmp(os_stat_struct_stri, os_fstat_struct_stri) != 0) {
      fprintf(versionFile, "#define os_fstat_struct %s\n", os_fstat_struct_stri);
    } /* if */
#endif
  } /* determineStatFunctions */



static void determineOsWCharFunctions (FILE *versionFile)

  { /* determineOsWCharFunctions */
    determineStatFunctions(versionFile);
#ifndef os_chdir
    if (compileAndLinkWithOptionsOk("#include <stdio.h>\n#include <direct.h>\n"
                                    "int main(int argc,char *argv[])\n"
                                    "{printf(\"%d\\n\",\n"
                                    "    _wchdir(L\"..\") != -1);\n"
                                    "return 0;}\n", "", SYSTEM_LIBS)) {
      fputs("#define os_chdir _wchdir\n", versionFile);
    } else {
      fprintf(logFile, "\n *** Cannot define os_chdir.\n");
      showErrors();
    } /* if */
#endif
#ifndef os_getcwd
    if (compileAndLinkWithOptionsOk("#include <stdio.h>\n#include <wchar.h>\n"
                                    "#include <direct.h>\n"
                                    "int main(int argc,char *argv[])\n"
                                    "{wchar_t buffer[1024];\n"
                                    "printf(\"%d\\n\",\n"
                                    "    _wgetcwd(buffer, 1024) != NULL);\n"
                                    "return 0;}\n", "", SYSTEM_LIBS)) {
      fputs("#define OS_GETCWD_MAX_BUFFER_SIZE INT_MAX\n", versionFile);
      fputs("#define os_getcwd(buf,size) _wgetcwd((buf),(int)(size))\n", versionFile);
      if (!compileAndLinkWithOptionsOk("#include <stdio.h>\n#include <wchar.h>\n"
                                       "int main(int argc,char *argv[])\n"
                                       "{wchar_t buffer[1024];\n"
                                       "printf(\"%d\\n\",\n"
                                       "    _wgetcwd(buffer, 1024) != NULL);\n"
                                       "return 0;}\n", "", SYSTEM_LIBS)) {
        fputs("#define OS_GETCWD_INCLUDE_DIRECT_H\n", versionFile);
      } /* if */
    } else {
      fprintf(logFile, "\n *** Cannot define os_getcwd.\n");
      showErrors();
    } /* if */
#endif
#ifndef os_mkdir
    if (compileAndLinkWithOptionsOk("#include <stdio.h>\n#include <direct.h>\n"
                                    "int main(int argc,char *argv[])\n"
                                    "{printf(\"%d\\n\",\n"
                                    "    _wmkdir(L\"testdir\") != -1);\n"
                                    "return 0;}\n", "", SYSTEM_LIBS)) {
      fputs("#define os_mkdir(path,mode) _wmkdir(path)\n", versionFile);
    } else {
      fprintf(logFile, "\n *** Cannot define os_mkdir.\n");
      showErrors();
    } /* if */
#endif
#ifndef os_rmdir
    if (compileAndLinkWithOptionsOk("#include <stdio.h>\n#include <direct.h>\n"
                                    "int main(int argc,char *argv[])\n"
                                    "{printf(\"%d\\n\",\n"
                                    "    _wrmdir(L\"testdir\") != -1);\n"
                                    "return 0;}\n", "", SYSTEM_LIBS)) {
      fputs("#define os_rmdir _wrmdir\n", versionFile);
    } else {
      fprintf(logFile, "\n *** Cannot define os_rmdir.\n");
      showErrors();
    } /* if */
#endif
#ifndef os_chmod
    fputs("#define DEFINE_WCHMOD_EXT\n", versionFile);
    fputs("#define os_chmod wchmodExt\n", versionFile);
    if (compileAndLinkWithOptionsOk("#include <stdio.h>\n#include <direct.h>\n"
                                    "int main(int argc,char *argv[])\n"
                                    "{printf(\"%d\\n\",\n"
                                    "    _wchmod(L\"testfile\",0777) != -1);\n"
                                    "return 0;}\n", "", SYSTEM_LIBS)) {
      fputs("#define os_chmod_orig _wchmod\n", versionFile);
    } else if (compileAndLinkWithOptionsOk("#include <stdio.h>\n#include <direct.h>\n\n"
                                           "#include <io.h>\n"
                                           "int main(int argc,char *argv[])\n"
                                           "{printf(\"%d\\n\",\n"
                                           "    _wchmod(L\"testfile\",0777) != -1);\n"
                                           "return 0;}\n", "", SYSTEM_LIBS)) {
      fputs("#define OS_CHMOD_INCLUDE_IO_H\n", versionFile);
      fputs("#define os_chmod_orig _wchmod\n", versionFile);
    } else {
      fprintf(logFile, "\n *** Cannot define os_chmod_orig.\n");
      showErrors();
    } /* if */
#endif
#ifndef os_chown
    fputs("#define os_chown(name,uid,gid)\n", versionFile);
#endif
#ifndef os_utime
    determineOsUtime(versionFile);
#endif
#ifndef os_remove
    if (compileAndLinkWithOptionsOk("#include <stdio.h>\n"
                                    "int main(int argc,char *argv[])\n"
                                    "{printf(\"%d\\n\", _wremove(L\"testfile\") != -1);\n"
                                    "return 0;}\n", "", SYSTEM_LIBS)) {
      fputs("#define os_remove _wremove\n", versionFile);
    } else {
      fprintf(logFile, "\n *** Cannot define os_remove.\n");
      showErrors();
    } /* if */
#endif
#ifndef os_rename
    fputs("#define DEFINE_WIN_RENAME\n", versionFile);
    fputs("#define os_rename winRename\n", versionFile);
#endif
#ifndef os_symlink
    if (compileAndLinkWithOptionsOk("#include <stdio.h>\n#include <windows.h>\n"
                                    "int main(int argc, char *argv[]){\n"
                                    "int okay=0;\n"
                                    "if (CreateSymbolicLinkW(L\"test_symlink\",L\"qwertzuiop\",0) != 0) {\n"
                                    "okay=1;\n"
                                    "remove(\"test_symlink\");}\n"
                                    "printf(\"%d\\n\", okay);\n"
                                    "return 0;}\n", "", SYSTEM_LIBS)) {
      fputs("#define DEFINE_WIN_SYMLINK\n", versionFile);
      fputs("#define os_symlink winSymlink\n", versionFile);
    } /* if */
    if (compileAndLinkWithOptionsOk("#include <stdio.h>\n#include <windows.h>\n"
                                    "int main(int argc, char *argv[]){\n"
                                    "int okay=0;\n"
                                    "if (CreateSymbolicLinkW(L\"test_symlink\",L\"qwertzuiop\",\n"
                                    "SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE) != 0) {\n"
                                    "okay=1;\n"
                                    "remove(\"test_symlink\");}\n"
                                    "printf(\"%d\\n\", okay);\n"
                                    "return 0;}\n", "", SYSTEM_LIBS) && doTest() == 1) {
      fputs("#define HAS_SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE\n", versionFile);
    } /* if */
#endif
#ifndef os_system
    if (compileAndLinkWithOptionsOk("#include <stdio.h>\n#include <stdlib.h>\n"
                                    "int main(int argc,char *argv[])\n"
                                    "{printf(\"%d\\n\",\n"
                                    "    _wsystem(L\"pwd\") != -1);\n"
                                    "return 0;}\n", "", SYSTEM_LIBS)) {
      fputs("#define os_system _wsystem\n", versionFile);
    } else {
      fprintf(logFile, "\n *** Cannot define os_system.\n");
      showErrors();
    } /* if */
#endif
#ifndef os_fopen
    if (compileAndLinkWithOptionsOk("#include <stdio.h>\n"
                                    "int main(int argc,char *argv[])\n"
                                    "{printf(\"%d\\n\",\n"
                                    "    _wfopen(L\"testfile\", L\"r\") != NULL);\n"
                                    "return 0;}\n", "", SYSTEM_LIBS)) {
      fputs("#define os_fopen _wfopen\n", versionFile);
    } else if (compileAndLinkWithOptionsOk("#include <stdio.h>\n"
                                           "int main(int argc,char *argv[])\n"
                                           "{printf(\"%d\\n\",\n"
                                           "    wfopen(L\"testfile\", L\"r\") != NULL);\n"
                                           "return 0;}\n", "", SYSTEM_LIBS)) {
      fputs("#define os_fopen wfopen\n", versionFile);
    } else {
      fprintf(logFile, "\n *** Cannot define os_fopen.\n");
      showErrors();
    } /* if */
#endif
#ifndef os_popen
    if (compileAndLinkOk("#include <stdio.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{printf(\"%d\\n\", _wpopen(L\"pwd\", L\"r\") != NULL);return 0;}\n")) {
      fputs("#define os_popen _wpopen\n", versionFile);
    } else if (compileAndLinkOk("#include <stdio.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{printf(\"%d\\n\", wpopen(L\"pwd\", L\"r\") != NULL);return 0;}\n")) {
      fputs("#define os_popen wpopen\n", versionFile);
    } else {
      fputs("#define DEFINE_WPOPEN FILE *wpopen (const wchar_t *command, const wchar_t *mode) { return NULL; }\n", versionFile);
      fputs("#define os_popen wpopen\n", versionFile);
    } /* if */
#endif
#ifndef os_pclose
    if (compileAndLinkOk("#include <stdio.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{printf(\"%d\\n\", _pclose(NULL) != -1);return 0;}\n")) {
      fputs("#define os_pclose _pclose\n", versionFile);
    } else {
      fprintf(logFile, "\n *** Cannot define os_pclose.\n");
      showErrors();
    } /* if */
#endif
#ifndef os_setmode
    if (compileAndLinkOk("#include <stdio.h>\n#include <io.h>\n"
                         "#include <fcntl.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{printf(\"%d\\n\", _setmode(0, _O_BINARY) != -1);return 0;}\n")) {
      fputs("#define os_setmode _setmode\n", versionFile);
    } /* if */
#endif
    if (compileAndLinkOk("#include <stdio.h>\n#include <windows.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{HANDLE fileHandle;\n"
                         "fileHandle = CreateFileA(\"tst_vers.h\", 0, FILE_SHARE_READ, NULL,\n"
                         "    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL |\n"
                         "    FILE_FLAG_BACKUP_SEMANTICS, NULL);\n"
                         "printf(\"%d\\n\", fileHandle != INVALID_HANDLE_VALUE &&\n"
                         "    GetFinalPathNameByHandleW(fileHandle,\n"
                         "        NULL, 0, FILE_NAME_OPENED) != 0);\n"
                         "return 0;}\n")) {
      fputs("#define HAS_GET_FINAL_PATH_NAME_BY_HANDLE\n", versionFile);
    } /* if */
    if (compileAndLinkOk("#include <stdio.h>\n#include <windows.h>\n"
                         "#ifndef FILE_FLAG_OPEN_REPARSE_POINT\n"
                         "#define FILE_FLAG_OPEN_REPARSE_POINT 0x200000\n"
                         "#endif\n"
                         "#ifndef FSCTL_GET_REPARSE_POINT\n"
                         "#define FSCTL_GET_REPARSE_POINT 0x900a8\n"
                         "#endif\n"
                         "int main(int argc,char *argv[])\n"
                         "{HANDLE fileHandle;\n"
                         "DWORD sz;\n"
                         "char buffer[1000];\n"
                         "fileHandle = CreateFileA(\"tst_vers.h\", GENERIC_READ, 0, NULL,\n"
                         "    OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT |\n"
                         "    FILE_FLAG_BACKUP_SEMANTICS, NULL);\n"
                         "printf(\"%d\\n\", fileHandle != INVALID_HANDLE_VALUE &&\n"
                         "    DeviceIoControl(fileHandle, FSCTL_GET_REPARSE_POINT,\n"
                         "        NULL, 0, buffer, sizeof(buffer), &sz, NULL) != 0);\n"
                         "return 0;}\n")) {
      fputs("#define HAS_DEVICE_IO_CONTROL\n", versionFile);
    } /* if */
    if (compileAndLinkOk("#include <stdio.h>\n#include <windows.h>\n"
                         "#ifndef FILE_FLAG_OPEN_REPARSE_POINT\n"
                         "#define FILE_FLAG_OPEN_REPARSE_POINT 0x200000\n"
                         "#endif\n"
                         "int main(int argc,char *argv[])\n"
                         "{HANDLE fileHandle;\n"
                         "FILE_BASIC_INFO fileBasicInfoData;\n"
                         "fileHandle = CreateFileA(\"tst_vers.h\", GENERIC_READ, 0, NULL,\n"
                         "    OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT |\n"
                         "    FILE_FLAG_BACKUP_SEMANTICS, NULL);\n"
                         "printf(\"%d\\n\", fileHandle != INVALID_HANDLE_VALUE &&\n"
                         "    GetFileInformationByHandleEx(fileHandle, FileBasicInfo,\n"
                         "        &fileBasicInfoData, sizeof(FILE_BASIC_INFO)) != 0);\n"
                         "return 0;}\n")) {
      fputs("#define HAS_GET_FILE_INFORMATION_BY_HANDLE_EX\n", versionFile);
    } /* if */
    assertCompAndLnk("#include <stdio.h>\n#include <windows.h>\n"
                     "int main(int argc,char *argv[]){\n"
                     "#ifdef INVALID_FILE_ATTRIBUTES\n"
                     "printf(\"1\\n\");\n"
                     "#else\n"
                     "printf(\"0\\n\");\n"
                     "#endif\n"
                     "return 0;}\n");
    if (doTest() == 0) {
      fputs("#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)\n", versionFile);
    } /* if */
    assertCompAndLnk("#include <stdio.h>\n#include <windows.h>\n"
                     "int main(int argc,char *argv[]){\n"
                     "#ifdef FILE_ATTRIBUTE_REPARSE_POINT\n"
                     "printf(\"1\\n\");\n"
                     "#else\n"
                     "printf(\"0\\n\");\n"
                     "#endif\n"
                     "return 0;}\n");
    if (doTest() == 0) {
      fputs("#define FILE_ATTRIBUTE_REPARSE_POINT 0x00000400\n", versionFile);
    } /* if */
    assertCompAndLnk("#include <stdio.h>\n#include <windows.h>\n"
                     "int main(int argc,char *argv[]){\n"
                     "#ifdef FILE_FLAG_OPEN_REPARSE_POINT\n"
                     "printf(\"1\\n\");\n"
                     "#else\n"
                     "printf(\"0\\n\");\n"
                     "#endif\n"
                     "return 0;}\n");
    if (doTest() == 0) {
      fputs("#define FILE_FLAG_OPEN_REPARSE_POINT 0x200000\n", versionFile);
    } /* if */
  } /* determineOsWCharFunctions */



static void determineTimeFunctionProperties (FILE *versionFile)

  { /* determineTimeFunctionProperties */
    if (compileAndLinkOk("#include <stdio.h>\n"
                         "#include <windows.h>\n"
                         "int main(int argc,char *argv[]){\n"
                         "SYSTEMTIME await_time_struct;\n"
                         "FILETIME filetime;\n"
                         "int test_28;\n"
                         "int test_29;\n"
                         "await_time_struct.wYear = 2001;\n"
                         "await_time_struct.wMonth = 2;\n"
                         "await_time_struct.wDay = 28;\n"
                         "await_time_struct.wHour = 0;\n"
                         "await_time_struct.wMinute = 0;\n"
                         "await_time_struct.wSecond = 0;\n"
                         "await_time_struct.wMilliseconds = 0;\n"
                         "test_28 = SystemTimeToFileTime(\n"
                         "    &await_time_struct, &filetime);\n"
                         "await_time_struct.wDay = 29;\n"
                         "test_29 = SystemTimeToFileTime(\n"
                         "    &await_time_struct, &filetime);\n"
                         "printf(\"%d\\n\", test_28 == 1 && test_29 == 0);\n"
                         "return 0;}\n") && doTest() != 1) {
      fprintf(versionFile, "#define CHECK_NON_LEAP_YEAR_FEBRURARY_29\n");
    } /* if */
  } /* determineTimeFunctionProperties */
#endif



static void determineIsattyFunction (FILE *versionFile)

  { /* determineIsattyFunction */
    if (compileAndLinkOk("#include <stdio.h>\n#include <io.h>\n"
                         "int main(int argc,char *argv[]){\n"
                         "printf(\"%d\\n\", _isatty(0)==0);\n"
                         "return 0;}\n")) {
      fprintf(versionFile, "#define ISATTY_INCLUDE_IO_H\n");
      fprintf(versionFile, "#define os_isatty _isatty\n");
    } else if (!compileAndLinkOk("#include <stdio.h>\n#include <unistd.h>\n"
                                  "int main(int argc,char *argv[]){\n"
                                  "printf(\"%d\\n\", isatty(0)==0);\n"
                                  "return 0;}\n")) {
      if (compileAndLinkOk("#include <stdio.h>\n#include <io.h>\n"
                           "int main(int argc,char *argv[]){\n"
                           "printf(\"%d\\n\", isatty(0)==0);\n"
                           "return 0;}\n")) {
        fprintf(versionFile, "#define ISATTY_INCLUDE_IO_H\n");
      } else {
        fprintf(logFile, "\n *** Cannot determine os_isatty.\n");
      } /* if */
    } /* if */
  } /* determineIsattyFunction */



static void determineFdopenFunction (FILE *versionFile)

  { /* determineFdopenFunction */
    if (compileAndLinkOk("#include <stdio.h>\n"
                         "int main(int argc,char *argv[]){\n"
                         "FILE *fp = _fdopen(0, \"r\");\n"
                         "printf(\"%d\\n\", fp!=NULL);\n"
                         "return 0;}\n")) {
      fprintf(versionFile, "#define os_fdopen _fdopen\n");
    } else if (!compileAndLinkOk("#include <stdio.h>\n"
                                  "int main(int argc,char *argv[]){\n"
                                  "FILE *fp = fdopen(0, \"r\");\n"
                                  "printf(\"%d\\n\", fp!=NULL);\n"
                                  "return 0;}\n")) {
      fprintf(logFile, "\n *** Cannot determine os_fdopen.\n");
    } /* if */
  } /* determineFdopenFunction */



static const char *determineFilenoFunction (FILE *versionFile)

  {
    char buffer[BUFFER_SIZE];
    const char *fileno = "fileno";

  /* determineFilenoFunction */
    if (compileAndLinkOk("#include <stdio.h>\n"
                         "int main(int argc,char *argv[]){\n"
                         "printf(\"%d\\n\", _fileno(stdin)==0);\n"
                         "return 0;}\n")) {
      fileno = "_fileno";
      fprintf(versionFile, "#define os_fileno _fileno\n");
    } else if (!compileAndLinkOk("#include <stdio.h>\n"
                                 "int main(int argc,char *argv[]){\n"
                                 "printf(\"%d\\n\", fileno(stdin)==0);\n"
                                 "return 0;}\n")) {
      fprintf(logFile, "\n *** Cannot determine os_fileno.\n");
    } /* if */
#ifndef FILENO_WORKS_FOR_NULL
    sprintf(buffer, "#include <stdlib.h>\n#include <stdio.h>\n#include <signal.h>\n"
                    "void handleSig(int sig){puts(\"2\");exit(0);}\n"
                    "int main(int argc, char *argv[]){\n"
                    "FILE *aFile = NULL;\n"
                    "signal(SIGSEGV,handleSig);\n"
                    "printf(\"%%d\\n\", %s(aFile) == -1); return 0;}\n", fileno);
    if (assertCompAndLnk(buffer)) {
      fprintf(versionFile, "#define FILENO_WORKS_FOR_NULL %d\n", doTest() == 1);
    } /* if */
#endif
    return fileno;
  } /* determineFilenoFunction */



static void determineOsFunctions (FILE *versionFile)

  {
    char buffer[BUFFER_SIZE];
    const char *fileno;

  /* determineOsFunctions */
    determineIsattyFunction(versionFile);
    determineFdopenFunction(versionFile);
    fileno = determineFilenoFunction(versionFile);
    determineSocketLib(versionFile);
    determineOsDirAccess(versionFile);
    determineFseekFunctions(versionFile, fileno);
    determineLseekFunction(versionFile);
    determineFtruncate(versionFile, fileno);
#if defined OS_STRI_WCHAR
    determineOsWCharFunctions(versionFile);
    determineTimeFunctionProperties(versionFile);
#elif PATH_DELIMITER == '\\'
    if (compileAndLinkOk("#include <stdio.h>\n#include <direct.h>\n"
                         "#include <string.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{char buffer[1024];\n"
                         "printf(\"%d\\n\", getcwd(buffer, 1024) != NULL &&\n"
                         "    strchr(buffer, '\\\\') == NULL);\n"
                         "return 0;}\n") && doTest() == 1) {
      fprintf(versionFile, "#define OS_GETCWD_RETURNS_SLASH\n");
    } /* if */
#endif
    determineEnvironDefines(versionFile);
    if (compileAndLinkWithOptionsOk("#include <stdio.h>\n#include <sys/time.h>\n"
                                    "int main(int argc,char *argv[])\n"
                                    "{struct timeval time_val[2];\n"
                                    "time_val[0].tv_sec = 0;\n"
                                    "time_val[0].tv_usec = 0;\n"
                                    "time_val[1].tv_sec = 0;\n"
                                    "time_val[1].tv_usec = 0;\n"
                                    "printf(\"%d\\n\", lutimes(\"testfile\", time_val) == 0);\n"
                                    "return 0;}\n", "", SYSTEM_LIBS)) {
      fputs("#define HAS_LUTIMES\n", versionFile);
      fputs("#define os_lutimes lutimes\n", versionFile);
    } /* if */
    if (!compileAndLinkOk("#include <stdio.h>\n"
                          "int main(int argc,char *argv[]){\n"
                          "flockfile(stdin);\n"
                          "printf(\"%d\\n\", getc_unlocked(stdin)==' ');\n"
                          "funlockfile(stdin);\n"
                          "return 0;}\n")) {
      fprintf(versionFile, "#define flockfile(aFile)\n");
      fprintf(versionFile, "#define funlockfile(aFile)\n");
      fprintf(versionFile, "#define getc_unlocked(aFile) getc(aFile)\n");
    } /* if */
    if (compileAndLinkOk("#include <stdio.h>\n#include <windows.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{SetErrorMode(SEM_NOGPFAULTERRORBOX);\n"
                         "printf(\"%d\\n\", fopen(\"ctstfile.txt\", \"we\") != NULL);\n"
                         "return 0;}\n")) {
      fprintf(versionFile, "#define FOPEN_SUPPORTS_CLOEXEC_MODE %d\n", doTestNoResultCheck() == 1);
    } else if (assertCompAndLnk("#include <stdio.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{printf(\"%d\\n\", fopen(\"ctstfile.txt\", \"we\") != NULL);"
                                "return 0;}\n")) {
      fprintf(versionFile, "#define FOPEN_SUPPORTS_CLOEXEC_MODE %d\n", doTest() == 1);
    } /* if */
    doRemove("ctstfile.txt");
    if (compileAndLinkOk("#include <stdio.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{FILE *aFile;size_t bytesRead;char buffer[1];\n"
                         "if ((aFile = fopen(\"ctstfile.txt\", \"w\")) != NULL) {\n"
                         "  printf(\"%d\\n\", fread(buffer, 1, 1, aFile) != 0 ||\n"
                         "                    ferror(aFile) == 0);\n"
                         "  fclose(aFile);\n"
                         "} else { printf(\"0\\n\"); }\n"
                         "return 0;}\n")) {
      fprintf(versionFile, "#define FERROR_WRONG_WHEN_READING_FROM_WRITE_ONLY_FILE %d\n", doTest() == 1);
    } /* if */
    doRemove("ctstfile.txt");
    sprintf(buffer, "#include <stdio.h>\n#include <unistd.h>\n"
                    "#include <fcntl.h>\n"
                    "int main(int argc,char *argv[])\n"
                    "{FILE *aFile;int fd;\n"
                    "printf(\"%%d\\n\",\n"
                    "(aFile = fopen(\"ctstfile.txt\", \"w\")) != NULL &&\n"
                    "(fd = %s(aFile)) != -1 &&\n"
                    "fcntl(fd,F_SETFD,FD_CLOEXEC) == 0);\n"
                    "return 0;}\n", fileno);
    doRemove("ctstfile.txt");
    fprintf(versionFile, "#define HAS_FCNTL_SETFD_CLOEXEC %d\n",
            compileAndLinkOk(buffer) && doTest() == 1);
    fprintf(versionFile,
            "#define HAS_PIPE2 %d\n",
            compileAndLinkOk("#include <stdio.h>\n#include <unistd.h>\n"
                             "int main(int argc,char *argv[])\n"
                             "{int pipefd[2];\n"
                             "printf(\"%d\\n\", pipe2(pipefd, 0) == 0);\n"
                             "return 0;}\n") &&
            doTest() == 1);
    fprintf(versionFile,
            "#define HAS_SNPRINTF %d\n",
            compileAndLinkOk("#include <stdio.h>\n"
                             "int main(int argc,char *argv[])\n"
                             "{printf(\"%d\\n\", snprintf(NULL, 0, \"asdf\") == 4);\n"
                             "return 0;}\n") &&
            doTest() == 1);
    fprintf(versionFile,
            "#define HAS_VSNPRINTF %d\n",
            compileAndLinkOk("#include <stdio.h>\n#include <stdarg.h>\n"
                             "int test(const char *format, ...)\n"
                             "{int result;va_list ap;va_start(ap, format);\n"
                             "result = vsnprintf(NULL, 0, format, ap);\n"
                             "va_end(ap);return result;}\n"
                             "int main(int argc,char *argv[])\n"
                             "{printf(\"%d\\n\", test(\"asdf\") == 4);\n"
                             "return 0;}\n") &&
            doTest() == 1);
  } /* determineOsFunctions */



#if defined MOUNT_NODEFS
static void determineCurrentWorkingDirectory (char *cwd, size_t bufferSize)

  { /* determineCurrentWorkingDirectory */
    cwd[0] = '\0';
    if (compileAndLinkWithOptionsOk("#include \"stdio.h\"\n"
                                    "#include \"emscripten.h\"\n"
                                    "int main(int argc,char *argv[]) {\n"
                                    "EM_ASM(\n"
                                    "  let bslash = String.fromCharCode(92);\n"
                                    "  let workDir = process.cwd().replace(new RegExp(bslash + bslash, 'g'), '/');\n"
                                    "  if (workDir.charAt(1) === ':' && workDir.charAt(2) === '/') {\n"
                                    "    workDir = '/' + workDir.charAt(0).toLowerCase() + workDir.substring(2);\n"
                                    "  }\n"
                                    "  console.log(workDir);\n"
                                    "); return 0; }\n", "", "")) {
      testOutputToBuffer(cwd, bufferSize);
    } /* if */
  } /* determineCurrentWorkingDirectory */



#elif defined OS_STRI_WCHAR
static void determineCurrentWorkingDirectory (char *cwd, size_t bufferSize)

  { /* determineCurrentWorkingDirectory */
    cwd[0] = '\0';
    if (compileAndLinkWithOptionsOk("#include <stdio.h>\n#include <wchar.h>\n"
                                    "#include <direct.h>\n#include <ctype.h>\n"
                                    "int main(int argc,char *argv[])\n"
                                    "{wchar_t buffer[8192];\n"
                                    "wchar_t *wstri;\n"
                                    "unsigned long utf32;\n"
                                    "if (_wgetcwd(buffer, 8192) != NULL) {\n"
                                    "  if (isalpha(buffer[0]) && buffer[1]==':') {\n"
                                    "    buffer[1] = tolower(buffer[0]);\n"
                                    "    buffer[0] = '/';\n"
                                    "  }\n"
                                    "  for (wstri = buffer; *wstri != '\\0'; wstri++) {\n"
                                    "    if (*wstri <= 0x7F) {\n"
                                    "      if (*wstri == '\\\\') {\n"
                                    "        putchar('/');\n"
                                    "      } else {\n"
                                    "        putchar((unsigned char) *wstri);\n"
                                    "      }\n"
                                    "    } else if (*wstri <= 0x7FF) {\n"
                                    "      putchar((unsigned char) (0xC0 | (*wstri >>  6)));\n"
                                    "      putchar((unsigned char) (0x80 |( *wstri        & 0x3F)));\n"
                                    "    } else if (*wstri >= 0xD800 && *wstri <= 0xDBFF &&\n"
                                    "               wstri[1] >= 0xDC00 && wstri[1] <= 0xDFFF) {\n"
                                    "      utf32 = ((((unsigned long) *wstri   - 0xD800) << 10) +\n"
                                    "                ((unsigned long) wstri[1] - 0xDC00) + 0x10000);\n"
                                    "      wstri++;\n"
                                    "      putchar((unsigned char) (0xF0 | (utf32 >> 18)));\n"
                                    "      putchar((unsigned char) (0x80 |((utf32 >> 12) & 0x3F)));\n"
                                    "      putchar((unsigned char) (0x80 |((utf32 >>  6) & 0x3F)));\n"
                                    "      putchar((unsigned char) (0x80 |( utf32        & 0x3F)));\n"
                                    "    } else {\n"
                                    "      putchar((unsigned char) (0xE0 | (*wstri >> 12)));\n"
                                    "      putchar((unsigned char) (0x80 |((*wstri >>  6) & 0x3F)));\n"
                                    "      putchar((unsigned char) (0x80 |( *wstri        & 0x3F)));\n"
                                    "    }\n"
                                    "  }\n"
                                    "}\n"
                                    "printf(\"\\n\"); return 0;}\n", "", SYSTEM_LIBS)) {
      testOutputToBuffer(cwd, bufferSize);
    } /* if */
  } /* determineCurrentWorkingDirectory */

#else



static void determineCurrentWorkingDirectory (char *cwd, size_t bufferSize)

  { /* determineCurrentWorkingDirectory */
    cwd[0] = '\0';
    if (compileAndLinkWithOptionsOk("#include <stdio.h>\n#include <unistd.h>\n"
                                    "#include <ctype.h>\n"
                                    "int main(int argc,char *argv[])\n"
                                    "{char buffer[8192];\n"
                                    "char *stri;\n"
                                    "if (getcwd(buffer, 8192) != NULL) {\n"
                                    "  if (isalpha(buffer[0]) && buffer[1]==':') {\n"
                                    "    buffer[1] = tolower(buffer[0]);\n"
                                    "    buffer[0] = '/';\n"
                                    "  }\n"
                                    "  for (stri = buffer; *stri != '\\0'; stri++) {\n"
                                    "    if (*stri == '\\\\') {\n"
                                    "      *stri = '/';\n"
                                    "    }\n"
                                    "  }\n"
                                    "  printf(\"%s\", buffer);\n"
                                    "}\n"
                                    "printf(\"\\n\"); return 0;}\n", "", SYSTEM_LIBS)) {
      testOutputToBuffer(cwd, BUFFER_SIZE);
    } else if (compileAndLinkWithOptionsOk("#include <stdio.h>\n#include <direct.h>\n"
                                           "#include <ctype.h>\n"
                                           "int main(int argc,char *argv[])\n"
                                           "{char buffer[8192];\n"
                                           "char *stri;\n"
                                           "if (getcwd(buffer, 8192) != NULL) {\n"
                                           "  if (isalpha(buffer[0]) && buffer[1]==':') {\n"
                                           "    buffer[1] = tolower(buffer[0]);\n"
                                           "    buffer[0] = '/';\n"
                                           "  }\n"
                                           "  for (stri = buffer; *stri != '\\0'; stri++) {\n"
                                           "    if (*stri == '\\\\') {\n"
                                           "      *stri = '/';\n"
                                           "    }\n"
                                           "  }\n"
                                           "  printf(\"%s\", buffer);\n"
                                           "}\n"
                                           "printf(\"\\n\"); return 0;}\n", "", SYSTEM_LIBS)) {
      testOutputToBuffer(cwd, bufferSize);
    } /* if */
  } /* determineCurrentWorkingDirectory */
#endif



#ifdef OS_STRI_USES_CODE_PAGE
static unsigned char *conv_437[] = {
    "\000", "\001", "\002", "\003", "\004", "\005", "\006", "\007",
    "\b", "\t", "\n", "\013", "\f", "\r", "\016", "\017",
    "\020", "\021", "\022", "\023", "\024", "\025", "\026", "\027",
    "\030", "\031", "\032", "\033", "\034", "\035", "\036", "\037",
    " ", "!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?",
    "@", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "[", "\\", "]", "^", "_",
    "`", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "{", "|", "}", "~", "\177",
    "\303\207", "\303\274", "\303\251", "\303\242",
    "\303\244", "\303\240", "\303\245", "\303\247",
    "\303\252", "\303\253", "\303\250", "\303\257",
    "\303\256", "\303\254", "\303\204", "\303\205",
    "\303\211", "\303\246", "\303\206", "\303\264",
    "\303\266", "\303\262", "\303\273", "\303\271",
    "\303\277", "\303\226", "\303\234", "\302\242",
    "\302\243", "\302\245", "\342\202\247", "\306\222",
    "\303\241", "\303\255", "\303\263", "\303\272",
    "\303\261", "\303\221", "\302\252", "\302\272",
    "\302\277", "\342\214\220", "\302\254", "\302\275",
    "\302\274", "\302\241", "\302\253", "\302\273",
    "\342\226\221", "\342\226\222", "\342\226\223", "\342\224\202",
    "\342\224\244", "\342\225\241", "\342\225\242", "\342\225\226",
    "\342\225\225", "\342\225\243", "\342\225\221", "\342\225\227",
    "\342\225\235", "\342\225\234", "\342\225\233", "\342\224\220",
    "\342\224\224", "\342\224\264", "\342\224\254", "\342\224\234",
    "\342\224\200", "\342\224\274", "\342\225\236", "\342\225\237",
    "\342\225\232", "\342\225\224", "\342\225\251", "\342\225\246",
    "\342\225\240", "\342\225\220", "\342\225\254", "\342\225\247",
    "\342\225\250", "\342\225\244", "\342\225\245", "\342\225\231",
    "\342\225\230", "\342\225\222", "\342\225\223", "\342\225\253",
    "\342\225\252", "\342\224\230", "\342\224\214", "\342\226\210",
    "\342\226\204", "\342\226\214", "\342\226\220", "\342\226\200",
    "\316\261", "\303\237", "\316\223", "\317\200",
    "\316\243", "\317\203", "\302\265", "\317\204",
    "\316\246", "\316\230", "\316\251", "\316\264",
    "\342\210\236", "\317\206", "\316\265", "\342\210\251",
    "\342\211\241", "\302\261", "\342\211\245", "\342\211\244",
    "\342\214\240", "\342\214\241", "\303\267", "\342\211\210",
    "\302\260", "\342\210\231", "\302\267", "\342\210\232",
    "\342\201\277", "\302\262", "\342\226\240", "\302\240"};

static unsigned char *conv_850[] = {
    "\000", "\001", "\002", "\003", "\004", "\005", "\006", "\007",
    "\b", "\t", "\n", "\013", "\f", "\r", "\016", "\017",
    "\020", "\021", "\022", "\023", "\024", "\025", "\026", "\027",
    "\030", "\031", "\032", "\033", "\034", "\035", "\036", "\037",
    " ", "!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?",
    "@", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "[", "\\", "]", "^", "_",
    "`", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "{", "|", "}", "~", "\177",
    "\303\207", "\303\274", "\303\251", "\303\242",
    "\303\244", "\303\240", "\303\245", "\303\247",
    "\303\252", "\303\253", "\303\250", "\303\257",
    "\303\256", "\303\254", "\303\204", "\303\205",
    "\303\211", "\303\246", "\303\206", "\303\264",
    "\303\266", "\303\262", "\303\273", "\303\271",
    "\303\277", "\303\226", "\303\234", "\303\270",
    "\302\243", "\303\230", "\303\227", "\306\222",
    "\303\241", "\303\255", "\303\263", "\303\272",
    "\303\261", "\303\221", "\302\252", "\302\272",
    "\302\277", "\302\256", "\302\254", "\302\275",
    "\302\274", "\302\241", "\302\253", "\302\273",
    "\342\226\221", "\342\226\222", "\342\226\223", "\342\224\202",
    "\342\224\244", "\303\201", "\303\202", "\303\200",
    "\302\251", "\342\225\243", "\342\225\221", "\342\225\227",
    "\342\225\235", "\302\242", "\302\245", "\342\224\220",
    "\342\224\224", "\342\224\264", "\342\224\254", "\342\224\234",
    "\342\224\200", "\342\224\274", "\303\243", "\303\203",
    "\342\225\232", "\342\225\224", "\342\225\251", "\342\225\246",
    "\342\225\240", "\342\225\220", "\342\225\254", "\302\244",
    "\303\260", "\303\220", "\303\212", "\303\213",
    "\303\210", "\304\261", "\303\215", "\303\216",
    "\303\217", "\342\224\230", "\342\224\214", "\342\226\210",
    "\342\226\204", "\302\246", "\303\214", "\342\226\200",
    "\303\223", "\303\237", "\303\224", "\303\222",
    "\303\265", "\303\225", "\302\265", "\303\276",
    "\303\236", "\303\232", "\303\233", "\303\231",
    "\303\275", "\303\235", "\302\257", "\302\264",
    "\302\255", "\302\261", "\342\200\227", "\302\276",
    "\302\266", "\302\247", "\303\267", "\302\270",
    "\302\260", "\302\250", "\302\267", "\302\271",
    "\302\263", "\302\262", "\342\226\240", "\302\240"};

static unsigned char *conv_858[] = {
    "\000", "\001", "\002", "\003", "\004", "\005", "\006", "\007",
    "\b", "\t", "\n", "\013", "\f", "\r", "\016", "\017",
    "\020", "\021", "\022", "\023", "\024", "\025", "\026", "\027",
    "\030", "\031", "\032", "\033", "\034", "\035", "\036", "\037",
    " ", "!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?",
    "@", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "[", "\\", "]", "^", "_",
    "`", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "{", "|", "}", "~", "\177",
    "\303\207", "\303\274", "\303\251", "\303\242",
    "\303\244", "\303\240", "\303\245", "\303\247",
    "\303\252", "\303\253", "\303\250", "\303\257",
    "\303\256", "\303\254", "\303\204", "\303\205",
    "\303\211", "\303\246", "\303\206", "\303\264",
    "\303\266", "\303\262", "\303\273", "\303\271",
    "\303\277", "\303\226", "\303\234", "\303\270",
    "\302\243", "\303\230", "\303\227", "\306\222",
    "\303\241", "\303\255", "\303\263", "\303\272",
    "\303\261", "\303\221", "\302\252", "\302\272",
    "\302\277", "\302\256", "\302\254", "\302\275",
    "\302\274", "\302\241", "\302\253", "\302\273",
    "\342\226\221", "\342\226\222", "\342\226\223", "\342\224\202",
    "\342\224\244", "\303\201", "\303\202", "\303\200",
    "\302\251", "\342\225\243", "\342\225\221", "\342\225\227",
    "\342\225\235", "\302\242", "\302\245", "\342\224\220",
    "\342\224\224", "\342\224\264", "\342\224\254", "\342\224\234",
    "\342\224\200", "\342\224\274", "\303\243", "\303\203",
    "\342\225\232", "\342\225\224", "\342\225\251", "\342\225\246",
    "\342\225\240", "\342\225\220", "\342\225\254", "\302\244",
    "\303\260", "\303\220", "\303\212", "\303\213",
    "\303\210", "\342\202\254", "\303\215", "\303\216",
    "\303\217", "\342\224\230", "\342\224\214", "\342\226\210",
    "\342\226\204", "\302\246", "\303\214", "\342\226\200",
    "\303\223", "\303\237", "\303\224", "\303\222",
    "\303\265", "\303\225", "\302\265", "\303\276",
    "\303\236", "\303\232", "\303\233", "\303\231",
    "\303\275", "\303\235", "\302\257", "\302\264",
    "\302\255", "\302\261", "\342\200\227", "\302\276",
    "\302\266", "\302\247", "\303\267", "\302\270",
    "\302\260", "\302\250", "\302\267", "\302\271",
    "\302\263", "\302\262", "\342\226\240", "\302\240"};



static void mapCodePageToUtf8 (unsigned char **codePageTable, char *stri)

  {
    unsigned char *codePageChar;
    unsigned char *utf8Char;
    size_t utf8CharLength;
    unsigned char *utf8;
    unsigned char utf8Buffer[BUFFER_SIZE];

  /* mapCodePageToUtf8 */
    utf8 = utf8Buffer;
    for (codePageChar = stri; *codePageChar != '\0'; codePageChar++) {
      utf8Char = codePageTable[(unsigned char) *codePageChar];
      utf8CharLength = strlen(utf8Char);
      memcpy(utf8, utf8Char, utf8CharLength);
      utf8 += utf8CharLength;
    } /* for */
    *utf8 = '\0';
    strcpy(stri, utf8Buffer);
  } /* mapCodePageToUtf8 */



static void mapToUtf8 (int codePage, char *stri)

  { /* mapToUtf8 */
    if (codePage == 437) {
      mapCodePageToUtf8(conv_437, stri);
    } else if (codePage == 850) {
      mapCodePageToUtf8(conv_850, stri);
    } else if (codePage == 858) {
      mapCodePageToUtf8(conv_858, stri);
    } /* if */
  } /* mapToUtf8 */
#endif



static int linkerOptionAllowed (const char *linkerOption)

  {
    int optionAllowed;

  /* linkerOptionAllowed */
    optionAllowed = compileAndLinkWithOptionsOk(
        "#include <stdio.h>\n"
        "int main(int argc,char *argv[]){\n"
        "printf(\"hello world\\n\");\n"
        "return 0;}\n", "", linkerOption);
    return optionAllowed;
  } /* linkerOptionAllowed */



static void appendToFile (const char *fileName, const char *data)

  {
    FILE *outFile;

  /* appendToFile */
    outFile = fopen(fileName, "a");
    if (outFile != NULL) {
      fputs(data, outFile);
      fclose(outFile);
    } /* if */
  } /* appendToFile */



static void appendToMakeMacros (const char *macroName, const char *macroValue)

  {
    char buffer[BUFFER_SIZE];

  /* appendToMakeMacros */
    sprintf(buffer, "%s = %s", macroName, macroValue);
    replaceNLBySpace(buffer);
    strcat(buffer, "\n");
    appendToFile("macros", buffer);
  } /* appendToMakeMacros */



static void defineCMacro (FILE *versionFile, const char *macroName,
    const char *macroValue)

  {
    char buffer[BUFFER_SIZE];

  /* defineCMacro */
    fprintf(versionFile, "#define %s \"", macroName);
    strcpy(buffer, macroValue);
    removeQuotesFromQuotations(buffer);
    escapeString(versionFile, buffer);
    fprintf(versionFile, "\"\n");
  } /* defineCMacro */



static void determineOptionForLinkTimeOptimization (FILE *versionFile)

  {
    char command[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    char libraryName[NAME_SIZE];
    char executableName[NAME_SIZE];
    int returncode;
    int canDoLinkTimeOptimization = 0;
    int linkerOptLtoMandatory = 0;
    int canDoNoLinkTimeOptimization = 0;

  /* determineOptionForLinkTimeOptimization */
    fprintf(logFile, "Check for link time optimization: ");
#ifdef CC_OPT_LINK_TIME_OPTIMIZATION
    if (compileAndLinkWithOptionsOk("#include <stdio.h>\n"
                                    "int main (int argc, char *argv[]) {\n"
                                    "printf(\"%d\\n\", 1);\n"
                                    "return 0; }\n",
                                    CC_OPT_LINK_TIME_OPTIMIZATION,
                                    CC_OPT_LINK_TIME_OPTIMIZATION) &&
        doTest() == 1) {
      if (compileWithOptionsOk("#include <stdio.h>\n"
                               "int main (int argc, char *argv[]) {\n"
                               "printf(\"%d\\n\", 1);\n"
                               "return 0; }\n",
                               CC_OPT_LINK_TIME_OPTIMIZATION)) {
        sprintf(libraryName, "ctest%d%s", testNumber, LIBRARY_FILE_EXTENSION);
        doRemove(libraryName);
        sprintf(command, "%s %s%s ctest%d%s",
                ARCHIVER, ARCHIVER_OPT_REPLACE, libraryName,
                testNumber, OBJECT_FILE_EXTENSION);
#ifdef ERROR_REDIRECTING_FAILS
        sprintf(&command[strlen(command)], " %s%s",
                REDIRECT_FILEDES_1, nullDevice);
#else
        sprintf(&command[strlen(command)], " %s%s %s%s",
                REDIRECT_FILEDES_1, nullDevice, REDIRECT_FILEDES_2, nullDevice);
#endif
        /* fprintf(logFile, "command: %s\n", command); */
        returncode = system(command);
        /* fprintf(logFile, "returncode: %d\n", returncode); */
        if (fileIsPresentPossiblyAfterDelay(libraryName)) {
          /* fprintf(logFile, " Library present.\n"); */
          if (doLink(libraryName, "")) {
            canDoLinkTimeOptimization = doTest() == 1;
          } else if (doLink(libraryName, CC_OPT_LINK_TIME_OPTIMIZATION)) {
            if (doTest() == 1) {
              canDoLinkTimeOptimization = 1;
              linkerOptLtoMandatory = 1;
            } /* if */
          } /* if */
#ifdef LINKER_OPT_NO_LTO
          if (canDoLinkTimeOptimization) {
            sprintf(executableName, "ctest%d%s", testNumber, LINKED_PROGRAM_EXTENSION);
            doRemove(executableName);
            if (doLink(libraryName, LINKER_OPT_NO_LTO)) {
              canDoNoLinkTimeOptimization = doTest() == 1;
            } /* if */
          } /* if */
#endif
          doRemove(libraryName);
        } /* if */
      } /* if */
    } /* if */
    if (canDoLinkTimeOptimization) {
      fprintf(logFile, " Done with the option: %s\n", CC_OPT_LINK_TIME_OPTIMIZATION);
      fprintf(versionFile, "#define CC_OPT_LINK_TIME_OPTIMIZATION \"%s\"\n",
              CC_OPT_LINK_TIME_OPTIMIZATION);
      sprintf(buffer, "CC_OPT_LINK_TIME_OPTIMIZATION = %s\n",
              CC_OPT_LINK_TIME_OPTIMIZATION);
      appendToFile("macros", buffer);
      fprintf(versionFile, "#define LINKER_OPT_LTO_MANDATORY %d\n", linkerOptLtoMandatory);
#ifdef LINKER_OPT_NO_LTO
      if (canDoNoLinkTimeOptimization) {
        fprintf(versionFile, "#define LINKER_OPT_NO_LTO \"%s\"\n", LINKER_OPT_NO_LTO);
        sprintf(buffer, "LINKER_OPT_NO_LTO = %s\n", LINKER_OPT_NO_LTO);
        appendToFile("macros", buffer);
      } /* if */
#endif
    } else {
      fprintf(logFile, " Not available.\n");
    } /* if */
#else
    fprintf(logFile, "Not available.\n");
#endif
  } /* determineOptionForLinkTimeOptimization */



#ifdef POTENTIAL_PARTIAL_LINKING_OPTIONS
static int checkPartialLinking (const char *ccOptPartialLinking)

  {
    char fileName[NAME_SIZE];
    char linkOptions[COMMAND_SIZE];
    int okay = 1;
    int partialLinkingOkay = 0;

  /* checkPartialLinking */
    okay = compileWithOptionsOk("int f_1(void) { return 1; }\n", "");
    sprintf(fileName, "ctest%d%s", testNumber, OBJECT_FILE_EXTENSION);
    okay = okay && rename(fileName, "ctest_f1" OBJECT_FILE_EXTENSION) == 0;
    okay = okay && compileWithOptionsOk("int f_1(void) { return 2; }\n", "");
    sprintf(fileName, "ctest%d%s", testNumber, OBJECT_FILE_EXTENSION);
    okay = okay && rename(fileName, "ctest_f2" OBJECT_FILE_EXTENSION) == 0;
    okay = okay && compileWithOptionsOk("int f_1(void);\n"
                                        "int f_3(void) { return f_1(); }\n", "");
    sprintf(fileName, "ctest%d%s", testNumber, OBJECT_FILE_EXTENSION);
    okay = okay && rename(fileName, "ctest_f3" OBJECT_FILE_EXTENSION) == 0;
    sprintf(linkOptions, "%s ctest_f1" OBJECT_FILE_EXTENSION, ccOptPartialLinking);
    okay = okay && doLink("ctest_f3" OBJECT_FILE_EXTENSION, linkOptions);
    sprintf(fileName, "ctest%d%s", testNumber, LINKED_PROGRAM_EXTENSION);
    okay = okay && rename(fileName, "ctest_f5" OBJECT_FILE_EXTENSION) == 0;
    okay = okay && system("objcopy -L f_1 ctest_f5" OBJECT_FILE_EXTENSION) != -1;
    okay = okay && compileWithOptionsOk("int f_1(void);\n"
                                        "int f_4(void) { return f_1(); }\n", "");
    sprintf(fileName, "ctest%d%s", testNumber, OBJECT_FILE_EXTENSION);
    okay = okay && rename(fileName, "ctest_f4" OBJECT_FILE_EXTENSION) == 0;
    sprintf(linkOptions, "%s ctest_f2" OBJECT_FILE_EXTENSION, ccOptPartialLinking);
    okay = okay && doLink("ctest_f4" OBJECT_FILE_EXTENSION, linkOptions);
    sprintf(fileName, "ctest%d%s", testNumber, LINKED_PROGRAM_EXTENSION);
    okay = okay && rename(fileName, "ctest_f6" OBJECT_FILE_EXTENSION) == 0;
    okay = okay && system("objcopy -L f_1 ctest_f6" OBJECT_FILE_EXTENSION) != -1;
    okay = okay && compileWithOptionsOk("#include \"stdio.h\"\n"
                                        "int f_3(void);\n"
                                        "int f_4(void);\n"
                                        "int main(void) {\n"
                                        "printf(\"%d\\n\", f_3() == 1 && f_4() == 2);\n"
                                        "return 0; }\n", "");
    sprintf(fileName, "ctest%d%s", testNumber, OBJECT_FILE_EXTENSION);
    okay = okay && rename(fileName, "ctest_f7" OBJECT_FILE_EXTENSION) == 0;
    okay = okay && doLink("ctest_f7" OBJECT_FILE_EXTENSION,
                          "ctest_f5" OBJECT_FILE_EXTENSION " ctest_f6" OBJECT_FILE_EXTENSION);
    sprintf(fileName, "ctest%d%s", testNumber, LINKED_PROGRAM_EXTENSION);
    if (okay && fileIsRegular(fileName)) {
      partialLinkingOkay = doTest() == 1;
    } /* if */
    doRemove("ctest_f1" OBJECT_FILE_EXTENSION);
    doRemove("ctest_f2" OBJECT_FILE_EXTENSION);
    doRemove("ctest_f3" OBJECT_FILE_EXTENSION);
    doRemove("ctest_f4" OBJECT_FILE_EXTENSION);
    doRemove("ctest_f5" OBJECT_FILE_EXTENSION);
    doRemove("ctest_f6" OBJECT_FILE_EXTENSION);
    doRemove("ctest_f7" OBJECT_FILE_EXTENSION);
    return partialLinkingOkay;
  } /* checkPartialLinking */
#endif



static void determinePartialLinking (FILE *versionFile)

  {
#ifdef POTENTIAL_PARTIAL_LINKING_OPTIONS
    const char *potentialPartialLinkingOptions[] = { POTENTIAL_PARTIAL_LINKING_OPTIONS };
#endif
    unsigned int pos;
    char buffer[BUFFER_SIZE];

  /* determinePartialLinking */
#ifdef POTENTIAL_PARTIAL_LINKING_OPTIONS
    fprintf(logFile, "Check for partial linking: ");
    for (pos = 0; pos < sizeof(potentialPartialLinkingOptions) / sizeof(char *) &&
         !supportsPartialLinking; pos++) {
      supportsPartialLinking = checkPartialLinking(potentialPartialLinkingOptions[pos]);
      if (supportsPartialLinking) {
        fprintf(logFile, " Supported.\n");
        fprintf(versionFile, "#define LINKER_OPT_PARTIAL_LINKING \"%s\"\n",
                potentialPartialLinkingOptions[pos]);
        sprintf(buffer, "LINKER_OPT_PARTIAL_LINKING = %s\n",
                potentialPartialLinkingOptions[pos]);
        appendToFile("macros", buffer);
        appendToFile("macros", "OBJCOPY = objcopy\n");
      } /* if */
    } /* for */
    if (!supportsPartialLinking) {
      fprintf(logFile, " Not supported.\n");
    } /* if */
#endif
    if (!supportsPartialLinking) {
      appendToFile("macros", "LINKER_OPT_PARTIAL_LINKING =\n");
      appendToFile("macros", "OBJCOPY = @echo \"No partial linking with objcopy\"\n");
    } /* if */
  } /* determinePartialLinking */



static void appendOption (char *options, const char *optionToAppend)

  {
    size_t length;
    char *startPos;
    char *optionFound;

  /* appendOption */
    if (optionToAppend[0] != '\0') {
      /* printf("appendOption(\"%s\", \"%s\")\n", options, optionToAppend); */
      length = strlen(optionToAppend);
      startPos = options;
      do {
        optionFound = strstr(startPos, optionToAppend);
        if (optionFound != NULL) {
          startPos = &optionFound[1];
        } /* if */
      } while (optionFound != NULL &&
               ((optionFound != options && optionFound[-1] != '\n') ||
                (optionFound[length] != '\0' && optionFound[length] != '\n')));
      if (optionFound == NULL ||
          (optionFound != options && optionFound[-1] != '\n') ||
          (optionFound[length] != '\0' && optionFound[length] != '\n')) {
        /* The optionToAppend is not an element in the list of options. */
        if (options[0] != '\0') {
          strcat(options, "\n");
        } /* if */
        strcat(options, optionToAppend);
      } /* if */
      /* printf("appendOption --> \"%s\"\n", options); */
    } /* if */
  } /* appendOption */



static const char *findIncludeFile (const char *scopeName, char *testProgram,
    const char *baseDir, const char **inclDirList, size_t inclDirListLength,
    const char *inclName, char *includeOption)

  {
    unsigned int dirIndex;
    char dirPath[PATH_SIZE];
    char filePath[PATH_SIZE + 1 + NAME_SIZE];
    char inclOption[PATH_SIZE + 4];
    const char *includeFileName = NULL;

  /* findIncludeFile */
    /* fprintf(logFile, "baseDir=%s\n", baseDir); */
    for (dirIndex = 0;
         includeFileName == NULL && dirIndex < inclDirListLength;
         dirIndex++) {
      sprintf(dirPath, "%s%s", baseDir, inclDirList[dirIndex]);
      /* printf("dirPath: %s\n", dirPath); */
      if (fileIsDir(dirPath)) {
        /* printf("fileIsDir(%s)\n", dirPath); */
        sprintf(filePath, "%s/%s", dirPath, inclName);
        /* printf("filePath: %s\n", filePath); */
        if (fileIsRegular(filePath)) {
          /* printf("fileIsRegular(%s)\n", filePath); */
          sprintf(inclOption, "-I\"%s\"", dirPath);
          /* fprintf(logFile, "inclOption=%s\n", inclOption); */
          if (compileAndLinkWithOptionsOk(testProgram, inclOption, "")) {
            includeFileName = inclName;
            fprintf(logFile, "\r%s: %s found in %s\n",
                    scopeName, includeFileName, dirPath);
            strcpy(includeOption, inclOption);
          } else {
            fprintf(logFile, "\r%s: The C compiler cannot include %s\n",
                    scopeName, filePath);
          } /* if */
        } /* if */
      } /* if */
    } /* for */
    return includeFileName;
  } /* findIncludeFile */



static int pointerSizeOfDynamicLibrary (const char *dllName)

  {
    FILE *dllFile;
    unsigned char buffer[BUFFER_SIZE];
    unsigned long offset;
    unsigned long machine;
    int pointerSize = 0;

  /* pointerSizeOfDynamicLibrary */
    dllFile = fopen(dllName, "rb");
    if (dllFile != NULL) {
      if (fread(buffer, 1, 5, dllFile) == 5) {
        if (memcmp(buffer, "\177ELF", 4) == 0) {
          if (buffer[4] == '\1') {
            pointerSize = 32;
          } else if (buffer[4] == '\2') {
            pointerSize = 64;
          } /* if */
        } else if (memcmp(buffer, "\376\355\372\316" /* 0xfeedface */, 4) == 0 ||
                   memcmp(buffer, "\316\372\355\376" /* 0xcefaedfe */, 4) == 0) {
          pointerSize = 32;
        } else if (memcmp(buffer, "\376\355\372\317" /* 0xfeedfacf */, 4) == 0 ||
                   memcmp(buffer, "\317\372\355\376" /* 0xcffaedfe */, 4) == 0) {
          pointerSize = 64;
        } else if (memcmp(buffer, "MZ", 2) == 0) {
          if (fread(&buffer[5], 1, 59, dllFile) == 59) {
            offset = (unsigned long) buffer[60] |
                     (unsigned long) buffer[61] << 8 |
                     (unsigned long) buffer[62] << 16 |
                     (unsigned long) buffer[63] << 24;
            if (offset <= BUFFER_SIZE - 6 &&
                fread(&buffer[64], 1, offset - 58, dllFile) == offset - 58) {
              if (memcmp(&buffer[offset], "PE\0\0", 4) == 0) {
                machine = (unsigned long) buffer[offset + 4] |
                          (unsigned long) buffer[offset + 5] << 8;
                if (machine ==  0x184 || machine ==  0x1d3 || machine ==  0x1c0 ||
                    machine ==  0x14c || machine == 0x6232 || machine == 0x9041 ||
                    machine == 0x5032 || machine ==  0x1a2 || machine ==  0x1a3 ||
                    machine ==  0x1a6) {
                  pointerSize = 32;
                } else if (machine ==  0x284 || machine == 0x8664 || machine == 0xaa64 ||
                           machine ==  0x200 || machine == 0x6264 || machine ==  0x1f0 ||
                           machine ==  0x1f1 || machine ==  0x166 || machine == 0x5064 ||
                           machine ==  0x1a8) {
                  pointerSize = 64;
                } /* if */
              } /* if */
            } /* if */
          } /* if */
        } /* if */
      } /* if */
      fclose(dllFile);
    } /* if */
    return pointerSize;
  } /* pointerSizeOfDynamicLibrary */



static void describeLibrary (const char *filePath)

  {
    int dllPointerSize;

  /* describeLibrary */
    dllPointerSize = pointerSizeOfDynamicLibrary(filePath);
    if (dllPointerSize != 0) {
      fprintf(logFile, " (%d-bit)", dllPointerSize);
    } /* if */
  } /* describeLibrary */



static int findStaticLib (const char *scopeName, const char *testProgram,
    const char *includeOption, const char *libraryOption, const char *baseDir,
    const char **libDirList, size_t libDirListLength,
    const char **libNameList, size_t libNameListLength, char *system_libs)

  {
    unsigned int dirIndex;
    unsigned int nameIndex;
    char dirPath[PATH_SIZE];
    char filePath[PATH_SIZE + 1 + NAME_SIZE];
    char linkParam[PATH_SIZE + 3 + NAME_SIZE];
    char linkOption[PATH_SIZE + 4 + 2 * NAME_SIZE];
    int libFound = 0;

  /* findStaticLib */
    for (dirIndex = 0;
         !libFound && dirIndex < libDirListLength;
         dirIndex++) {
      sprintf(dirPath, "%s%s", baseDir, libDirList[dirIndex]);
      /* printf("dirPath: %s\n", dirPath); */
      if (fileIsDir(dirPath)) {
        /* printf("fileIsDir(%s)\n", dirPath); */
        for (nameIndex = 0;
             !libFound && nameIndex < libNameListLength;
             nameIndex++) {
          sprintf(filePath, "%s/%s", dirPath, libNameList[nameIndex]);
          /* printf("filePath: %s\n", filePath); */
          if (fileIsRegular(filePath)) {
            /* printf("fileIsRegular(%s)\n", filePath); */
            if (strchr(filePath, ' ') != NULL) {
              sprintf(linkParam, "\"%s\"", filePath);
            } else {
              strcpy(linkParam, filePath);
            } /* if */
            linkOption[0] = '\0';
            appendOption(linkOption, libraryOption);
            appendOption(linkOption, linkParam);
            /* fprintf(logFile, "includeOption: \"%s\"\n", includeOption);
            fprintf(logFile, "linkParam: \"%s\"\n", linkParam);
            fprintf(logFile, "linkOption: \"%s\"\n", linkOption); */
            if (compileAndLinkWithOptionsOk(testProgram, includeOption, linkOption)) {
              if (doTest() == 1) {
                fprintf(logFile, "\r%s: %s", scopeName, libNameList[nameIndex]);
                describeLibrary(filePath);
                fprintf(logFile, " found in: %s\n", dirPath);
                appendOption(system_libs, linkParam);
                libFound = 1;
              } else {
                fprintf(logFile, "\r%s: Cannot execute with %s", scopeName, filePath);
                describeLibrary(filePath);
                fprintf(logFile, "\n");
              } /* if */
            } else {
              fprintf(logFile, "\r%s: Cannot link %s", scopeName, filePath);
              describeLibrary(filePath);
              fprintf(logFile, "\n");
            } /* if */
          } else {
            if (strchr(dirPath, ' ') != NULL) {
              sprintf(linkParam, "\"-L%s\"", dirPath);
            } else {
              sprintf(linkParam, "-L%s", dirPath);
            } /* if */
            linkOption[0] = '\0';
            appendOption(linkOption, libraryOption);
            appendOption(linkOption, linkParam);
            appendOption(linkOption, libNameList[nameIndex]);
            /* fprintf(logFile, "includeOption: \"%s\"\n", includeOption);
               fprintf(logFile, "linkParam: \"%s\"\n", linkParam); */
            if (compileAndLinkWithOptionsOk(testProgram, includeOption, linkOption)) {
              if (doTest() == 1) {
                fprintf(logFile, "\r%s: %s found with the option: %s\n",
                        scopeName, libNameList[nameIndex], linkParam);
                appendOption(system_libs, linkParam);
                appendOption(system_libs, libNameList[nameIndex]);
                libFound = 1;
              } /* if */
            } /* if */
          } /* if */
        } /* for */
      } /* if */
    } /* for */
    return libFound;
  } /* findStaticLib */



static int findLinkerOption (const char *scopeName, const char *testProgram,
    const char *includeOption, const char *libraryOption, const char **libNameList,
    size_t libNameListLength, char *system_libs)

  {
    unsigned int nameIndex;
    char linkParam[PATH_SIZE];
    int libFound = 0;

  /* findLinkerOption */
    for (nameIndex = 0;
         !libFound && nameIndex < libNameListLength;
         nameIndex++) {
      linkParam[0] = '\0';
      appendOption(linkParam, libraryOption);
      appendOption(linkParam, libNameList[nameIndex]);
      /* fprintf(logFile, "includeOption: \"%s\"\n", includeOption);
         fprintf(logFile, "linkParam: \"%s\"\n", linkParam); */
      if (compileAndLinkWithOptionsOk(testProgram, includeOption, linkParam)) {
        if (doTest() == 1) {
          fprintf(logFile, "\r%s: Linker option: %s\n",
                  scopeName, libNameList[nameIndex]);
          appendOption(system_libs, libraryOption);
          appendOption(system_libs, libNameList[nameIndex]);
          libFound = 1;
        } /* if */
      } /* if */
    } /* for */
    return libFound;
  } /* findLinkerOption */



static int findLinkerOptionForDll (const char *scopeName, const char *testProgram,
    const char *includeOption, const char *libraryOption, const char **dllNameList,
    size_t dllNameListLength, char *system_libs)

  {
    unsigned int nameIndex;
    char linkParam[PATH_SIZE];
    char linkOption[PATH_SIZE];
    int libFound = 0;

  /* findLinkerOptionForDll */
    for (nameIndex = 0;
         !libFound && nameIndex < dllNameListLength;
         nameIndex++) {
      sprintf(linkOption, "-l:%s", dllNameList[nameIndex]);
      linkParam[0] = '\0';
      appendOption(linkParam, libraryOption);
      appendOption(linkParam, linkOption);
      /* fprintf(logFile, "includeOption: \"%s\"\n", includeOption);
         fprintf(logFile, "linkParam: \"%s\"\n", linkParam); */
      if (compileAndLinkWithOptionsOk(testProgram, includeOption, linkParam)) {
        if (doTest() == 1) {
          fprintf(logFile, "\r%s: Linker option: %s\n",
                  scopeName, linkOption);
          appendOption(system_libs, libraryOption);
          appendOption(system_libs, linkOption);
          libFound = 1;
        } /* if */
      } /* if */
    } /* for */
    return libFound;
  } /* findLinkerOptionForDll */



static int canLoadDynamicLibrary (const char *dllName)

  {
    char dllPath[PATH_SIZE];
    char testProgram[BUFFER_SIZE];
    const char *srcChar;
    char *destChar;

  /* canLoadDynamicLibrary */
#if LIBRARY_TYPE == WINDOWS_LIBRARIES
    for (srcChar = &dllName[0], destChar = dllPath;
         *srcChar != '\0';
         srcChar++) {
      if (*srcChar == '/' || *srcChar == '\\') {
        *destChar++ = '\\';
        *destChar++ = '\\';
      } else {
        *destChar++ = *srcChar;
      } /* if */
    } /* for */
    *destChar = '\0';
    sprintf(testProgram,
            "#include <stdio.h>\n#include <windows.h>\n"
            "int main (int argc, char *argv[]) {\n"
            "SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);\n"
            "printf(\"%%d\\n\", LoadLibrary(\"%s\") != NULL);\n"
            "return 0; }\n",
            dllPath);
#else
    sprintf(testProgram,
            "#include <stdio.h>\n#include <dlfcn.h>\n"
            "int main (int argc, char *argv[]) {\n"
            "printf(\"%%d\\n\", dlopen(\"%s\", RTLD_LAZY) != NULL);\n"
            "return 0; }\n",
            dllName);
#endif
    if (compileAndLinkWithOptionsOk(testProgram, "", LINKER_OPT_DYN_LINK_LIBS)) {
      return doTest() == 1;
    } else {
      return 0;
    } /* if */
  } /* canLoadDynamicLibrary */



static void listDynamicLibs (const char *scopeName, const char *baseDir,
    const char **dllDirList, size_t dllDirListLength,
    const char **dllNameList, size_t dllNameListLength, FILE *versionFile)

  {
    unsigned int dirIndex;
    unsigned int nameIndex;
    int dllPointerSize;
    char dirPath[PATH_SIZE];
    char filePath[PATH_SIZE + 1 + NAME_SIZE];

  /* listDynamicLibs */
    for (dirIndex = 0;
         dirIndex < dllDirListLength;
         dirIndex++) {
      sprintf(dirPath, "%s%s", baseDir, dllDirList[dirIndex]);
      /* printf("dirPath: %s\n", dirPath); */
      if (fileIsDir(dirPath)) {
        /* printf("fileIsDir(%s)\n", dirPath); */
        for (nameIndex = 0;
             nameIndex < dllNameListLength;
             nameIndex++) {
          sprintf(filePath, "%s/%s", dirPath, dllNameList[nameIndex]);
          /* printf("filePath: %s\n", filePath); */
          if (fileIsRegular(filePath)) {
            /* printf("fileIsRegular(%s)\n", filePath); */
            fprintf(logFile, "\r%s: DLL / Shared library: %s (%s)",
                    scopeName, filePath,
                    canLoadDynamicLibrary(filePath) ? "present" : "cannot load");
            dllPointerSize = pointerSizeOfDynamicLibrary(filePath);
            if (dllPointerSize != 0) {
              fprintf(logFile, " (%d-bit)", dllPointerSize);
            } /* if */
            fprintf(logFile, "\n");
            fprintf(versionFile, " \"");
            escapeString(versionFile, filePath);
            fprintf(versionFile, "\",");
          } /* if */
        } /* for */
      } /* if */
    } /* for */
  } /* listDynamicLibs */



static void listDynamicLibsInSameDir (const char *scopeName, const char *baseDllPath,
    const char **dllNameList, size_t dllNameListLength, FILE *versionFile)

  {
    const char *slashPos;
    const char *backslashPos;
    const char *dirPathEnd;
    unsigned int nameIndex;
    int dllPointerSize;
    char dirPath[PATH_SIZE];
    char filePath[PATH_SIZE + 1 + NAME_SIZE];

  /* listDynamicLibsInSameDir */
    slashPos = strrchr(baseDllPath, '/');
    backslashPos = strrchr(baseDllPath, '\\');
    if (slashPos != NULL || backslashPos != NULL) {
      if (slashPos != NULL) {
        if (backslashPos != NULL) {
          if (slashPos > backslashPos) {
            dirPathEnd = slashPos;
          } else {
            dirPathEnd = backslashPos;
          } /* if */
        } else {
          dirPathEnd = slashPos;
        } /* if */
      } else {
        dirPathEnd = backslashPos;
      } /* if */
      memcpy(dirPath, baseDllPath, dirPathEnd - baseDllPath);
      dirPath[dirPathEnd - baseDllPath] = '\0';
      for (nameIndex = 0;
           nameIndex < dllNameListLength;
           nameIndex++) {
        sprintf(filePath, "%s/%s", dirPath, dllNameList[nameIndex]);
        /* printf("filePath: %s\n", filePath); */
        if (fileIsRegular(filePath)) {
          /* printf("fileIsRegular(%s)\n", filePath); */
          fprintf(logFile, "\r%s: DLL / Shared library: %s (%s)",
                  scopeName, filePath,
                  canLoadDynamicLibrary(filePath) ? "present" : "cannot load");
          dllPointerSize = pointerSizeOfDynamicLibrary(filePath);
          if (dllPointerSize != 0) {
            fprintf(logFile, " (%d-bit)", dllPointerSize);
          } /* if */
          fprintf(logFile, "\n");
          fprintf(versionFile, " \"");
          escapeString(versionFile, filePath);
          fprintf(versionFile, "\",");
        } /* if */
      } /* for */
    } /* if */
  } /* listDynamicLibsInSameDir */



static void defineLibraryMacro (const char *scopeName, int dbHomeExists,
    char *dbHome, const char *macroName, const char **dllDirList,
    size_t dllDirListLength, const char **baseDllNameList,
    size_t baseDllNameListLength, const char **dllNameList,
    size_t dllNameListLength, FILE *versionFile)

  {
    unsigned int nameIndex;

  /* defineLibraryMacro */
    if (dllNameListLength > 1 ||
        (dllNameListLength == 1 && dllNameList[0][0] != '\0')) {
      fprintf(versionFile, "#define %s", macroName);
      if (dbHomeExists) {
        listDynamicLibs(scopeName, dbHome,
                        dllDirList, dllDirListLength,
                        dllNameList, dllNameListLength, versionFile);
      } /* if */
      for (nameIndex = 0; nameIndex < baseDllNameListLength; nameIndex++) {
        listDynamicLibsInSameDir(scopeName, baseDllNameList[nameIndex], dllNameList,
                                 dllNameListLength, versionFile);
      } /* for */
      for (nameIndex = 0; nameIndex < dllNameListLength; nameIndex++) {
        fprintf(logFile, "\r%s: DLL / Shared library: %s (%s)\n",
                scopeName, dllNameList[nameIndex],
                canLoadDynamicLibrary(dllNameList[nameIndex]) ? "present" : "cannot load");
        fprintf(versionFile, " \"%s\",", dllNameList[nameIndex]);
      } /* for */
      fprintf(versionFile, "\n");
    } /* if */
  } /* defineLibraryMacro */



static void addDynamicLibToDllListWithRpath (const char *scopeName, const char *baseDir,
    const char **dllDirList, size_t dllDirListLength,
    const char **dllNameList, size_t dllNameListLength, char *rpath,
    char *dllList)

  {
    unsigned int dirIndex;
    unsigned int nameIndex;
    int dllPointerSize;
    char dirPath[PATH_SIZE];
    char filePath[PATH_SIZE + 1 + NAME_SIZE];
    int found = 0;

  /* addDynamicLibToDllListWithRpath */
    for (dirIndex = 0;
         !found && dirIndex < dllDirListLength;
         dirIndex++) {
      sprintf(dirPath, "%s%s", baseDir, dllDirList[dirIndex]);
      /* printf("dirPath: %s\n", dirPath); */
      if (fileIsDir(dirPath)) {
        /* printf("fileIsDir(%s)\n", dirPath); */
        for (nameIndex = 0;
             nameIndex < dllNameListLength;
             nameIndex++) {
          sprintf(filePath, "%s/%s", dirPath, dllNameList[nameIndex]);
          /* printf("filePath: %s\n", filePath); */
          if (fileIsRegular(filePath)) {
            /* printf("fileIsRegular(%s)\n", filePath); */
            fprintf(logFile, "\r%s: DLL / Shared library: %s",
                    scopeName, filePath);
            dllPointerSize = pointerSizeOfDynamicLibrary(filePath);
            if (dllPointerSize != 0) {
              fprintf(logFile, " (%d-bit)", dllPointerSize);
            } /* if */
            fprintf(logFile, "\n");
            if (rpath == NULL) {
              appendOption(dllList, filePath);
            } /* if */
            appendOption(dllList, dllNameList[nameIndex]);
            if (rpath != NULL) {
              if (strstr(rpath, dirPath) == NULL) {
                if (rpath[0] != '\0') {
                  strcat(rpath, ":");
                } /* if */
                strcat(rpath, dirPath);
              } /* if */
              found = 1;
            } /* if */
          } /* if */
        } /* for */
      } /* if */
    } /* for */
  } /* addDynamicLibToDllListWithRpath */



static void addDynamicLib (const char *scopeName,
    const char **dllNameList, size_t dllNameListLength,
    char *dllList)

  {
    unsigned int nameIndex;

  /* addDynamicLib */
    for (nameIndex = 0;
         nameIndex < dllNameListLength;
         nameIndex++) {
      fprintf(logFile, "\r%s: DLL / Shared library: %s (%s)\n",
              scopeName, dllNameList[nameIndex],
              canLoadDynamicLibrary(dllNameList[nameIndex]) ? "present" : "cannot load");
      appendOption(dllList, dllNameList[nameIndex]);
    } /* for */
  } /* addDynamicLib */



static void addDynamicLibsWithRpath (const char *scopeName, int baseDirExists,
    const char *baseDir, const char **dllDirList, size_t dllDirListLength,
    const char **dllNameList, size_t dllNameListLength, char *rpath,
    FILE *versionFile)

  {
    char dllList[BUFFER_SIZE];
    char *startPos;
    char *nlPos;

  /* addDynamicLibsWithRpath */
    dllList[0] = '\0';
    if (baseDirExists) {
      addDynamicLibToDllListWithRpath(scopeName, baseDir,
                                      dllDirList, dllDirListLength,
                                      dllNameList, dllNameListLength, rpath,
                                      dllList);
    } /* if */
    addDynamicLib(scopeName, dllNameList, dllNameListLength, dllList);
    startPos = dllList;
    if (startPos[0] != '\0') {
      nlPos = strchr(startPos, '\n');
      while (nlPos != NULL) {
        *nlPos = '\0';
        fprintf(versionFile, " \"");
        escapeString(versionFile, startPos);
        fprintf(versionFile, "\",");
        startPos = &nlPos[1];
        nlPos = strchr(startPos, '\n');
      } /* while */
      fprintf(versionFile, " \"");
      escapeString(versionFile, startPos);
      fprintf(versionFile, "\",");
    } /* if */
  } /* addDynamicLibsWithRpath */



static int visualDepthOf32BitsSupported (const char *x11IncludeCommand,
    const char *includeOption, const char *systemDrawLibs)

  {
    char testProgram[BUFFER_SIZE];
    int testResult = 0;

  /* visualDepthOf32BitsSupported */
    sprintf(testProgram, "#include<stdio.h>\n%s"
                         "int main(int argc,char *argv[]){\n"
                         "Display* display;\n"
                         "XVisualInfo vinfo;\n"
                         "display = XOpenDisplay(NULL);\n"
                         "if (display == NULL) {\n"
                         "  display = XOpenDisplay(\":0\");\n"
                         "}\n"
                         "printf(\"%%d\\n\",\n"
                         "    display != NULL &&\n"
                         "    XMatchVisualInfo(display, DefaultScreen(display),\n"
                         "                     32, TrueColor, &vinfo) != 0);\n"
                         "return 0;}\n",
            x11IncludeCommand);
    if (compileAndLinkWithOptionsOk(testProgram, includeOption, systemDrawLibs)) {
      testResult = doTest();
      if (testResult < 0) {
        testResult = 0;
      } /* if */
    } /* if */
    return testResult;
  } /* visualDepthOf32BitsSupported */



static void defineX11rgbToPixelMacro (FILE *versionFile, const char *x11IncludeCommand,
    const char *includeOption, const char *systemDrawLibs)

  {
    const char *colorNames[] = {"red", "green", "blue"};
    const char *colorNamesUc[] = {"RED", "GREEN", "BLUE"};
    char testProgram[BUFFER_SIZE];
    char macroPart[BUFFER_SIZE];
    char macroBody[BUFFER_SIZE];
    int colorIndex;
    int testResult;
    int highestMaskBit;
    int lowZeroBitsInMask;
    unsigned long colorMask;
    int okay = 1;

  /* defineX11rgbToPixelMacro */
    macroBody[0] = '\0';
    for (colorIndex = 0; colorIndex < 3; colorIndex++) {
      sprintf(testProgram, "#include<stdio.h>\n%s"
                           "int getHighestSetBit(unsigned long number)\n"
                           "{ int result = 0;\n"
                           "while (number != 0) {\n"
                           "  number >>= 1;\n"
                           "  result++;\n"
                           "} return result; }\n"
                           "int countLowestZeroBits(unsigned long number)\n"
                           "{ int result = 0;\n"
                           "while (number != 0 && (number & 1) == 0) {\n"
                           "  number >>= 1;\n"
                           "  result++;\n"
                           "} return result; }\n"
                           "int main(int argc,char *argv[]){\n"
                           "Display *display;\n"
                           "int screen;\n"
                           "Visual *defaultVisual;\n"
                           "display = XOpenDisplay(NULL);\n"
                           "if (display == NULL) {\n"
                           "  display = XOpenDisplay(\":0\");\n"
                           "}\n"
                           "if (display != NULL) {\n"
                           "  screen = DefaultScreen(display);\n"
                           "  defaultVisual = XDefaultVisual(display, screen);\n"
                           "  printf(\"%%d\\n\", getHighestSetBit(defaultVisual->%s_mask) << 8 |\n"
                           "          countLowestZeroBits(defaultVisual->%s_mask));\n"
                           "} else { printf(\"0\\n\"); }\n"
                           "return 0;}\n",
              x11IncludeCommand, colorNames[colorIndex], colorNames[colorIndex]);
      if (compileAndLinkWithOptionsOk(testProgram, includeOption, systemDrawLibs)) {
        testResult = doTest();
        if (testResult > 0) {
          highestMaskBit = testResult >> 8;
          lowZeroBitsInMask = testResult & 0xff;
          colorMask = ~((~(unsigned int) 0) << (highestMaskBit - lowZeroBitsInMask)) << lowZeroBitsInMask;
          fprintf(versionFile, "#define PIXEL_%s_MASK \"%lx\"\n", colorNamesUc[colorIndex], colorMask);
          if (highestMaskBit > 16) {
            sprintf(macroPart, "((((unsigned long) (%s)) << %d) & 0x%lx)",
                    colorNames[colorIndex], highestMaskBit - 16, colorMask);
          } else if (highestMaskBit < 16) {
            sprintf(macroPart, "((((unsigned long) (%s)) >> %d) & 0x%lx)",
                    colorNames[colorIndex], 16 - highestMaskBit, colorMask);
          } else {
            sprintf(macroPart, "(((unsigned long) (%s)) & 0x%lx)",
                    colorNames[colorIndex], colorMask);
          } /* if */
          if (macroBody[0] != '\0') {
            strcat(macroBody, " | ");
          } /* if */
          strcat(macroBody, macroPart);
        } else {
          okay = 0;
        } /* if */
      } else {
        okay = 0;
      } /* if */
    } /* for */
    if (okay) {
      fprintf(versionFile, "#define RGB_TO_PIXEL_FLAG_NAME \"useRgbToPixel\"\n");
      fprintf(versionFile, "#define rgbToPixel(red, green, blue) (%s)\n", macroBody);
    } /* if */
  } /* defineX11rgbToPixelMacro */



static void determineX11Defines (FILE *versionFile, char *include_options,
    char *system_draw_libs)

  {
    const char *inclDirList[] = {"/opt/X11/include", "/opt/homebrew/include"};
#ifdef X11_LIBRARY_PATH
    const char *libDirList[] = { X11_LIBRARY_PATH };
#endif
    const char *usrLib[] = {"/usr/lib"};
    const char *usrLib64[] = {"/usr/lib64"};
#ifdef X11_LIBS
    const char *libNameList[] = { X11_LIBS };
#else
    const char *libNameList[] = {"-lX11"};
#endif
#if LIBRARY_TYPE == UNIX_LIBRARIES
    const char *xRenderLibNameList[] = {"-lXrender", "libXrender.so", "libXrender.so.1"};
#elif LIBRARY_TYPE == MACOS_LIBRARIES
    const char *xRenderLibNameList[] = {"-lXrender", "libXrender.dylib", "libXrender.1.dylib"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *xRenderLibNameList[] = {"-lXrender", "libXrender.dll"};
#endif
#ifdef X11_DLL
    const char *dllNameList[] = { X11_DLL };
#elif LIBRARY_TYPE == UNIX_LIBRARIES
    const char *dllNameList[] = {"libX11.so", "libX11.so.6", "libX11.so.5"};
    const char *xRenderDllNameList[] = {"libXrender.so", "libXrender.so.1"};
#elif LIBRARY_TYPE == MACOS_LIBRARIES
    const char *dllNameList[] = {"libX11.dylib"};
    const char *xRenderDllNameList[] = {"libXrender.dylib", "libXrender.1.dylib"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *dllNameList[] = {"x11.dll"};
    const char *xRenderDllNameList[] = {"xrender.dll"};
#endif
    char includeOption[PATH_SIZE];
    const char *x11Include = NULL;
    const char *x11IncludeCommand;
    const char *x11XrenderInclude = NULL;
    const char *x11XrenderIncludeCommand = "";
    char testProgram[BUFFER_SIZE];
    unsigned int nameIndex;
    int searchForX11Lib = 1;
    int searchForXrenderLib = 1;

  /* determineX11Defines */
#ifndef NO_X11_SYSTEM_INCLUDES
    sprintf(testProgram, "#include <X11/X.h>\n"
                         "#include <X11/Xlib.h>\n"
                         "#include <X11/Xutil.h>\n"
                         "int main(int argc,char *argv[]){\n"
                         "Display *display;\n"
                         "GC gc;\n"
                         "Window window;\n"
                         "Pixmap pixmap;\n"
                         "return 0;}\n");
    if (compileAndLinkWithOptionsOk(testProgram, "", "")) {
      includeOption[0] = '\0';
      x11Include = "X11/X.h";
      fprintf(logFile, "\rX11: %s found in system include directory.\n", x11Include);
    } else {
      x11Include = findIncludeFile("X11", testProgram, "",
                                    inclDirList, sizeof(inclDirList) / sizeof(char *),
                                   "X11/X.h", includeOption);
      if (x11Include != NULL) {
        appendOption(include_options, includeOption);
      } /* if */
    } /* if */
#endif
    if (x11Include != NULL) {
      x11IncludeCommand = "#include <X11/X.h>\n"
                          "#include <X11/Xlib.h>\n"
                          "#include <X11/Xutil.h>\n";
    } else {
#ifdef ALLOW_REPLACEMENT_OF_SYSTEM_HEADERS
      includeOption[0] = '\0';
      x11Include = "x11_x.h";
      fprintf(versionFile, "#define X11_INCLUDE \"%s\"\n", x11Include);
      fprintf(logFile, "\rX11: %s found in Seed7 include directory.\n", x11Include);
      x11IncludeCommand = "#include \"x11_x.h\"\n";
#else
      x11IncludeCommand = "";
#endif
    } /* if */
    sprintf(testProgram, "%s"
                         "#include <X11/extensions/Xrender.h>\n"
                         "int main(int argc,char *argv[]){\n"
                         "Display *display;\n"
                         "XRenderPictFormat pictFormat;\n"
                         "XRenderPictureAttributes pictureAttributes;\n"
                         "return 0;}\n", x11IncludeCommand);
    if (compileAndLinkWithOptionsOk(testProgram, includeOption, "")) {
      x11XrenderInclude="X11/extensions/Xrender.h";
      fprintf(versionFile, "#define X11_XRENDER_INCLUDE \"%s\"\n", x11XrenderInclude);
      fprintf(logFile, "\rX11: %s found in system include directory.\n", x11XrenderInclude);
      x11XrenderIncludeCommand = "#include <X11/extensions/Xrender.h>\n";
#ifdef ALLOW_REPLACEMENT_OF_SYSTEM_HEADERS
    } else {
      x11XrenderInclude = "x11_rend.h";
      fprintf(versionFile, "#define X11_XRENDER_INCLUDE \"%s\"\n", x11XrenderInclude);
      fprintf(logFile, "\rX11: %s found in Seed7 include directory.\n", x11XrenderInclude);
      x11XrenderIncludeCommand = "#include \"x11_rend.h\"\n";
#endif
    } /* if */
    if (x11Include == NULL) {
      fprintf(versionFile, "#define FORWARD_X11_CALLS 0\n");
    } else {
      sprintf(testProgram, "#include<stdio.h>\n%s"
                           "int main(int argc,char *argv[]){\n"
                           "printf(\"%%u\\n\", (unsigned) sizeof(XID));\n"
                           "return 0;}\n", x11IncludeCommand);
      if (compileAndLinkWithOptionsOk(testProgram, includeOption, "")) {
        fprintf(versionFile, "#define XID_SIZE %d\n", 8 * doTest());
      } /* if */
      sprintf(testProgram, "#include<stdio.h>\n%s"
                           "int main(int argc,char *argv[]){\n"
                           "printf(\"%%u\\n\", (unsigned) sizeof(Atom));\n"
                           "return 0;}\n", x11IncludeCommand);
      if (compileAndLinkWithOptionsOk(testProgram, includeOption, "")) {
        fprintf(versionFile, "#define ATOM_SIZE %d\n", 8 * doTest());
      } /* if */
#ifndef X11_USE_DLL
      /* Handle static libraries: */
      sprintf(testProgram, "#include<stdio.h>\n%s"
                           "int main(int argc,char *argv[]){\n"
                           "Display *display;\n"
                           "display = XOpenDisplay(NULL);\n"
                           "if (display == NULL) {\n"
                           "  display = XOpenDisplay(\":0\");\n"
                           "}\n"
                           "printf(\"1\\n\");\n"
                           "return 0;}\n", x11IncludeCommand);
      /* fprintf(logFile, "%s\n", testProgram);
         fprintf(logFile, "x11Include: \"%s\"\n", x11Include); */
#ifdef X11_LIBRARY_PATH
      if (findStaticLib("X11", testProgram, includeOption, "", "",
                        libDirList, sizeof(libDirList) / sizeof(char *),
                        libNameList, sizeof(libNameList) / sizeof(char *),
                        system_draw_libs)) {
        searchForX11Lib = 0;
      } /* if */
#endif
      if (searchForX11Lib) {
        if (findLinkerOption("X11", testProgram, includeOption, "",
                             libNameList, sizeof(libNameList) / sizeof(char *),
                             system_draw_libs)) {
          searchForX11Lib = 0;
        } /* if */
      } /* if */
      if (searchForX11Lib) {
        fprintf(logFile, "\rX11: Cannot link to X11 library.\n");
      } else {
        /* The X11 library was found. Now search for the Xrender library: */
        sprintf(testProgram, "#include<stdio.h>\n%s%s"
                             "int main(int argc,char *argv[]){\n"
                             "Display *display;\n"
                             "int xrender;\n"
                             "int event_basep;\n"
                             "int error_basep;\n"
                             "display = XOpenDisplay(NULL);\n"
                             "if (display == NULL) {\n"
                             "  display = XOpenDisplay(\":0\");\n"
                             "}\n"
                             "if (display != NULL) {\n"
                             "  xrender = XRenderQueryExtension(display, &event_basep, &error_basep);\n"
                             "}\n"
                             "printf(\"1\\n\");\n"
                             "return 0;}\n", x11IncludeCommand, x11XrenderIncludeCommand);
        /* fprintf(logFile, "%s\n", testProgram);
           fprintf(logFile, "x11Include: \"%s\"\n", x11Include); */
#ifdef X11_LIBRARY_PATH
        if (findStaticLib("Xrender", testProgram, includeOption, system_draw_libs, "",
                          libDirList, sizeof(libDirList) / sizeof(char *),
                          xRenderLibNameList, sizeof(xRenderLibNameList) / sizeof(char *),
                          system_draw_libs)) {
          fprintf(versionFile, "#define HAS_XRENDER_EXTENSION\n");
          searchForXrenderLib = 0;
        } /* if */
#endif
        if (searchForXrenderLib) {
          if (findLinkerOption("Xrender", testProgram, includeOption, system_draw_libs,
                               xRenderLibNameList, sizeof(xRenderLibNameList) / sizeof(char *),
                               system_draw_libs)) {
            fprintf(versionFile, "#define HAS_XRENDER_EXTENSION\n");
            searchForXrenderLib = 0;
          } else if (sizeof_pointer == 4) {
            if (findStaticLib("Xrender", testProgram, includeOption, system_draw_libs, "",
                              usrLib, sizeof(usrLib) / sizeof(char *),
                              xRenderLibNameList, sizeof(xRenderLibNameList) / sizeof(char *),
                              system_draw_libs)) {
              fprintf(versionFile, "#define HAS_XRENDER_EXTENSION\n");
              searchForXrenderLib = 0;
            } /* if */
          } else {
            if (findStaticLib("Xrender", testProgram, includeOption, system_draw_libs, "",
                              usrLib64, sizeof(usrLib64) / sizeof(char *),
                              xRenderLibNameList, sizeof(xRenderLibNameList) / sizeof(char *),
                              system_draw_libs)) {
              fprintf(versionFile, "#define HAS_XRENDER_EXTENSION\n");
              searchForXrenderLib = 0;
            } /* if */
          } /* if */
        } /* if */
        if (searchForXrenderLib) {
          fprintf(logFile, "\rX11: Cannot link to Xrender library.\n");
        } /* if */
      } /* if */
#endif
      fprintf(versionFile, "#define VISUAL_DEPTH_OF_32_BITS_SUPPORTED %d\n",
              visualDepthOf32BitsSupported(x11IncludeCommand, includeOption, system_draw_libs));
      defineX11rgbToPixelMacro(versionFile, x11IncludeCommand, includeOption, system_draw_libs);
      fprintf(versionFile, "#define FORWARD_X11_CALLS %d\n",
              searchForX11Lib || searchForXrenderLib);
      if (searchForX11Lib || searchForXrenderLib) {
        fprintf(versionFile, "#define HAS_XRENDER_EXTENSION\n");
        system_draw_libs[0] = '\0';
        appendOption(system_draw_libs, LINKER_OPT_DYN_LINK_LIBS);
        /* Handle dynamic libraries: */
        fprintf(versionFile, "#define X11_DLL");
#ifdef X11_LIBRARY_PATH
        listDynamicLibs("X11", "",
                        libDirList, sizeof(libDirList) / sizeof(char *),
                        dllNameList, sizeof(dllNameList) / sizeof(char *),
                        versionFile);
#endif
        for (nameIndex = 0;
             nameIndex < sizeof(dllNameList) / sizeof(char *);
             nameIndex++) {
          fprintf(logFile, "\rX11: DLL / Shared library: %s (%s)\n",
                  dllNameList[nameIndex],
                  canLoadDynamicLibrary(dllNameList[nameIndex]) ? "present" : "cannot load");
          fprintf(versionFile, " \"%s\",", dllNameList[nameIndex]);
        } /* for */
        fprintf(versionFile, "\n");
        fprintf(versionFile, "#define X11_XRENDER_DLL");
#ifdef X11_LIBRARY_PATH
        listDynamicLibs("Xrender", "",
                        libDirList, sizeof(libDirList) / sizeof(char *),
                        xRenderDllNameList, sizeof(xRenderDllNameList) / sizeof(char *),
                        versionFile);
#endif
        for (nameIndex = 0;
             nameIndex < sizeof(xRenderDllNameList) / sizeof(char *);
             nameIndex++) {
          fprintf(logFile, "\rX11_XRENDER: DLL / Shared library: %s (%s)\n",
                  xRenderDllNameList[nameIndex],
                  canLoadDynamicLibrary(xRenderDllNameList[nameIndex]) ? "present" : "cannot load");
          fprintf(versionFile, " \"%s\",", xRenderDllNameList[nameIndex]);
        } /* for */
        fprintf(versionFile, "\n");
      } /* if */
    } /* if */
  } /* determineX11Defines */



static void determineConsoleDefines (FILE *versionFile, char *include_options,
    char *system_console_libs)

  {
#ifdef CONSOLE_LIBS
    const char *libNameList[] = { CONSOLE_LIBS };
#else
    const char *libNameList[] = {"-lncurses"};
#endif
#ifdef CONSOLE_DLL
    const char *dllNameList[] = { CONSOLE_DLL };
#elif LIBRARY_TYPE == UNIX_LIBRARIES
    const char *dllNameList[] = {"libtinfo.so", "libtinfo.so.5"};
#elif LIBRARY_TYPE == MACOS_LIBRARIES
    const char *dllNameList[] = {"libtinfo.dylib"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *dllNameList[] = {"ncurses.dll"};
#endif
    const char *consoleInclude = NULL;
    int useSystemHeader = 1;
    unsigned int nameIndex;
    int searchForLib = 1;
    char testProgram[BUFFER_SIZE];

  /* determineConsoleDefines */
    if (compileAndLinkOk("#include \"stdio.h\"\n"
                         "#include \"termios.h\"\n"
                         "int main(int argc,char *argv[]){\n"
                         "struct termios termiosData;\n"
                         "printf(\"%d\\n\", tcgetattr(0, &termiosData) == 0);\n"
                         "return 0;}\n")) {
      fprintf(versionFile, "#define HAS_TERMIOS_H\n");
    } /* if */
    if (compileAndLinkOk("#include \"stdio.h\"\n"
                         "#include \"term.h\"\n"
                         "int main(int argc,char *argv[]){\n"
                         "TERMINAL *aTerminal;\n"
                         "return 0;}\n")) {
      fprintf(versionFile, "#define USE_TERMINFO\n");
      consoleInclude = "term.h";
      fprintf(logFile, "\rConsole: %s found in system include directory.\n", consoleInclude);
    } else if (compileAndLinkOk("#include \"stdio.h\"\n"
                                "#include \"ncurses/term.h\"\n"
                                "int main(int argc,char *argv[]){\n"
                                "TERMINAL *aTerminal;\n"
                                "return 0;}\n")) {
      fprintf(versionFile, "#define USE_TERMINFO\n");
      fprintf(versionFile, "#define INCL_NCURSES_TERM\n");
      consoleInclude = "ncurses/term.h";
      fprintf(logFile, "\rConsole: %s found in system include directory.\n", consoleInclude);
    } /* if */
#ifdef ALLOW_REPLACEMENT_OF_SYSTEM_HEADERS
    if (consoleInclude == NULL) {
      if (compileAndLinkOk("#include \"stdio.h\"\n"
                           "#include \"fwd_term.h\"\n"
                           "int main(int argc,char *argv[]){\n"
                           "TERMINAL *aTerminal;\n"
                           "return 0;}\n")) {
        useSystemHeader = 0;
        fprintf(versionFile, "#define TERM_INCLUDE \"fwd_term.h\"\n");
        /* fprintf(versionFile, "#define FORWARD_TERM_CALLS %d\n", 0); */
        fprintf(versionFile, "#define USE_TERMINFO\n");
        consoleInclude = "fwd_term.h";
        fprintf(logFile, "\rConsole: %s found in Seed7 include directory.\n", consoleInclude);
      } /* if */
    } /* if */
#endif
    if (compileAndLinkOk("#include<poll.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{struct pollfd pollFd[1];\n"
                         "poll(pollFd, 1, 0);\n"
                         "return 0;}\n")) {
      fprintf(versionFile, "#define USE_KBD_POLL\n");
    } else {
      fprintf(versionFile, "#define USE_KBD_INF\n");
    } /* if */
    if (consoleInclude == NULL) {
      fprintf(versionFile, "#define FORWARD_TERM_CALLS 0\n");
    } else {
      sprintf(testProgram, "#include<stdio.h>\n#include \"%s\"\n"
                           "int main(int argc,char *argv[]){\n"
                           "TERMINAL *aTerminal;\n"
                           "int errret;\n"
                           "setupterm(\"vt100\", fileno(stdout), &errret);\n"
                           "printf(\"1\\n\");\n"
                           "return 0;}\n", consoleInclude);
      /* fprintf(logFile, "%s\n", testProgram);
         fprintf(logFile, "consoleInclude: \"%s\"\n", consoleInclude); */
      if (useSystemHeader) {
        if (findLinkerOption("Console", testProgram, "", "",
                             libNameList, sizeof(libNameList) / sizeof(char *),
                             system_console_libs)) {
          searchForLib = 0;
        } /* if */
#ifdef ALLOW_REPLACEMENT_OF_SYSTEM_HEADERS
        if (searchForLib) {
          if (compileAndLinkOk("#include \"stdio.h\"\n"
                               "#include \"fwd_term.h\"\n"
                               "int main(int argc,char *argv[]){\n"
                               "TERMINAL *aTerminal;\n"
                               "return 0;}\n")) {
            useSystemHeader = 0;
            fprintf(versionFile, "#define TERM_INCLUDE \"fwd_term.h\"\n");
            /* fprintf(versionFile, "#define FORWARD_TERM_CALLS %d\n", 0); */
            consoleInclude = "fwd_term.h";
            fprintf(logFile, "\rConsole: %s found in Seed7 include directory.\n", consoleInclude);
          } /* if */
        } /* if */
#endif
      } /* if */
      fprintf(versionFile, "#define FORWARD_TERM_CALLS %d\n",
              searchForLib && !useSystemHeader);
      if (searchForLib && !useSystemHeader) {
        system_console_libs[0] = '\0';
        appendOption(system_console_libs, LINKER_OPT_DYN_LINK_LIBS);
        /* Handle dynamic libraries: */
        fprintf(versionFile, "#define CONSOLE_DLL");
        for (nameIndex = 0;
             nameIndex < sizeof(dllNameList) / sizeof(char *);
             nameIndex++) {
          fprintf(logFile, "\rConsole: DLL / Shared library: %s (%s)\n",
                  dllNameList[nameIndex],
                  canLoadDynamicLibrary(dllNameList[nameIndex]) ? "present" : "cannot load");
          fprintf(versionFile, " \"%s\",", dllNameList[nameIndex]);
        } /* for */
        fprintf(versionFile, "\n");
      } /* if */
    } /* if */
  } /* determineConsoleDefines */



static void determineMySqlDefines (FILE *versionFile,
    char *include_options, char *system_database_libs)

  {
    const char *dbHomeSys[] = {"MariaDB/MariaDB C Client Library",
                               "MariaDB/MariaDB C Client Library 64-bit",
                               "MariaDB 10.3",
                               "MySQL/MySQL Connector C 6.1"};
#ifdef MYSQL_LIBS
    const char *libNameList[] = { MYSQL_LIBS };
#elif LIBRARY_TYPE == UNIX_LIBRARIES || LIBRARY_TYPE == MACOS_LIBRARIES
    const char *libNameList[] = {"-lmysqlclient"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *libNameList[] = {"mariadbclient.lib", "vs11/mysqlclient.lib"};
#endif
#ifdef MYSQL_DLL
    const char *dllNameList[] = { MYSQL_DLL };
#elif LIBRARY_TYPE == UNIX_LIBRARIES
    const char *dllNameList[] = {"libmariadb.so", "libmariadb.so.3", "libmysqlclient.so"};
#elif LIBRARY_TYPE == MACOS_LIBRARIES
    const char *dllNameList[] = {"libmysqlclient.dylib"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *dllNameList[] = {"libmariadb.dll", "libmysql.dll"};
#endif
    const char *inclDirList[] = {"/include"};
    const char *libDirList[] = {"/lib"};
    const char *dllDirList[] = {"/lib"};
    unsigned int dirIndex;
    unsigned int nameIndex;
    int searchForLib = 1;
    const char *programFilesX86 = NULL;
    const char *programFiles = NULL;
    char dbHome[PATH_SIZE];
    char includeOption[PATH_SIZE];
    const char *mySqlInclude = NULL;
    char testProgram[BUFFER_SIZE];
    int dbHomeExists = 0;

  /* determineMySqlDefines */
#ifdef MYSQL_INCLUDE_OPTIONS
    strcpy(includeOption, MYSQL_INCLUDE_OPTIONS);
#else
    includeOption[0] = '\0';
#endif
    programFilesX86 = getenv("ProgramFiles(x86)");
    /* fprintf(logFile, "programFilesX86: %s\n", programFilesX86); */
    programFiles = getenv("ProgramFiles");
    /* fprintf(logFile, "programFiles: %s\n", programFiles); */
    if (programFiles != NULL) {
      if (sizeof(char *) == 4 && programFilesX86 != NULL) {
        programFiles = programFilesX86;
      } /* if */
      for (dirIndex = 0; !dbHomeExists && dirIndex < sizeof(dbHomeSys) / sizeof(char *); dirIndex++) {
        sprintf(dbHome, "%s/%s", programFiles, dbHomeSys[dirIndex]);
        /* fprintf(logFile, "dbHome: <%s>\n", dbHome); */
        if (fileIsDir(dbHome)) {
          dbHomeExists = 1;
        } /* if */
      } /* for */
    } /* if */
    if (dbHomeExists) {
      /* fprintf(logFile, "dbHome=%s\n", dbHome); */
      sprintf(testProgram, "#include \"stdlib.h\"\n"
                           "#include \"mysql.h\"\n"
                           "int main(int argc,char *argv[]){"
                           "MYSQL *connection; return 0;}\n");
      mySqlInclude = findIncludeFile("MySql/MariaDb", testProgram, dbHome,
                                     inclDirList, sizeof(inclDirList) / sizeof(char *),
                                     "mysql.h", includeOption);
      if (mySqlInclude != NULL) {
        appendOption(include_options, includeOption);
      } /* if */
    } /* if */
    if (mySqlInclude == NULL) {
      if (compileAndLinkWithOptionsOk("#include <mysql.h>\n"
                                      "int main(int argc,char *argv[]){"
                                      "MYSQL *connection; return 0;}\n",
                                      "", "")) {
        mySqlInclude = "mysql.h";
        fprintf(logFile, "\rMySql/MariaDb: %s found in system include directory.\n", mySqlInclude);
      } else if (compileAndLinkWithOptionsOk("#include <mysql/mysql.h>\n"
                                             "int main(int argc,char *argv[]){"
                                             "MYSQL *connection; return 0;}\n",
                                             "", "")) {
        mySqlInclude = "mysql/mysql.h";
        fprintf(logFile, "\rMySql/MariaDb: %s found in system include directory.\n", mySqlInclude);
      } else if (compileAndLinkWithOptionsOk("#include <mysql.h>\n"
                                             "int main(int argc,char *argv[]){"
                                             "MYSQL *connection; return 0;}\n",
                                             includeOption, "")) {
        mySqlInclude = "mysql.h";
        fprintf(logFile, "\rMySql/MariaDb: %s found with option %s.\n", mySqlInclude, includeOption);
        appendOption(include_options, includeOption);
      } else if (compileAndLinkWithOptionsOk("#include <mysql/mysql.h>\n"
                                             "int main(int argc,char *argv[]){"
                                             "MYSQL *connection; return 0;}\n",
                                             includeOption, "")) {
        mySqlInclude = "mysql/mysql.h";
        fprintf(logFile, "\rMySql/MariaDb: %s found with option %s.\n", mySqlInclude, includeOption);
        appendOption(include_options, includeOption);
      } else if (compileAndLinkWithOptionsOk("#include \"db_my.h\"\n"
                                             "int main(int argc,char *argv[]){\n"
                                             "MYSQL *connection; return 0;}\n",
                                             "", "") ||
                 compileAndLinkWithOptionsOk("#define STDCALL\n"
                                             "#include \"db_my.h\"\n"
                                             "int main(int argc,char *argv[]){\n"
                                             "MYSQL *connection; return 0;}\n",
                                             "", "")) {
        mySqlInclude = "db_my.h";
        fprintf(logFile, "\rMySql/MariaDb: %s found in Seed7 include directory.\n", mySqlInclude);
      } /* if */
    } /* if */
    if (mySqlInclude != NULL) {
      fprintf(versionFile, "#define MYSQL_INCLUDE \"%s\"\n", mySqlInclude);
    } /* if */
#ifndef MYSQL_USE_DLL
    /* Handle static libraries: */
    sprintf(testProgram, "#include<stdio.h>\n#include<stdlib.h>\n"
                         "#include \"%s\"\n"
                         "int main(int argc,char *argv[]){\n"
                         "MYSQL *conn;\n"
                         "conn = mysql_init(NULL);\n"
                         "if (conn != NULL) {\n"
                         "mysql_real_connect(conn, \"\", \"\", \"\", \"\", 0, NULL, 0);\n"
                         "mysql_close(conn);\n"
                         "printf(\"1\\n\");\n"
                         "}\nreturn 0;\n}\n", mySqlInclude);
    /* fprintf(logFile, "%s\n", testProgram);
       fprintf(logFile, "mySqlInclude: \"%s\"\n", mySqlInclude); */
    if (dbHomeExists) {
      if (findStaticLib("MySql/MariaDb", testProgram, includeOption, "", dbHome,
                        libDirList, sizeof(libDirList) / sizeof(char *),
                        libNameList, sizeof(libNameList) / sizeof(char *),
                        system_database_libs)) {
        searchForLib = 0;
      } /* if */
    } /* if */
    if (searchForLib) {
      if (findLinkerOption("MySql/MariaDb", testProgram, includeOption, MYSQL_LIBRARY_PATH,
                           libNameList, sizeof(libNameList) / sizeof(char *),
                           system_database_libs)) {
        searchForLib = 0;
      } else if (buildTimeLinking) {
        if (findLinkerOptionForDll("MySql/MariaDb", testProgram, includeOption,
                                   MYSQL_LIBRARY_PATH, dllNameList,
                                   sizeof(dllNameList) / sizeof(char *),
                                   system_database_libs)) {
          searchForLib = 0;
        } /* if */
      } /* if */
    } /* if */
#endif
    if (searchForLib) {
      /* Handle dynamic libraries: */
      appendOption(system_database_libs, LINKER_OPT_DYN_LINK_LIBS);
      fprintf(versionFile, "#define MYSQL_DLL");
      if (dbHomeExists) {
        listDynamicLibs("MySql/MariaDb", dbHome,
                        dllDirList, sizeof(dllDirList) / sizeof(char *),
                        dllNameList, sizeof(dllNameList) / sizeof(char *), versionFile);
      } /* if */
      for (nameIndex = 0; nameIndex < sizeof(dllNameList) / sizeof(char *); nameIndex++) {
        fprintf(logFile, "\rMySql/MariaDb: DLL / Shared library: %s (%s)\n",
                dllNameList[nameIndex],
                canLoadDynamicLibrary(dllNameList[nameIndex]) ? "present" : "cannot load");
        fprintf(versionFile, " \"%s\",", dllNameList[nameIndex]);
      } /* for */
      fprintf(versionFile, "\n");
    } /* if */
  } /* determineMySqlDefines */



static void determineSqliteDefines (FILE *versionFile,
    char *include_options, char *system_database_libs)

  {
    const char *dbHomeDirs[] = {"C:/sqlite", "D:/sqlite"};
#ifdef SQLITE_LIBS
    const char *libNameList[] = { SQLITE_LIBS };
#elif LIBRARY_TYPE == UNIX_LIBRARIES || LIBRARY_TYPE == MACOS_LIBRARIES
    const char *libNameList[] = {"-lsqlite3"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *libNameList[] = {"sqlite3.lib"};
#endif
#ifdef SQLITE_DLL
    const char *dllNameList[] = { SQLITE_DLL };
#elif LIBRARY_TYPE == UNIX_LIBRARIES
    const char *dllNameList[] = {"libsqlite3.so", "libsqlite3.so.0"};
#elif LIBRARY_TYPE == MACOS_LIBRARIES
    const char *dllNameList[] = {"libsqlite3.dylib"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *dllNameList[] = {"sqlite3.dll"};
#endif
    const char *libDirList[] = {""};
    const char *dllDirList[] = {""};
    unsigned int dirIndex;
    unsigned int nameIndex;
    int searchForLib = 1;
    char dbHome[PATH_SIZE];
    char includeOption[PATH_SIZE + 4];
    const char *sqliteInclude = NULL;
    char testProgram[BUFFER_SIZE];
    char dllPath[PATH_SIZE + 1 + NAME_SIZE];
    int dllPointerSize;
    int dbHomeExists = 0;

  /* determineSqliteDefines */
#ifdef SQLITE_INCLUDE_OPTIONS
    strcpy(includeOption, SQLITE_INCLUDE_OPTIONS);
#else
    includeOption[0] = '\0';
#endif
    for (dirIndex = 0; !dbHomeExists && dirIndex < sizeof(dbHomeDirs) / sizeof(char *); dirIndex++) {
      strcpy(dbHome, dbHomeDirs[dirIndex]);
      if (fileIsDir(dbHome)) {
        dbHomeExists = 1;
      } /* if */
    } /* for */
    if (dbHomeExists) {
      /* fprintf(logFile, "dbHome=%s\n", dbHome); */
      sprintf(includeOption, "-I\"%s\"", dbHome);
      /* fprintf(logFile, "includeOption=%s\n", includeOption); */
      if (compileAndLinkWithOptionsOk("#include <sqlite3.h>\n"
                                      "int main(int argc,char *argv[]){\n"
                                      "sqlite3_stmt *ppStmt;\n"
                                      "return 0;}\n",
                                      includeOption, "")) {
        sqliteInclude = "sqlite3.h";
        fprintf(logFile, "\rSQLite: %s found in %s\n", sqliteInclude, dbHome);
        appendOption(include_options, includeOption);
      } /* if */
    } /* if */
    if (sqliteInclude == NULL) {
      if (compileAndLinkWithOptionsOk("#include <sqlite3.h>\n"
                                      "int main(int argc,char *argv[]){\n"
                                      "sqlite3_stmt *ppStmt;\n"
                                      "return 0;}\n",
                                      includeOption, "")) {
        sqliteInclude = "sqlite3.h";
        fprintf(logFile, "\rSQLite: %s found in system include directory.\n", sqliteInclude);
        appendOption(include_options, includeOption);
      } else if (compileAndLinkWithOptionsOk("#include \"tst_vers.h\"\n"
                                             "#include \"db_lite.h\"\n"
                                             "int main(int argc,char *argv[]){\n"
                                             "sqlite3_stmt *ppStmt;\n"
                                             "return 0;}\n",
                                             "", "") ||
                 compileAndLinkWithOptionsOk("#define CDECL\n"
                                             "#include \"tst_vers.h\"\n"
                                             "#include \"db_lite.h\"\n"
                                             "int main(int argc,char *argv[]){\n"
                                             "sqlite3_stmt *ppStmt;\n"
                                             "return 0;}\n",
                                             "", "")) {
        sqliteInclude = "db_lite.h";
        fprintf(logFile, "\rSQLite: %s found in Seed7 include directory.\n", sqliteInclude);
      } /* if */
    } /* if */
    if (sqliteInclude == NULL) {
      fprintf(logFile, "\rSQLite: ***** No include file found. Cannot define SQLITE_INCLUDE.\n");
    } else {
      fprintf(versionFile, "#define SQLITE_INCLUDE \"%s\"\n", sqliteInclude);
    } /* if */
#ifndef SQLITE_USE_DLL
    /* Handle static libraries: */
    sprintf(testProgram, "#include \"tst_vers.h\"\n#include<stdio.h>\n"
                         "#include \"%s\"\n"
                         "int main(int argc,char *argv[]){\n"
                         "sqlite3 *conn;\n"
                         "sqlite3_open(\"\", &conn);\n"
                         "sqlite3_close(conn);\n"
                         "printf(\"1\\n\");\n"
                         "return 0;\n}\n", sqliteInclude);
    /* fprintf(logFile, "%s\n", testProgram);
       fprintf(logFile, "sqliteInclude: \"%s\"\n", sqliteInclude); */
    if (dbHomeExists) {
      if (findStaticLib("SQLite", testProgram, includeOption, "", dbHome,
                        libDirList, sizeof(libDirList) / sizeof(char *),
                        libNameList, sizeof(libNameList) / sizeof(char *),
                        system_database_libs)) {
        searchForLib = 0;
      } /* if */
    } /* if */
    if (searchForLib) {
      if (findLinkerOption("SQLite", testProgram, includeOption, SQLITE_LIBRARY_PATH,
                           libNameList, sizeof(libNameList) / sizeof(char *),
                           system_database_libs)) {
        searchForLib = 0;
      } else if (buildTimeLinking) {
        if (findLinkerOptionForDll("SQLite", testProgram, includeOption,
                                   SQLITE_LIBRARY_PATH, dllNameList,
                                   sizeof(dllNameList) / sizeof(char *),
                                   system_database_libs)) {
          searchForLib = 0;
        } /* if */
      } /* if */
    } /* if */
#endif
    if (searchForLib) {
      /* Handle dynamic libraries: */
      appendOption(system_database_libs, LINKER_OPT_DYN_LINK_LIBS);
      fprintf(versionFile, "#define SQLITE_DLL");
      if (dbHomeExists) {
        listDynamicLibs("SQLite", dbHome,
                        dllDirList, sizeof(dllDirList) / sizeof(char *),
                        dllNameList, sizeof(dllNameList) / sizeof(char *), versionFile);
      } /* if */
      if (s7LibDir[0] != '\0') {
        for (nameIndex = 0; nameIndex < sizeof(dllNameList) / sizeof(char *); nameIndex++) {
          sprintf(dllPath, "%s/%s", s7LibDir, dllNameList[nameIndex]);
          if (dllPath[0] == '/' && isalpha(dllPath[1]) && dllPath[2] == '/') {
            dllPath[0] = dllPath[1];
            dllPath[1] = ':';
          } /* if */
          fprintf(logFile, "\rSQLite: DLL / Shared library: %s", dllPath);
          if (fileIsRegular(dllPath)) {
            fprintf(logFile, " (%s)",
                    canLoadDynamicLibrary(dllPath) ? "present" : "cannot load");
            dllPointerSize = pointerSizeOfDynamicLibrary(dllPath);
            if (dllPointerSize != 0) {
              fprintf(logFile, " (%d-bit)", dllPointerSize);
            } /* if */
          } else {
            fprintf(logFile, " (missing)");
          } /* if */
          fprintf(logFile, "\n");
          fprintf(versionFile, " \"%s\",", dllPath);
        } /* for */
      } /* if */
      for (nameIndex = 0; nameIndex < sizeof(dllNameList) / sizeof(char *); nameIndex++) {
        fprintf(logFile, "\rSQLite: DLL / Shared library: %s (%s)\n",
                dllNameList[nameIndex],
                canLoadDynamicLibrary(dllNameList[nameIndex]) ? "present" : "cannot load");
        fprintf(versionFile, " \"%s\",", dllNameList[nameIndex]);
      } /* for */
      fprintf(versionFile, "\n");
    } /* if */
  } /* determineSqliteDefines */



static int extractPostgresOid (const char* pgTypeFileName)
  {
    const char *oidNames[] = {
      "ABSTIMEOID",     "ACLITEMOID",      "ANYARRAYOID",     "ANYELEMENTOID",    "ANYENUMOID",
      "ANYNONARRAYOID", "ANYOID",          "BITOID",          "BOOLOID",          "BOXOID",
      "BPCHAROID",      "BYTEAOID",        "CASHOID",         "CHAROID",          "CIDOID",
      "CIDROID",        "CIRCLEOID",       "CSTRINGARRAYOID", "CSTRINGOID",       "DATEOID",
      "FDW_HANDLEROID", "FLOAT4ARRAYOID",  "FLOAT4OID",       "FLOAT8OID",        "GTSVECTOROID",
      "INETOID",        "INT2ARRAYOID",    "INT2OID",         "INT2VECTOROID",    "INT4ARRAYOID",
      "INT4OID",        "INT8OID",         "INTERNALOID",     "INTERVALOID",      "LANGUAGE_HANDLEROID",
      "LINEOID",        "LSEGOID",         "MACADDROID",      "NAMEOID",          "NUMERICOID",
      "OIDARRAYOID",    "OIDOID",          "OIDVECTOROID",    "OPAQUEOID",        "PATHOID",
      "PGNODETREEOID",  "POINTOID",        "POLYGONOID",      "RECORDARRAYOID",   "RECORDOID",
      "REFCURSOROID",   "REGCLASSOID",     "REGCONFIGOID",    "REGDICTIONARYOID", "REGOPERATOROID",
      "REGOPEROID",     "REGPROCEDUREOID", "REGPROCOID",      "REGTYPEARRAYOID",  "REGTYPEOID",
      "RELTIMEOID",     "TEXTARRAYOID",    "TEXTOID",         "TIDOID",           "TIMEOID",
      "TIMESTAMPOID",   "TIMESTAMPTZOID",  "TIMETZOID",       "TINTERVALOID",     "TRIGGEROID",
      "TSQUERYOID",     "TSVECTOROID",     "UNKNOWNOID",      "VARBITOID",        "VARCHAROID",
      "VOIDOID",        "XIDOID",          "XMLOID"};
    FILE *pgTypeFile;
    FILE *oidFile;
    char buffer[BUFFER_SIZE];
    char *line;
    int pos;
    size_t idx;
    int spaces;
    int anOidWasFound = 0;

  /* extractPostgresOid */
    fprintf(logFile, "\rExtracting OIDs from: %s\n", pgTypeFileName);
    pgTypeFile = fopen(pgTypeFileName, "r");
    if (pgTypeFile != NULL) {
      oidFile = fopen("pg_type.h", "w");
      if (oidFile != NULL) {
        fputs("/* Do not edit this file. It has been generated by chkccomp.c. */\n", oidFile);
        fputs("/* The contents of this file have been extracted from: */\n", oidFile);
        fprintf(oidFile, "/*   %s */\n", pgTypeFileName);
        fputs("\n", oidFile);
        while ((line = fgets(buffer, 4096, pgTypeFile)) != NULL) {
          pos = 0;
          while (line[pos] == ' ' || line[pos] == '\t') {
            pos++;
          } /* if */
          if (line[pos] == '#') {
            pos++;
            while (line[pos] == ' ' || line[pos] == '\t') {
              pos++;
            } /* if */
            if (strncmp(&line[pos], "define", 6) == 0) {
              pos += 6;
              while (line[pos] == ' ' || line[pos] == '\t') {
                pos++;
              } /* if */
              for (idx = 0; idx < sizeof(oidNames) / sizeof(char *); idx++) {
                if (strncmp(&line[pos], oidNames[idx], strlen(oidNames[idx])) == 0) {
                  anOidWasFound = 1;
                  fprintf(oidFile, "#define %s ", oidNames[idx]);
                  pos += strlen(oidNames[idx]);
                  while (line[pos] == ' ' || line[pos] == '\t') {
                    pos++;
                  } /* if */
                  spaces = 24 - strlen(oidNames[idx]) - strlen(&line[pos]);
                  while (spaces > 0) {
                    fputc(' ', oidFile);
                    spaces--;
                  } /* while */
                  fputs(&line[pos], oidFile);
                  idx = sizeof(oidNames) / sizeof(char *);
                } /* if */
              } /* for */
            } /* if */
          } /* if */
        } /* while */
        fclose(oidFile);
        if (!anOidWasFound) {
          fprintf(logFile, "\rExtracting OIDs failed.\n");
          doRemove("pg_type.h");
        } /* if */
      } /* if */
      fclose(pgTypeFile);
    } /* if */
    return anOidWasFound;
  } /* extractPostgresOid */



static int findPgTypeInclude (const char *includeOption, const char *pgTypeInclude)

  {
    const char *optionPos;
    const char *optionEnd;
    size_t optionLen;
    char includeDir[PATH_SIZE];
    char pgTypeFileName[PATH_SIZE + 1 + NAME_SIZE];
    int found = 0;

  /* findPgTypeInclude */
    /* fprintf(logFile, "findPgTypeInclude(\"%s\")\n", includeOption); */
    while (includeOption != NULL && (optionPos = strstr(includeOption, "-I")) != NULL) {
      if (optionPos[2] == '\"') {
        optionEnd = strchr(&optionPos[3], '\"');
        if (optionEnd == NULL) {
          strcpy(includeDir, &optionPos[3]);
          includeOption = NULL;
        } else {
          memcpy(includeDir, &optionPos[3], optionEnd - &optionPos[3]);
          includeDir[optionEnd - &optionPos[3]] = '\0';
          includeOption = optionEnd + 1;
        } /* if */
      } else {
        optionLen = strcspn(&optionPos[2], " \n");
        if (optionLen == 0) {
          strcpy(includeDir, &optionPos[2]);
          includeOption = NULL;
        } else {
          memcpy(includeDir, &optionPos[2], optionLen);
          includeDir[optionLen] = '\0';
          includeOption = &optionPos[2] + optionLen + 1;
        } /* if */
      } /* if */
      /* fprintf(logFile, "includeDir: \"%s\"\n", includeDir); */
      if (includeDir[0] != '\0'  && fileIsDir(includeDir)) {
        sprintf(pgTypeFileName, "%s/%s", includeDir, pgTypeInclude);
        if (fileIsRegular(pgTypeFileName)) {
          if (extractPostgresOid(pgTypeFileName)) {
            includeOption = NULL;
            found = 1;
          } /* if */
        } /* if */
      } /* if */
    } /* while */
    return found;
  } /* findPgTypeInclude */



 static const char *findPgTypeH (char *includeOption, char *serverIncludeOption)

  {
    char testProgram[BUFFER_SIZE];
    const char *pgTypeInclude = NULL;

  /* findPgTypeH */
    if (compileAndLinkWithOptionsOk("#include <server/catalog/pg_type.h>\n"
                                    "int main(int argc,char *argv[]){"
                                    "printf(\"%d\\n\", INT4OID == 23);\n"
                                    "return 0;}\n",
                                    includeOption, "")) {
      pgTypeInclude = "server/catalog/pg_type.h";
    } else if (compileAndLinkWithOptionsOk("#include <server/catalog/pg_type_d.h>\n"
                                           "int main(int argc,char *argv[]){"
                                           "printf(\"%d\\n\", INT4OID == 23);\n"
                                           "return 0;}\n",
                                           includeOption, "")) {
      pgTypeInclude = "server/catalog/pg_type_d.h";
    } else if (compileAndLinkWithOptionsOk("#include <catalog/pg_type.h>\n"
                                           "int main(int argc,char *argv[]){"
                                           "printf(\"%d\\n\", INT4OID == 23);\n"
                                           "return 0;}\n",
                                           includeOption, "")) {
      pgTypeInclude = "catalog/pg_type.h";
    } else if (compileAndLinkWithOptionsOk("#include <catalog/pg_type_d.h>\n"
                                           "int main(int argc,char *argv[]){"
                                           "printf(\"%d\\n\", INT4OID == 23);\n"
                                           "return 0;}\n",
                                           includeOption, "")) {
      pgTypeInclude = "catalog/pg_type_d.h";
    } else {
      appendOption(includeOption, serverIncludeOption);
      if (compileAndLinkWithOptionsOk("#include <server/catalog/pg_type.h>\n"
                                      "int main(int argc,char *argv[]){"
                                      "printf(\"%d\\n\", INT4OID == 23);\n"
                                      "return 0;}\n",
                                      includeOption, "")) {
        pgTypeInclude = "server/catalog/pg_type.h";
      } else if (compileAndLinkWithOptionsOk("#include <server/catalog/pg_type_d.h>\n"
                                             "int main(int argc,char *argv[]){"
                                             "printf(\"%d\\n\", INT4OID == 23);\n"
                                             "return 0;}\n",
                                             includeOption, "")) {
        pgTypeInclude = "server/catalog/pg_type_d.h";
      } else if (compileAndLinkWithOptionsOk("#include <catalog/pg_type.h>\n"
                                             "int main(int argc,char *argv[]){"
                                             "printf(\"%d\\n\", INT4OID == 23);\n"
                                             "return 0;}\n",
                                             includeOption, "")) {
        pgTypeInclude = "catalog/pg_type.h";
      } else if (compileAndLinkWithOptionsOk("#include <catalog/pg_type_d.h>\n"
                                             "int main(int argc,char *argv[]){"
                                             "printf(\"%d\\n\", INT4OID == 23);\n"
                                             "return 0;}\n",
                                             includeOption, "")) {
        pgTypeInclude = "catalog/pg_type_d.h";
      } else {
        pgTypeInclude = "server/catalog/pg_type.h";
      } /* if */
    } /* if */
    sprintf(testProgram, "#include <%s>\n"
                         "int main(int argc,char *argv[]){"
                         "printf(\"%%d\\n\", INT4OID == 23);\n"
                         "return 0;}\n",
                         pgTypeInclude);
    if (compileAndLinkWithOptionsOk(testProgram, includeOption, "")) {
      fprintf(logFile, "\rPostgreSQL: %s found in system include directory.\n", pgTypeInclude);
    } else if (findPgTypeInclude(includeOption, pgTypeInclude)) {
      pgTypeInclude = "pg_type.h";
      fprintf(logFile, "\rPostgreSQL: %s found in Seed7 include directory.\n", pgTypeInclude);
    } else {
      fprintf(logFile, "\rPostgreSQL: %s not found in include directories.\n", pgTypeInclude);
      pgTypeInclude = NULL;
    } /* if */
    return pgTypeInclude;
  } /* findPgTypeH */



static void determinePostgresDefines (FILE *versionFile,
    char *include_options, char *system_database_libs)

  {
    const char *dbVersion[] = {"15", "14", "13", "12", "11", "10",
                               "9.6", "9.5", "9.4", "9.3",
                               "9.2", "9.1", "9.0", "8.4", "8.3"};
#ifdef POSTGRESQL_LIBS
    const char *libNameList[] = { POSTGRESQL_LIBS };
#elif LIBRARY_TYPE == UNIX_LIBRARIES || LIBRARY_TYPE == MACOS_LIBRARIES
    const char *libNameList[] = {"-lpq"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *libNameList[] = {"libpq.lib"};
#endif
#ifdef POSTGRESQL_DLL
    const char *dllNameList[] = { POSTGRESQL_DLL };
#elif LIBRARY_TYPE == UNIX_LIBRARIES
    const char *dllNameList[] = {"libpq.so", "libpq.so.5"};
#elif LIBRARY_TYPE == MACOS_LIBRARIES
    const char *dllNameList[] = {"libpq.dylib"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *dllNameList[] = {"libpq.dll"};
#endif
#if LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *libIntlDllList[] = {"libintl.dll", "libintl-8.dll", "libintl-9.dll"};
    const char *libeay32DllList[] = {"libeay32.dll"};
    const char *libcryptoDllList[] = {"libcrypto-1_1-x64.dll"};
    const char *libsslDllList[] = {"libssl-1_1-x64.dll"};
#else
    const char *libIntlDllList[] = {""};
    const char *libeay32DllList[] = {""};
    const char *libcryptoDllList[] = {""};
    const char *libsslDllList[] = {""};
#endif
    const char *libDirList[] = {"/lib"};
    const char *dllDirList[] = {"/lib", "/bin"};
    unsigned int dirIndex;
    unsigned int nameIndex;
    int searchForLib = 1;
    char filePath[PATH_SIZE + 1 + PATH_SIZE];
    const char *programFilesX86 = NULL;
    const char *programFiles = NULL;
    char dbHome[PATH_SIZE];
    char includeOption[PATH_SIZE + 5 + PATH_SIZE];
    char serverIncludeOption[PATH_SIZE + 5 + PATH_SIZE];
    const char *postgresqlInclude = NULL;
    const char *pgTypeInclude = NULL;
    char testProgram[BUFFER_SIZE];
    int dbHomeExists = 0;

  /* determinePostgresDefines */
#ifdef POSTGRESQL_INCLUDE_OPTIONS
    strcpy(includeOption, POSTGRESQL_INCLUDE_OPTIONS);
#else
    if (fileIsDir("/usr/include/postgresql")) {
      strcpy(includeOption, "-I/usr/include/postgresql");
    } else if (fileIsDir("/usr/include/pgsql")) {
      strcpy(includeOption, "-I/usr/include/pgsql");
    } else {
      includeOption[0] = '\0';
    } /* if */
#endif
    if (fileIsDir("/usr/include/postgresql/server")) {
      strcpy(serverIncludeOption, "-I/usr/include/postgresql/server");
    } else if (fileIsDir("/usr/include/pgsql/server")) {
      strcpy(serverIncludeOption, "-I/usr/include/pgsql/server");
    } else {
      serverIncludeOption[0] = '\0';
    } /* if */
    programFilesX86 = getenv("ProgramFiles(x86)");
    /* fprintf(logFile, "programFilesX86: %s\n", programFilesX86); */
    programFiles = getenv("ProgramFiles");
    /* fprintf(logFile, "programFiles: %s\n", programFiles); */
    if (programFiles != NULL) {
      if (sizeof(char *) == 4 && programFilesX86 != NULL) {
        programFiles = programFilesX86;
      } /* if */
      for (dirIndex = 0; !dbHomeExists && dirIndex < sizeof(dbVersion) / sizeof(char *); dirIndex++) {
        sprintf(dbHome, "%s/PostgreSQL/%s", programFiles, dbVersion[dirIndex]);
        if (fileIsDir(dbHome)) {
          dbHomeExists = 1;
        } /* if */
      } /* for */
    } /* if */
    if (dbHomeExists) {
      /* fprintf(logFile, "dbHome=%s\n", dbHome); */
      sprintf(includeOption, "-I\"%s/include\"", dbHome);
      sprintf(serverIncludeOption, "-I\"%s/include/server\"", dbHome);
      /* fprintf(logFile, "includeOption=%s\n", includeOption); */
      if (compileAndLinkWithOptionsOk("#include \"libpq-fe.h\"\n"
                                      "int main(int argc,char *argv[]){"
                                      "PGconn *connection; return 0;}\n",
                                      includeOption, "")) {
        postgresqlInclude = "libpq-fe.h";
        fprintf(logFile, "\rPostgreSQL: %s found in %s/include\n", postgresqlInclude, dbHome);
        pgTypeInclude = findPgTypeH(includeOption, serverIncludeOption);
      } else {
        sprintf(filePath, "%s/include/libpq-fe.h", dbHome);
        if (fileIsRegular(filePath)) {
          fprintf(logFile, "\rPostgreSQL: The C compiler cannot include %s\n", filePath);
        } /* if */
        postgresqlInclude = NULL;
      } /* if */
    } /* if */
    if (postgresqlInclude == NULL &&
        compileAndLinkWithOptionsOk("#include <libpq-fe.h>\n"
                                    "int main(int argc,char *argv[]){"
                                    "PGconn *connection; return 0;}\n",
                                    includeOption, "")) {
      postgresqlInclude = "libpq-fe.h";
      fprintf(logFile, "\rPostgreSQL: %s found in system include directory.\n", postgresqlInclude);
      pgTypeInclude = findPgTypeH(includeOption, serverIncludeOption);
    } /* if */
    if ((postgresqlInclude == NULL || pgTypeInclude == NULL) &&
        (compileAndLinkWithOptionsOk("#include \"db_post.h\"\n"
                                     "int main(int argc,char *argv[]){\n"
                                     "PGconn *connection; return 0;}\n",
                                     "", "") ||
         compileAndLinkWithOptionsOk("#define CDECL\n"
                                     "include \"db_post.h\"\n"
                                     "int main(int argc,char *argv[]){\n"
                                     "PGconn *connection; return 0;}\n",
                                     "", ""))) {
      postgresqlInclude = "db_post.h";
      fprintf(logFile, "\rPostgreSQL: %s found in Seed7 include directory.\n", postgresqlInclude);
      includeOption[0] = '\0';
    } /* if */
    if (postgresqlInclude != NULL) {
      if (includeOption[0] != '\0') {
        appendOption(include_options, includeOption);
      } /* if */
      fprintf(versionFile, "#define POSTGRESQL_INCLUDE \"%s\"\n", postgresqlInclude);
    } /* if */
    if (pgTypeInclude != NULL) {
      fprintf(versionFile, "#define POSTGRESQL_PG_TYPE_H \"%s\"\n", pgTypeInclude);
    } /* if */
#ifndef POSTGRESQL_USE_DLL
    /* Handle static libraries: */
    sprintf(testProgram, "#include<stdio.h>\n#include<stdlib.h>\n"
                         "#include \"%s\"\n"
                         "int main(int argc,char *argv[]){\n"
                         "PGconn *conn;\n"
                         "conn = PQsetdbLogin(\"\", NULL, NULL, NULL, \"\", \"\", \"\");\n"
                         "PQfinish(conn);\n"
                         "printf(\"1\\n\");\n"
                         "return 0;\n}\n", postgresqlInclude);
    /* fprintf(logFile, "%s\n", testProgram);
       fprintf(logFile, "postgresqlInclude: \"%s\"\n", postgresqlInclude); */
    if (dbHomeExists) {
      if (findStaticLib("PostgreSQL", testProgram, includeOption, "", dbHome,
                        libDirList, sizeof(libDirList) / sizeof(char *),
                        libNameList, sizeof(libNameList) / sizeof(char *),
                        system_database_libs)) {
        searchForLib = 0;
      } /* if */
    } /* if */
    if (searchForLib) {
      if (findLinkerOption("PostgreSQL", testProgram, includeOption, POSTGRESQL_LIBRARY_PATH,
                           libNameList, sizeof(libNameList) / sizeof(char *),
                           system_database_libs)) {
        searchForLib = 0;
      } else if (buildTimeLinking) {
        if (findLinkerOptionForDll("PostgreSQL", testProgram, includeOption,
                                   POSTGRESQL_LIBRARY_PATH, dllNameList,
                                   sizeof(dllNameList) / sizeof(char *),
                                   system_database_libs)) {
          searchForLib = 0;
        } /* if */
      } /* if */
    } /* if */
#endif
    if (searchForLib) {
      /* Handle dynamic libraries: */
      appendOption(system_database_libs, LINKER_OPT_DYN_LINK_LIBS);
      fprintf(versionFile, "#define POSTGRESQL_DLL");
      if (dbHomeExists) {
        listDynamicLibs("PostgreSQL", dbHome,
                        dllDirList, sizeof(dllDirList) / sizeof(char *),
                        dllNameList, sizeof(dllNameList) / sizeof(char *), versionFile);
      } /* if */
      for (nameIndex = 0; nameIndex < sizeof(dllNameList) / sizeof(char *); nameIndex++) {
        fprintf(logFile, "\rPostgreSQL: DLL / Shared library: %s (%s)\n",
                dllNameList[nameIndex],
                canLoadDynamicLibrary(dllNameList[nameIndex]) ? "present" : "cannot load");
        fprintf(versionFile, " \"%s\",", dllNameList[nameIndex]);
      } /* for */
      fprintf(versionFile, "\n");
      defineLibraryMacro("PostgreSQL", dbHomeExists, dbHome, "LIBINTL_DLL",
                         dllDirList, sizeof(dllDirList) / sizeof(char *),
                         dllNameList, sizeof(dllNameList) / sizeof(char *),
                         libIntlDllList, sizeof(libIntlDllList) / sizeof(char *),
                         versionFile);
      defineLibraryMacro("PostgreSQL", dbHomeExists, dbHome, "LIBEAY32_DLL",
                         dllDirList, sizeof(dllDirList) / sizeof(char *),
                         dllNameList, sizeof(dllNameList) / sizeof(char *),
                         libeay32DllList, sizeof(libeay32DllList) / sizeof(char *),
                         versionFile);
      defineLibraryMacro("PostgreSQL", dbHomeExists, dbHome, "LIBCRYPTO_DLL",
                         dllDirList, sizeof(dllDirList) / sizeof(char *),
                         dllNameList, sizeof(dllNameList) / sizeof(char *),
                         libcryptoDllList, sizeof(libcryptoDllList) / sizeof(char *),
                         versionFile);
      defineLibraryMacro("PostgreSQL", dbHomeExists, dbHome, "LIBSSL_DLL",
                         dllDirList, sizeof(dllDirList) / sizeof(char *),
                         dllNameList, sizeof(dllNameList) / sizeof(char *),
                         libsslDllList, sizeof(libsslDllList) / sizeof(char *),
                         versionFile);
    } /* if */
  } /* determinePostgresDefines */



static void determineOdbcDefines (FILE *versionFile,
    char *include_options, char *system_database_libs)

  {
#ifdef ODBC_LIBS
    const char *libNameList[] = { ODBC_LIBS };
#elif LIBRARY_TYPE == UNIX_LIBRARIES
    const char *libNameList[] = {"-lodbc"};
#elif LIBRARY_TYPE == MACOS_LIBRARIES
    const char *libNameList[] = {"-liodbc"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *libNameList[] = {"-lodbc32", "odbc32.lib"};
#endif
#ifdef ODBC_DLL
    const char *dllNameList[] = { ODBC_DLL };
#elif LIBRARY_TYPE == UNIX_LIBRARIES
    const char *dllNameList[] = {"libodbc.so", "libodbc.so.2"};
#elif LIBRARY_TYPE == MACOS_LIBRARIES
    const char *dllNameList[] = {"libiodbc.dylib"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *dllNameList[] = {"odbc32.dll"};
#endif
    unsigned int nameIndex;
    int searchForLib = 1;
    char includeOption[PATH_SIZE];
    int includeWindows = 0;
    int includeSqlext = 0;
    const char *odbcInclude = NULL;
    char testProgram[BUFFER_SIZE];

  /* determineOdbcDefines */
#ifdef ODBC_INCLUDE_OPTIONS
    strcpy(includeOption, ODBC_INCLUDE_OPTIONS);
#else
    includeOption[0] = '\0';
#endif
    if (compileAndLinkWithOptionsOk("#include <windows.h>\n"
                                    "#include <sql.h>\n"
                                    "#include <sqlext.h>\n"
                                    "int main(int argc,char *argv[]){\n"
                                    "SQLHDBC conn; SQLHSTMT stmt;\n"
                                    "SQLSMALLINT h = SQL_HANDLE_STMT;\n"
                                    "return 0;}\n",
                                    includeOption, "")) {
      includeWindows = 1;
      includeSqlext = 1;
      odbcInclude = "sql.h";
      fprintf(logFile, "\rOdbc: %s found in system include directory.\n",
              odbcInclude);
    } else if (compileAndLinkWithOptionsOk("#include <sql.h>\n"
                                           "#include <sqlext.h>\n"
                                           "int main(int argc,char *argv[]){\n"
                                           "SQLHDBC conn; SQLHSTMT stmt;\n"
                                           "SQLSMALLINT h = SQL_HANDLE_STMT;\n"
                                           "return 0;}\n",
                                           includeOption, "")) {
      includeSqlext = 1;
      odbcInclude = "sql.h";
      fprintf(logFile, "\rOdbc: %s found in system include directory.\n",
              odbcInclude);
    } else if (compileAndLinkWithOptionsOk("#include \"tst_vers.h\"\n"
                                           "#include \"db_odbc.h\"\n"
                                           "int main(int argc,char *argv[]){\n"
                                           "SQLHDBC conn; SQLHSTMT stmt;\n"
                                           "SQLSMALLINT h = SQL_HANDLE_STMT;\n"
                                           "return 0;}\n",
                                           "", "") ||
               compileAndLinkWithOptionsOk("#define STDCALL\n"
                                           "#include \"tst_vers.h\"\n"
                                           "#include \"db_odbc.h\"\n"
                                           "int main(int argc,char *argv[]){\n"
                                           "SQLHDBC conn; SQLHSTMT stmt;\n"
                                           "SQLSMALLINT h = SQL_HANDLE_STMT;\n"
                                           "return 0;}\n",
                                           "", "")) {
      odbcInclude = "db_odbc.h";
      fprintf(logFile, "\rOdbc: %s found in Seed7 include directory.\n",
              odbcInclude);
      includeOption[0] = '\0';
    } /* if */
    if (odbcInclude != NULL) {
      fprintf(versionFile, "#define ODBC_INCLUDE_WINDOWS_H %d\n", includeWindows);
      fprintf(versionFile, "#define ODBC_INCLUDE \"%s\"\n", odbcInclude);
      fprintf(versionFile, "#define ODBC_INCLUDE_SQLEXT_H %d\n", includeSqlext);
      appendOption(include_options, includeOption);
      sprintf(testProgram, "#include \"tst_vers.h\"\n#include<stdio.h>\n"
                           "%s#include \"%s\"\n%s"
                           "int main(int argc,char *argv[]){\n"
                           "printf(\"%%d\\n\", sizeof(SQLWCHAR));\n"
                           "return 0;\n}\n",
                           includeWindows ? "#include \"windows.h\"\n" : "", odbcInclude,
                           includeSqlext ? "#include \"sqlext.h\"\n" : "");
      if (compileAndLinkWithOptionsOk(testProgram, includeOption, "")) {
        fprintf(versionFile, "#define ODBC_SIZEOF_SQLWCHAR %d\n", doTest());
      } /* if */
    } /* if */
#ifndef ODBC_USE_DLL
    /* Handle static libraries: */
    sprintf(testProgram, "#include \"tst_vers.h\"\n#include<stdio.h>\n"
                         "%s#include \"%s\"\n%s"
                         "int main(int argc,char *argv[]){\n"
                         "SQLHENV sql_env;\n"
                         "SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sql_env);\n"
                         "SQLFreeHandle(SQL_HANDLE_ENV, sql_env);\n"
                         "printf(\"1\\n\");\n"
                         "return 0;\n}\n",
                         includeWindows ? "#include \"windows.h\"\n" : "", odbcInclude,
                         includeSqlext ? "#include \"sqlext.h\"\n" : "");
    /* fprintf(logFile, "%s\n", testProgram);
       fprintf(logFile, "odbcInclude: \"%s\"\n", odbcInclude); */
    if (findLinkerOption("Odbc", testProgram, includeOption, ODBC_LIBRARY_PATH,
                         libNameList, sizeof(libNameList) / sizeof(char *),
                         system_database_libs)) {
      searchForLib = 0;
    } else if (buildTimeLinking) {
      if (findLinkerOptionForDll("Odbc", testProgram, includeOption,
                                 ODBC_LIBRARY_PATH, dllNameList,
                                 sizeof(dllNameList) / sizeof(char *),
                                 system_database_libs)) {
        searchForLib = 0;
      } /* if */
    } /* if */
#endif
    if (searchForLib) {
      /* Handle dynamic libraries: */
      appendOption(system_database_libs, LINKER_OPT_DYN_LINK_LIBS);
      fprintf(versionFile, "#define ODBC_DLL");
      for (nameIndex = 0; nameIndex < sizeof(dllNameList) / sizeof(char *); nameIndex++) {
        fprintf(logFile, "\rOdbc: DLL / Shared library: %s (%s)\n",
                dllNameList[nameIndex],
                canLoadDynamicLibrary(dllNameList[nameIndex]) ? "present" : "cannot load");
        fprintf(versionFile, " \"%s\",", dllNameList[nameIndex]);
      } /* for */
      fprintf(versionFile, "\n");
    } /* if */
  } /* determineOdbcDefines */



static void determineOciDefines (FILE *versionFile,
    char *include_options, char *system_database_libs, char *rpath)

  {
    const char *dbHome;
#ifdef OCI_LIBS
    const char *libNameList[] = { OCI_LIBS };
#elif LIBRARY_TYPE == UNIX_LIBRARIES || LIBRARY_TYPE == MACOS_LIBRARIES
    const char *libNameList[] = {"-lclntsh"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *libNameList[] = {"oci.lib"};
#endif
#ifdef OCI_DLL
    const char *dllNameList[] = { OCI_DLL };
#elif LIBRARY_TYPE == UNIX_LIBRARIES
    const char *dllNameList[] = {"libclntsh.so"};
#elif LIBRARY_TYPE == MACOS_LIBRARIES
    const char *dllNameList[] = {"libclntsh.dylib"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *dllNameList[] = {"oci.dll"};
#endif
    const char *inclDirList[] = {"/rdbms/public", "/oci/include", "/sdk/include"};
    const char *libDirList[] = {"/lib"};
    const char *dllDirList[] = {"/lib", "/bin", ""};
    int searchForLib = 1;
    char includeOption[PATH_SIZE];
    const char *ociInclude = NULL;
    char testProgram[BUFFER_SIZE];
    int dbHomeExists = 0;

  /* determineOciDefines */
#ifdef OCI_INCLUDE_OPTIONS
    strcpy(includeOption, OCI_INCLUDE_OPTIONS);
#else
    includeOption[0] = '\0';
#endif
    sprintf(testProgram, "#include \"oci.h\"\n"
                         "int main(int argc,char *argv[]){\n"
                         "OCIEnv *oci_environment; return 0;}\n");
    dbHome = getenv("ORACLE_HOME");
    if (dbHome != NULL && fileIsDir(dbHome)) {
      fprintf(logFile, "\rOracle: ORACLE_HOME=%s\n", dbHome);
      dbHomeExists = 1;
      ociInclude = findIncludeFile("Oracle", testProgram, dbHome,
                                   inclDirList, sizeof(inclDirList) / sizeof(char *),
                                   "oci.h", includeOption);
      if (ociInclude != NULL) {
        appendOption(include_options, includeOption);
      } /* if */
    } else if (compileAndLinkWithOptionsOk("#include <oci.h>\n"
                                           "int main(int argc,char *argv[]){"
                                           "OCIEnv *oci_environment; return 0;}\n",
                                           includeOption, "")) {
      ociInclude = "oci.h";
      fprintf(logFile, "\rOracle: %s found in system include directory.\n", ociInclude);
      appendOption(include_options, includeOption);
    } /* if */
    if (ociInclude == NULL &&
        compileAndLinkWithOptionsOk("#include \"tst_vers.h\"\n"
                                    "#include \"stdlib.h\"\n"
                                    "#include \"db_oci.h\"\n"
                                    "int main(int argc,char *argv[]){"
                                    "OCIEnv *oci_environment; return 0;}\n",
                                    "", "")) {
      ociInclude = "db_oci.h";
      fprintf(logFile, "\rOracle: %s found in Seed7 include directory.\n", ociInclude);
    } /* if */
    if (ociInclude != NULL) {
      fprintf(versionFile, "#define OCI_INCLUDE \"%s\"\n", ociInclude);
    } /* if */
#ifndef OCI_USE_DLL
    /* Handle static libraries: */
    sprintf(testProgram, "#include \"tst_vers.h\"\n"
                         "#include<stdio.h>\n#include<stdlib.h>\n"
                         "#include \"%s\"\n"
                         "int main(int argc,char *argv[]){\n"
                         "OCIEnv *oci_env;\n"
                         "OCIEnvCreate(&oci_env, OCI_DEFAULT, NULL, NULL, NULL, NULL, 0, NULL);\n"
                         "OCIHandleFree(oci_env, OCI_HTYPE_ENV);\n"
                         "printf(\"1\\n\");\n"
                         "return 0;\n}\n", ociInclude);
    /* fprintf(logFile, "%s\n", testProgram);
       fprintf(logFile, "ociInclude: \"%s\"\n", ociInclude); */
    if (dbHomeExists) {
      if (findStaticLib("Oracle", testProgram, includeOption, "", dbHome,
                        libDirList, sizeof(libDirList) / sizeof(char *),
                        libNameList, sizeof(libNameList) / sizeof(char *),
                        system_database_libs)) {
        searchForLib = 0;
      } /* if */
    } /* if */
    if (searchForLib) {
      if (findLinkerOption("Oracle", testProgram, includeOption, OCI_LIBRARY_PATH,
                           libNameList, sizeof(libNameList) / sizeof(char *),
                           system_database_libs)) {
        searchForLib = 0;
      } else if (buildTimeLinking) {
        if (findLinkerOptionForDll("Oracle", testProgram, includeOption,
                                   OCI_LIBRARY_PATH, dllNameList,
                                   sizeof(dllNameList) / sizeof(char *),
                                   system_database_libs)) {
          searchForLib = 0;
        } /* if */
      } /* if */
    } /* if */
#endif
    if (searchForLib) {
      /* Handle dynamic libraries: */
      appendOption(system_database_libs, LINKER_OPT_DYN_LINK_LIBS);
      fprintf(versionFile, "#define OCI_DLL");
      /* The oci library has many dependencies to other shared  */
      /* object libraries, which are in the directory of the    */
      /* main oci library. To dynamically link with ldopen() at */
      /* the run-time of the program either this directory must */
      /* be listed in the LD_LIBRARY_PATH or the -rpath option  */
      /* must be used. Note that the meaning of the -rpath      */
      /* option has changed and it must be combined with the    */
      /* option --disable-new-dtags to get the old behavior.    */
      addDynamicLibsWithRpath("Oracle", dbHomeExists, dbHome,
                             dllDirList, sizeof(dllDirList) / sizeof(char *),
                             dllNameList, sizeof(dllNameList) / sizeof(char *),
                             rpath, versionFile);
      fprintf(versionFile, "\n");
    } /* if */
  } /* determineOciDefines */



static void determineFireDefines (FILE *versionFile,
    char *include_options, char *system_database_libs)

  {
    const char *dbHomeSys[] = {"Firebird/Firebird_3_0",
                               "Firebird/Firebird_2_0"};
#ifdef FIRE_LIBS
    const char *libNameList[] = { FIRE_LIBS };
#elif LIBRARY_TYPE == UNIX_LIBRARIES || LIBRARY_TYPE == MACOS_LIBRARIES
    const char *libNameList[] = {"-lfbclient"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *libNameList[] = {"-lfbclient", "gds32.lib"};
#endif
#ifdef FIRE_DLL
    const char *dllNameList[] = { FIRE_DLL };
#elif LIBRARY_TYPE == UNIX_LIBRARIES
    const char *dllNameList[] = {"libfbclient.so"};
#elif LIBRARY_TYPE == MACOS_LIBRARIES
    const char *dllNameList[] = {"libfbclient.dylib"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *dllNameList[] = {"fbclient.dll", "gds32.dll"};
#endif
    const char *inclDirList[] = {"/include"};
    const char *dllDirList[] = {""};
    unsigned int dirIndex;
    unsigned int nameIndex;
    int searchForLib = 1;
    const char *programFilesX86 = NULL;
    const char *programFiles = NULL;
    char dbHome[PATH_SIZE];
    char includeOption[PATH_SIZE];
    const char *fireInclude = NULL;
    char testProgram[BUFFER_SIZE];
    int dbHomeExists = 0;

  /* determineFireDefines */
#ifdef FIRE_INCLUDE_OPTIONS
    strcpy(includeOption, FIRE_INCLUDE_OPTIONS);
#else
    includeOption[0] = '\0';
#endif
    programFilesX86 = getenv("ProgramFiles(x86)");
    /* fprintf(logFile, "programFilesX86: %s\n", programFilesX86); */
    programFiles = getenv("ProgramFiles");
    /* fprintf(logFile, "programFiles: %s\n", programFiles); */
    if (programFiles != NULL) {
      if (sizeof(char *) == 4 && programFilesX86 != NULL) {
        programFiles = programFilesX86;
      } /* if */
      for (dirIndex = 0; !dbHomeExists && dirIndex < sizeof(dbHomeSys) / sizeof(char *); dirIndex++) {
        sprintf(dbHome, "%s/%s", programFiles, dbHomeSys[dirIndex]);
        /* fprintf(logFile, "dbHome: <%s>\n", dbHome); */
        if (fileIsDir(dbHome)) {
          dbHomeExists = 1;
        } /* if */
      } /* for */
    } /* if */
    if (dbHomeExists) {
      /* fprintf(logFile, "dbHome=%s\n", dbHome); */
      sprintf(testProgram, "#include \"ibase.h\"\n"
                           "int main(int argc,char *argv[]){"
                           "isc_db_handle conn;\n"
                           "isc_stmt_handle stmt;\n"
                           "ISC_STATUS status_vector[20];\n"
                           "char name = isc_dpb_user_name;\n"
                           "char passwd = isc_dpb_password;\n"
                           "return 0;}\n");
      fireInclude = findIncludeFile("Firebird", testProgram, dbHome,
                                    inclDirList, sizeof(inclDirList) / sizeof(char *),
                                    "ibase.h", includeOption);
      if (fireInclude != NULL) {
        appendOption(include_options, includeOption);
      } /* if */
    } /* if */
    if (fireInclude == NULL) {
      if (compileAndLinkWithOptionsOk("#include <ibase.h>\n"
                                      "int main(int argc,char *argv[]){\n"
                                      "isc_db_handle conn;\n"
                                      "isc_stmt_handle stmt;\n"
                                      "ISC_STATUS status_vector[20];\n"
                                      "char name = isc_dpb_user_name;\n"
                                      "char passwd = isc_dpb_password;\n"
                                      "return 0;}\n",
                                      includeOption, "")) {
        fireInclude = "ibase.h";
        fprintf(logFile, "\rFirebird: %s found in system include directory.\n",
                fireInclude);
        appendOption(include_options, includeOption);
      } else if (compileAndLinkWithOptionsOk("#include <firebird/ibase.h>\n"
                                             "int main(int argc,char *argv[]){\n"
                                             "isc_db_handle conn;\n"
                                             "isc_stmt_handle stmt;\n"
                                             "ISC_STATUS status_vector[20];\n"
                                             "char name = isc_dpb_user_name;\n"
                                             "char passwd = isc_dpb_password;\n"
                                             "return 0;}\n",
                                             includeOption, "")) {
        fireInclude = "firebird/ibase.h";
        fprintf(logFile, "\rFirebird: %s found in system include directory.\n",
                fireInclude);
        appendOption(include_options, includeOption);
      } else if (compileAndLinkWithOptionsOk("#include \"tst_vers.h\"\n"
                                             "#include \"db_fire.h\"\n"
                                             "int main(int argc,char *argv[]){\n"
                                             "isc_db_handle conn;\n"
                                             "isc_stmt_handle stmt;\n"
                                             "ISC_STATUS status_vector[20];\n"
                                             "char name = isc_dpb_user_name;\n"
                                             "char passwd = isc_dpb_password;\n"
                                             "return 0;}\n",
                                             "", "") ||
                 compileAndLinkWithOptionsOk("#define STDCALL\n"
                                             "#include \"tst_vers.h\"\n"
                                             "#include \"db_fire.h\"\n"
                                             "int main(int argc,char *argv[]){\n"
                                             "isc_db_handle conn;\n"
                                             "isc_stmt_handle stmt;\n"
                                             "ISC_STATUS status_vector[20];\n"
                                             "char name = isc_dpb_user_name;\n"
                                             "char passwd = isc_dpb_password;\n"
                                             "return 0;}\n",
                                             "", "")) {
        fireInclude = "db_fire.h";
        fprintf(logFile, "\rFirebird: %s found in Seed7 include directory.\n",
                fireInclude);
        includeOption[0] = '\0';
      } /* if */
    } /* if */
    if (fireInclude != NULL) {
      fprintf(versionFile, "#define FIRE_INCLUDE \"%s\"\n", fireInclude);
    } /* if */
#ifndef FIRE_USE_DLL
    /* Handle static libraries: */
    sprintf(testProgram, "#include \"tst_vers.h\"\n#include<stdio.h>\n"
                         "#include \"%s\"\n"
                         "int main(int argc,char *argv[]){\n"
                         "ISC_STATUS status_vector[20];\n"
                         "isc_stmt_handle stmt_handle;\n"
                         "XSQLDA *out_sqlda;\n"
                         "out_sqlda = (XSQLDA *) malloc(XSQLDA_LENGTH(10));\n"
                         "isc_dsql_describe(status_vector, &stmt_handle, 1, out_sqlda);\n"
                         "printf(\"1\\n\");\n"
                         "return 0;\n}\n", fireInclude);
    /* fprintf(logFile, "%s\n", testProgram);
       fprintf(logFile, "fireInclude: \"%s\"\n", fireInclude); */
    if (findLinkerOption("Firebird", testProgram, includeOption, FIRE_LIBRARY_PATH,
                         libNameList, sizeof(libNameList) / sizeof(char *),
                         system_database_libs)) {
      searchForLib = 0;
    } else if (buildTimeLinking) {
      if (findLinkerOptionForDll("Firebird", testProgram, includeOption,
                                 FIRE_LIBRARY_PATH, dllNameList,
                                 sizeof(dllNameList) / sizeof(char *),
                                 system_database_libs)) {
        searchForLib = 0;
      } /* if */
    } /* if */
#endif
    if (searchForLib) {
      /* Handle dynamic libraries: */
      appendOption(system_database_libs, LINKER_OPT_DYN_LINK_LIBS);
      fprintf(versionFile, "#define FIRE_DLL");
      if (dbHomeExists) {
        listDynamicLibs("Firebird", dbHome,
                        dllDirList, sizeof(dllDirList) / sizeof(char *),
                        dllNameList, sizeof(dllNameList) / sizeof(char *), versionFile);
      } /* if */
      for (nameIndex = 0; nameIndex < sizeof(dllNameList) / sizeof(char *); nameIndex++) {
        fprintf(logFile, "\rFirebird: DLL / Shared library: %s (%s)\n",
                dllNameList[nameIndex],
                canLoadDynamicLibrary(dllNameList[nameIndex]) ? "present" : "cannot load");
        fprintf(versionFile, " \"%s\",", dllNameList[nameIndex]);
      } /* for */
      fprintf(versionFile, "\n");
    } /* if */
  } /* determineFireDefines */



static void determineDb2Defines (FILE *versionFile,
    char *include_options, char *system_database_libs)

  {
    const char *dbHome;
#ifdef DB2_LIBS
    const char *libNameList[] = { DB2_LIBS };
#elif LIBRARY_TYPE == UNIX_LIBRARIES || LIBRARY_TYPE == MACOS_LIBRARIES
    const char *libNameList[] = {"libdb2.a"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *libNameList[] = {"db2cli.lib"};
#endif
#ifdef DB2_DLL
    const char *dllNameList[] = { DB2_DLL };
#elif LIBRARY_TYPE == UNIX_LIBRARIES
    const char *dllNameList[] = {"libdb2.so"};
#elif LIBRARY_TYPE == MACOS_LIBRARIES
    const char *dllNameList[] = {"libdb2.dylib"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *dllNameList[] = {"db2cli.dll"};
#endif
    const char *db2DSDriverDir[] = {"C:/Program Files/IBM/DB2DSDriver",
                                    "D:/Program Files/IBM/DB2DSDriver"};
    const char *inclDirList[] = {"/include"};
    const char *libDirList[] = {"/lib"};
    unsigned int driverDirIndex;
    unsigned int nameIndex;
    int searchForLib = 1;
    char includeOption[PATH_SIZE];
    char makeDefinition[PATH_SIZE + 4 + NAME_SIZE];
    const char *db2Include = NULL;
    char testProgram[BUFFER_SIZE];
    char db2_libs[BUFFER_SIZE];
    int dbHomeExists = 0;

  /* determineDb2Defines */
#ifdef DB2_INCLUDE_OPTIONS
    strcpy(includeOption, DB2_INCLUDE_OPTIONS);
#else
    includeOption[0] = '\0';
#endif
    sprintf(testProgram, "#include <sqlcli1.h>\n"
                         "int main(int argc,char *argv[]){\n"
                         "SQLHDBC conn; SQLHSTMT stmt;\n"
                         "SQLSMALLINT h = SQL_HANDLE_STMT;\n"
                         "return 0;}\n");
    dbHome = getenv("DB2_HOME");
    if (dbHome != NULL && fileIsDir(dbHome)) {
      fprintf(logFile, "\rDB2: DB2_HOME=%s\n", dbHome);
      dbHomeExists = 1;
      db2Include = findIncludeFile("DB2", testProgram, dbHome,
                                   inclDirList, sizeof(inclDirList) / sizeof(char *),
                                   "sqlcli1.h", includeOption);
    } /* if */
    for (driverDirIndex = 0;
         db2Include == NULL && driverDirIndex < sizeof(db2DSDriverDir) / sizeof(char *);
         driverDirIndex++) {
      /* printf("driverDir: %s\n", db2DSDriverDir[driverDirIndex]); */
      if (fileIsDir(db2DSDriverDir[driverDirIndex])) {
        /* printf("fileIsDir(%s)\n", db2DSDriverDir[driverDirIndex]); */
        db2Include = findIncludeFile("DB2", testProgram, db2DSDriverDir[driverDirIndex],
                                     inclDirList, sizeof(inclDirList) / sizeof(char *),
                                     "sqlcli1.h", includeOption);
        if (db2Include != NULL) {
          dbHome = db2DSDriverDir[driverDirIndex];
          dbHomeExists = 1;
        } /* if */
      } /* if */
    } /* for */
    if (db2Include == NULL) {
      if (compileAndLinkWithOptionsOk("#include \"tst_vers.h\"\n"
                                      "#include \"db_odbc.h\"\n"
                                      "int main(int argc,char *argv[]){\n"
                                      "SQLHDBC conn; SQLHSTMT stmt;\n"
                                      "SQLSMALLINT h = SQL_HANDLE_STMT;\n"
                                      "return 0;}\n",
                                      "", "") ||
          compileAndLinkWithOptionsOk("#define STDCALL\n"
                                      "#include \"tst_vers.h\"\n"
                                      "#include \"db_odbc.h\"\n"
                                      "int main(int argc,char *argv[]){\n"
                                      "SQLHDBC conn; SQLHSTMT stmt;\n"
                                      "SQLSMALLINT h = SQL_HANDLE_STMT;\n"
                                      "return 0;}\n",
                                      "", "")) {
        db2Include = "db_odbc.h";
        fprintf(logFile, "\rDB2: %s found in Seed7 include directory.\n",
                db2Include);
        includeOption[0] = '\0';
      } /* if */
    } /* if */
    if (db2Include != NULL) {
      fprintf(versionFile, "#define DB2_INCLUDE \"%s\"\n", db2Include);
      fprintf(versionFile, "#define DB2_INCLUDE_OPTION \"");
      escapeString(versionFile, includeOption);
      fprintf(versionFile, "\"\n");
      sprintf(makeDefinition, "DB2_INCLUDE_OPTION = %s\n", includeOption);
      appendToFile("macros", makeDefinition);
      sprintf(testProgram, "#include \"tst_vers.h\"\n#include<stdio.h>\n"
                           "#include \"%s\"\n"
                           "int main(int argc,char *argv[]){\n"
                           "printf(\"%%d\\n\", sizeof(SQLWCHAR));\n"
                           "return 0;\n}\n",
                           db2Include);
      if (compileAndLinkWithOptionsOk(testProgram, includeOption, "")) {
        fprintf(versionFile, "#define DB2_SIZEOF_SQLWCHAR %d\n", doTest());
      } /* if */
    } /* if */
#ifndef DB2_USE_DLL
    if (supportsPartialLinking) {
      /* Handle static libraries: */
      sprintf(testProgram, "#include \"tst_vers.h\"\n#include<stdio.h>\n"
                           "#include \"%s\"\n"
                           "int main(int argc,char *argv[]){\n"
                           "SQLHENV sql_env;\n"
                           "SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sql_env);\n"
                           "SQLFreeHandle(SQL_HANDLE_ENV, sql_env);\n"
                           "printf(\"1\\n\");\n"
                           "return 0;\n}\n",
                           db2Include);
      /* fprintf(logFile, "%s\n", testProgram);
         fprintf(logFile, "db2Include: \"%s\"\n", db2Include); */
      db2_libs[0] = '\0';
      if (dbHomeExists) {
        if (findStaticLib("DB2", testProgram, includeOption, "", dbHome,
                          libDirList, sizeof(libDirList) / sizeof(char *),
                          libNameList, sizeof(libNameList) / sizeof(char *),
                          db2_libs)) {
          appendToMakeMacros("DB2_LIBS", db2_libs);
          searchForLib = 0;
        } /* if */
      } /* if */
      if (searchForLib) {
        if (findLinkerOption("DB2", testProgram, includeOption, DB2_LIBRARY_PATH,
                             libNameList, sizeof(libNameList) / sizeof(char *),
                             db2_libs)) {
          appendToMakeMacros("DB2_LIBS", db2_libs);
          searchForLib = 0;
        } else if (buildTimeLinking) {
          if (findLinkerOptionForDll("DB2", testProgram, includeOption,
                                     DB2_LIBRARY_PATH, dllNameList,
                                     sizeof(dllNameList) / sizeof(char *),
                                     db2_libs)) {
            appendToMakeMacros("DB2_LIBS", db2_libs);
            searchForLib = 0;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
#endif
    if (searchForLib) {
      /* Handle dynamic libraries: */
      appendOption(system_database_libs, LINKER_OPT_DYN_LINK_LIBS);
      fprintf(versionFile, "#define DB2_DLL");
      for (nameIndex = 0; nameIndex < sizeof(dllNameList) / sizeof(char *); nameIndex++) {
        fprintf(logFile, "\rDB2: DLL / Shared library: %s (%s)\n",
                dllNameList[nameIndex],
                canLoadDynamicLibrary(dllNameList[nameIndex]) ? "present" : "cannot load");
        fprintf(versionFile, " \"%s\",", dllNameList[nameIndex]);
      } /* for */
      fprintf(versionFile, "\n");
    } /* if */
  } /* determineDb2Defines */



static void determineInformixDefines (FILE *versionFile,
    char *include_options, char *system_database_libs, char *rpath)

  {
    const char *dbHomeSys[] = {"Informix"};
    const char *informixSdk[] = {"/opt/informix",
                                 "/opt/IBM/Informix_Client-SDK"};
#ifdef INFORMIX_LIBS
    const char *libNameList[] = { INFORMIX_LIBS };
#elif LIBRARY_TYPE == UNIX_LIBRARIES || LIBRARY_TYPE == MACOS_LIBRARIES
    const char *libNameList[] = {"iclit09b.a"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *libNameList[] = {"iclit09b.lib"};
#endif
#ifdef INFORMIX_DLL
    const char *dllNameList[] = { INFORMIX_DLL };
#elif LIBRARY_TYPE == UNIX_LIBRARIES
    const char *dllNameList[] = {"iclit09b.so"};
    const char *libIfglsDllList[] = {"libifgls.so"};
#elif LIBRARY_TYPE == MACOS_LIBRARIES
    const char *dllNameList[] = {"iclit09b.dylib"};
    const char *libIfglsDllList[] = {"libifgls.dylib"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *dllNameList[] = {"iclit09b.dll"};
    const char *libIfglsDllList[] = {""};
#endif
    const char *inclDirList[] = {"/incl/cli"};
    const char *libDirList[] = {"/lib", "/lib/cli"};
    const char *dllDirList[] = {"/bin", "/lib/cli", "/lib/esql"};
    unsigned int dirIndex;
    int searchForLib = 1;
    const char *informixDir = NULL;
    const char *programFilesX86 = NULL;
    const char *programFiles = NULL;
    char dbHome[PATH_SIZE];
    char includeOption[PATH_SIZE];
    char makeDefinition[PATH_SIZE + 4 + NAME_SIZE];
    int includeWindows = 0;
    int includeSqlext = 0;
    const char *informixInclude = NULL;
    char testProgram[BUFFER_SIZE];
    char informix_libs[BUFFER_SIZE];
    int dbHomeExists = 0;

  /* determineInformixDefines */
#ifdef INFORMIX_INCLUDE_OPTIONS
    strcpy(includeOption, INFORMIX_INCLUDE_OPTIONS);
#else
    includeOption[0] = '\0';
#endif
    informixDir = getenv("INFORMIXDIR");
    if (informixDir != NULL) {
      if (strlen(informixDir) < BUFFER_SIZE && fileIsDir(informixDir)) {
        strcpy(dbHome, informixDir);
        dbHomeExists = 1;
      } /* if */
    } /* if */
    if (!dbHomeExists) {
      programFilesX86 = getenv("ProgramFiles(x86)");
      /* fprintf(logFile, "programFilesX86: %s\n", programFilesX86); */
      programFiles = getenv("ProgramFiles");
      /* fprintf(logFile, "programFiles: %s\n", programFiles); */
      if (programFiles != NULL) {
        if (sizeof(char *) == 4 && programFilesX86 != NULL) {
          programFiles = programFilesX86;
        } /* if */
        for (dirIndex = 0; !dbHomeExists && dirIndex < sizeof(dbHomeSys) / sizeof(char *); dirIndex++) {
          sprintf(dbHome, "%s/%s", programFiles, dbHomeSys[dirIndex]);
          /* fprintf(logFile, "dbHome: <%s>\n", dbHome); */
          if (fileIsDir(dbHome)) {
            dbHomeExists = 1;
          } /* if */
        } /* for */
      } /* if */
    } /* if */
    if (!dbHomeExists) {
      for (dirIndex = 0; !dbHomeExists && dirIndex < sizeof(informixSdk) / sizeof(char *); dirIndex++) {
        if (fileIsDir(informixSdk[dirIndex])) {
          strcpy(dbHome, informixSdk[dirIndex]);
          dbHomeExists = 1;
        } /* if */
      } /* for */
    } /* if */
    if (dbHomeExists) {
      fprintf(versionFile, "#define INFORMIXDIR \"%s\"\n", dbHome);
    } /* if */
    sprintf(testProgram, "#include <infxcli.h>\n"
                         "int main(int argc,char *argv[]){\n"
                         "SQLHDBC conn; SQLHSTMT stmt;\n"
                         "SQLSMALLINT h = SQL_HANDLE_STMT;\n"
                         "return 0;}\n");
    if (dbHomeExists) {
      informixInclude = findIncludeFile("Informix", testProgram, dbHome,
                                        inclDirList, sizeof(inclDirList) / sizeof(char *),
                                        "infxcli.h", includeOption);
    } /* if */
    if (informixInclude == NULL) {
      if (compileAndLinkWithOptionsOk("#include <windows.h>\n"
                                      "#include <sql.h>\n"
                                      "#include <sqlext.h>\n"
                                      "int main(int argc,char *argv[]){\n"
                                      "SQLHDBC conn; SQLHSTMT stmt;\n"
                                      "SQLSMALLINT h = SQL_HANDLE_STMT;\n"
                                      "return 0;}\n",
                                      includeOption, "")) {
        includeWindows = 1;
        includeSqlext = 1;
        informixInclude = "sql.h";
        fprintf(logFile, "\rInformix: %s found in system include directory.\n",
                informixInclude);
      } else if (compileAndLinkWithOptionsOk("#include <sql.h>\n"
                                             "#include <sqlext.h>\n"
                                             "int main(int argc,char *argv[]){\n"
                                             "SQLHDBC conn; SQLHSTMT stmt;\n"
                                             "SQLSMALLINT h = SQL_HANDLE_STMT;\n"
                                             "return 0;}\n",
                                             includeOption, "")) {
        includeSqlext = 1;
        informixInclude = "sql.h";
        fprintf(logFile, "\rInformix: %s found in system include directory.\n",
                informixInclude);
      } else if (compileAndLinkWithOptionsOk("#include \"tst_vers.h\"\n"
                                             "#include \"db_odbc.h\"\n"
                                             "int main(int argc,char *argv[]){\n"
                                             "SQLHDBC conn; SQLHSTMT stmt;\n"
                                             "SQLSMALLINT h = SQL_HANDLE_STMT;\n"
                                             "return 0;}\n",
                                             "", "") ||
                 compileAndLinkWithOptionsOk("#define STDCALL\n"
                                             "#include \"tst_vers.h\"\n"
                                             "#include \"db_odbc.h\"\n"
                                             "int main(int argc,char *argv[]){\n"
                                             "SQLHDBC conn; SQLHSTMT stmt;\n"
                                             "SQLSMALLINT h = SQL_HANDLE_STMT;\n"
                                             "return 0;}\n",
                                             "", "")) {
        informixInclude = "db_odbc.h";
        fprintf(logFile, "\rInformix: %s found in Seed7 include directory.\n",
                informixInclude);
        includeOption[0] = '\0';
      } /* if */
    } /* if */
    if (informixInclude != NULL) {
      fprintf(versionFile, "#define INFORMIX_INCLUDE_WINDOWS_H %d\n", includeWindows);
      fprintf(versionFile, "#define INFORMIX_INCLUDE \"%s\"\n", informixInclude);
      fprintf(versionFile, "#define INFORMIX_INCLUDE_SQLEXT_H %d\n", includeSqlext);
      fprintf(versionFile, "#define INFORMIX_INCLUDE_OPTION \"");
      escapeString(versionFile, includeOption);
      fprintf(versionFile, "\"\n");
      sprintf(makeDefinition, "INFORMIX_INCLUDE_OPTION = %s\n", includeOption);
      appendToFile("macros", makeDefinition);
      sprintf(testProgram, "#include \"tst_vers.h\"\n#include<stdio.h>\n"
                           "%s#include \"%s\"\n%s"
                           "int main(int argc,char *argv[]){\n"
                           "printf(\"%%d\\n\", sizeof(SQLWCHAR));\n"
                           "return 0;\n}\n",
                           includeWindows ? "#include \"windows.h\"\n" : "",
                           informixInclude,
                           includeSqlext ? "#include \"sqlext.h\"\n" : "");
      if (compileAndLinkWithOptionsOk(testProgram, includeOption, "")) {
        fprintf(versionFile, "#define INFORMIX_SIZEOF_SQLWCHAR %d\n", doTest());
      } /* if */
    } /* if */
#ifndef INFORMIX_USE_DLL
    if (supportsPartialLinking) {
      /* Handle static libraries: */
      sprintf(testProgram, "#include \"tst_vers.h\"\n#include<stdio.h>\n"
                           "%s#include \"%s\"\n%s"
                           "int main(int argc,char *argv[]){\n"
                           "SQLHENV sql_env;\n"
                           "SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sql_env);\n"
                           "SQLFreeHandle(SQL_HANDLE_ENV, sql_env);\n"
                           "printf(\"1\\n\");\n"
                           "return 0;\n}\n",
                           includeWindows ? "#include \"windows.h\"\n" : "",
                           informixInclude,
                           includeSqlext ? "#include \"sqlext.h\"\n" : "");
      /* fprintf(logFile, "%s\n", testProgram);
         fprintf(logFile, "informixInclude: \"%s\"\n", informixInclude); */
      informix_libs[0] = '\0';
      if (dbHomeExists) {
        if (findStaticLib("Informix", testProgram, includeOption, "-lcrypt -lm", dbHome,
                          libDirList, sizeof(libDirList) / sizeof(char *),
                          libNameList, sizeof(libNameList) / sizeof(char *),
                          informix_libs)) {
          /* appendOption(informix_libs, "-lcrypt");
             appendOption(informix_libs, "-lm"); */
          appendToMakeMacros("INFORMIX_LIBS", informix_libs);
          searchForLib = 0;
        } /* if */
      } /* if */
      if (searchForLib) {
        if (findLinkerOption("Informix", testProgram, includeOption, INFORMIX_LIBRARY_PATH,
                             libNameList, sizeof(libNameList) / sizeof(char *),
                             informix_libs)) {
          appendToMakeMacros("INFORMIX_LIBS", informix_libs);
          searchForLib = 0;
        } else if (buildTimeLinking) {
          if (findLinkerOptionForDll("Informix", testProgram, includeOption,
                                     INFORMIX_LIBRARY_PATH, dllNameList,
                                     sizeof(dllNameList) / sizeof(char *),
                                     informix_libs)) {
            appendToMakeMacros("INFORMIX_LIBS", informix_libs);
            searchForLib = 0;
          } /* if */
        } /* if */
      } /* if */
    } /* if */
#endif
    if (searchForLib) {
      /* Handle dynamic libraries: */
      appendOption(system_database_libs, LINKER_OPT_DYN_LINK_LIBS);
      fprintf(versionFile, "#define INFORMIX_DLL");
      addDynamicLibsWithRpath("Informix", dbHomeExists, dbHome,
                              dllDirList, sizeof(dllDirList) / sizeof(char *),
                              dllNameList, sizeof(dllNameList) / sizeof(char *),
                              rpath, versionFile);
      fprintf(versionFile, "\n");
      if (sizeof(libIfglsDllList) / sizeof(char *) > 1 ||
          (sizeof(libIfglsDllList) / sizeof(char *) == 1 &&
          libIfglsDllList[0][0] != '\0')) {
        fprintf(versionFile, "#define IFGLS_DLL");
        addDynamicLibsWithRpath("Informix", dbHomeExists, dbHome,
                                dllDirList, sizeof(dllDirList) / sizeof(char *),
                                libIfglsDllList, sizeof(libIfglsDllList) / sizeof(char *),
                                rpath, versionFile);
        fprintf(versionFile, "\n");
      } /* if */
    } /* if */
  } /* determineInformixDefines */



static void determineSqlServerDefines (FILE *versionFile,
    char *include_options, char *system_database_libs)

  {
#ifdef SQL_SERVER_LIBS
    const char *libNameList[] = { SQL_SERVER_LIBS };
#elif LIBRARY_TYPE == UNIX_LIBRARIES || LIBRARY_TYPE == MACOS_LIBRARIES
    const char *libNameList[] = {"libtdsodbc.a"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *libNameList[] = {"sqlsrv32.lib"};
#endif
#ifdef SQL_SERVER_DLL
    const char *dllNameList[] = { SQL_SERVER_DLL };
#elif LIBRARY_TYPE == UNIX_LIBRARIES
    const char *dllNameList[] = {"libtdsodbc.so", "libtdsodbc.so.0"};
#elif LIBRARY_TYPE == MACOS_LIBRARIES
    const char *dllNameList[] = {"libtdsodbc.dylib"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    /* sqlncli11.dll is omitted, because it truncates fields. */
    const char *dllNameList[] = {"sqlsrv32.dll"};
#endif
    unsigned int nameIndex;
    int searchForLib = 1;
    char includeOption[PATH_SIZE];
    char makeDefinition[PATH_SIZE + 4 + NAME_SIZE];
    int freeTdsLibrary = 0;
    int includeWindows = 0;
    int includeSqlext = 0;
    const char *sqlServerInclude = NULL;
    char testProgram[BUFFER_SIZE];
    char sql_server_libs[BUFFER_SIZE];

  /* determineSqlServerDefines */
#if LIBRARY_TYPE == UNIX_LIBRARIES || LIBRARY_TYPE == MACOS_LIBRARIES
    freeTdsLibrary = 1;
#endif
#ifdef SQL_SERVER_INCLUDE_OPTIONS
    strcpy(includeOption, SQL_SERVER_INCLUDE_OPTIONS);
#else
    includeOption[0] = '\0';
#endif
    if (compileAndLinkWithOptionsOk("#include <windows.h>\n"
                                    "#include <sql.h>\n"
                                    "#include <sqlext.h>\n"
                                    "int main(int argc,char *argv[]){\n"
                                    "SQLHDBC conn; SQLHSTMT stmt;\n"
                                    "SQLSMALLINT h = SQL_HANDLE_STMT;\n"
                                    "return 0;}\n",
                                    includeOption, "")) {
      includeWindows = 1;
      includeSqlext = 1;
      sqlServerInclude = "sql.h";
      fprintf(logFile, "\rSQL Server: %s found in system include directory.\n",
              sqlServerInclude);
    } else if (compileAndLinkWithOptionsOk("#include <sql.h>\n"
                                           "#include <sqlext.h>\n"
                                           "int main(int argc,char *argv[]){\n"
                                           "SQLHDBC conn; SQLHSTMT stmt;\n"
                                           "SQLSMALLINT h = SQL_HANDLE_STMT;\n"
                                           "return 0;}\n",
                                           includeOption, "")) {
      includeSqlext = 1;
      sqlServerInclude = "sql.h";
      fprintf(logFile, "\rSQL Server: %s found in system include directory.\n",
              sqlServerInclude);
    } else if (compileAndLinkWithOptionsOk("#include \"tst_vers.h\"\n"
                                           "#include \"db_odbc.h\"\n"
                                           "int main(int argc,char *argv[]){\n"
                                           "SQLHDBC conn; SQLHSTMT stmt;\n"
                                           "SQLSMALLINT h = SQL_HANDLE_STMT;\n"
                                           "return 0;}\n",
                                           "", "") ||
               compileAndLinkWithOptionsOk("#define STDCALL\n"
                                           "#include \"tst_vers.h\"\n"
                                           "#include \"db_odbc.h\"\n"
                                           "int main(int argc,char *argv[]){\n"
                                           "SQLHDBC conn; SQLHSTMT stmt;\n"
                                           "SQLSMALLINT h = SQL_HANDLE_STMT;\n"
                                           "return 0;}\n",
                                           "", "")) {
      sqlServerInclude = "db_odbc.h";
      fprintf(logFile, "\rSQL Server: %s found in Seed7 include directory.\n",
              sqlServerInclude);
      includeOption[0] = '\0';
    } /* if */
    if (sqlServerInclude != NULL) {
      fprintf(versionFile, "#define SQL_SERVER_INCLUDE_WINDOWS_H %d\n", includeWindows);
      fprintf(versionFile, "#define SQL_SERVER_INCLUDE \"%s\"\n", sqlServerInclude);
      fprintf(versionFile, "#define SQL_SERVER_INCLUDE_SQLEXT_H %d\n", includeSqlext);
      fprintf(versionFile, "#define SQL_SERVER_INCLUDE_OPTION \"");
      escapeString(versionFile, includeOption);
      fprintf(versionFile, "\"\n");
      sprintf(makeDefinition, "SQL_SERVER_INCLUDE_OPTION = %s\n", includeOption);
      appendToFile("macros", makeDefinition);
      fprintf(versionFile, "#define FREETDS_SQL_SERVER_CONNECTION %d\n", freeTdsLibrary);
      fprintf(versionFile, "#define SPECIFY_SQL_SERVER_PORT_EXPLICIT %d\n", freeTdsLibrary);
      sprintf(testProgram, "#include \"tst_vers.h\"\n#include<stdio.h>\n"
                           "%s#include \"%s\"\n%s"
                           "int main(int argc,char *argv[]){\n"
                           "printf(\"%%d\\n\", sizeof(SQLWCHAR));\n"
                           "return 0;\n}\n",
                           includeWindows ? "#include \"windows.h\"\n" : "", sqlServerInclude,
                           includeSqlext ? "#include \"sqlext.h\"\n" : "");
      if (compileAndLinkWithOptionsOk(testProgram, includeOption, "")) {
        fprintf(versionFile, "#define SQL_SERVER_SIZEOF_SQLWCHAR %d\n", doTest());
      } /* if */
    } /* if */
#ifndef SQL_SERVER_USE_DLL
    if (supportsPartialLinking) {
      /* Handle static libraries: */
      sprintf(testProgram, "#include \"tst_vers.h\"\n#include<stdio.h>\n"
                           "%s#include \"%s\"\n%s"
                           "int main(int argc,char *argv[]){\n"
                           "SQLHENV sql_env;\n"
                           "SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sql_env);\n"
                           "SQLFreeHandle(SQL_HANDLE_ENV, sql_env);\n"
                           "printf(\"1\\n\");\n"
                           "return 0;\n}\n",
                           includeWindows ? "#include \"windows.h\"\n" : "", sqlServerInclude,
                           includeSqlext ? "#include \"sqlext.h\"\n" : "");
      /* fprintf(logFile, "%s\n", testProgram);
         fprintf(logFile, "sqlServerInclude: \"%s\"\n", sqlServerInclude); */
      sql_server_libs[0] = '\0';
      if (findLinkerOption("SQL Server", testProgram, includeOption, SQL_SERVER_LIBRARY_PATH,
                           libNameList, sizeof(libNameList) / sizeof(char *),
                           sql_server_libs)) {
        appendToMakeMacros("SQL_SERVER_LIBS", sql_server_libs);
        searchForLib = 0;
      } else if (buildTimeLinking) {
        if (findLinkerOptionForDll("SQL Server", testProgram, includeOption,
                                   SQL_SERVER_LIBRARY_PATH, dllNameList,
                                   sizeof(dllNameList) / sizeof(char *),
                                   sql_server_libs)) {
          appendToMakeMacros("SQL_SERVER_LIBS", sql_server_libs);
          searchForLib = 0;
        } /* if */
      } /* if */
    } /* if */
#endif
    if (searchForLib) {
      /* Handle dynamic libraries: */
      appendOption(system_database_libs, LINKER_OPT_DYN_LINK_LIBS);
      fprintf(versionFile, "#define SQL_SERVER_DLL");
      for (nameIndex = 0; nameIndex < sizeof(dllNameList) / sizeof(char *); nameIndex++) {
        fprintf(logFile, "\rSQL Server: DLL / Shared library: %s (%s)\n",
                dllNameList[nameIndex],
                canLoadDynamicLibrary(dllNameList[nameIndex]) ? "present" : "cannot load");
        fprintf(versionFile, " \"%s\",", dllNameList[nameIndex]);
      } /* for */
      fprintf(versionFile, "\n");
    } /* if */
  } /* determineSqlServerDefines */



static void determineTdsDefines (FILE *versionFile,
    char *include_options, char *system_database_libs)

  {
#ifdef TDS_LIBS
    const char *libNameList[] = { TDS_LIBS };
#elif LIBRARY_TYPE == UNIX_LIBRARIES || LIBRARY_TYPE == MACOS_LIBRARIES
    const char *libNameList[] = {"-lsybdb"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *libNameList[] = {"-lsybdb"};
#endif
#ifdef TDS_DLL
    const char *dllNameList[] = { TDS_DLL };
#elif LIBRARY_TYPE == UNIX_LIBRARIES
    const char *dllNameList[] = {"libsybdb.so", "libsybdb.so.5"};
#elif LIBRARY_TYPE == MACOS_LIBRARIES
    const char *dllNameList[] = {"libsybdb.dylib"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *dllNameList[] = {"sybdb.dll"};
#endif
    unsigned int nameIndex;
    int searchForLib = 1;
    char includeOption[PATH_SIZE];
    int includeSybfront = 0;
    const char *tdsInclude = NULL;
    char testProgram[BUFFER_SIZE];

  /* determineTdsDefines */
#ifdef TDS_INCLUDE_OPTIONS
    strcpy(includeOption, TDS_INCLUDE_OPTIONS);
#else
    includeOption[0] = '\0';
#endif
    if (compileAndLinkWithOptionsOk("#include <sybfront.h>\n"
                                    "#include <sybdb.h>\n"
                                    "int main(int argc,char *argv[]){\n"
                                    "DBINT aDbInt; DBPROCESS *dbproc;\n"
                                    "RETCODE rc = NO_MORE_ROWS;\n"
                                    "return 0;}\n",
                                    includeOption, "")) {
      includeSybfront = 1;
      tdsInclude = "sybdb.h";
      fprintf(logFile, "\rTDS: %s found in system include directory.\n",
              tdsInclude);
    } else if (compileAndLinkWithOptionsOk("#include \"tst_vers.h\"\n"
                                           "#include \"db_tds.h\"\n"
                                           "int main(int argc,char *argv[]){\n"
                                           "DBINT aDbInt; DBPROCESS *dbproc;\n"
                                           "RETCODE rc = NO_MORE_ROWS;\n"
                                           "return 0;}\n",
                                           "", "")) {
      tdsInclude = "db_tds.h";
      fprintf(logFile, "\rTDS: %s found in Seed7 include directory.\n",
              tdsInclude);
      includeOption[0] = '\0';
    } /* if */
    if (tdsInclude != NULL) {
      fprintf(versionFile, "#define TDS_INCLUDE \"%s\"\n", tdsInclude);
      fprintf(versionFile, "#define TDS_INCLUDE_SYBFRONT_H %d\n", includeSybfront);
    } /* if */
#ifndef TDS_USE_DLL
    /* Handle static libraries: */
    sprintf(testProgram, "#include \"tst_vers.h\"\n#include<stdio.h>\n"
                         "#include \"%s\"\n"
                         "int errHandler (DBPROCESS *dbproc, int severity, int dberr,\n"
                         "    int oserr, char *dberrstr, char *oserrstr) {\n"
                         "  if (dberr==SYBETIME) return INT_TIMEOUT; else return INT_CANCEL;\n"
                         "}\n"
                         "int msgHandler (DBPROCESS *dbproc, DBINT msgno, int msgstate,\n"
                         "    int severity, char *msgtext, char *srvname, char *procname, int line) {\n"
                         "  return 0;\n"
                         "}\n"
                         "int main(int argc,char *argv[]){\n"
                         "if (dbinit() != FAIL) {\n"
                         "  dberrhandle(errHandler);\n"
                         "  dbmsghandle(msgHandler);\n"
                         "  dblogin();\n"
                         "}\n"
                         "printf(\"1\\n\");\n"
                         "return 0;\n}\n", tdsInclude);
    /* fprintf(logFile, "%s\n", testProgram);
       fprintf(logFile, "tdsInclude: \"%s\"\n", tdsInclude); */
    if (findLinkerOption("TDS", testProgram, includeOption, TDS_LIBRARY_PATH,
                         libNameList, sizeof(libNameList) / sizeof(char *),
                         system_database_libs)) {
      searchForLib = 0;
    } else if (buildTimeLinking) {
      if (findLinkerOptionForDll("TDS", testProgram, includeOption,
                                 TDS_LIBRARY_PATH, dllNameList,
                                 sizeof(dllNameList) / sizeof(char *),
                                 system_database_libs)) {
        searchForLib = 0;
      } /* if */
    } /* if */
#endif
    if (searchForLib) {
      /* Handle dynamic libraries: */
      appendOption(system_database_libs, LINKER_OPT_DYN_LINK_LIBS);
      fprintf(versionFile, "#define TDS_DLL");
      for (nameIndex = 0; nameIndex < sizeof(dllNameList) / sizeof(char *); nameIndex++) {
        fprintf(logFile, "\rTDS: DLL / Shared library: %s (%s)\n",
                dllNameList[nameIndex],
                canLoadDynamicLibrary(dllNameList[nameIndex]) ? "present" : "cannot load");
        fprintf(versionFile, " \"%s\",", dllNameList[nameIndex]);
      } /* for */
      fprintf(versionFile, "\n");
    } /* if */
  } /* determineTdsDefines */



static void determineBigIntDefines (FILE *versionFile,
    char *include_options, char *system_bigint_libs)

  {
    const char *gmpLinkerOption;
    char linkerOptions[BUFFER_SIZE];

  /* determineBigIntDefines */
#if defined USE_GMP && USE_GMP == 1
#ifdef BIGINT_LIBS
    gmpLinkerOption = BIGINT_LIBS;
#else
    gmpLinkerOption = "-lgmp";
#endif
    linkerOptions[0] = '\0';
    appendOption(linkerOptions, gmpLinkerOption);
    if (compileAndLinkWithOptionsOk("#include<stdio.h>\n#include<stdlib.h>\n#include<gmp.h>\n"
                                    "int main(int argc,char *argv[]){\n"
                                    "mpz_ptr aBigInteger;\n"
                                    "aBigInteger=(mpz_ptr) malloc(sizeof(__mpz_struct));\n"
                                    "mpz_init_set_si(aBigInteger, 12345);\n"
                                    "mpz_add_ui(aBigInteger, aBigInteger, 87655);\n"
                                    "printf(\"%d\\n\", 100000 == mpz_get_si(aBigInteger));\n"
                                    "return 0;}\n",
                                    "", linkerOptions) &&
        doTest() == 1) {
      fputs("#define BIG_GMP_LIBRARY 2\n", versionFile);
      fputs("#define BIGINT_LIBRARY BIG_GMP_LIBRARY\n", versionFile);
      appendOption(system_bigint_libs, gmpLinkerOption);
    } else {
      fputs("#define BIG_RTL_LIBRARY 1\n", versionFile);
      fputs("#define BIGINT_LIBRARY BIG_RTL_LIBRARY\n", versionFile);
    } /* if */
#else
    fputs("#define BIG_RTL_LIBRARY 1\n", versionFile);
    fputs("#define BIGINT_LIBRARY BIG_RTL_LIBRARY\n", versionFile);
#endif
  } /* determineBigIntDefines */



static void determineIncludesAndLibs (FILE *versionFile)

  {
    char include_options[BUFFER_SIZE];
    char system_bigint_libs[BUFFER_SIZE];
    char system_console_libs[BUFFER_SIZE];
    char system_database_libs[BUFFER_SIZE];
    char system_draw_libs[BUFFER_SIZE];
    char rpath_buffer[BUFFER_SIZE];
    char *rpath = NULL;
    char rpathOption[2 * BUFFER_SIZE];

  /* determineIncludesAndLibs */
#if LIBRARY_TYPE == UNIX_LIBRARIES
    fprintf(logFile, "\rUsing Linux/Unix/BSD libraries\n");
#elif LIBRARY_TYPE == MACOS_LIBRARIES
    fprintf(logFile, "\rUsing Mac OS libraries\n");
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    fprintf(logFile, "\rUsing Windows libraries\n");
#endif
    include_options[0] = '\0';
    system_bigint_libs[0] = '\0';
    system_console_libs[0] = '\0';
    system_database_libs[0] = '\0';
    system_draw_libs[0] = '\0';
    if (linkerOptionAllowed("-Wl,--disable-new-dtags")) {
      rpath_buffer[0] = '\0';
      rpath = rpath_buffer;
    } /* if */
#ifdef SYSTEM_CONSOLE_LIBS
    replaceUnquotedSpacesWithNl(system_console_libs, SYSTEM_CONSOLE_LIBS);
#endif
#ifdef SYSTEM_DRAW_LIBS
    replaceUnquotedSpacesWithNl(system_draw_libs, SYSTEM_DRAW_LIBS);
#endif
#if defined MOUNT_NODEFS
    fputs("#define PIXEL_ALPHA_MASK \"ff000000\"\n", versionFile);
    fputs("#define PIXEL_RED_MASK \"ff0000\"\n", versionFile);
    fputs("#define PIXEL_GREEN_MASK \"ff00\"\n", versionFile);
    fputs("#define PIXEL_BLUE_MASK \"ff\"\n", versionFile);
    fputs("#define createARGB(alpha, red, green, blue) "
              "(ARGB) ((((DWORD) alpha) << 24) | "
                      "(((DWORD) red) << 16) | "
                      "(((DWORD) green) << 8) | "
                       "((DWORD) blue))\n", versionFile);
    fputs("#define POINT_LIST_INT_SIZE 32\n", versionFile);
    fputs("#define POINT_LIST_ABSOLUTE 1\n", versionFile);
    fputs("#define POINT_LIST_ALIGNMENT 2\n", versionFile);
#elif LIBRARY_TYPE == UNIX_LIBRARIES || LIBRARY_TYPE == MACOS_LIBRARIES
    determineConsoleDefines(versionFile, include_options, system_console_libs);
#ifdef USE_GLX
    fputs("#define PIXEL_RED_MASK \"ff\"\n", versionFile);
    fputs("#define PIXEL_GREEN_MASK \"ff00\"\n", versionFile);
    fputs("#define PIXEL_BLUE_MASK \"ff0000\"\n", versionFile);
    fputs("#define POINT_LIST_INT_SIZE 16\n", versionFile);
    fputs("#define POINT_LIST_ABSOLUTE 1\n", versionFile);
#else
    determineX11Defines(versionFile, include_options, system_draw_libs);
    fputs("#define POINT_LIST_INT_SIZE 16\n", versionFile);
    fputs("#define POINT_LIST_ABSOLUTE 0\n", versionFile);
#endif
#elif defined OS_STRI_WCHAR
    /* fputs("#define PIXEL_ALPHA_MASK \"ff000000\"\n", versionFile); */
    fputs("#define PIXEL_RED_MASK \"ff\"\n", versionFile);
    fputs("#define PIXEL_GREEN_MASK \"ff00\"\n", versionFile);
    fputs("#define PIXEL_BLUE_MASK \"ff0000\"\n", versionFile);
    fputs("#define createARGB(alpha, red, green, blue) "
              "(ARGB) ((((DWORD) alpha) << 24) | "
                      "(((DWORD) red)) | "
                      "(((DWORD) green) << 8) | "
                       "((DWORD) blue) << 16)\n", versionFile);
    fputs("#define POINT_LIST_INT_SIZE 32\n", versionFile);
    fputs("#define POINT_LIST_ABSOLUTE 1\n", versionFile);
#endif
    determineMySqlDefines(versionFile, include_options, system_database_libs);
    determineSqliteDefines(versionFile, include_options, system_database_libs);
    determinePostgresDefines(versionFile, include_options, system_database_libs);
    determineOdbcDefines(versionFile, include_options, system_database_libs);
    determineOciDefines(versionFile, include_options, system_database_libs, rpath);
    determineFireDefines(versionFile, include_options, system_database_libs);
    determineDb2Defines(versionFile, include_options, system_database_libs);
    determineInformixDefines(versionFile, include_options, system_database_libs, rpath);
    determineSqlServerDefines(versionFile, include_options, system_database_libs);
    determineTdsDefines(versionFile, include_options, system_database_libs);
    determineBigIntDefines(versionFile, include_options, system_bigint_libs);
    if (rpath != NULL && rpath[0] != '\0') {
      sprintf(rpathOption, "-Wl,--disable-new-dtags,-rpath=%s", rpath);
      appendOption(system_database_libs, rpathOption);
    } /* if */
    appendToMakeMacros("INCLUDE_OPTIONS", include_options);
    fprintf(versionFile, "#define INCLUDE_OPTIONS \"");
    escapeString(versionFile, include_options);
    fprintf(versionFile, "\"\n");
    appendToMakeMacros("SYSTEM_BIGINT_LIBS", system_bigint_libs);
    defineCMacro(versionFile, "SYSTEM_BIGINT_LIBS", system_bigint_libs);
#if LIBRARY_TYPE == UNIX_LIBRARIES || LIBRARY_TYPE == MACOS_LIBRARIES
    appendToMakeMacros("SYSTEM_CONSOLE_LIBS", system_console_libs);
#endif
    defineCMacro(versionFile, "SYSTEM_CONSOLE_LIBS", system_console_libs);
    appendToMakeMacros("SYSTEM_DATABASE_LIBS", system_database_libs);
    defineCMacro(versionFile, "SYSTEM_DATABASE_LIBS", system_database_libs);
#if LIBRARY_TYPE == UNIX_LIBRARIES || LIBRARY_TYPE == MACOS_LIBRARIES
    appendToMakeMacros("SYSTEM_DRAW_LIBS", system_draw_libs);
#endif
    defineCMacro(versionFile, "SYSTEM_DRAW_LIBS", system_draw_libs);
  } /* determineIncludesAndLibs */



static void writeReadBufferEmptyMacro (FILE *versionFile)

  {
    const char *define_read_buffer_empty;
    int offset_to_count;
    char macro_buffer[BUFFER_SIZE];
    char buffer[2 * BUFFER_SIZE];

  /* writeReadBufferEmptyMacro */
    if (compileAndLinkOk("#include<stdio.h>\nint main(int argc,char *argv[])\n"
                         "{FILE*fp;fp->_IO_read_ptr>=fp->_IO_read_end;return 0;}\n")) {
      define_read_buffer_empty = "#define read_buffer_empty(fp) "
                                 "((fp)->_IO_read_ptr >= (fp)->_IO_read_end)";
    } else if (compileAndLinkOk("#include<stdio.h>\nint main(int argc,char *argv[])\n"
                                "{FILE*fp;fp->_cnt <= 0;return 0;}\n")) {
      define_read_buffer_empty = "#define read_buffer_empty(fp) ((fp)->_cnt <= 0)";
    } else if (compileAndLinkOk("#include<stdio.h>\nint main(int argc,char *argv[])\n"
                                "{FILE*fp;fp->__cnt <= 0;return 0;}\n")) {
      define_read_buffer_empty = "#define read_buffer_empty(fp) ((fp)->__cnt <= 0)";
    } else if (compileAndLinkOk("#include<stdio.h>\nint main(int argc,char *argv[])\n"
                                "{FILE*fp;fp->level <= 0;return 0;}\n")) {
      define_read_buffer_empty = "#define read_buffer_empty(fp) ((fp)->level <= 0)";
    } else if (compileAndLinkOk("#include<stdio.h>\nint main(int argc,char *argv[])\n"
                                "{FILE*fp;fp->_r <= 0;return 0;}\n")) {
      define_read_buffer_empty = "#define read_buffer_empty(fp) ((fp)->_r <= 0)";
    } else if (compileAndLinkOk("#include<stdio.h>\nint main(int argc,char *argv[])\n"
                                "{FILE*fp;fp->ptr >= fp->getend;return 0;}\n")) {
      define_read_buffer_empty = "#define read_buffer_empty(fp) ((fp)->ptr >= (fp)->getend)";
    } else if (compileAndLinkOk("#include<stdio.h>\nint main(int argc,char *argv[])\n"
                                "{FILE stru; FILE*fp=&stru;char**base;char**pointer;int*count;\n"
                                "_get_stream_buffer_pointers(fp,&base,&pointer,&count);\n"
                                "printf(\"%d\\n\",(int)((char *)count-(char *)fp)); return 0;}\n")) {
      if ((offset_to_count = doTest()) != -1) {
        sprintf(macro_buffer,
                "#define read_buffer_empty(fp) (*((int *)&((char *)(fp))[%d])==0)",
                offset_to_count);
        define_read_buffer_empty = macro_buffer;
      } else {
        define_read_buffer_empty = NULL;
      } /* if */
#if defined FILE_STRUCT_READ_PTR_OFFSET && defined FILE_STRUCT_READ_END_OFFSET
    } else {
      sprintf(macro_buffer,
              "#define read_buffer_empty(fp) "
              "(*((int **)&((char *)(fp))[%d]) >= *((int **)&((char *)(fp))[%d]))",
              FILE_STRUCT_READ_PTR_OFFSET, FILE_STRUCT_READ_END_OFFSET);
      define_read_buffer_empty = macro_buffer;
#else
    } else if (compileAndLinkOk("#include<stdio.h>\n"
                                "typedef struct {unsigned flags; unsigned char *rpos, *rend;} MY_FILE;\n"
                                "int main(int argc,char *argv[])\n"
                                "{FILE*fp;((MY_FILE*)(fp))->rpos>=((MY_FILE*)(fp))->rend;return 0;}\n")) {
      define_read_buffer_empty = "typedef struct {unsigned flags; unsigned char *rpos, *rend;} MY_FILE;\n"
                                 "#define read_buffer_empty(fp)"
                                 " (((MY_FILE*)(fp))->rpos>=((MY_FILE*)(fp))->rend)";
    } else {
      define_read_buffer_empty = NULL;
#endif
    } /* if */
    if (define_read_buffer_empty != NULL) {
      sprintf(buffer,
              "#include<stdio.h>\n"
              "%s\n"
              "int main(int argc,char *argv[]){\n"
              "FILE *aFile;\n"
              "aFile = fopen(\"ctstfile.txt\", \"w\");\n"
              "if (aFile != NULL) {\n"
              "  fputs(\"abcdefghijklmnopqrstuvwxyz\\n\", aFile);\n"
              "  fclose(aFile);\n"
              "}\n"
              "aFile = fopen(\"ctstfile.txt\", \"r\");\n"
              "if (aFile == NULL) {\n"
              "  puts(\"0\");\n"
              "} else if (!read_buffer_empty(aFile)) {\n"
              "  fclose(aFile);\n"
              "  puts(\"0\");\n"
              "} else {\n"
              "  getc(aFile);\n"
              "  printf(\"%%d\\n\", read_buffer_empty(aFile)?0:1);\n"
              "  fclose(aFile);\n"
              "}\n"
              "return 0;}\n", define_read_buffer_empty);
      if (!compileAndLinkOk(buffer) || doTest() != 1) {
        fprintf(logFile, "\n *** %s does not work.\n",
                define_read_buffer_empty);
        define_read_buffer_empty = NULL;
      } /* if */
      doRemove("ctstfile.txt");
    } else {
      fprintf(logFile, "\n *** Could not define macro read_buffer_empty.\n");
    } /* if */
    if (define_read_buffer_empty != NULL) {
      fprintf(versionFile, "%s\n", define_read_buffer_empty);
      fprintf(logFile, "\rMacro read_buffer_empty defined.\n");
    } /* if */
  } /* writeReadBufferEmptyMacro */



#ifdef OS_STRI_USES_CODE_PAGE
static int getCodePage (void)

  {
    int codePage = 0;

  /* getCodePage */
    if (compileAndLinkOk("#include<stdio.h>\n"
                         "#include<dos.h>\n"
                         "int main (int argc, char *argv[]) {\n"
                         "union REGS r;\n"
                         "int code_page;\n"
                         "r.h.ah = (unsigned char) 0x66;\n"
                         "r.h.al = (unsigned char) 0x01;\n"
                         "int86(0x21, &r, &r);\n"
                         "code_page = r.w.bx;\n"
                         "printf(\"%d\\n\", code_page);\n"
                         "return 0; }\n")) {
      codePage = doTest();
    } /* if */
  } /* getCodePage */
#endif


static void setS7LibDir (const char *const s7_lib_dir_arg,
    const char *const build_directory, char *s7_lib_dir)

  {
    size_t path_length;

  /* setS7LibDir */
    if (s7_lib_dir_arg != NULL) {
      strcpy(s7_lib_dir, s7_lib_dir_arg);
    } else {
      path_length = strlen(build_directory);
      if (path_length >= 4 && memcmp(&build_directory[path_length - 4], "/src", 4) == 0) {
        memcpy(s7_lib_dir, build_directory, path_length - 3);
        memcpy(&s7_lib_dir[path_length - 3], "bin", 3);
        s7_lib_dir[path_length] = '\0';
      } else {
        s7_lib_dir[0] = '\0';
      } /* if */
    } /* if */
  } /* setS7LibDir */



static void setSeed7Library (const char *const seed7_library_arg,
    const char *const build_directory, char *seed7_library)

  {
    size_t path_length;

  /* setSeed7Library */
    if (seed7_library_arg != NULL) {
      strcpy(seed7_library, seed7_library_arg);
    } else {
      path_length = strlen(build_directory);
      if (path_length >= 4 && memcmp(&build_directory[path_length - 4], "/src", 4) == 0) {
        memcpy(seed7_library, build_directory, path_length - 3);
        if (fileIsDir("../lib")) {
          memcpy(&seed7_library[path_length - 3], "lib", 3);
        } else {
          memcpy(&seed7_library[path_length - 3], "prg", 3);
        } /* if */
        seed7_library[path_length] = '\0';
      } else {
        seed7_library[0] = '\0';
      } /* if */
    } /* if */
  } /* setSeed7Library */



static void setPaths (FILE *versionFile, char *s7_lib_dir, char *seed7_library,
    char *cc_environment_ini)

  { /* setPaths */
    if (cc_environment_ini != NULL) {
      fprintf(versionFile, "#define CC_ENVIRONMENT_INI \"%s/%s\"\n",
              s7_lib_dir, cc_environment_ini);
    } /* if */
#if defined C_COMPILER_SCRIPT && !defined C_COMPILER
    fprintf(versionFile, "#define C_COMPILER \"%s/%s\"\n",
            s7_lib_dir, C_COMPILER_SCRIPT);
    fputs("#define CALL_C_COMPILER_FROM_SHELL 1\n", versionFile);
#endif
    fprintf(versionFile, "#define S7_LIB_DIR \"%s\"\n", s7_lib_dir);
    fprintf(versionFile, "#define SEED7_LIBRARY \"%s\"\n", seed7_library);
  } /* setPaths */



static FILE *openVersionFile (const char *versionFileName)

  {
    FILE *versionFile = NULL;

  /* openVersionFile */
    if (versionFileName != NULL) {
      versionFile = fopen(versionFileName, "a");
    } /* if */
    if (versionFile == NULL) {
      versionFile = stdout;
      if (logFile == NULL) {
        logFile = fopen("chkccomp.log", "w");
        if (logFile == NULL) {
          logFile = stdout;
        } /* if */
      } /* if */
    } /* if */
    return versionFile;
  } /* openVersionFile */



static void closeVersionFile (FILE *versionFile)

  { /* closeVersionFile */
    if (versionFile != NULL && versionFile != stdout) {
      fclose(versionFile);
    } /* if */
  } /* closeVersionFile */



/**
 *  Program to Check properties of C compiler and runtime.
 */
int main (int argc, char **argv)

  {
    char *versionFileName = NULL;
    FILE *versionFile = NULL;
    char **curr_arg;
    char *s7_lib_dir_arg = NULL;
    char *seed7_library_arg = NULL;
    char *cc_environment_ini_arg = NULL;
    char *link_time_arg = NULL;
    int codePage = 0;
    int driveLetters;

  /* main */
    if (!fileIsRegular("settings.h")) {
      fprintf(stdout, "\n *** fileIsRegular(\"%s\") fails.\n", "settings.h");
    } /* if */
    if (argc >= 2) {
      versionFileName = argv[1];
      for (curr_arg = &argv[2]; *curr_arg != NULL; curr_arg++) {
        if (memcmp(*curr_arg, "S7_LIB_DIR=", 11 * sizeof(char)) == 0 &&
            (*curr_arg)[11] != '\0') {
          s7_lib_dir_arg = &(*curr_arg)[11];
        } else if (memcmp(*curr_arg, "SEED7_LIBRARY=", 14 * sizeof(char)) == 0 &&
                   (*curr_arg)[14] != '\0') {
          seed7_library_arg = &(*curr_arg)[14];
        } else if (memcmp(*curr_arg, "CC_ENVIRONMENT_INI=", 19 * sizeof(char)) == 0 &&
                   (*curr_arg)[19] != '\0') {
          cc_environment_ini_arg = &(*curr_arg)[19];
        } else if (memcmp(*curr_arg, "LINK_TIME=", 10 * sizeof(char)) == 0 &&
                   (*curr_arg)[10] != '\0') {
          link_time_arg = &(*curr_arg)[10];
          if (strcmp(link_time_arg, "BUILD") == 0) {
            fprintf(stdout, "Link at build time.\n");
            buildTimeLinking = 1;
          } else if (strcmp(link_time_arg, "RUN") != 0) {
            fprintf(stdout, "\n *** LINK_TIME is neither BUILD nor RUN.\n");
          } /* if */
        } /* if */
      } /* for */
      if (fileIsRegular(versionFileName)) {
        fprintf(stdout, "Truncating existing \"%s\".\n", versionFileName);
      } /* if */
      copyFile("base.h", versionFileName);
      appendFile("settings.h", versionFileName);
      copyFile(versionFileName, "tst_vers.h");
    } else {
      copyFile("base.h", "tst_vers.h");
      appendFile("settings.h", "tst_vers.h");
    } /* if */
    versionFile = openVersionFile(versionFileName);
    if (logFile == NULL) {
      logFile = stdout;
    } /* if */
    initializeNullDevice();
    fprintf(logFile, "Chkccomp uses %s as null device.\n", nullDevice);
    fprintf(logFile, "Prepare compile command: ");
    fflush(logFile);
    prepareCompileCommand();
    determineCompilerVersion(versionFile);
    prepareDoSleep();
#ifdef CC_FLAGS64
    if (getSizeof("char *") == 8) {
      fprintf(versionFile, "#define CC_FLAGS \"%s\"\n", CC_FLAGS " " CC_FLAGS64);
    } else {
      fprintf(versionFile, "#define CC_FLAGS \"%s\"\n", CC_FLAGS);
    } /* if */
#endif
    if (assertCompAndLnk("#include <stdio.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "#if defined(__STDC__)\n"
                         "#if defined(__STDC_VERSION__)\n"
                         "  printf(\"%ld\\n\", __STDC_VERSION__);\n"
                         "#else\n"
                         "  puts(\"1989\");\n"
                         "#endif\n"
                         "#else\n"
                         "  puts(\"1978\");\n"
                         "#endif\n"
                         "return 0;}\n")) {
      fprintf(versionFile, "#define C_VERSION %d\n", doTest());
    } /* if */
#ifdef OS_STRI_USES_CODE_PAGE
    codePage = getCodePage();
    fprintf(versionFile, "#define DEFAULT_CODE_PAGE %d\n", codePage);
#endif
    determineCurrentWorkingDirectory(buildDirectory, sizeof(buildDirectory));
#ifdef OS_STRI_USES_CODE_PAGE
    mapToUtf8(codePage, buildDirectory);
#endif
    fprintf(versionFile, "#define BUILD_DIRECTORY \"%s\"\n", buildDirectory);
    setS7LibDir(s7_lib_dir_arg, buildDirectory, s7LibDir);
    setSeed7Library(seed7_library_arg, buildDirectory, seed7Library);
    fprintf(logFile, " done\n");
    determineOptionForLinkTimeOptimization(versionFile);
    determinePartialLinking(versionFile);
    numericSizes(versionFile);
    fprintf(logFile, "General settings: ");
    determineNullDevice(versionFile);
    determineCallingConventions(versionFile);
    fprintf(versionFile, "#define LINE_DIRECTIVE_ACCEPTS_UTF8 %d\n",
            compileAndLinkOk("#include <stdio.h>\n#include <string.h>\n"
                             "int main(int argc, char *argv[]) {\n"
                             "#line 1 \"\303\244\303\266\303\274.c\"\n"
                             "printf(\"%d\\n\", strcmp(__FILE__,\n"
                             "    \"\\303\\244\\303\\266\\303\\274.c\") == 0);\n"
                             "return 0; }\n") && doTest() == 1);
    fprintf(versionFile, "#define UNISTD_H_PRESENT %d\n",
            compileAndLinkOk("#include <unistd.h>\n"
                             "int main(int argc,char *argv[]){return 0;}\n"));
    fprintf(versionFile, "#define STDINT_H_PRESENT %d\n",
            compileAndLinkOk("#include <stdint.h>\n"
                             "int main(int argc,char *argv[]){return 0;}\n"));
    fprintf(versionFile, "#define ACLAPI_H_PRESENT %d\n",
            compileAndLinkOk("#include <aclapi.h>\n"
                             "int main(int argc,char *argv[]){return 0;}\n"));
    fprintf(versionFile, "#define WINDOWSX_H_PRESENT %d\n",
            compileAndLinkOk("#include <stdio.h>\n#include <windows.h>\n"
                             "#include <windowsx.h>\n"
                             "int main(int argc,char *argv[])\n"
                             "{printf(\"%d\\n\", GET_X_LPARAM(12345) == 12345);\n"
                             "return 0;}\n") && doTest() == 1);
    checkSignal(versionFile);
    writeMacroDefs(versionFile);
    closeVersionFile(versionFile);
    copyFile(versionFileName, "tst_vers.h");
    versionFile = openVersionFile(versionFileName);
    makeDirDefinition = defineMakeDir();
    removeDirDefinition = defineRemoveDir();
    determineOsFunctions(versionFile);
    checkPopen(versionFile);
    checkSystemResult(versionFile);
    if (assertCompAndLnk("#include <stdio.h>\nint main(int argc, char *argv[])\n"
                         "{printf(\"%d\\n\", fseek(stdin, 0,  SEEK_SET) == 0);\n"
                         "return 0;}\n")) {
      fprintf(versionFile, "#define FSEEK_SUCCEEDS_FOR_STDIN %d\n", doTest() == 1);
    } /* if */
    if (assertCompAndLnk("#include <stdio.h>\nint main(int argc, char *argv[])\n"
                         "{FILE *aFile; aFile=fopen(\".\",\"r\");\n"
                         "printf(\"%d\\n\",aFile!=NULL);\n"
                         "if(aFile!=NULL)fclose(aFile);return 0;}\n")) {
      fprintf(versionFile, "#define FOPEN_OPENS_DIRECTORIES %d\n", doTest() == 1);
    } /* if */
    if (assertCompAndLnk("#include <stdio.h>\nint main(int argc, char *argv[])\n"
                         "{int canWrite=0;FILE *aFile;\n"
                         "if((aFile=fopen(\"tmp_test_file\",\"w\"))!=NULL){\n"
                         " fwrite(\"asdf\",1,4,aFile);fclose(aFile);\n"
                         " if((aFile=fopen(\"tmp_test_file\",\"r\"))!=NULL){\n"
                         "  canWrite=fwrite(\"qwert\",1,5,aFile)!=0;fclose(aFile);}\n"
                         " remove(\"tmp_test_file\");}\n"
                         "printf(\"%d\\n\",canWrite);return 0;}\n")) {
      fprintf(versionFile, "#define FWRITE_WRONG_FOR_READ_ONLY_FILES %d\n", doTest() == 1);
    } /* if */
    if (assertCompAndLnk("#include <stdio.h>\nint main(int argc, char *argv[])\n"
                         "{int canRead=0;FILE *aFile;char buffer[5];\n"
                         "if((aFile=fopen(\"tmp_test_file\",\"w\"))!=NULL){\n"
                         " canRead=fread(buffer,1,4,aFile)!=0||!ferror(aFile);fclose(aFile);\n"
                         " remove(\"tmp_test_file\");}\n"
                         "printf(\"%d\\n\",canRead);return 0;}\n")) {
      fprintf(versionFile, "#define FREAD_WRONG_FOR_WRITE_ONLY_FILES %d\n", doTest() == 1);
    } /* if */
    if (assertCompAndLnk("#include <stdio.h>\n#include <string.h>\n"
                         "int main(int argc,char *argv[]){\n"
                         "int closeFails=0;FILE *aFile;char buffer[5];\n"
                         "if((aFile=fopen(\"tmp_test_file\",\"w\"))!=NULL) {\n"
                         " fread(buffer,1,4,aFile);if (fclose(aFile)!=0) {closeFails = 1;}\n"
                         " remove(\"tmp_test_file\");}\n"
                         "if((aFile=fopen(\"tmp_test_file\",\"w\"))!=NULL) {\n"
                         " memcpy(buffer, \"abcd\", 5);\n"
                         " fwrite(buffer,1,4,aFile);\n"
                         " fclose(aFile);}\n"
                         "if((aFile=fopen(\"tmp_test_file\",\"r\"))!=NULL) {\n"
                         " memcpy(buffer, \"abcd\", 5);\n"
                         " fwrite(buffer,1,4,aFile);if (fclose(aFile)!=0) {closeFails = 1;}\n"
                         " remove(\"tmp_test_file\");}\n"
                         "printf(\"%d\\n\", closeFails);return 0;}\n")) {
      fprintf(versionFile, "#define FCLOSE_FAILS_AFTER_PREVIOUS_ERROR %d\n", doTest() == 1);
    } /* if */
    checkRemoveDir(makeDirDefinition, versionFile);
    if (compileAndLinkOk("#include <stdio.h>\n#include <unistd.h>\n#include <ctype.h>\n"
                         "int main(int argc, char *argv[])\n"
                         "{char buffer[8192]; char *cwd;\n"
                         "cwd = getcwd(buffer, 8192);\n"
                         "printf(\"%d\\n\", cwd!=NULL && isalpha(cwd[0]) && cwd[1]==':');\n"
                         "return 0;}\n") ||
        compileAndLinkOk("#include <stdio.h>\n#include <unistd.h>\n#include <ctype.h>\n"
                         "int main(int argc, char *argv[])\n"
                         "{char buffer[8192]; char *cwd;\n"
                         "cwd = _getcwd(buffer, 8192);\n"
                         "printf(\"%d\\n\", cwd!=NULL && isalpha(cwd[0]) && cwd[1]==':');\n"
                         "return 0;}\n") ||
        compileAndLinkOk("#include <stdio.h>\n#include <direct.h>\n#include <ctype.h>\n"
                         "int main(int argc, char *argv[])\n"
                         "{char buffer[8192]; char *cwd;\n"
                         "cwd = getcwd(buffer, 8192);\n"
                         "printf(\"%d\\n\", cwd!=NULL && isalpha(cwd[0]) && cwd[1]==':');\n"
                         "return 0;}\n") ||
        compileAndLinkOk("#include <stdio.h>\n#include <direct.h>\n#include <ctype.h>\n"
                         "int main(int argc, char *argv[])\n"
                         "{char buffer[8192]; char *cwd;\n"
                         "cwd = _getcwd(buffer, 8192);\n"
                         "printf(\"%d\\n\", cwd!=NULL && isalpha(cwd[0]) && cwd[1]==':');\n"
                         "return 0;}\n")) {
      driveLetters = doTest() == 1;
      fprintf(versionFile, "#define OS_PATH_HAS_DRIVE_LETTERS %d\n", driveLetters);
      if (driveLetters) {
        /* The check for HOME and USERPROFILE is done with a program,         */
        /* because some compilers (e.g.: emcc) provide their own environment. */
        if (assertCompAndLnk("#include <stdio.h>\n#include <stdlib.h>\n"
                             "int main(int argc, char *argv[])\n"
                             "{printf(\"%d\\n\", getenv(\"USERPROFILE\") != NULL);\n"
                             "return 0;}\n") && doTest() == 1) {
          /* If USERPROFILE is defined then it is used, even if HOME is defined. */
          fputs("#define HOME_DIR_ENV_VAR {'U', 'S', 'E', 'R', "
                "'P', 'R', 'O', 'F', 'I', 'L', 'E', 0}\n", versionFile);
        } else if (assertCompAndLnk("#include <stdio.h>\n#include <stdlib.h>\n"
                                    "int main(int argc, char *argv[])\n"
                                    "{printf(\"%d\\n\", getenv(\"HOME\") != NULL);\n"
                                    "return 0;}\n") && doTest() == 1) {
          fputs("#define HOME_DIR_ENV_VAR {'H', 'O', 'M', 'E', 0}\n", versionFile);
        } else {
          fputs("#define HOME_DIR_ENV_VAR {'H', 'O', 'M', 'E', 0}\n", versionFile);
          fputs("#define DEFAULT_HOME_DIR {'C', ':', '\\\\', 0}\n", versionFile);
        } /* if */
      } else {
        fputs("#define HOME_DIR_ENV_VAR {'H', 'O', 'M', 'E', 0}\n", versionFile);
      } /* if */
    } else {
      fputs("#define HOME_DIR_ENV_VAR {'H', 'O', 'M', 'E', 0}\n", versionFile);
    } /* if */
#if defined OS_STRI_WCHAR && defined _WIN32
    /* Under Windows a rename between different    */
    /* devices fails with EACCES instead of EXDEV. */
    fputs("#define USE_EACCES_INSTEAD_OF_EXDEV\n", versionFile);
    /* Windows uses pending deletes which cause    */
    /* problems if a file with the same name is    */
    /* created shortly after the delete. To avoid  */
    /* problems files are renamed before they are  */
    /* removed.                                    */
    fputs("#define RENAME_BEFORE_REMOVE\n", versionFile);
#endif
    fprintf(logFile, " determined\n");
    numericProperties(versionFile);
    fprintf(logFile, "Advanced settings: ");
    fflush(logFile);
    determineMallocProperties(versionFile);
    if (compileAndLinkOk("#include<signal.h>\nint main(int argc, char *argv[]){\n"
                         "signal(SIGBUS,SIG_DFL); return 0;}\n")) {
      if (assertCompAndLnk("#include<stdlib.h>\n#include <stdio.h>\n#include<signal.h>\n"
                           "void handleSig(int sig){puts(\"2\");exit(0);}\n"
                           "int main(int argc, char *argv[])\n"
                           "{int p[3]={12,34,56}, q, *pp;\n"
                           "signal(SIGBUS,handleSig);\n"
                           "pp=(int *)((char *)&p[1]+1); q=*pp;\n"
                           "printf(\"1\\n\"); return 0;}\n")) {
        fprintf(versionFile, "#define UNALIGNED_MEMORY_ACCESS_OKAY %d\n", doTest() == 1);
      } /* if */
    } else if (assertCompAndLnk("#include <stdio.h>\n"
                                "int main(int argc, char *argv[])\n"
                                "{int p[3]={12,34,56}, q, *pp;\n"
                                "pp=(int *)((char *)&p[1]+1); q=*pp;\n"
                                "printf(\"1\\n\"); return 0;}\n")) {
      fprintf(versionFile, "#define UNALIGNED_MEMORY_ACCESS_OKAY %d\n", doTest() == 1);
    } /* if */
    if (assertCompAndLnk("#include <stdio.h>\n#include <string.h>\n"
                         "int main(int argc, char *argv[])\n"
                         "{union{char ch;unsigned long gen;}aUnion;\n"
                         "memset(&aUnion,0,sizeof(aUnion));aUnion.ch='X';\n"
                         "printf(\"%d\\n\",aUnion.ch==(char)aUnion.gen);return 0;}\n")) {
      fprintf(versionFile, "#define CASTING_GETS_A_UNION_ELEMENT %d\n", doTest() == 1);
    } /* if */
    if (assertCompAndLnk("#include <stdio.h>\n#include <string.h>\n"
                         "int main(int argc,char *argv[]){\n"
                         "char buffer[5];\n"
                         "memcpy(buffer, \"abcd\", 5);\n"
                         "memcpy(&buffer[1], \"xy\", 0);\n"
                         "printf(\"%d\\n\", memcmp(buffer, \"abcd\", 5) == 0);\n"
                         "return 0;}\n")) {
      fprintf(versionFile, "#define MEMCPY_ZERO_BYTES_DOES_NOTHING %d\n", doTest() == 1);
    } /* if */
    if (assertCompAndLnk("#include <stdio.h>\nint main(int argc, char *argv[])\n"
                         "{printf(\"%d\\n\", EOF == -1); return 0;}\n")) {
      fprintf(versionFile, "#define EOF_IS_MINUS_ONE %d\n", doTest());
    } /* if */
    fprintf(versionFile, "#define EMPTY_STRUCTS_ALLOWED %d\n",
            compileAndLinkOk("#include <stdio.h>\n"
                             "typedef struct emptyStruct { } emptyRecord;\n"
                             "int main(int argc, char *argv[]){\n"
                             "return 0;}\n"));
    fprintf(versionFile, "#define INITIALIZING_WITH_ADDRESS_ALLOWED %d\n",
            compileAndLinkOk("#include <stdio.h>\n"
                             "typedef struct testStruct { int number; } *testType;\n"
                             "struct testStruct aRecord;\n"
                             "int main(int argc, char *argv[])\n"
                             "{ testType aVariable = &aRecord;\n"
                             "int counter;\n"
                             "printf(\"0\\n\"); return 0;}\n"));
    checkForLimitedStringLiteralLength(versionFile);
    checkForLimitedArrayLiteralLength(versionFile);
    checkForSwitchWithInt64Type(versionFile);
    determineStackDirection(versionFile);
    determineLanguageProperties(versionFile);
    determinePreprocessorProperties(versionFile);
#ifndef DEFAULT_STACK_SIZE
    if (sizeof(char *) == 8) { /* Machine with 64-bit addresses */
      /* Due to alignment some 64-bit machines have huge stack requirements. */
      fputs("#define DEFAULT_STACK_SIZE 16777216\n", versionFile);
    } else {
      fputs("#define DEFAULT_STACK_SIZE 8388608\n", versionFile);
    } /* if */
#endif
    localtimeProperties(versionFile);
    /* Make sure that the file version.h up to this position is copied to tst_vers.h. */
    closeVersionFile(versionFile);
    copyFile(versionFileName, "tst_vers.h");
    versionFile = openVersionFile(versionFileName);
    determineGetaddrlimit(versionFile);
    fprintf(versionFile, "#define STRCMP_RETURNS_SIGNUM %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <string.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "char stri1[3], stri2[3];\n"
                         "strcpy(stri1, \"za\");\n"
                         "strcpy(stri2, \"az\");\n"
                         "printf(\"%d\\n\",\n"
                         "       strcmp(stri1, stri2, 2) == 1 &&\n"
                         "       strcmp(stri2, stri1, 2) == -1);\n"
                         "return 0;}\n") && doTest() == 1);
    fprintf(versionFile, "#define MEMCMP_RETURNS_SIGNUM %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <string.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "char stri1[3], stri2[3];\n"
                         "strcpy(stri1, \"za\");\n"
                         "strcpy(stri2, \"az\");\n"
                         "printf(\"%d\\n\",\n"
                         "       memcmp(stri1, stri2, 2) == 1 &&\n"
                         "       memcmp(stri2, stri1, 2) == -1);\n"
                         "return 0;}\n") && doTest() == 1);
    fprintf(versionFile, "#define HAS_WMEMCMP %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <wchar.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "wchar_t str1[] = {0x0201, 0x0102, 0};\n"
                         "wchar_t str2[] = {0x0102, 0x0201, 0};\n"
                         "printf(\"%d\\n\", wmemcmp(str1, str2, 2) > 0);\n"
                         "return 0;}\n") && doTest() == 1);
    fprintf(versionFile, "#define WMEMCMP_RETURNS_SIGNUM %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <string.h>\n"
                         "#include <wchar.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "wchar_t str1[] = {0x0201, 0x0102, 0};\n"
                         "wchar_t str2[] = {0x0102, 0x0201, 0};\n"
                         "printf(\"%d\\n\",\n"
                         "       wmemcmp(str1, str2, 2) == 1 &&\n"
                         "       wmemcmp(str2, str1, 2) == -1);\n"
                         "return 0;}\n") && doTest() == 1);
    fprintf(versionFile, "#define MEMSET_OF_ZERO_BYTES_DOES_NOTHING %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <string.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "char stri1[17];\n"
                         "int zero;\n"
                         "strcpy(stri1, \"abcdefghijklmnop\");\n"
                         "zero = strlen(stri1) - 16;\n"
                         "memset(&stri1[8], 0, zero);\n"
                         "printf(\"%d\\n\",\n"
                         "       memcmp(stri1, \"abcdefghijklmnop\", 17) == 0);\n"
                         "return 0;}\n") && doTest() == 1);
    fprintf(versionFile, "#define HAS_WMEMSET %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <wchar.h>\n#include <string.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "wchar_t str1[4];\n"
                         "wchar_t str2[] = {0, 0, 0, 0};\n"
                         "wchar_t ch1 = 0x00;\n"
                         "wmemset(str1, ch1, 4);\n"
                         "printf(\"%d\\n\", memcmp(str1, str2, 4 * sizeof(wchar_t)) == 0);\n"
                         "return 0;}\n") && doTest() == 1);
    fprintf(versionFile, "#define HAS_WMEMCHR %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <wchar.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "wchar_t str1[] = {0x0201, 0x0102, 0};\n"
                         "wchar_t ch1 = 0x0102;\n"
                         "printf(\"%d\\n\", wmemchr(str1, ch1, 2) ==  &str1[1]);\n"
                         "return 0;}\n") && doTest() == 1);
    fprintf(versionFile, "#define HAS_WCSNLEN %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <wchar.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "wchar_t str1[] = {0x0201, 0x0102, 0x0303, 0, 0x0301, 0};\n"
                         "printf(\"%d\\n\", wcsnlen(str1, 5) == 3 &&\n"
                         "                  wcsnlen(str1, 4) == 3 &&\n"
                         "                  wcsnlen(str1, 3) == 3 &&\n"
                         "                  wcsnlen(str1, 2) == 2);\n"
                         "return 0;}\n") && doTest() == 1);
    fprintf(versionFile, "#define HAS_STRNCASECMP %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <strings.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "printf(\"%d\\n\", strncasecmp(\"abc\", \"abcd\", 3) == 0);\n"
                         "return 0;}\n") && doTest() == 1);
    determineGrpAndPwFunctions(versionFile);
    fprintf(versionFile, "#define HAS_SETJMP %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <setjmp.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "jmp_buf env; int ret_code = 4; int count = 2;\n"
                         "if ((ret_code=setjmp(env)) == 0) {\n"
                         "  count--;\n"
                         "  if (count != 0) {\n"
                         "    longjmp(env, count); printf(\"3\\n\");\n"
                         "  } else printf(\"5\\n\");\n"
                         "} else printf(\"%d\\n\", ret_code);\n"
                         "return 0;}\n") && doTest() == 1);
    fprintf(versionFile, "#define HAS_SIGSETJMP %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <setjmp.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "sigjmp_buf env; int ret_code = 4; int count = 2;\n"
                         "if ((ret_code=sigsetjmp(env, 1)) == 0) {\n"
                         "  count--;\n"
                         "  if (count != 0) {\n"
                         "    siglongjmp(env, count); printf(\"3\\n\");\n"
                         "  } else printf(\"5\\n\");\n"
                         "} else printf(\"%d\\n\", ret_code);\n"
                         "return 0;}\n") && doTest() == 1);
    fprintf(versionFile, "#define HAS_SYMBOLIC_LINKS %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <unistd.h>\n#include <string.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "char buf[256]; ssize_t link_len; int okay=0;\n"
                         "if (symlink(\"qwertzuiop\",\"test_symlink\") == 0){;\n"
                         "link_len=readlink(\"test_symlink\", buf, 256);\n"
                         "okay = link_len == 10 && memcmp(buf,\"qwertzuiop\",10) == 0;\n"
                         "remove(\"test_symlink\");}\n"
                         "printf(\"%d\\n\", okay);\n"
                         "return 0;}\n") && doTest() == 1);
    fprintf(versionFile, "#define HAS_SYMLINK %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <unistd.h>\n#include <string.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "char buf[256]; ssize_t link_len; int okay=0;\n"
                         "if (symlink(\"qwertzuiop\",\"test_symlink\") == 0){;\n"
                         "okay=1;\n"
                         "remove(\"test_symlink\");}\n"
                         "printf(\"%d\\n\", okay);\n"
                         "return 0;}\n") && doTest() == 1);
    fprintf(versionFile, "#define HAS_READLINK %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <unistd.h>\n"
                         "#include <string.h>\n#include <errno.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "char buf[256]; ssize_t link_len;\n"
                         "link_len = readlink(\" does_not_exist \", buf, 256);\n"
                         "printf(\"%d\\n\", link_len == -1);\n"
                         "return 0;}\n") && doTest() == 1);
    fprintf(versionFile, "#define HAS_FIFO_FILES %d\n",
        compileAndLinkOk("#include <sys/types.h>\n#include <sys/stat.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "int ret_code;\n"
                         "ret_code=mkfifo(\"qwertzuiop\", 0);\n"
                         "return 0;}\n"));
    fprintf(versionFile, "#define HAS_SELECT %d\n",
        compileAndLinkOk("#include<sys/select.h>\n#include<stddef.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{fd_set readfds, writefds, exceptfds;\n"
                         "select(15, &readfds, &writefds, &exceptfds, NULL);\n"
                         "return 0;}\n") ||
        compileAndLinkWithOptionsOk("#include<winsock2.h>\n#include<stddef.h>\n"
                                    "int main(int argc,char *argv[])\n"
                                    "{fd_set readfds, writefds, exceptfds;\n"
                                    "select(15, &readfds, &writefds, &exceptfds, NULL);\n"
                                    "return 0;}\n", "", SYSTEM_LIBS));
    fprintf(versionFile, "#define HAS_POLL %d\n",
        compileAndLinkOk("#include<poll.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{struct pollfd pollFd[1];\n"
                         "poll(pollFd, 1, 0);\n"
                         "return 0;}\n"));
    fprintf(versionFile, "#define HAS_MMAP %d\n",
        compileAndLinkOk("#include<stddef.h>\n#include<sys/mman.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{mmap(NULL, 12345, PROT_READ, MAP_PRIVATE, 3, 0);\n"
                         "return 0;}\n"));
    fprintf(logFile, " determined\n");
    determineIncludesAndLibs(versionFile);
    writeReadBufferEmptyMacro(versionFile);
    cleanUpCompilation("ctest", testNumber);
    removeDoSleep();
    fprintf(versionFile, "#define REMOVE_REATTEMPTS %lu\n", removeReattempts);
    if (removeReattempts != 0) {
      fprintf(logFile, "%lu times removing a file needed a reattempt.\n", removeReattempts);
    } /* if */
    fprintf(versionFile, "#define FILE_PRESENT_AFTER_DELAY %lu\n", filePresentAfterDelay);
    if (filePresentAfterDelay != 0) {
      fprintf(logFile, "%lu times the test output was present after a delay.\n",
              filePresentAfterDelay);
    } /* if */
    fprintf(versionFile, "#define NUMBER_OF_SUCCESSFUL_TESTS_AFTER_RESTART %lu\n",
            numberOfSuccessfulTestsAfterRestart);
    if (numberOfSuccessfulTestsAfterRestart != 0) {
      fprintf(logFile, "%lu times tests succeeded after restart.\n",
              numberOfSuccessfulTestsAfterRestart);
    } /* if */
    setPaths(versionFile, s7LibDir, seed7Library, cc_environment_ini_arg);
    closeVersionFile(versionFile);
    if (fileIsRegular("tst_vers.h")) {
      remove("tst_vers.h");
    } /* if */
    fprintf(logFile, " \b");
    return 0;
  } /* main */

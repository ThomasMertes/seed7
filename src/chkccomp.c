/********************************************************************/
/*                                                                  */
/*  chkccomp.c    Check properties of C compiler and runtime.       */
/*  Copyright (C) 2010 - 2020  Thomas Mertes                        */
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
/*  Changes: 2010 - 2020  Thomas Mertes                             */
/*  Content: Program to Check properties of C compiler and runtime. */
/*                                                                  */
/********************************************************************/

#include "base.h"

/**
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
 *  The file chkccomp.h is NOT part of the Seed7 release.
 *  Instead chkccomp.h is generated by the makefile and
 *  removed after chkccomp was compiled and executed.
 *  In chkccomp.h the following macros might be defined:
 *
 *  TEST_C_COMPILER:
 *      If TEST_C_COMPILER is defined it is used instead of C_COMPILER
 *      as command of the stand-alone C compiler and linker.
 *  LIST_DIRECTORY_CONTENTS;
 *      Either "ls" or "dir".
 *      E.g.: #define LIST_DIRECTORY_CONTENTS "ls"
 *            #define LIST_DIRECTORY_CONTENTS "dir"
 *  LINKER_OPT_STATIC_LINKING: (optional)
 *      Contains the linker option to force static linking (e.g.: "-static").
 *  SUPPORTS_PARTIAL_LINKING: (optional)
 *      Defined if partial/incremental linking is prossible.
 *      In this case source code can be compiled with the options -r -c.
 *      The option -r produces a relocatable object as output. This is
 *      also known as partial linking. The tool objcopy is used also.
 *      Objcopy is used with the option -L symbolname which converts
 *      a global or weak symbol called symbolname into a local symbol.
 *      This way the symbol is not visible externally.
 *  ALLOW_REPLACEMENT_OF_SYSTEM_HEADERS: (optional)
 *      Defined if X11 or ncurses header files can be replaced by header
 *      files provided by Seed7.
 *  PRINTF_MAXIMUM_FLOAT_PRECISION: (optional)
 *      Precision up to which writing a float with printf (using format %e or
 *      %f) will always work ok.
 *  The macros described can be defined in a makefile and they are only used
 *  in chkccomp.c. This macros are not used in the Seed7 Interpreter (s7) or
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

#ifndef CC_OPT_TRAP_OVERFLOW
#define CC_OPT_TRAP_OVERFLOW ""
#endif

#ifndef CC_OPT_VERSION_INFO
#define CC_OPT_VERSION_INFO ""
#endif

#ifndef CC_FLAGS
#define CC_FLAGS ""
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

#define NAME_SIZE    1024
#define COMMAND_SIZE 1024
#define BUFFER_SIZE  4096

static int testNumber = 0;
static char c_compiler[COMMAND_SIZE];
static char *nullDevice = NULL;
static FILE *logFile = NULL;
static unsigned long removeReattempts = 0;

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



static void prepareCompileCommand (void)

  {
    int mapAbsolutePathToDriveLetters = 0;
    int pos;
    int quote_command = 0;
    int len;

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



static void replaceNLBySpace (char *text)

  { /* replaceNLBySpace */
    while (*text != '\0') {
      if (*text == '\n') {
        *text = ' ';
      } /* if */
      text++;
    } /* while */
  } /* replaceNLBySpace */



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
    sprintf(command, "%s %s %s cc_vers1.txt %s cc_vers2.txt",
            c_compiler, CC_OPT_VERSION_INFO,
            REDIRECT_FILEDES_1, REDIRECT_FILEDES_2);
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
        sprintf(command, "%s %s %s cc_vers.txt %s %s",
                c_compiler, CC_OPT_VERSION_INFO, REDIRECT_FILEDES_1,
                REDIRECT_FILEDES_2, nullDevice);
      } else {
        sprintf(command, "%s %s %s cc_vers.txt",
                c_compiler, CC_OPT_VERSION_INFO, REDIRECT_FILEDES_1);
      } /* if */
    } else if (cc_version_info_filedes == 2) {
      if (nullDevice != NULL) {
        sprintf(command, "%s %s %s cc_vers.txt %s %s",
                c_compiler, CC_OPT_VERSION_INFO, REDIRECT_FILEDES_2,
                REDIRECT_FILEDES_1, nullDevice);
      } else {
        sprintf(command, "%s %s %s cc_vers.txt",
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



static void cleanUpCompilation (int testNumber)

  {
    char fileName[NAME_SIZE];

  /* cleanUpCompilation */
    sprintf(fileName, "ctest%d.c", testNumber);
    doRemove(fileName);
    sprintf(fileName, "ctest%d.cerrs", testNumber);
    doRemove(fileName);
    sprintf(fileName, "ctest%d.lerrs", testNumber);
    doRemove(fileName);
    sprintf(fileName, "ctest%d%s", testNumber, OBJECT_FILE_EXTENSION);
    doRemove(fileName);
    sprintf(fileName, "ctest%d%s", testNumber, LINKED_PROGRAM_EXTENSION);
    doRemove(fileName);
    sprintf(fileName, "ctest%d.out", testNumber);
    doRemove(fileName);
  } /* cleanUpCompilation */



static int doCompile (const char *compilerOptions, int testNumber)

  {
    char command[COMMAND_SIZE];
    int len;
    char fileName[NAME_SIZE];
    int returncode;
    int okay = 0;

  /* doCompile */
    fprintf(logFile, "*");
    fflush(logFile);
    sprintf(command, "%s %s %s -c ctest%d.c",
            c_compiler, compilerOptions, CC_FLAGS, testNumber);
    replaceNLBySpace(command);
#ifdef CC_ERROR_FILEDES
    /* A missing CC_ERROR_FILEDES or an CC_ERROR_FILEDES of zero means: Do not redirect. */
    if (CC_ERROR_FILEDES == 1) {
      sprintf(&command[strlen(command)], " %sctest%d.cerrs %s%s",
              REDIRECT_FILEDES_1, testNumber, REDIRECT_FILEDES_2, nullDevice);
    } else if (CC_ERROR_FILEDES == 2) {
      sprintf(&command[strlen(command)], " %sctest%d.cerrs %s%s",
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
    sprintf(fileName, "ctest%d%s", testNumber, OBJECT_FILE_EXTENSION);
    if (fileIsRegular(fileName)) {
      if (returncode == 0) {
        okay = 1;
      } else {
        /* fprintf(logFile, "\n *** The compiler %s fails, but creates an executable.\n", c_compiler); */
      } /* if */
    } else {
      /* fprintf(logFile, "\n *** The compiler %s produces no executable: %s\n", c_compiler, fileName); */
    } /* if */
#ifdef DEBUG_CHKCCOMP
    fprintf(logFile, "command: %s\n", command);
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
    cleanUpCompilation(testNumber);
    testNumber++;
    cleanUpCompilation(testNumber);
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
    char command[COMMAND_SIZE];
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
    if (fileIsRegular(fileName)) {
      if (returncode == 0) {
        okay = 1;
      } else {
        /* fprintf(logFile, "\n *** The compiler %s fails, but creates an executable.\n", c_compiler); */
      } /* if */
    } else {
      /* fprintf(logFile, "\n *** The compiler %s produces no executable: %s\n", c_compiler, fileName); */
    } /* if */
#ifdef DEBUG_CHKCCOMP
    fprintf(logFile, "command: %s\n", command);
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



static int doCompileAndLink (const char *compilerOptions, const char *linkerOptions, int testNumber)

  {
    char command[COMMAND_SIZE];
    int len;
    char fileName[NAME_SIZE];
    int returncode;
    int okay = 0;

  /* doCompileAndLink */
    fprintf(logFile, "*");
    fflush(logFile);
#ifdef LINKER
    sprintf(command, "%s %s %s -c ctest%d.c",
            c_compiler, compilerOptions, CC_FLAGS, testNumber);
#else
    sprintf(command, "%s %s %s ctest%d.c %s",
            c_compiler, compilerOptions, CC_FLAGS, testNumber, linkerOptions);
#endif
    replaceNLBySpace(command);
#if !defined LINKER && defined LINKER_OPT_OUTPUT_FILE && !defined CC_NO_OPT_OUTPUT_FILE
    sprintf(&command[strlen(command)], " %sctest%d%s",
            LINKER_OPT_OUTPUT_FILE, testNumber, LINKED_PROGRAM_EXTENSION);
#endif
#ifdef CC_ERROR_FILEDES
    /* A missing CC_ERROR_FILEDES or an CC_ERROR_FILEDES of zero means: Do not redirect. */
    if (CC_ERROR_FILEDES == 1) {
      sprintf(&command[strlen(command)], " %sctest%d.cerrs %s%s",
              REDIRECT_FILEDES_1, testNumber, REDIRECT_FILEDES_2, nullDevice);
    } else if (CC_ERROR_FILEDES == 2) {
      sprintf(&command[strlen(command)], " %sctest%d.cerrs %s%s",
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
#ifdef LINKER
    if (returncode == 0) {
      /* fprintf(logFile, "returncode: %d\n", returncode); */
      sprintf(command, "%s ctest%d%s %s %sctest%d%s",
              LINKER, testNumber, OBJECT_FILE_EXTENSION, linkerOptions,
              LINKER_OPT_OUTPUT_FILE, testNumber, LINKED_PROGRAM_EXTENSION);
      /* fprintf(logFile, "command: %s\n", command); */
      returncode = system(command);
    } /* if */
#endif
    sprintf(fileName, "ctest%d%s", testNumber, LINKED_PROGRAM_EXTENSION);
    if (fileIsRegular(fileName)) {
      if (returncode == 0) {
        okay = 1;
      } else {
        /* fprintf(logFile, "\n *** The compiler %s fails, but creates an executable.\n", c_compiler); */
      } /* if */
    } else {
      /* fprintf(logFile, "\n *** The compiler %s produces no executable: %s\n", c_compiler, fileName); */
    } /* if */
#ifdef DEBUG_CHKCCOMP
    fprintf(logFile, "command: %s\n", command);
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



static int compileAndLinkWithOptionsOk (const char *content, const char *compilerOptions,
    const char *linkerOptions)

  {
    char fileName[NAME_SIZE];
    FILE *testFile;
    int okay = 0;

  /* compileAndLinkWithOptionsOk */
    /* fprintf(logFile, "compileAndLinkWithOptionsOk(%s)\n", content); */
    cleanUpCompilation(testNumber);
    testNumber++;
    cleanUpCompilation(testNumber);
    sprintf(fileName, "ctest%d.c", testNumber);
    testFile = fopen(fileName, "w");
    if (testFile != NULL) {
      fprintf(testFile, "%s", content);
      fclose(testFile);
      okay = doCompileAndLink(compilerOptions, linkerOptions, testNumber);
    } /* if */
#ifdef DEBUG_CHKCCOMP
    fprintf(logFile, "content: %s\n", content);
#endif
    /* fprintf(logFile, "compileAndLinkWithOptionsOk --> %d\n", okay); */
    return okay;
  } /* compileAndLinkWithOptionsOk */



static void showErrors (void)

  {
    char fileName[NAME_SIZE];
    FILE *errorFile;
    int ch;

  /* showErrors */
    sprintf(fileName, "ctest%d.cerrs", testNumber);
    errorFile = fopen(fileName, "r");
    if (errorFile != NULL) {
      fprintf(logFile, "\nCompiler errors:\n");
      while ((ch = getc(errorFile)) != EOF) {
        fputc(ch, logFile);
      } /* while */
      fclose(errorFile);
      fprintf(logFile, "\n");
    } /* if */
    sprintf(fileName, "ctest%d.lerrs", testNumber);
    errorFile = fopen(fileName, "r");
    if (errorFile != NULL) {
      fprintf(logFile, "\nLinker errors:\n");
      while ((ch = getc(errorFile)) != EOF) {
        fputc(ch, logFile);
      } /* while */
      fclose(errorFile);
      fprintf(logFile, "\n");
    } /* if */
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



static int doTest (void)

  {
    char command[COMMAND_SIZE];
    char fileName[NAME_SIZE];
    int returncode;
    FILE *outFile;
    int result = -1;

  /* doTest */
    fprintf(logFile, "+");
    fflush(logFile);
#ifdef INTERPRETER_FOR_LINKED_PROGRAM
    sprintf(command, "%s .%cctest%d%s>ctest%d.out",
            INTERPRETER_FOR_LINKED_PROGRAM, PATH_DELIMITER, testNumber,
            LINKED_PROGRAM_EXTENSION, testNumber);
#else
    sprintf(command, ".%cctest%d%s>ctest%d.out", PATH_DELIMITER,
            testNumber, LINKED_PROGRAM_EXTENSION, testNumber);
#endif
    returncode = system(command);
    if (returncode != -1) {
      sprintf(fileName, "ctest%d.out", testNumber);
      outFile = fopen(fileName, "r");
      if (outFile != NULL) {
        fscanf(outFile, "%d", &result);
        fclose(outFile);
      } /* if */
    } /* if */
    fprintf(logFile, "\b");
    fflush(logFile);
    return result;
  } /* doTest */



static int expectTestResult (const char *content, int expected)

  {
    int testResult = -1;
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



static void testOutputToVersionFile (FILE *versionFile)

  {
    char command[COMMAND_SIZE];
    char fileName[NAME_SIZE];
    int returncode;
    FILE *outFile;
    int ch;

  /* testOutputToVersionFile */
    fprintf(logFile, ">");
    fflush(logFile);
#ifdef INTERPRETER_FOR_LINKED_PROGRAM
    sprintf(command, "%s .%cctest%d%s>ctest%d.out",
            INTERPRETER_FOR_LINKED_PROGRAM, PATH_DELIMITER, testNumber,
            LINKED_PROGRAM_EXTENSION, testNumber);
#else
    sprintf(command, ".%cctest%d%s>ctest%d.out", PATH_DELIMITER,
            testNumber, LINKED_PROGRAM_EXTENSION, testNumber);
#endif
    returncode = system(command);
    if (returncode != -1) {
      sprintf(fileName, "ctest%d.out", testNumber);
      outFile = fopen(fileName, "r");
      if (outFile != NULL) {
        while ((ch = getc(outFile)) != EOF) {
          putc(ch, versionFile);
        } /* while */
        fclose(outFile);
      } /* if */
    } /* if */
    fprintf(logFile, "\b");
    fflush(logFile);
  } /* testOutputToVersionFile */



static int isNullDevice (char *fileName, int eofChar)

  {
    FILE *aFile;
    int fileIsEmpty;
    int fileIsNullDevice = 0;

  /* isNullDevice */
    aFile = fopen(fileName, "r");
    if (aFile != NULL) {
      /* The file exists. Now check if the file is empty. */
      fileIsEmpty = getc(aFile) == eofChar;
      fclose(aFile);
      if (fileIsEmpty) {
        /* Reading from a null device returns always EOF. */
        aFile = fopen(fileName, "r+");
        if (aFile != NULL) {
          /* Writing of 'X' to a null device should be ignored. */
          putc('X', aFile);
          fclose(aFile);
          aFile = fopen(fileName, "r");
          if (aFile != NULL) {
            /* Check if the file is still empty. */
            fileIsEmpty = getc(aFile) == eofChar;
            fclose(aFile);
            if (fileIsEmpty) {
              /* Everything written to the null device is ignored. */
              fileIsNullDevice = 1;
            } else {
              /* The file is not empty, so it is not a null device. */
              aFile = fopen(fileName, "w");
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



static int checkIfNullDevice (char *fileName, int eofChar)

  {
    char buffer[BUFFER_SIZE];
    int fileIsNullDevice = 0;

  /* checkIfNullDevice */
    sprintf(buffer,
            "#include <stdio.h>\n"
            "int main(int argc, char *argv[]) {\n"
            "FILE *aFile;\n"
            "int fileIsEmpty;\n"
            "char *fileName = \"%s\";\n"
            "int eofChar = %d;\n"
            "int fileIsNullDevice = 0;\n"
            "aFile = fopen(fileName, \"r\");\n"
            "if (aFile != NULL) {\n"
            "  fileIsEmpty = getc(aFile) == eofChar;\n"
            "  fclose(aFile);\n"
            "  if (fileIsEmpty) {\n"
            "    aFile = fopen(fileName, \"r+\");\n"
            "    if (aFile != NULL) {\n"
            "      putc('X', aFile);\n"
            "      fclose(aFile);\n"
            "      aFile = fopen(fileName, \"r\");\n"
            "      if (aFile != NULL) {\n"
            "        fileIsEmpty = getc(aFile) == eofChar;\n"
            "        fclose(aFile);\n"
            "        if (fileIsEmpty) {\n"
            "          fileIsNullDevice = 1;\n"
            "        } else {\n"
            "          aFile = fopen(fileName, \"w\");\n"
            "          if (aFile != NULL) {\n"
            "            fclose(aFile);\n"
            "          }\n"
            "        }\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "}\n"
            "printf(\"%%d\\n\", fileIsNullDevice);\n"
            "return 0;}",
            fileName, eofChar);
    if (assertCompAndLnk(buffer)) {
      fileIsNullDevice = doTest() == 1;
    } /* if */
    return fileIsNullDevice;
  } /* checkIfNullDevice */



static void determineNullDevice (FILE *versionFile)

  {
    char *nullDevice = NULL;

  /* determineNullDevice */
    if (checkIfNullDevice("/dev/null", EOF)) {
      nullDevice = "/dev/null";
    } else if (checkIfNullDevice("NUL:", EOF)) {
      nullDevice = "NUL:";
    } else if (checkIfNullDevice("NUL", EOF)) {
      nullDevice = "NUL";
    } else if (checkIfNullDevice("NUL:", 0)) {
      nullDevice = "NUL:";
    } else if (checkIfNullDevice("NUL", 0)) {
      nullDevice = "NUL";
    } /* if */
    if (nullDevice == NULL) {
      fputs("\n **** Unable to determine a null device.\n", logFile);
      nullDevice = "null_device";
    } /* if */
    fprintf(versionFile, "#define NULL_DEVICE \"%s\"\n", nullDevice);
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
                         "void fatal (void) __attribute__ ((noreturn));\n"
                         "void fatal (void) {exit(1);}\n"
                         "int main(int argc,char *argv[])\n"
                         "{return 0;}\n")) {
      fputs("#define NORETURN __attribute__ ((noreturn))\n", versionFile);
      strcat(macroDefs, "#define NORETURN __attribute__ ((noreturn))\\n");
    } else {
      fputs("#define NORETURN\n", versionFile);
      strcat(macroDefs, "#define NORETURN\\n");
    } /* if */
    fprintf(versionFile, "#define MACRO_DEFS \"%s\"\n", macroDefs);
  } /* writeMacroDefs */



static void checkPopen (FILE *versionFile)

  {
    char *popen = NULL;
    int binary_mode_supported;
    char *binary_mode = "";
    char buffer[BUFFER_SIZE];
    char fileName[NAME_SIZE];

  /* checkPopen */
    if (compileAndLinkOk("#include <stdio.h>\nint main(int argc, char *argv[])\n"
                         "{FILE *aFile; aFile=popen(\""
                         LIST_DIRECTORY_CONTENTS
                         "\", \"r\");\n"
                         "printf(\"%d\\n\", aFile != NULL); return 0;}\n") && doTest() == 1) {
      popen = "popen";
    } else if (compileAndLinkOk("#include <stdio.h>\nint main(int argc, char *argv[])\n"
                                "{FILE *aFile; aFile=_popen(\""
                                LIST_DIRECTORY_CONTENTS
                                "\", \"r\");\n"
                                "printf(\"%d\\n\", aFile != NULL); return 0;}\n") && doTest() == 1) {
      popen = "_popen";
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
        fprintf(versionFile, "#define POPEN_SUPPORTS_CLOEXEC_MODE %d\n", doTest() == 1);
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
    char fileName[NAME_SIZE];

  /* checkSystemResult */
    if (assertCompAndLnk("int main (int argc, char *argv[])\n"
                         "{ return 0; }\n")) {
      sprintf(fileName, "ctest%d%s", testNumber, LINKED_PROGRAM_EXTENSION);
      if (rename(fileName, "ctest_b" LINKED_PROGRAM_EXTENSION) == 0) {
        sprintf(buffer, "#include <stdio.h>\n#include <stdlib.h>\n"
                        "int main(int argc, char *argv[])\n"
                        "{char buffer[5]; int retVal; retVal=system(\""
                        ".%s%cctest_b%s"
                        "\");\n"
                        "printf(\"%%d\\n\", retVal); return 0;}\n",
                        PATH_DELIMITER == '\\' ? "\\" : "", PATH_DELIMITER,
                        LINKED_PROGRAM_EXTENSION);
        if (assertCompAndLnk(buffer)) {
          testResult = doTest();
          fprintf(versionFile, "#define SYSTEM_RESULT_FOR_RETURN_0 %d\n", testResult);
          if (testResult != 0) {
            fprintf(logFile, "\n *** System result for return 0 is %d\n", testResult);
          } /* if */
        } /* if */
        doRemove("ctest_b" LINKED_PROGRAM_EXTENSION);
      } else {
        fprintf(logFile, "\n *** Unable to rename %s to ctest_b%s\n",
                fileName, LINKED_PROGRAM_EXTENSION);
      } /* if */
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
    int sizeof_int;
    int sizeof_long;
    int sizeof_long_long;
    int sizeof_int64;

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
    fprintf(versionFile, "#define POINTER_SIZE %d\n",     char_bit * getSizeof("char *"));
    fprintf(versionFile, "#define FLOAT_SIZE %d\n",       char_bit * getSizeof("float"));
    fprintf(versionFile, "#define DOUBLE_SIZE %d\n",      char_bit * getSizeof("double"));
    fprintf(versionFile, "#define WCHAR_T_SIZE %d\n",     char_bit * getSizeof("wchar_t"));
    fprintf(versionFile, "#define TIME_T_SIZE %d\n",      char_bit * getSizeof("time_t"));
    fprintf(versionFile, "#define TIME_T_SIGNED %d\n", isSignedType("time_t"));
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
#else
    check_int_div_by_zero =
        !compileAndLinkOk("#include<stdio.h>\n"
                          "int main(int argc,char *argv[]){"
                          "printf(\"%d\\n\", 1/0);return 0;}\n") ||
        !compileAndLinkOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                          "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                          "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                          "int main(int argc,char *argv[]){\n"
                          "signal(SIGFPE,handleSigfpe);\n"
                          "signal(SIGILL,handleSigill);\n"
                          "printf(\"%d\\n\",1/0==0);return 0;}\n") || doTest() != 2 ||
        !compileAndLinkOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                          "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                          "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                          "int main(int argc,char *argv[]){\n"
                          "int zero=0;\n"
                          "signal(SIGFPE,handleSigfpe);\n"
                          "signal(SIGILL,handleSigill);\n"
                          "printf(\"%d\\n\",1/zero==0);return 0;}\n") || doTest() != 2 ||
        !compileAndLinkOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                          "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                          "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                          "int main(int argc,char *argv[]){\n"
                          "int one=0;\n"
                          "signal(SIGFPE,handleSigfpe);\n"
                          "signal(SIGILL,handleSigill);\n"
                          "printf(\"%d\\n\",one/0==0);return 0;}\n") || doTest() != 2 ||
        !compileAndLinkOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                          "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                          "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                          "int main(int argc,char *argv[]){\n"
                          "int one=0;\n"
                          "int zero=0;\n"
                          "signal(SIGFPE,handleSigfpe);\n"
                          "signal(SIGILL,handleSigill);\n"
                          "printf(\"%d\\n\",one/zero==0);return 0;}\n") || doTest() != 2 ||
        !compileAndLinkOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                          "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                          "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                          "int main(int argc,char *argv[]){\n"
                          "int zero=0;\n"
                          "signal(SIGFPE,handleSigfpe);\n"
                          "signal(SIGILL,handleSigill);\n"
                          "printf(\"%d\\n\",zero/0==0);return 0;}\n") || doTest() != 2 ||
        !compileAndLinkOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                          "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                          "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                          "int main(int argc,char *argv[]){\n"
                          "int zero1=0;\n"
                          "int zero2=0;\n"
                          "signal(SIGFPE,handleSigfpe);\n"
                          "signal(SIGILL,handleSigill);\n"
                          "printf(\"%d\\n\",zero1/zero2==0);return 0;}\n") || doTest() != 2;

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

    check_int_rem_by_zero =
        !compileAndLinkOk("#include<stdio.h>\n"
                          "int main(int argc,char *argv[]){"
                          "printf(\"%d\\n\", 1%0);return 0;}\n") ||
        !compileAndLinkOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                          "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                          "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                          "int main(int argc,char *argv[]){\n"
                          "signal(SIGFPE,handleSigfpe);\n"
                          "signal(SIGILL,handleSigill);\n"
                          "printf(\"%d\\n\",1%0==0);return 0;}\n") || doTest() != 2 ||
        !compileAndLinkOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                          "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                          "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                          "int main(int argc,char *argv[]){\n"
                          "int zero=0;\n"
                          "signal(SIGFPE,handleSigfpe);\n"
                          "signal(SIGILL,handleSigill);\n"
                          "printf(\"%d\\n\",1%zero==0);return 0;}\n") || doTest() != 2 ||
        !compileAndLinkOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                          "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                          "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                          "int main(int argc,char *argv[]){\n"
                          "int one=0;\n"
                          "signal(SIGFPE,handleSigfpe);\n"
                          "signal(SIGILL,handleSigill);\n"
                          "printf(\"%d\\n\",one%0==0);return 0;}\n") || doTest() != 2 ||
        !compileAndLinkOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                          "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                          "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                          "int main(int argc,char *argv[]){\n"
                          "int one=0, zero=0;\n"
                          "signal(SIGFPE,handleSigfpe);\n"
                          "signal(SIGILL,handleSigill);\n"
                          "printf(\"%d\\n\",one%zero==0);return 0;}\n") || doTest() != 2 ||
        !compileAndLinkOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                          "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                          "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                          "int main(int argc,char *argv[]){\n"
                          "int zero=0;\n"
                          "signal(SIGFPE,handleSigfpe);\n"
                          "signal(SIGILL,handleSigill);\n"
                          "printf(\"%d\\n\",zero%0==0);return 0;}\n") || doTest() != 2 ||
        !compileAndLinkOk("#include<stdlib.h>\n#include<stdio.h>\n#include<signal.h>\n"
                          "void handleSigfpe(int sig){puts(\"2\");exit(0);}\n"
                          "void handleSigill(int sig){puts(\"3\");exit(0);}\n"
                          "int main(int argc,char *argv[]){\n"
                          "int zero1=0, zero2=0;\n"
                          "signal(SIGFPE,handleSigfpe);\n"
                          "signal(SIGILL,handleSigill);\n"
                          "printf(\"%d\\n\",zero1%zero2==0);return 0;}\n") || doTest() != 2;

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
#ifndef DO_SIGFPE_WITH_DIV_BY_ZERO
    fprintf(versionFile, "#define DO_SIGFPE_WITH_DIV_BY_ZERO %d\n", !check_int_div_by_zero);
#endif
    fprintf(versionFile, "#define CHECK_INT_DIV_BY_ZERO %d\n", check_int_div_by_zero);
    fprintf(versionFile, "#define CHECK_INT_DIV_ZERO_BY_ZERO %d\n", check_int_div_zero_by_zero);
    fprintf(versionFile, "#define CHECK_INT_REM_BY_ZERO %d\n", check_int_rem_by_zero);
    fprintf(versionFile, "#define CHECK_INT_REM_ZERO_BY_ZERO %d\n", check_int_rem_zero_by_zero);
  } /* checkIntDivisions */



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
    if (compileAndLinkOk(buffer) && doTest() == 1) {
      macro_definition = os_isnan_definition;
    } /* if */
    return macro_definition;
  } /* determine_os_isnan_definition */



static void defineTransferUnions (char * buffer)

  { /* defineTransferUnions */
    strcat(buffer,
           "union {\n"
           "  ");
    switch (getSizeof("float")) {
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
    switch (getSizeof("double")) {
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
    char buffer[10240];
    char computeValues[BUFFER_SIZE];
    const char *builtin_add_overflow = "unexisting_function";
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
    sprintf(buffer, "#include<stdlib.h>\n#include<stdio.h>\n#include<limits.h>\n"
                    "#include<signal.h>\n"
                    "void handleSigill(int sig){puts(\"2\");exit(0);}\n"
                    "void handleSigabrt(int sig){puts(\"3\");exit(0);}\n"
                    "int main(int argc,char *argv[]){\n"
                    "%s a=0x7fffffffffffffff,b=1,c=2;\n"
                    "signal(SIGILL,handleSigill);\nsignal(SIGABRT,handleSigabrt);\n"
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
    if (getSizeof("int") == 8) {
      builtin_add_overflow = "__builtin_sadd_overflow";
    } else if (getSizeof("long") == 8) {
      builtin_add_overflow = "__builtin_saddl_overflow";
    } else if (getSizeof("long long") == 8) {
      builtin_add_overflow = "__builtin_saddll_overflow";
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
      fprintf(versionFile, "#define FLOAT_STR_LARGE_NUMBER 1.0e%d\n", testResult);
    } /* if */
    if (assertCompAndLnk("#include<stdio.h>\n#include<float.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{printf(\"%d\\n\",DBL_DIG);return 0;}\n")) {
      testResult = doTest();
      fprintf(versionFile, "#define FMT_E_DBL \"%%1.%de\"\n", testResult - 1);
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
    has_log2 =
        compileAndLinkWithOptionsOk("#include<stdio.h>\n#include<float.h>\n#include<math.h>\n"
                                    "int main(int argc,char *argv[]){\n"
                                    "float num1 = 2.0;\n"
                                    "double num2 = 2.0;\n"
                                    "printf(\"%d\\n\",\n"
                                    "       log2(num1) == 1.0 &&\n"
                                    "       log2(num2) == 1.0);\n"
                                    "return 0;}\n",
                                    "", SYSTEM_LIBS) && doTest() == 1;
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
                                    "", SYSTEM_LIBS) && doTest() == 1);
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
    } else if (os_isnan_definition == NULL) {
      os_isnan_definition = determine_os_isnan_definition(computeValues, "#define os_isnan(x) (x != x)");
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
            "      0.0 >= doubleNanValue2));\n"
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
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS) && doTest() == 1);
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
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS) && doTest() == 1);
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
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS) && doTest() == 1);
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
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS) && doTest() == 1);
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
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS) && doTest() == 1);
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
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS) && doTest() == 1);
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
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS) && doTest() == 1);
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
            compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS) ? doTest() : 0);
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
            compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS) ? doTest() : 0);
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
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS) && doTest() == 1);
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
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS) && doTest() == 1);
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
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS) && doTest() == 1);
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
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS) && doTest() == 1);
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
        compileAndLinkWithOptionsOk(buffer, "", SYSTEM_LIBS) && doTest() == 1);
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
            "    0.0 * floatPlusInf != floatNanValue1 ||\n"
            "    0.0 * floatMinusInf != floatNanValue1 ||\n"
            "    floatPlusInf * 0.0 != floatNanValue1 ||\n"
            "    floatMinusInf * 0.0 != floatNanValue1 ||\n"
            "    0.0 * doublePlusInf != doubleNanValue1 ||\n"
            "    0.0 * doubleMinusInf != doubleNanValue1 ||\n"
            "    doublePlusInf * 0.0 != doubleNanValue1 ||\n"
            "    doubleMinusInf * 0.0 != doubleNanValue1);\n"
            "printf(\"#define NAN_MULTIPLICATION_OKAY %%d\\n\",\n"
            "    os_isnan(floatNanValue1 * 1.0) &&\n"
            "    os_isnan(floatNanValue1 * floatOne) &&\n"
            "    os_isnan(1.0 * floatNanValue1) &&\n"
            "    os_isnan(floatOne * floatNanValue1) &&\n"
            "    os_isnan(floatNanValue1 * floatNanValue2) &&\n"
            "    os_isnan(doubleNanValue1 * 1.0) &&\n"
            "    os_isnan(doubleNanValue1 * doubleOne) &&\n"
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
    if (assertCompAndLnkWithOptions(buffer, "", SYSTEM_LIBS)) {
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
            "int main(int argc,char *argv[]){\n"
            "typedef %s int64Type;\n"
            "int64Type number;\n"
            "int64Type intFirst = (int64Type) (-9223372036854775807 - 1);\n"
            "int64Type intLast = (int64Type) 9223372036854775807;\n"
            "double argument;\n"
            "int foundMin = 0;\n"
            "int foundMax = 0;\n"
#ifdef TURN_OFF_FP_EXCEPTIONS
            "_control87(MCW_EM, MCW_EM);\n"
#endif
            "for (number = 0; !(foundMin && foundMax) && number < 1000; number++) {\n"
            "  argument = (double) (intFirst + number);\n"
            "  if (!foundMin && (int64Type) argument != 0) {\n"
            "    printf(\"#define MINIMUM_TRUNC_ARGUMENT %%%sd\\n\",\n"
            "           intFirst + number);\n"
            "    foundMin = 1;\n"
            "  }\n"
            "  argument = (double) (intLast - number);\n"
            "  if (!foundMax && (int64Type) argument > 0) {\n"
            "    printf(\"#define MAXIMUM_TRUNC_ARGUMENT %%%sd\\n\",\n"
            "           intLast - number);\n"
            "    foundMax = 1;\n"
            "  }\n"
            "}\n"
            "return 0;}\n",
            int64TypeStri, int64TypeFormat, int64TypeFormat);
    if (assertCompAndLnk(buffer)) {
      testOutputToVersionFile(versionFile);
    } /* if */
    if (assertCompAndLnk("#include<stdio.h>\n#include<string.h>\n"
                         "int main(int argc,char *argv[]){\n"
                         "char buffer[100010];\n"
                         "sprintf(buffer, \"%1.100000e\", 1.0);\n"
                         "printf(\"%lu\\n\", (unsigned long) (strchr(buffer, 'e') - buffer));\n"
                         "return 0;}\n")) {
      testResult = doTest();
      if (testResult == -1) {
        /* The test program crashed. Assume a low precision limit. */
        testResult = 102;
        fputs("#define LIMIT_FMT_E_MAXIMUM_FLOAT_PRECISION \"100\"\n", versionFile);
      } /* if */
      if (testResult >= 2 && testResult < 100002) {
        testResult -= 2;
#ifdef PRINTF_MAXIMUM_FLOAT_PRECISION
        fprintf(versionFile, "/* PRINTF_FMT_E_MAXIMUM_FLOAT_PRECISION %d */\n", testResult);
        testResult = PRINTF_MAXIMUM_FLOAT_PRECISION;
#endif
        fprintf(versionFile, "#define PRINTF_FMT_E_MAXIMUM_FLOAT_PRECISION %d\n", testResult);
      } /* if */
    } /* if */
    if (assertCompAndLnk("#include<stdio.h>\n#include<string.h>\n"
                         "int main(int argc,char *argv[]){\n"
                         "char buffer[100010];\n"
                         "sprintf(buffer, \"%1.100000f\", 1.0);\n"
                         "printf(\"%lu\\n\", (unsigned long) strlen(buffer));\n"
                         "return 0;}\n")) {
      testResult = doTest();
      if (testResult == -1) {
        /* The test program crashed. Assume a low precision limit. */
        testResult = 102;
        fputs("#define LIMIT_FMT_F_MAXIMUM_FLOAT_PRECISION \"100\"\n", versionFile);
      } /* if */
      if (testResult >= 2 && testResult < 100002) {
        testResult -= 2;
#ifdef PRINTF_MAXIMUM_FLOAT_PRECISION
        fprintf(versionFile, "/* PRINTF_FMT_F_MAXIMUM_FLOAT_PRECISION %d */\n", testResult);
        testResult = PRINTF_MAXIMUM_FLOAT_PRECISION;
#endif
        fprintf(versionFile, "#define PRINTF_FMT_F_MAXIMUM_FLOAT_PRECISION %d\n", testResult);
      } /* if */
    } /* if */
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
    const int repeatCount = 1000;
    const int charsInLine = 100;
    int lineLength;
    int totalLength;
    int count;
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
      if (testResult == repeatCount * charsInLine) {
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
    /* The definitions below correspond to a array literal length of 100000. */
    const int repeatCount = 5000;
    const int elementsInLine = 20;
    int lineLength;
    int totalLength;
    int count;
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
      if (testResult == repeatCount * elementsInLine) {
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
    const char *makeDirDefinition = NULL;

  /* defineMakeDir */
    if (compileAndLinkOk("#include <stdio.h>\n#include <direct.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{mkdir(\"tmp_empty_dir1\");return 0;}\n")) {
      makeDirDefinition = "#include <direct.h>\n"
                          "#define makeDir(path,mode) mkdir(path)\n";
    } else if (compileAndLinkOk("#include <stdio.h>\n"
                                "#include <sys/stat.h>\n#include <sys/types.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{mkdir(\"tmp_empty_dir1\");return 0;}\n")) {
      makeDirDefinition = "#include <sys/stat.h>\n#include <sys/types.h>\n"
                          "#define makeDir(path,mode) mkdir(path)\n";
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <direct.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{_mkdir(\"tmp_empty_dir1\");return 0;}\n")) {
      makeDirDefinition = "#include <direct.h>\n"
                          "#define makeDir(path,mode) _mkdir(path)\n";
    } else if (compileAndLinkOk("#include <stdio.h>\n"
                                "#include <sys/stat.h>\n#include <sys/types.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{_mkdir(\"tmp_empty_dir1\");return 0;}\n")) {
      makeDirDefinition = "#include <sys/stat.h>\n#include <sys/types.h>\n"
                          "#define makeDir(path,mode) _mkdir(path)\n";
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <direct.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{mkdir(\"tmp_empty_dir1\", 0755);return 0;}\n")) {
      makeDirDefinition = "#include <direct.h>\n"
                          "#define makeDir(path,mode) mkdir(path,mode)\n";
    } else if (compileAndLinkOk("#include <stdio.h>\n"
                                "#include <sys/stat.h>\n#include <sys/types.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{mkdir(\"tmp_empty_dir1\", 0755);return 0;}\n")) {
      makeDirDefinition = "#include <sys/stat.h>\n#include <sys/types.h>\n"
                          "#define makeDir(path,mode) mkdir(path,mode)\n";
    } /* if */
    /* fprintf(logFile, "Internal makeDirDefinition:\n%s\n", makeDirDefinition); */
    return makeDirDefinition;
  } /* defineMakeDir */



static const char *defineRemoveDir (void)

  {
    const char *removeDirDefinition = NULL;

  /* defineRemoveDir */
    if (compileAndLinkOk("#include <stdio.h>\n#include <direct.h>\n"
                         "int main(int argc,char *argv[])\n"
                         "{rmdir(\"tmp_empty_dir1\");return 0;}\n")) {
      removeDirDefinition = "#include <direct.h>\n"
                            "#define removeDir(path) rmdir(path)\n";
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <unistd.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{rmdir(\"tmp_empty_dir1\");return 0;}\n")) {
      removeDirDefinition = "#include <unistd.h>\n"
                            "#define removeDir(path) rmdir(path)\n";
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <direct.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{_rmdir(\"tmp_empty_dir1\");return 0;}\n")) {
      removeDirDefinition = "#include <direct.h>\n"
                            "#define removeDir(path) _rmdir(path)\n";
    } else if (compileAndLinkOk("#include <stdio.h>\n#include <unistd.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{_rmdir(\"tmp_empty_dir1\");return 0;}\n")) {
      removeDirDefinition = "#include <unistd.h>\n"
                            "#define removeDir(path) _rmdir(path)\n";
    } /* if */
    /* fprintf(logFile, "Internal removeDirDefinition:\n%s\n", removeDirDefinition); */
    return removeDirDefinition;
  } /* defineRemoveDir */



static void checkRemoveDir (const char *makeDirDefinition, FILE *versionFile)

  {
    char buffer[BUFFER_SIZE];

  /* checkRemoveDir */
    sprintf(buffer, "#include <stdio.h>\n#include <unistd.h>\n%s"
                    "int main(int argc,char *argv[])\n"
                    "{int rmFail=0;makeDir(\"tmp_empty_dir1\",0755);\n"
                    "if(remove(\"tmp_empty_dir1\")!=0){rmFail=1;rmdir(\"tmp_empty_dir1\");}\n"
                    "printf(\"%%d\\n\",rmFail);return 0;}\n",
                    makeDirDefinition);
    if (compileAndLinkOk(buffer)) {
      fprintf(versionFile, "#define REMOVE_FAILS_FOR_EMPTY_DIRS %d\n", doTest() == 1);
    } else {
      sprintf(buffer, "#include <stdio.h>\n#include <direct.h>\n%s"
                      "int main(int argc,char *argv[])\n"
                      "{int rmFail=0;makeDir(\"tmp_empty_dir1\",0755);\n"
                      "if(remove(\"tmp_empty_dir1\")!=0){rmFail=1;rmdir(\"tmp_empty_dir1\");}\n"
                      "printf(\"%%d\\n\",rmFail);return 0;}\n",
                      makeDirDefinition);
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
    char *directory_lib = NULL;
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
    int sizeof_long;
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
      sizeof_long = getSizeof("long");
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
    } else {
      fprintf(logFile, "\n *** Cannot define os_fseek and os_ftell.\n");
    } /* if */
    doRemove("ctstfile.txt");
#else
      fprintf(versionFile, "#define OS_OFF_T_SIZE %d\n", 8 * getSizeof("os_off_t"));
#endif
  } /* determineFseekFunctions */



static void determineFtruncate (FILE *versionFile, const char *fileno)

  {
    char buffer[BUFFER_SIZE];
    char *os_ftruncate_stri = NULL;
    char *ftruncate_size_in_bits;
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
        sprintf(size_buffer, "%d", getSizeof("long") * 8);
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
        sprintf(size_buffer, "%d", getSizeof("long") * 8);
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
        sprintf(size_buffer, "%d", getSizeof("long") * 8);
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
    char buffer[BUFFER_SIZE];
    char getenv_definition[BUFFER_SIZE];
    char setenv_definition[BUFFER_SIZE];
    char *os_environ_stri = NULL;
    int declare_os_environ = 0;
    int use_get_environment = 0;
    int initialize_os_environ = 0;
    int define_wgetenv = 0;
    int define_wsetenv = 0;
    int define_wunsetenv = 0;
    int test_result;
    int putenv_can_remove_keys = 0;
    int getenv_is_case_sensitive = -1;
    char *os_getenv_stri = NULL;
    char *os_putenv_stri = NULL;
    char *os_setenv_stri = NULL;
    char *os_unsetenv_stri = NULL;

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
    os_setenv_stri = "setenv7";
    os_unsetenv_stri = "unsetenv7";
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



static void determineOsUtime (FILE *versionFile)

  {
    char *utime_include = NULL;
    char *os_utimbuf_struct_stri = NULL;
    char *os_utime_stri = NULL;
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



#ifdef OS_STRI_WCHAR
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
#if !defined os_lstat || !defined os_stat
    fputs("#define DEFINE_WSTATI64_EXT\n", versionFile);
#endif
#ifndef os_lstat
    fputs("#define os_lstat wstati64Ext\n", versionFile);
#endif
#ifndef os_stat
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
    if (compileAndLinkWithOptionsOk("#include <stdio.h>\n#include <direct.h>\n"
                                    "int main(int argc,char *argv[])\n"
                                    "{printf(\"%d\\n\",\n"
                                    "    _wchmod(L\"testfile\",0777) != -1);\n"
                                    "return 0;}\n", "", SYSTEM_LIBS)) {
      fputs("#define os_chmod _wchmod\n", versionFile);
    } else if (compileAndLinkWithOptionsOk("#include <stdio.h>\n#include <direct.h>\n\n"
                                           "#include <io.h>\n"
                                           "int main(int argc,char *argv[])\n"
                                           "{printf(\"%d\\n\",\n"
                                           "    _wchmod(L\"testfile\",0777) != -1);\n"
                                           "return 0;}\n", "", SYSTEM_LIBS)) {
      fputs("#define OS_CHMOD_INCLUDE_IO_H\n", versionFile);
      fputs("#define os_chmod _wchmod\n", versionFile);
    } else {
      fprintf(logFile, "\n *** Cannot define os_chmod.\n");
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
    if (compileAndLinkWithOptionsOk("#include <stdio.h>\n"
                                    "int main(int argc,char *argv[])\n"
                                    "{printf(\"%d\\n\",\n"
                                    "    _wrename(L\"testfile\", L\"newname\") == 0);\n"
                                    "return 0;}\n", "", SYSTEM_LIBS)) {
      fputs("#define os_rename _wrename\n", versionFile);
    } else {
      fprintf(logFile, "\n *** Cannot define os_rename.\n");
      showErrors();
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
  } /* determineOsWCharFunctions */
#endif



static void determineIsattyFunction (FILE *versionFile)

  { /* determineIsattyFunction */
    if (!compileAndLinkOk("#include <stdio.h>\n#include <unistd.h>\n"
                          "int main(int argc,char *argv[]){\n"
                          "printf(\"%d\\n\", isatty(0)==0);\n"
                          "return 0;}\n")) {
      if (compileAndLinkOk("#include <stdio.h>\n#include <io.h>\n"
                           "int main(int argc,char *argv[]){\n"
                           "printf(\"%d\\n\", isatty(0)==0);\n"
                           "return 0;}\n")) {
        fprintf(versionFile, "#define ISATTY_INCLUDE_IO_H\n");
      } else if (compileAndLinkOk("#include <stdio.h>\n#include <io.h>\n"
                                  "int main(int argc,char *argv[]){\n"
                                  "printf(\"%d\\n\", _isatty(0)==0);\n"
                                  "return 0;}\n")) {
        fprintf(versionFile, "#define ISATTY_INCLUDE_IO_H\n");
        fprintf(versionFile, "#define os_isatty _isatty\n");
      } /* if */
    } /* if */
  } /* determineIsattyFunction */



static const char *determineFilenoFunction (FILE *versionFile)

  {
    char buffer[BUFFER_SIZE];
    const char *fileno = "fileno";

  /* determineFilenoFunction */
    if (!compileAndLinkOk("#include <stdio.h>\n"
                          "int main(int argc,char *argv[]){\n"
                          "printf(\"%d\\n\", fileno(stdin)==0);\n"
                          "return 0;}\n") &&
        compileAndLinkOk("#include <stdio.h>\n"
                         "int main(int argc,char *argv[]){\n"
                         "printf(\"%d\\n\", _fileno(stdin)==0);\n"
                         "return 0;}\n")) {
      fileno = "_fileno";
      fprintf(versionFile, "#define os_fileno _fileno\n");
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
    fileno = determineFilenoFunction(versionFile);
    determineSocketLib(versionFile);
    determineOsDirAccess(versionFile);
    determineFseekFunctions(versionFile, fileno);
    determineFtruncate(versionFile, fileno);
#if defined OS_STRI_WCHAR
    determineOsWCharFunctions(versionFile);
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
      fprintf(versionFile, "#define FOPEN_SUPPORTS_CLOEXEC_MODE %d\n", doTest() == 1);
    } else if (assertCompAndLnk("#include <stdio.h>\n"
                                "int main(int argc,char *argv[])\n"
                                "{printf(\"%d\\n\", fopen(\"ctstfile.txt\", \"we\") != NULL);"
                                "return 0;}\n")) {
      fprintf(versionFile, "#define FOPEN_SUPPORTS_CLOEXEC_MODE %d\n", doTest() == 1);
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
        sprintf(&command[strlen(command)], " %s%s %s%s",
                REDIRECT_FILEDES_1, nullDevice, REDIRECT_FILEDES_2, nullDevice);
        /* fprintf(logFile, "command: %s\n", command); */
        returncode = system(command);
        /* fprintf(logFile, "returncode: %d\n", returncode); */
        if (fileIsRegular(libraryName)) {
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



static void appendOption (char *options, const char *optionToAppend)

  { /* appendOption */
    if (strstr(options, optionToAppend) == NULL) {
      if (options[0] != '\0' && optionToAppend[0] != '\0') {
        strcat(options, "\n");
      } /* if */
      strcat(options, optionToAppend);
    } /* if */
  } /* appendOption */



static const char *findIncludeFile (char *scopeName, char *testProgram,
    const char *baseDir, const char **inclDirList, size_t inclDirListLength,
    const char *inclName, char *includeOption)

  {
    int dirIndex;
    int nameIndex;
    char dirPath[BUFFER_SIZE];
    char filePath[BUFFER_SIZE];
    char inclOption[BUFFER_SIZE];
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



static int findStaticLib (const char *scopeName, const char *testProgram,
    const char *includeOption, const char *baseDir, const char **libDirList,
    size_t libDirListLength, const char **libNameList, size_t libNameListLength,
    char *system_libs)

  {
    int dirIndex;
    int nameIndex;
    char dirPath[BUFFER_SIZE];
    char filePath[BUFFER_SIZE];
    char linkParam[BUFFER_SIZE];
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
            sprintf(linkParam, "\"%s\"", filePath);
            /* fprintf(logFile, "includeOption: \"%s\"\n", includeOption);
            fprintf(logFile, "linkParam: \"%s\"\n", linkParam); */
            if (compileAndLinkWithOptionsOk(testProgram, includeOption, linkParam)) {
              if (doTest() == 1) {
                fprintf(logFile, "\r%s: %s found in: %s\n",
                        scopeName, libNameList[nameIndex], dirPath);
                appendOption(system_libs, linkParam);
                libFound = 1;
              } else {
                fprintf(logFile, "\r%s: Cannot execute with %s\n", scopeName, filePath);
              } /* if */
            } else {
              fprintf(logFile, "\r%s: Cannot link %s\n", scopeName, filePath);
            } /* if */
          } else {
            sprintf(linkParam, "-L%s %s", dirPath, libNameList[nameIndex]);
            /* fprintf(logFile, "includeOption: \"%s\"\n", includeOption);
            fprintf(logFile, "linkParam: \"%s\"\n", linkParam); */
            if (compileAndLinkWithOptionsOk(testProgram, includeOption, linkParam)) {
              if (doTest() == 1) {
                fprintf(logFile, "\r%s: %s found in: %s\n",
                        scopeName, libNameList[nameIndex], dirPath);
                sprintf(linkParam, "-L%s", dirPath);
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
    int nameIndex;
    char linkParam[BUFFER_SIZE];
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



static void listDynamicLibs (char *scopeName, const char *baseDir,
    const char **dllDirList, size_t dllDirListLength,
    const char **dllNameList, size_t dllNameListLength, FILE *versionFile)

  {
    int dirIndex;
    int nameIndex;
    char dirPath[BUFFER_SIZE];
    char filePath[BUFFER_SIZE];

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
            fprintf(logFile, "\r%s: DLL / Shared library: %s\n",
                    scopeName, filePath);
            fprintf(versionFile, " \"");
            escapeString(versionFile, filePath);
            fprintf(versionFile, "\",");
          } /* if */
        } /* for */
      } /* if */
    } /* for */
  } /* listDynamicLibs */



static void determineX11Defines (FILE *versionFile, char *include_options)

  {
    const char *inclDirList[] = {"/opt/X11/include"};
#ifdef X11_LIBRARY_PATH
    const char *libDirList[] = { X11_LIBRARY_PATH };
#endif
#ifdef X11_LIBS
    const char *libNameList[] = { X11_LIBS };
#else
    const char *libNameList[] = {"-lX11"};
#endif
#ifdef X11_DLL
    const char *dllNameList[] = { X11_DLL };
#elif LIBRARY_TYPE == UNIX_LIBRARIES
    const char *dllNameList[] = {"libX11.so", "libX11.so.6", "libX11.so.5"};
#elif LIBRARY_TYPE == MACOS_LIBRARIES
    const char *dllNameList[] = {"libX11.dylib"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *dllNameList[] = {"x11.dll"};
#endif
    char includeOption[BUFFER_SIZE];
    const char *x11Include = NULL;
    const char *x11IncludeCommand;
    char testProgram[BUFFER_SIZE];
    int nameIndex;
    int searchForLib = 1;
    char makeDefinition[BUFFER_SIZE];
    char system_draw_libs[BUFFER_SIZE];

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
#ifdef ALLOW_REPLACEMENT_OF_SYSTEM_HEADERS
    } else {
      includeOption[0] = '\0';
      x11Include = "x11_x.h";
      fprintf(versionFile, "#define X11_INCLUDE \"%s\"\n", x11Include);
      fprintf(logFile, "\rX11: %s found in Seed7 include directory.\n", x11Include);
      x11IncludeCommand = "#include \"x11_x.h\"\n";
#endif
    } /* if */
    if (x11Include == NULL) {
      fprintf(versionFile, "#define SYSTEM_DRAW_LIBS \"\"\n");
      appendToFile("macros", "SYSTEM_DRAW_LIBS =\n");
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
                           "display = XOpenDisplay(\"\");\n"
                           "printf(\"1\\n\");\n"
                           "return 0;}\n", x11IncludeCommand);
      /* fprintf(logFile, "%s\n", testProgram);
         fprintf(logFile, "x11Include: \"%s\"\n", x11Include); */
      system_draw_libs[0] = '\0';
#ifdef X11_LIBRARY_PATH
      if (findStaticLib("X11", testProgram, includeOption, "",
                        libDirList, sizeof(libDirList) / sizeof(char *),
                        libNameList, sizeof(libNameList) / sizeof(char *),
                        system_draw_libs)) {
        sprintf(makeDefinition, "SYSTEM_DRAW_LIBS = %s", system_draw_libs);
        replaceNLBySpace(makeDefinition);
        strcat(makeDefinition, "\n");
        appendToFile("macros", makeDefinition);
        fprintf(versionFile, "#define SYSTEM_DRAW_LIBS \"");
        /* The SYSTEM_DRAW_LIBS are space separated: */
        replaceNLBySpace(system_draw_libs);
        escapeString(versionFile, system_draw_libs);
        fprintf(versionFile, "\"\n");
        searchForLib = 0;
      } /* if */
#endif
      if (searchForLib) {
        if (findLinkerOption("X11", testProgram, includeOption, "",
                             libNameList, sizeof(libNameList) / sizeof(char *),
                             system_draw_libs)) {
          sprintf(makeDefinition, "SYSTEM_DRAW_LIBS = %s", system_draw_libs);
          replaceNLBySpace(makeDefinition);
          strcat(makeDefinition, "\n");
          appendToFile("macros", makeDefinition);
          fprintf(versionFile, "#define SYSTEM_DRAW_LIBS \"");
          /* The SYSTEM_DRAW_LIBS are space separated: */
          replaceNLBySpace(system_draw_libs);
          escapeString(versionFile, system_draw_libs);
          fprintf(versionFile, "\"\n");
          searchForLib = 0;
        } /* if */
      } /* if */
#endif
      fprintf(versionFile, "#define FORWARD_X11_CALLS %d\n", searchForLib);
      if (searchForLib) {
        fprintf(versionFile, "#define SYSTEM_DRAW_LIBS \"\"\n");
        appendToFile("macros", "SYSTEM_DRAW_LIBS =\n");
        /* Handle dynamic libraries: */
        fprintf(versionFile, "#define X11_DLL");
        for (nameIndex = 0;
             nameIndex < sizeof(dllNameList) / sizeof(char *);
             nameIndex++) {
          fprintf(logFile, "\rX11: DLL / Shared library: %s\n", dllNameList[nameIndex]);
          fprintf(versionFile, " \"%s\",", dllNameList[nameIndex]);
        } /* for */
        fprintf(versionFile, "\n");
      } /* if */
    } /* if */
  } /* determineX11Defines */



static void determineConsoleDefines (FILE *versionFile, char *include_options)

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
    int nameIndex;
    int searchForLib = 1;
    char testProgram[BUFFER_SIZE];
    char makeDefinition[BUFFER_SIZE];
    char system_console_libs[BUFFER_SIZE];

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
      fprintf(versionFile, "#define SYSTEM_CONSOLE_LIBS \"\"\n");
      appendToFile("macros", "SYSTEM_CONSOLE_LIBS =\n");
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
      system_console_libs[0] = '\0';
      if (useSystemHeader) {
        if (findLinkerOption("Console", testProgram, "", "",
                             libNameList, sizeof(libNameList) / sizeof(char *),
                             system_console_libs)) {
          sprintf(makeDefinition, "SYSTEM_CONSOLE_LIBS = %s", system_console_libs);
          replaceNLBySpace(makeDefinition);
          strcat(makeDefinition, "\n");
          appendToFile("macros", makeDefinition);
          fprintf(versionFile, "#define SYSTEM_CONSOLE_LIBS \"");
          /* The SYSTEM_CONSOLE_LIBS are space separated: */
          replaceNLBySpace(system_console_libs);
          escapeString(versionFile, system_console_libs);
          fprintf(versionFile, "\"\n");
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
        fprintf(versionFile, "#define SYSTEM_CONSOLE_LIBS \"\"\n");
        appendToFile("macros", "SYSTEM_CONSOLE_LIBS =\n");
        /* Handle dynamic libraries: */
        fprintf(versionFile, "#define CONSOLE_DLL");
        for (nameIndex = 0;
             nameIndex < sizeof(dllNameList) / sizeof(char *);
             nameIndex++) {
          fprintf(logFile, "\rConsole: DLL / Shared library: %s\n", dllNameList[nameIndex]);
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
    const char *dllNameList[] = {"libmysqlclient.so"};
#elif LIBRARY_TYPE == MACOS_LIBRARIES
    const char *dllNameList[] = {"libmysqlclient.dylib"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *dllNameList[] = {"libmariadb.dll", "libmysql.dll"};
#endif
    const char *inclDirList[] = {"/include"};
    const char *libDirList[] = {"/lib"};
    const char *dllDirList[] = {"/lib"};
    int dirIndex;
    int nameIndex;
    int searchForLib = 1;
    char dirPath[BUFFER_SIZE];
    char filePath[BUFFER_SIZE];
    const char *programFilesX86 = NULL;
    const char *programFiles = NULL;
    char dbHome[BUFFER_SIZE];
    char includeOption[BUFFER_SIZE];
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
      if (findStaticLib("MySql/MariaDb", testProgram, includeOption, dbHome,
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
      } /* if */
    } /* if */
#endif
    if (searchForLib) {
      /* Handle dynamic libraries: */
      fprintf(versionFile, "#define MYSQL_DLL");
      if (dbHomeExists) {
        listDynamicLibs("MySql/MariaDb", dbHome,
                        dllDirList, sizeof(dllDirList) / sizeof(char *),
                        dllNameList, sizeof(dllNameList) / sizeof(char *), versionFile);
      } /* if */
      for (nameIndex = 0; nameIndex < sizeof(dllNameList) / sizeof(char *); nameIndex++) {
        fprintf(logFile, "\rMySql/MariaDb: DLL / Shared library: %s\n", dllNameList[nameIndex]);
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
    const char *dllNameList[] = {"libsqlite3.so"};
#elif LIBRARY_TYPE == MACOS_LIBRARIES
    const char *dllNameList[] = {"libsqlite3.dylib"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *dllNameList[] = {"sqlite3.dll"};
#endif
    const char *libDirList[] = {""};
    const char *dllDirList[] = {""};
    int dirIndex;
    int nameIndex;
    int searchForLib = 1;
    char dirPath[BUFFER_SIZE];
    char filePath[BUFFER_SIZE];
    char dbHome[BUFFER_SIZE];
    char includeOption[BUFFER_SIZE];
    const char *sqliteInclude = NULL;
    char testProgram[BUFFER_SIZE];
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
    } else if (compileAndLinkWithOptionsOk("#include <sqlite3.h>\n"
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
    if (sqliteInclude != NULL) {
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
      if (findStaticLib("SQLite", testProgram, includeOption, dbHome,
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
      } /* if */
    } /* if */
#endif
    if (searchForLib) {
      /* Handle dynamic libraries: */
      fprintf(versionFile, "#define SQLITE_DLL");
      if (dbHomeExists) {
        listDynamicLibs("SQLite", dbHome,
                        dllDirList, sizeof(dllDirList) / sizeof(char *),
                        dllNameList, sizeof(dllNameList) / sizeof(char *), versionFile);
      } /* if */
      for (nameIndex = 0; nameIndex < sizeof(dllNameList) / sizeof(char *); nameIndex++) {
        fprintf(logFile, "\rSQLite: DLL / Shared library: %s\n", dllNameList[nameIndex]);
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
    int idx;
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
    char includeDir[BUFFER_SIZE];
    char pgTypeFileName[BUFFER_SIZE];
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



static void determinePostgresDefines (FILE *versionFile,
    char *include_options, char *system_database_libs)

  {
    const char *dbVersion[] = {"12", "11", "10",
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
    const char *libDirList[] = {"/lib"};
    const char *dllDirList[] = {"/lib", "/bin"};
    const char *libIntlDllList[] = {"libintl.dll", "libintl-8.dll", "libintl-9.dll"};
    const char *libeay32DllList[] = {"libeay32.dll"};
    int dirIndex;
    int nameIndex;
    int searchForLib = 1;
    char dirPath[BUFFER_SIZE];
    char filePath[BUFFER_SIZE];
    const char *programFilesX86 = NULL;
    const char *programFiles = NULL;
    char dbHome[BUFFER_SIZE];
    char includeOption[BUFFER_SIZE];
    char serverIncludeOption[BUFFER_SIZE];
    const char *postgresqlInclude = NULL;
    const char *postgresInclude = NULL;
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
      /* fprintf(logFile, "includeOption=%s\n", includeOption); */
      if (compileAndLinkWithOptionsOk("#include \"libpq-fe.h\"\n"
                                      "int main(int argc,char *argv[]){"
                                      "PGconn *connection; return 0;}\n",
                                      includeOption, "")) {
        postgresqlInclude = "libpq-fe.h";
        fprintf(logFile, "\rPostgreSQL: %s found in %s/include\n", postgresqlInclude, dbHome);
        sprintf(dirPath, "%s/include/server", dbHome);
        if (fileIsDir(dirPath)) {
          sprintf(filePath, "%s/include/server/catalog/pg_type.h", dbHome);
        } else {
          sprintf(filePath, "%s/include/catalog/pg_type.h", dbHome);
        } /* if */
        if (extractPostgresOid(filePath)) {
          pgTypeInclude = "pg_type.h";
          fprintf(logFile, "\rPostgreSQL: %s found in Seed7 directory.\n", pgTypeInclude);
        } /* if */
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
      if (compileAndLinkWithOptionsOk("#include <server/postgres.h>\n"
                                      "int main(int argc,char *argv[]){"
                                      "return 0;}\n",
                                      includeOption, "")) {
        postgresInclude = "server/postgres.h";
        pgTypeInclude = "server/catalog/pg_type.h";
      } else {
        appendOption(includeOption, serverIncludeOption);
        if (compileAndLinkWithOptionsOk("#include <server/postgres.h>\n"
                                        "int main(int argc,char *argv[]){"
                                        "return 0;}\n",
                                        includeOption, "")) {
          postgresInclude = "server/postgres.h";
          pgTypeInclude = "server/catalog/pg_type.h";
        } else {
          postgresInclude = "postgres.h";
          pgTypeInclude = "catalog/pg_type.h";
        } /* if */
      } /* if */
      sprintf(testProgram, "#include <%s>\n#include <%s>\n"
                           "int main(int argc,char *argv[]){"
                           "return 0;}\n",
                           postgresInclude, pgTypeInclude);
      if (compileAndLinkWithOptionsOk(testProgram, includeOption, "")) {
        fprintf(logFile, "\rPostgreSQL: %s found in system include directory.\n", pgTypeInclude);
      } else if (findPgTypeInclude(includeOption, pgTypeInclude)) {
        pgTypeInclude = "pg_type.h";
        fprintf(logFile, "\rPostgreSQL: %s found in Seed7 include directory.\n", pgTypeInclude);
      } else {
        fprintf(logFile, "\rPostgreSQL: %s not found in include directories.\n", pgTypeInclude);
        pgTypeInclude = NULL;
      } /* if */
      sprintf(testProgram, "#include <%s>\n"
                           "int main(int argc,char *argv[]){"
                           "return 0;}\n",
                           postgresInclude);
      if (compileAndLinkWithOptionsOk(testProgram, includeOption, "")) {
        fprintf(logFile, "\rPostgreSQL: %s found in system include directory.\n", postgresInclude);
      } else {
        fprintf(logFile, "\rPostgreSQL: %s not found in include directories.\n", postgresInclude);
        postgresInclude = NULL;
      } /* if */
    } /* if */
    if ((postgresqlInclude == NULL || postgresInclude == NULL || pgTypeInclude == NULL) &&
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
    if (postgresInclude != NULL) {
      fprintf(versionFile, "#define POSTGRESQL_POSTGRES_H \"%s\"\n", postgresInclude);
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
      if (findStaticLib("PostgreSQL", testProgram, includeOption, dbHome,
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
      } /* if */
    } /* if */
#endif
    if (searchForLib) {
      /* Handle dynamic libraries: */
      fprintf(versionFile, "#define POSTGRESQL_DLL");
      if (dbHomeExists) {
        listDynamicLibs("PostgreSQL", dbHome,
                        dllDirList, sizeof(dllDirList) / sizeof(char *),
                        dllNameList, sizeof(dllNameList) / sizeof(char *), versionFile);
      } /* if */
      for (nameIndex = 0; nameIndex < sizeof(dllNameList) / sizeof(char *); nameIndex++) {
        fprintf(logFile, "\rPostgreSQL: DLL / Shared library: %s\n", dllNameList[nameIndex]);
        fprintf(versionFile, " \"%s\",", dllNameList[nameIndex]);
      } /* for */
      fprintf(versionFile, "\n");
      fprintf(versionFile, "#define LIBINTL_DLL");
      if (dbHomeExists) {
        listDynamicLibs("PostgreSQL", dbHome,
                        dllDirList, sizeof(dllDirList) / sizeof(char *),
                        libIntlDllList, sizeof(libIntlDllList) / sizeof(char *), versionFile);
      } /* if */
      for (nameIndex = 0; nameIndex < sizeof(libIntlDllList) / sizeof(char *); nameIndex++) {
        fprintf(logFile, "\rPostgreSQL: DLL / Shared library: %s\n", libIntlDllList[nameIndex]);
        fprintf(versionFile, " \"%s\",", libIntlDllList[nameIndex]);
      } /* for */
      fprintf(versionFile, "\n");
      fprintf(versionFile, "#define LIBEAY32_DLL");
      if (dbHomeExists) {
        listDynamicLibs("PostgreSQL", dbHome,
                        dllDirList, sizeof(dllDirList) / sizeof(char *),
                        libeay32DllList, sizeof(libeay32DllList) / sizeof(char *), versionFile);
      } /* if */
      for (nameIndex = 0; nameIndex < sizeof(libeay32DllList) / sizeof(char *); nameIndex++) {
        fprintf(logFile, "\rPostgreSQL: DLL / Shared library: %s\n", libeay32DllList[nameIndex]);
        fprintf(versionFile, " \"%s\",", libeay32DllList[nameIndex]);
      } /* for */
      fprintf(versionFile, "\n");
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
    const char *dllNameList[] = {"libodbc.so"};
#elif LIBRARY_TYPE == MACOS_LIBRARIES
    const char *dllNameList[] = {"libiodbc.dylib"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *dllNameList[] = {"odbc32.dll"};
#endif
    int nameIndex;
    int searchForLib = 1;
    char includeOption[BUFFER_SIZE];
    int windowsOdbc = 0;
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
      windowsOdbc = 1;
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
      fprintf(versionFile, "#define WINDOWS_ODBC %d\n", windowsOdbc);
      fprintf(versionFile, "#define ODBC_INCLUDE \"%s\"\n", odbcInclude);
      fprintf(versionFile, "#define ODBC_INCLUDE_SQLEXT_H %d\n", includeSqlext);
      appendOption(include_options, includeOption);
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
                         windowsOdbc ? "#include \"windows.h\"\n" : "", odbcInclude,
                         includeSqlext ? "#include \"sqlext.h\"\n" : "");
    /* fprintf(logFile, "%s\n", testProgram);
       fprintf(logFile, "odbcInclude: \"%s\"\n", odbcInclude); */
    if (findLinkerOption("Odbc", testProgram, includeOption, ODBC_LIBRARY_PATH,
                         libNameList, sizeof(libNameList) / sizeof(char *),
                         system_database_libs)) {
      searchForLib = 0;
    } /* if */
#endif
    if (searchForLib) {
      /* Handle dynamic libraries: */
      fprintf(versionFile, "#define ODBC_DLL");
      for (nameIndex = 0; nameIndex < sizeof(dllNameList) / sizeof(char *); nameIndex++) {
        fprintf(logFile, "\rOdbc: DLL / Shared library: %s\n", dllNameList[nameIndex]);
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
    int dirIndex;
    int nameIndex;
    int searchForLib = 1;
    char dirPath[BUFFER_SIZE];
    char filePath[BUFFER_SIZE];
    char includeOption[BUFFER_SIZE];
    const char *ociInclude = NULL;
    char testProgram[BUFFER_SIZE];
    int dbHomeExists = 0;
    int found = 0;

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
      if (findStaticLib("Oracle", testProgram, includeOption, dbHome,
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
      } /* if */
    } /* if */
#endif
    if (searchForLib) {
      /* Handle dynamic libraries: */
      fprintf(versionFile, "#define OCI_DLL");
      if (dbHomeExists && rpath != NULL) {
        listDynamicLibs("Oracle", dbHome,
                        dllDirList, sizeof(dllDirList) / sizeof(char *),
                        dllNameList, sizeof(dllNameList) / sizeof(char *), versionFile);
      } /* if */
      for (nameIndex = 0; nameIndex < sizeof(dllNameList) / sizeof(char *); nameIndex++) {
        fprintf(logFile, "\rOracle: DLL / Shared library: %s\n", dllNameList[nameIndex]);
        fprintf(versionFile, " \"%s\",", dllNameList[nameIndex]);
      } /* for */
      fprintf(versionFile, "\n");
      if (dbHomeExists && rpath != NULL) {
        /* The oci library has many dependencies to other shared  */
        /* object libraries, which are in the directory of the    */
        /* main oci library. To dynamically link with ldopen() at */
        /* the run-time of the program either this directory must */
        /* be listed in the LD_LIBRARY_PATH or the -rpath option  */
        /* must be used. Note that the meaning of the -rpath      */
        /* option has changed and it must be combined with the    */
        /* option --disable-new-dtags to get the old behavior.    */
        for (dirIndex = 0; !found && dirIndex < sizeof(dllDirList) / sizeof(char *); dirIndex++) {
          sprintf(dirPath, "%s%s", dbHome, dllDirList[dirIndex]);
          if (fileIsDir(dirPath)) {
            for (nameIndex = 0; nameIndex < sizeof(dllNameList) / sizeof(char *); nameIndex++) {
              sprintf(filePath, "%s/%s", dirPath, dllNameList[nameIndex]);
              if (fileIsRegular(filePath)) {
                fprintf(logFile, "\rOracle: %s found in %s\n", dllNameList[nameIndex], dirPath);
                if (rpath[0] != '\0') {
                  strcat(rpath, ":");
                } /* if */
                strcat(rpath, dirPath);
                found = 1;
              } /* if */
            } /* for */
          } /* if */
        } /* for */
      } /* if */
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
    int dirIndex;
    int nameIndex;
    int searchForLib = 1;
    char dirPath[BUFFER_SIZE];
    char filePath[BUFFER_SIZE];
    const char *programFilesX86 = NULL;
    const char *programFiles = NULL;
    char dbHome[BUFFER_SIZE];
    char includeOption[BUFFER_SIZE];
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
    } /* if */
#endif
    if (searchForLib) {
      /* Handle dynamic libraries: */
      fprintf(versionFile, "#define FIRE_DLL");
      if (dbHomeExists) {
        listDynamicLibs("Firebird", dbHome,
                        dllDirList, sizeof(dllDirList) / sizeof(char *),
                        dllNameList, sizeof(dllNameList) / sizeof(char *), versionFile);
      } /* if */
      for (nameIndex = 0; nameIndex < sizeof(dllNameList) / sizeof(char *); nameIndex++) {
        fprintf(logFile, "\rFirebird: DLL / Shared library: %s\n", dllNameList[nameIndex]);
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
    int driverDirIndex;
    int dirIndex;
    int nameIndex;
    int searchForLib = 1;
    char dirPath[BUFFER_SIZE];
    char filePath[BUFFER_SIZE];
    char includeOption[BUFFER_SIZE];
    char makeDefinition[BUFFER_SIZE];
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
    } /* if */
#if !defined DB2_USE_DLL && defined SUPPORTS_PARTIAL_LINKING
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
      if (findStaticLib("DB2", testProgram, includeOption, dbHome,
                        libDirList, sizeof(libDirList) / sizeof(char *),
                        libNameList, sizeof(libNameList) / sizeof(char *),
                        db2_libs)) {
        sprintf(makeDefinition, "DB2_LIBS = %s", db2_libs);
        replaceNLBySpace(makeDefinition);
        strcat(makeDefinition, "\n");
        appendToFile("macros", makeDefinition);
        searchForLib = 0;
      } /* if */
    } /* if */
    if (searchForLib) {
      if (findLinkerOption("DB2", testProgram, includeOption, DB2_LIBRARY_PATH,
                           libNameList, sizeof(libNameList) / sizeof(char *),
                           db2_libs)) {
        sprintf(makeDefinition, "DB2_LIBS = %s", db2_libs);
        replaceNLBySpace(makeDefinition);
        strcat(makeDefinition, "\n");
        appendToFile("macros", makeDefinition);
        searchForLib = 0;
      } /* if */
    } /* if */
#endif
    if (searchForLib) {
      /* Handle dynamic libraries: */
      fprintf(versionFile, "#define DB2_DLL");
      for (nameIndex = 0; nameIndex < sizeof(dllNameList) / sizeof(char *); nameIndex++) {
        fprintf(logFile, "\rDB2: DLL / Shared library: %s\n", dllNameList[nameIndex]);
        fprintf(versionFile, " \"%s\",", dllNameList[nameIndex]);
      } /* for */
      fprintf(versionFile, "\n");
    } /* if */
  } /* determineDb2Defines */



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
    const char *dllNameList[] = {"libtdsodbc.so"};
#elif LIBRARY_TYPE == MACOS_LIBRARIES
    const char *dllNameList[] = {"libtdsodbc.dylib"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    /* sqlncli11.dll is omitted, because it truncates fields. */
    const char *dllNameList[] = {"sqlsrv32.dll"};
#endif
    int nameIndex;
    int searchForLib = 1;
    char includeOption[BUFFER_SIZE];
    char makeDefinition[BUFFER_SIZE];
    int freeTdsLibrary = 0;
    int windowsSqlServer = 0;
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
      windowsSqlServer = 1;
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
      fprintf(versionFile, "#define WINDOWS_SQL_SERVER %d\n", windowsSqlServer);
      fprintf(versionFile, "#define SQL_SERVER_INCLUDE \"%s\"\n", sqlServerInclude);
      fprintf(versionFile, "#define SQL_SERVER_INCLUDE_SQLEXT_H %d\n", includeSqlext);
      fprintf(versionFile, "#define SQL_SERVER_INCLUDE_OPTION \"");
      escapeString(versionFile, includeOption);
      fprintf(versionFile, "\"\n");
      sprintf(makeDefinition, "SQL_SERVER_INCLUDE_OPTION = %s\n", includeOption);
      appendToFile("macros", makeDefinition);
      fprintf(versionFile, "#define FREETDS_SQL_SERVER_CONNECTION %d\n", freeTdsLibrary);
      fprintf(versionFile, "#define SPECIFY_SQL_SERVER_PORT_EXPLICIT %d\n", freeTdsLibrary);
    } /* if */
#if !defined SQL_SERVER_USE_DLL && defined SUPPORTS_PARTIAL_LINKING
    /* Handle static libraries: */
    sprintf(testProgram, "#include \"tst_vers.h\"\n#include<stdio.h>\n"
                         "%s#include \"%s\"\n%s"
                         "int main(int argc,char *argv[]){\n"
                         "SQLHENV sql_env;\n"
                         "SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sql_env);\n"
                         "SQLFreeHandle(SQL_HANDLE_ENV, sql_env);\n"
                         "printf(\"1\\n\");\n"
                         "return 0;\n}\n",
                         windowsSqlServer ? "#include \"windows.h\"\n" : "", sqlServerInclude,
                         includeSqlext ? "#include \"sqlext.h\"\n" : "");
    /* fprintf(logFile, "%s\n", testProgram);
       fprintf(logFile, "sqlServerInclude: \"%s\"\n", sqlServerInclude); */
    sql_server_libs[0] = '\0';
    if (findLinkerOption("SQL Server", testProgram, includeOption, SQL_SERVER_LIBRARY_PATH,
                         libNameList, sizeof(libNameList) / sizeof(char *),
                         sql_server_libs)) {
      sprintf(makeDefinition, "SQL_SERVER_LIBS = %s", sql_server_libs);
      replaceNLBySpace(makeDefinition);
      strcat(makeDefinition, "\n");
      appendToFile("macros", makeDefinition);
      searchForLib = 0;
    } /* if */
#endif
    if (searchForLib) {
      /* Handle dynamic libraries: */
      fprintf(versionFile, "#define SQL_SERVER_DLL");
      for (nameIndex = 0; nameIndex < sizeof(dllNameList) / sizeof(char *); nameIndex++) {
        fprintf(logFile, "\rSQL Server: DLL / Shared library: %s\n", dllNameList[nameIndex]);
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
    const char *dllNameList[] = {"libsybdb.so"};
#elif LIBRARY_TYPE == MACOS_LIBRARIES
    const char *dllNameList[] = {"libsybdb.dylib"};
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    const char *dllNameList[] = {"sybdb.dll"};
#endif
    int nameIndex;
    int searchForLib = 1;
    char includeOption[BUFFER_SIZE];
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
    } /* if */
#endif
    if (searchForLib) {
      /* Handle dynamic libraries: */
      fprintf(versionFile, "#define TDS_DLL");
      for (nameIndex = 0; nameIndex < sizeof(dllNameList) / sizeof(char *); nameIndex++) {
        fprintf(logFile, "\rTDS: DLL / Shared library: %s\n", dllNameList[nameIndex]);
        fprintf(versionFile, " \"%s\",", dllNameList[nameIndex]);
      } /* for */
      fprintf(versionFile, "\n");
    } /* if */
  } /* determineTdsDefines */



static void determineBigIntDefines (FILE *versionFile,
    char *include_options, char *additional_system_libs)

  {
    char *gmpLinkerOption;
    char linkerOptions[BUFFER_SIZE];

  /* determineBigIntDefines */
#if !defined BIGINT_LIB || BIGINT_LIB != BIG_RTL_LIBRARY
#ifdef BIGINT_LIBS
    gmpLinkerOption = BIGINT_LIBS;
#else
    gmpLinkerOption = "-lgmp";
#endif
    linkerOptions[0] = '\0';
#ifdef LINKER_OPT_STATIC_LINKING
    appendOption(linkerOptions, LINKER_OPT_STATIC_LINKING);
#endif
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
      fputs("#define BIGINT_LIB BIG_GMP_LIBRARY\n", versionFile);
      appendOption(additional_system_libs, gmpLinkerOption);
    } else {
      fputs("#define BIG_RTL_LIBRARY 1\n", versionFile);
      fputs("#define BIGINT_LIB BIG_RTL_LIBRARY\n", versionFile);
    } /* if */
#endif
  } /* determineBigIntDefines */



static void determineIncludesAndLibs (FILE *versionFile)

  {
    char include_options[BUFFER_SIZE];
    char system_database_libs[BUFFER_SIZE];
    char additional_system_libs[BUFFER_SIZE];
    char rpath_buffer[BUFFER_SIZE];
    char *rpath = NULL;
    char rpathOption[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

  /* determineIncludesAndLibs */
#if LIBRARY_TYPE == UNIX_LIBRARIES
    fprintf(logFile, "\rUsing Linux/Unix/BSD libraries\n");
#elif LIBRARY_TYPE == MACOS_LIBRARIES
    fprintf(logFile, "\rUsing Mac OS libraries\n");
#elif LIBRARY_TYPE == WINDOWS_LIBRARIES
    fprintf(logFile, "\rUsing Windows libraries\n");
#endif
    include_options[0] = '\0';
    system_database_libs[0] = '\0';
    additional_system_libs[0] = '\0';
    if (linkerOptionAllowed("-Wl,--disable-new-dtags")) {
      rpath_buffer[0] = '\0';
      rpath = rpath_buffer;
    } /* if */
#if LIBRARY_TYPE == UNIX_LIBRARIES || LIBRARY_TYPE == MACOS_LIBRARIES
    determineX11Defines(versionFile, include_options);
    determineConsoleDefines(versionFile, include_options);
#endif
    determineMySqlDefines(versionFile, include_options, system_database_libs);
    determineSqliteDefines(versionFile, include_options, system_database_libs);
    determinePostgresDefines(versionFile, include_options, system_database_libs);
    determineOdbcDefines(versionFile, include_options, system_database_libs);
    determineOciDefines(versionFile, include_options, system_database_libs, rpath);
    determineFireDefines(versionFile, include_options, system_database_libs);
    determineDb2Defines(versionFile, include_options, system_database_libs);
    determineSqlServerDefines(versionFile, include_options, system_database_libs);
    determineTdsDefines(versionFile, include_options, system_database_libs);
    determineBigIntDefines(versionFile, include_options, additional_system_libs);
    if (rpath != NULL && rpath[0] != '\0') {
      sprintf(rpathOption, "-Wl,--disable-new-dtags,-rpath=%s", rpath);
      appendOption(system_database_libs, rpathOption);
    } /* if */
    sprintf(buffer, "INCLUDE_OPTIONS = %s", include_options);
    replaceNLBySpace(buffer);
    strcat(buffer, "\n");
    appendToFile("macros", buffer);
    sprintf(buffer, "SYSTEM_DATABASE_LIBS = %s", system_database_libs);
    replaceNLBySpace(buffer);
    strcat(buffer, "\n");
    appendToFile("macros", buffer);
    sprintf(buffer, "ADDITIONAL_SYSTEM_LIBS = %s", additional_system_libs);
    replaceNLBySpace(buffer);
    strcat(buffer, "\n");
    appendToFile("macros", buffer);
    fprintf(versionFile, "#define INCLUDE_OPTIONS \"");
    escapeString(versionFile, include_options);
    fprintf(versionFile, "\"\n");
    fprintf(versionFile, "#define SYSTEM_DATABASE_LIBS \"");
    escapeString(versionFile, system_database_libs);
    fprintf(versionFile, "\"\n");
    fprintf(versionFile, "#define ADDITIONAL_SYSTEM_LIBS \"");
    escapeString(versionFile, additional_system_libs);
    fprintf(versionFile, "\"\n");
  } /* determineIncludesAndLibs */



static void writeReadBufferEmptyMacro (FILE *versionFile)

  {
    const char *define_read_buffer_empty;
    int offset_to_count;
    char macro_buffer[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

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
      printf("%s\n", define_read_buffer_empty);
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
    int driveLetters;

  /* main */
    if (!fileIsRegular("settings.h")) {
      fprintf(stdout, "\n *** fileIsRegular(\"%s\") fails.\n", "settings.h");
    } /* if */
    if (argc >= 2) {
      versionFileName = argv[1];
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
    fprintf(logFile, "done\n");
    determineOptionForLinkTimeOptimization(versionFile);
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
    if (assertCompAndLnk("#include <stdio.h>\nint main(int argc, char *argv[])\n"
                         "{printf(\"%d\\n\", EOF == -1); return 0;}\n")) {
      fprintf(versionFile, "#define EOF_IS_MINUS_ONE %d\n", doTest());
    } /* if */
    fprintf(versionFile, "#define EMPTY_STRUCTS_ALLOWED %d\n",
            compileAndLinkOk("#include <stdio.h>\n"
                             "typedef struct emptyStruct { } emptyRecord;\n"
                             "int main(int argc, char *argv[]){\n"
                             "return 0;}\n"));
    checkForLimitedStringLiteralLength(versionFile);
    checkForLimitedArrayLiteralLength(versionFile);
    checkForSwitchWithInt64Type(versionFile);
    determineStackDirection(versionFile);
    determineLanguageProperties(versionFile);
    determinePreprocessorProperties(versionFile);
#ifndef STACK_SIZE
    if (sizeof(char *) == 8) { /* Machine with 64-bit addresses */
      /* Due to alignment some 64-bit machines have huge stack requirements. */
      fputs("#define STACK_SIZE 0x1000000\n", versionFile); /* 16777216 bytes */
    } else {
      fputs("#define STACK_SIZE 0x800000\n", versionFile); /* 8388608 bytes */
    } /* if */
#endif
    localtimeProperties(versionFile);
    /* Make sure that the file version.h up to this position is copied to tst_vers.h. */
    closeVersionFile(versionFile);
    copyFile(versionFileName, "tst_vers.h");
    versionFile = openVersionFile(versionFileName);
    determineGetaddrlimit(versionFile);
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
                         "wchar_t stri1[3], stri2[3];\n"
                         "memcpy(stri1, str1, 2 * sizeof(wchar_t));\n"
                         "memcpy(stri2, str2, 2 * sizeof(wchar_t));\n"
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
    fprintf(versionFile, "#define HAS_GETGRNAM_R %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <sys/types.h>\n"
                         "#include <grp.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "int funcRes; struct group grp;\n"
                         "char buffer[2048]; struct group *grpResult;\n"
                         "funcRes = getgrnam_r(\"root\", &grp,\n"
                         "buffer, sizeof(buffer), &grpResult);\n"
                         "printf(\"%d\\n\", funcRes==0 && grpResult==&grp);\n"
                         "return 0;}\n") && doTest() == 1);
    fprintf(versionFile, "#define HAS_GETGRNAM %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <sys/types.h>\n"
                         "#include <grp.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "struct group *grpResult;\n"
                         "grpResult = getgrnam(\"root\");\n"
                         "printf(\"%d\\n\", grpResult!=NULL);\n"
                         "return 0;}\n") && doTest() == 1);
    fprintf(versionFile, "#define HAS_GETPWUID_R %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <sys/types.h>\n"
                         "#include <pwd.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "int funcRes; struct passwd pwd;\n"
                         "char buffer[2048]; struct passwd *pwdResult;\n"
                         "funcRes = getpwuid_r((uid_t) 0, &pwd,\n"
                         "buffer, sizeof(buffer), &pwdResult);\n"
                         "printf(\"%d\\n\", funcRes==0 && pwdResult==&pwd);\n"
                         "return 0;}\n") && doTest() == 1);
    fprintf(versionFile, "#define HAS_GETPWUID %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <sys/types.h>\n"
                         "#include <pwd.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "struct passwd *pwdResult;\n"
                         "pwdResult = getpwuid((uid_t) 0);\n"
                         "printf(\"%d\\n\", pwdResult!=NULL);\n"
                         "return 0;}\n") && doTest() == 1);
    fprintf(versionFile, "#define HAS_GETPWNAM_R %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <sys/types.h>\n"
                         "#include <pwd.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "int funcRes; struct passwd pwd;\n"
                         "char buffer[2048]; struct passwd *pwdResult;\n"
                         "funcRes = getpwnam_r(\"root\", &pwd,\n"
                         "buffer, sizeof(buffer), &pwdResult);\n"
                         "printf(\"%d\\n\", funcRes==0 && pwdResult==&pwd);\n"
                         "return 0;}\n") && doTest() == 1);
    fprintf(versionFile, "#define HAS_GETPWNAM %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <sys/types.h>\n"
                         "#include <pwd.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "struct passwd *pwdResult;\n"
                         "pwdResult = getpwnam(\"root\");\n"
                         "printf(\"%d\\n\", pwdResult!=NULL);\n"
                         "return 0;}\n") && doTest() == 1);
    fprintf(versionFile, "#define HAS_SETJMP %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <setjmp.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "jmp_buf env; int ret_code = 4; int count = 2;\n"
                         "if ((ret_code=setjmp(env)) == 0) {\n"
                         "count--; longjmp(env, count); printf(\"3\\n\");\n"
                         "} else printf(\"%d\\n\", ret_code);\n"
                         "return 0;}\n") && doTest() == 1);
    fprintf(versionFile, "#define HAS_SIGSETJMP %d\n",
        compileAndLinkOk("#include <stdio.h>\n#include <setjmp.h>\n"
                         "int main(int argc, char *argv[]){\n"
                         "sigjmp_buf env; int ret_code = 4; int count = 2;\n"
                         "if ((ret_code=sigsetjmp(env, 1)) == 0) {\n"
                         "count--; siglongjmp(env, count); printf(\"3\\n\");\n"
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
    cleanUpCompilation(testNumber);
    fprintf(versionFile, "#define REMOVE_REATTEMPTS %lu\n", removeReattempts);
    closeVersionFile(versionFile);
    if (fileIsRegular("tst_vers.h")) {
      remove("tst_vers.h");
    } /* if */
    fprintf(logFile, " \b");
    return 0;
  } /* main */

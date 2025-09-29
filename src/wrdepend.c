/********************************************************************/
/*                                                                  */
/*  wrdepend.c    Write the C header file dependencies.             */
/*  Copyright (C) 2014  Thomas Mertes                               */
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
/*  Module: Wrdepend                                                */
/*  File: seed7/src/wrdepend.c                                      */
/*  Changes: 2014  Thomas Mertes                                    */
/*  Content: Write the C header file dependencies.                  */
/*                                                                  */
/********************************************************************/

#include "version.h"

/**
 *  From version.h the following defines are used (for details see: read_me.txt):
 *
 *  C_COMPILER:
 *      Contains the command to call the stand-alone C compiler and linker.
 *  INCLUDE_OPTIONS:
 *      Options to specify include directories.
 *  DB2_INCLUDE_OPTION:
 *      Option to specify the include directory of DB2.
 *  INFORMIX_INCLUDE_OPTION:
 *      Option to specify the include directory of Informix.
 *  SQL_SERVER_INCLUDE_OPTION:
 *      Option to specify the include directory of SQL Server.
 */

#include "stdlib.h"
#include "string.h"
#include "stdio.h"

#include "config.h"

/**
 *  From config.h the following defines are used (for details see: read_me.txt):
 *
 *  MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
 *      Map absolute paths to operating system paths with drive letter.
 */


#define COMMAND_SIZE 16384

static char c_compiler[COMMAND_SIZE];



static void prepareCompileCommand (void)

  {
    int pos;
    int quote_command = 0;
    int len;

  /* prepareCompileCommand */
    strcpy(c_compiler, C_COMPILER);
#if MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
    if (c_compiler[0] == '/') {
      c_compiler[0] = c_compiler[1];
      c_compiler[1] = ':';
    } /* if */
#endif
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



static void checkForLevelH (void)

  {
    FILE *levelFile;
    FILE *levelRlFile;
    char buffer1[16];
    char buffer2[16];
    long level;

  /* checkForLevelH */
    levelFile = fopen("level.h", "r");
    if (levelFile != NULL) {
      fclose(levelFile);
    } else {
      levelRlFile = fopen("level_rl.h", "r");
      if (levelRlFile != NULL) {
        if (fscanf(levelRlFile, "%15s %15s %ld\n",
                   buffer1, buffer2, &level) == 3 &&
            strcmp(buffer1, "#define") == 0 &&
            strcmp(buffer2, "LEVEL") == 0) {
          levelFile = fopen("level.h", "wb");
          if (levelFile != NULL) {
            fprintf(levelFile, "#define LEVEL %ld\n", level);
            fclose(levelFile);
          } /* if */
        } /* if */
        fclose(levelRlFile);
      } /* if */
    } /* if */
  } /* checkForLevelH */



const char *writeOption (const char *option, char *command)

  {
    const char *nlPtr;
    unsigned long length;
    unsigned long cmdLen;
    const char *nextOption;

  /* writeOption */
    nlPtr = strchr(option, '\n');
    if (nlPtr != NULL) {
      length = nlPtr - option;
      nextOption = nlPtr + 1;
    } else {
      length = strlen(option);
      nlPtr = &option[length];
      nextOption = NULL;
    } /* if */
    if (length != 0) {
      strcat(command, " ");
      cmdLen = strlen(command);
      memcpy(&command[cmdLen], option, length);
      command[cmdLen + length] = '\0';
    } /* if */
    return nextOption;
  } /* writeOption */



void writeOptionList (const char *optionList, char *command)

  { /* writeOptionList */
    do {
      optionList = writeOption(optionList, command);
    } while (optionList != NULL);
  } /* writeOptionList */



/**
 *  Program to write the C header file dependencies.
 */
int main (int argc, char **argv)

  {
    int idx = 1;
    char *curr_arg;
    char *include_option;
    char command[COMMAND_SIZE];

  /* main */
    prepareCompileCommand();
    sprintf(command, "%s", c_compiler);
    checkForLevelH();
    curr_arg = argv[idx];
    if (memcmp(curr_arg, "OPTION=", 7 * sizeof(char)) == 0 &&
        (curr_arg)[7] != '\0') {
      /* fprintf(stderr, "arg[%d]: %s\n", idx, curr_arg); */
      include_option = &(curr_arg)[7];
      if (strcmp(include_option, "DB2_INCLUDE_OPTION") == 0 &&
          DB2_INCLUDE_OPTION[0] != '\0') {
        writeOptionList(DB2_INCLUDE_OPTION, command);
      } else if (strcmp(include_option, "INFORMIX_INCLUDE_OPTION") == 0 &&
          INFORMIX_INCLUDE_OPTION[0] != '\0') {
        writeOptionList(INFORMIX_INCLUDE_OPTION, command);
      } else if (strcmp(include_option, "SQL_SERVER_INCLUDE_OPTION") == 0 &&
          SQL_SERVER_INCLUDE_OPTION[0] != '\0') {
        writeOptionList(SQL_SERVER_INCLUDE_OPTION, command);
      } else if (strcmp(include_option, "INCLUDE_OPTIONS") == 0 &&
          INCLUDE_OPTIONS[0] != '\0') {
        writeOptionList(INCLUDE_OPTIONS, command);
      } /* if */
      idx++;
    } /* if */
    for (; idx < argc; idx++) {
      curr_arg = argv[idx];
      strcat(command, " ");
      if (strchr(curr_arg, ' ') != NULL && curr_arg[0] != '>') {
        strcat(command, "\"");
        strcat(command, curr_arg);
        strcat(command, "\"");
      } else {
        strcat(command, curr_arg);
      } /* if */
      /* fprintf(stderr, "arg[%d]: %s\n", idx, curr_arg); */
    } /* for */
    /* fprintf(stderr, "command: %s\n", command); */
    system(command);
    return 0;
  } /* main */

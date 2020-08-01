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
    int idx;
    char command[COMMAND_SIZE];

  /* main */
    prepareCompileCommand();
    sprintf(command, "%s", c_compiler);
    writeOptionList(INCLUDE_OPTIONS, command);
    for (idx = 1; idx < argc; idx++) {
      strcat(command, " ");
      if (strchr(argv[idx], ' ') != NULL && argv[idx][0] != '>') {
        strcat(command, "\"");
        strcat(command, argv[idx]);
        strcat(command, "\"");
      } else {
        strcat(command, argv[idx]);
      } /* if */
      /* printf("arg[%d]: %s\n", idx, argv[idx]); */
    } /* for */
    /* printf("command: %s\n", command); */
    system(command);
    return 0;
  } /* main */

/********************************************************************/
/*                                                                  */
/*  wrdepend   Write the C header file dependencies.                */
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
/*  Module: Chkccomp                                                */
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


char c_compiler[1024];



void prepareCompileCommand (void)

  {
    int pos;
    int quote_command = 0;
    int len;

  /* prepareCompileCommand */
    strcpy(c_compiler, C_COMPILER);
#ifdef MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
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



/**
 *  Program to write the C header file dependencies.
 */
int main (int argc, char **argv)

  {
    int idx;
    char command[8192];

  /* main */
    prepareCompileCommand();
    sprintf(command, "%s %s", c_compiler, INCLUDE_OPTIONS);
    for (idx = 1; idx < argc; idx++) {
      strcat(command, " ");
      strcat(command, argv[idx]);
    } /* for */
    system(command);
    return 0;
  } /* main */

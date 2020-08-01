/********************************************************************/
/*                                                                  */
/*  sudo.c        Execute command as administrator under Windows.   */
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
/*  Module: Sudo                                                    */
/*  File: seed7/src/sudo.c                                          */
/*  Changes: 2014  Thomas Mertes                                    */
/*  Content: Execute command as administrator under Windows.        */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "windows.h"
#include "shellapi.h"


int main (int argc, char *argv[])

  {
    int length = 0;
    char *parameters;
    int idx;
    int mainResult = 0;

  /* main */
    if (argc < 2) {
      printf("usage: sudo command [parameters]\n");
    } else {
      for (idx = 2; idx < argc; idx++) {
        length += strlen(argv[idx]) + 1;
      } /* for */
      if (length > 0) {
        length--;
      } /* if */
      parameters = (char *) malloc(length + 1);
      if (parameters == NULL) {
        mainResult = -1;
      } else {
        parameters[0] = '\0';
        if (argc > 2) {
          strcat(parameters, argv[2]);
          for (idx = 3; idx < argc; idx++) {
            strcat(parameters, " ");
            strcat(parameters, argv[idx]);
          } /* for */
        } /* if */
        printf("%s %s\n", argv[1], parameters);
        if (ShellExecute(NULL, "runas", argv[1], parameters, NULL, SW_HIDE) <= 32) {
          mainResult = -1;
        } /* if */
        free(parameters);
      } /* if */
    } /* if */
    return mainResult;
  } /* main */

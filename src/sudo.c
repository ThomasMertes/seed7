/********************************************************************/
/*                                                                  */
/*  sudo.c        Execute command as administrator under Windows.   */
/*  Copyright (C) 2014, 2016, 2018, 2026  Thomas Mertes             */
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
/*  Changes: 2014, 2016, 2018, 2026  Thomas Mertes                  */
/*  Content: Execute command as administrator under Windows.        */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "windows.h"
#include "shellapi.h"

/* To support older compilers SIZE_MAX is not used. */
#define SIZE_T_MAXIMUM ((size_t) -1)


static size_t estimateQuotedLength (char *sourceChar)

  {
    size_t length = 3; /* a leading space + 2 surrounding quotes */
                       /* The caller might add a leading space. */

  /* estimateQuotedLength */
    while (*sourceChar != '\0') {
      if (*sourceChar == '\\' || *sourceChar == '"') {
        /* Escape embedded quotes and possibly also backslashes. */
        if (length < SIZE_T_MAXIMUM) {
          length++;
        } /* if */
      } /* if */
      if (length < SIZE_T_MAXIMUM) {
        length++;
      } /* if */
      sourceChar++;
    } /* while */
    return length;
  } /* estimateQuotedLength */


static char *copyQuotedPart (char *sourceChar, char *destChar)

  {
    size_t numberOfBackslashes;

  /* copyQuotedPart */
    *destChar++ = '"';
    while (*sourceChar != '\0') {
      if (*sourceChar == '\\') {
        sourceChar++;
        numberOfBackslashes = 1;
        while (*sourceChar == '\\') {
          sourceChar++;
          numberOfBackslashes++;
        } /* while */
        if (*sourceChar == '"' || *sourceChar == '\0') {
          numberOfBackslashes <<= 1;
        } /* if */
        do {
          *destChar++ = '\\';
          numberOfBackslashes--;
        } while (numberOfBackslashes != 0);
      } else {
        if (*sourceChar == '"') {
          *destChar++ = '\\';
        } /* if */
        *destChar++ = *sourceChar++;
      } /* if */
    } /* while */
    *destChar++ = '"';
    return destChar;
  } /* copyQuotedPart */


/**
 *  Execute a command as administrator.
 *  This program can be used to install Seed7 with 'sudo make install'.
 *  It has been created only for this purpose.
 *  Using this program for other purposes is not suggested.
 */
int main (int argc, char *argv[])

  {
    size_t paramLength;
    size_t parametersLength = 0;
    char *parameters;
    char *destChar;
    int idx;
    size_t cwdBufferSize;
    size_t actualSize;
    char *cwd = NULL;
    int returnValue;
    int mainResult = 0;

  /* main */
    if (argc < 2) {
      printf("usage: sudo command [parameters]\n");
    } else {
      /* Estimate the total length of the quoted parameters. */
      for (idx = 2; idx < argc; idx++) {
        paramLength = estimateQuotedLength(argv[idx]);
        if (parametersLength <= SIZE_T_MAXIMUM - paramLength) {
          parametersLength += paramLength;
        } else {
          parametersLength = SIZE_T_MAXIMUM;
        } /* if */
      } /* for */
      if (parametersLength > 0) {
        parametersLength--; /* Remove the leading space of the first parameter. */
      } /* if */
      if (parametersLength == SIZE_T_MAXIMUM ||
          (parameters = (char *) malloc(parametersLength + 1)) == NULL) {
        printf("sudo: out of memory\n");
        mainResult = 1;
      } else if ((cwdBufferSize = (size_t) GetCurrentDirectoryA(0, NULL)) == 0) {
        printf("sudo: cannot determine current working directory\n");
        mainResult = 1;
      } else if ((cwd = (char *) malloc(cwdBufferSize)) == NULL) {
        printf("sudo: out of memory\n");
        mainResult = 1;
      } else if ((actualSize = GetCurrentDirectoryA(cwdBufferSize, cwd)) == 0 ||
                  actualSize >= cwdBufferSize) {
        printf("sudo: cannot determine current working directory\n");
        mainResult = 1;
      } else {
        destChar = parameters;
        for (idx = 2; idx < argc; idx++) {
          if (idx > 2) {
            *destChar++ = ' ';
          } /* if */
          destChar = copyQuotedPart(argv[idx], destChar);
        } /* for */
        *destChar = '\0';
        printf("%s %s\n", argv[1], parameters);
        /* The result type of ShellExecuteA() is an HINSTANCE for   */
        /* backward compatibility with 16-bit Windows applications. */
        /* It is not a true HINSTANCE, however. It can be cast only */
        /* to an int and compared to either 32 or an error code.    */
        returnValue = (int) ShellExecuteA(NULL, "runas", argv[1],
                                          argc == 2 ? NULL : parameters,
                                          cwd, SW_HIDE);
        /* printf("returnValue: %d\n", returnValue); */
        if (returnValue <= 32) {
          /* The function ShellExecuteA() failed. */
          /* Try to execute the program without administrator privileges. */
          returnValue = (int) ShellExecuteA(NULL, NULL, argv[1],
                                            argc == 2 ? NULL : parameters,
                                            cwd, SW_HIDE);
          /* printf("returnValue: %d\n", returnValue); */
          if (returnValue <= 32) {
            mainResult = 1;
          } /* if */
        } /* if */
      } /* if */
      free(parameters);
      free(cwd);
    } /* if */
    return mainResult;
  } /* main */

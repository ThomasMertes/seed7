/********************************************************************/
/*                                                                  */
/*  esc2qte.c     Find escaped spaces and quote instead.            */
/*  Copyright (C) 2014, 2017  Thomas Mertes                         */
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
/*  File: seed7/src/esc2qte.c                                       */
/*  Changes: 2014  Thomas Mertes                                    */
/*  Content: Find escaped spaces and quote instead.                 */
/*                                                                  */
/********************************************************************/

#include "stdio.h"

#define BUFFER_SIZE 65536


int main (int argc, char *argv)

  {
    char line[BUFFER_SIZE];
    unsigned int pos;
    unsigned int searchPos;
    int escapeFound;

  /* main */
    while (fgets(line, BUFFER_SIZE, stdin) != NULL) {
      pos = 0;
      while (line[pos] != '\0') {
        for (; line[pos] == ' ' || line[pos] == '\t' ||
             line[pos] == '\r' || line[pos] == '\n'; pos++) {
          fputc(line[pos], stdout);
        } /* for */
        escapeFound = 0;
        for (searchPos = pos;
             line[searchPos] != ' ' && line[searchPos] != '\t' &&
             line[searchPos] != '\r' && line[searchPos] != '\n' &&
             line[searchPos] != '\0'; searchPos++) {
          if (line[searchPos] == '\\' && line[searchPos + 1] == ' ') {
            escapeFound = 1;
          } /* if */
        } /* for */
        if (escapeFound) {
          fputc('"', stdout);
          for (; line[pos] != ' ' && line[pos] != '\t' &&
               line[pos] != '\r' && line[pos] != '\n' &&
               line[pos] != '\0'; pos++) {
            if (line[pos] == '\\' && line[pos + 1] == ' ') {
              pos++;
            } /* if */
            fputc(line[pos], stdout);
          } /* for */
          fputc('"', stdout);
        } else {
          for (; line[pos] != ' ' && line[pos] != '\t' &&
               line[pos] != '\r' && line[pos] != '\n' &&
               line[pos] != '\0'; pos++) {
            fputc(line[pos], stdout);
          } /* for */
        } /* if */
      } /* while */
    } /* while */
    return 0;
  } /* main */

/********************************************************************/
/*                                                                  */
/*  chkccomp   Write definitions for Seed7 specific paths.          */
/*  Copyright (C) 2011  Thomas Mertes                               */
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
/*  Module: Chkcomp                                                 */
/*  File: seed7/src/setpaths.c                                      */
/*  Changes: 2011  Thomas Mertes                                    */
/*  Content: Program to write definitions for Seed7 specific paths. */
/*                                                                  */
/********************************************************************/

#include "stdio.h"
#include "stddef.h"
#include "ctype.h"
#include "direct.h"


void write_wstri_as_utf8 (wchar_t *wstri)

  { /* write_wstri_as_utf8 */
    for (; *wstri != '\0'; wstri++) {
      if (*wstri <= 0x7F) {
        putchar((unsigned char) *wstri);
      } else if (*wstri <= 0x7FF) {
        putchar((unsigned char) (0xC0 | (*wstri >>  6)));
        putchar((unsigned char) (0x80 |( *wstri        & 0x3F)));
      } else {
        putchar((unsigned char) (0xE0 | (*wstri >> 12)));
        putchar((unsigned char) (0x80 |((*wstri >>  6) & 0x3F)));
        putchar((unsigned char) (0x80 |( *wstri        & 0x3F)));
      } /* if */
    } /* for */
  } /* write_wstri_as_utf8 */


int main (int argc, char **argv)

  {
    wchar_t buffer[4096];
    int position;

  /* main */
    chdir("../bin");
    _wgetcwd(buffer, sizeof(buffer));
    printf("#define SEED7_LIB \"");
    for (position = 0; buffer[position] != '\0'; position++) {
      if (buffer[position] == '\\') {
        buffer[position] = '/';
      } /* if */
    } /* for */
    if (buffer[1] == ':') {
      buffer[1] = tolower(buffer[0]);
      buffer[0] = '/';
    } /* if */
    write_wstri_as_utf8(buffer);
    printf("/%s\"\n", argv[1]);
    _wgetcwd(buffer, sizeof(buffer));
    printf("#define COMP_DATA_LIB \"");
    for (position = 0; buffer[position] != '\0'; position++) {
      if (buffer[position] == '\\') {
        buffer[position] = '/';
      } /* if */
    } /* for */
    if (buffer[1] == ':') {
      buffer[1] = tolower(buffer[0]);
      buffer[0] = '/';
    } /* if */
    write_wstri_as_utf8(buffer);
    printf("/%s\"\n", argv[2]);
    _wgetcwd(buffer, sizeof(buffer));
    printf("#define COMPILER_LIB \"");
    for (position = 0; buffer[position] != '\0'; position++) {
      if (buffer[position] == '\\') {
        buffer[position] = '/';
      } /* if */
    } /* for */
    if (buffer[1] == ':') {
      buffer[1] = tolower(buffer[0]);
      buffer[0] = '/';
    } /* if */
    write_wstri_as_utf8(buffer);
    printf("/%s\"\n", argv[3]);
    chdir("../lib");
    _wgetcwd(buffer, sizeof(buffer));
    printf("#define SEED7_LIBRARY \"");
    for (position = 0; buffer[position] != '\0'; position++) {
      if (buffer[position] == '\\') {
        buffer[position] = '/';
      } /* if */
    } /* for */
    if (buffer[1] == ':') {
      buffer[1] = tolower(buffer[0]);
      buffer[0] = '/';
    } /* if */
    write_wstri_as_utf8(buffer);
    printf("\"\n");
    return 0;
  } /* main */

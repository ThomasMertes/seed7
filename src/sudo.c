/********************************************************************/
/*                                                                  */
/*  sudo.c        Execute as administrator under Windows.           */
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
/*  Module: Setwpath                                                */
/*  File: seed7/src/setwpath.c                                      */
/*  Changes: 2014  Thomas Mertes                                    */
/*  Content: Set the search path (PATH variable) under Windows.     */
/*                                                                  */
/********************************************************************/

#include "stdio.h"
#include "string.h"
#include "windows.h"
#include "shellapi.h"


int main (int argc, char *argv[])

  {
    char parameters[4096];
    int idx;

  /* main */
    if (argc >= 2) {
      parameters[0] = '\0';
      for (idx = 2; idx < argc; idx++) {
        if (idx != 2) {
          strcat(parameters, " ");
        } /* if */
        strcat(parameters, argv[idx]);
      } /* for */
      printf("%s %s\n", argv[1], parameters);
      ShellExecute(NULL, "runas", argv[1], parameters, NULL, SW_HIDE);
    } /* if */
    return 0;
  } /* main */

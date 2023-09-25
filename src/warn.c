/********************************************************************/
/*                                                                  */
/*  warn.c        Warn if essential header files are missing.       */
/*  Copyright (C) 2019, 2021  Thomas Mertes                         */
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
/*  File: seed7/src/warn.c                                          */
/*  Changes: 2019, 2021  Thomas Mertes                              */
/*  Content: Warn if essential header files are missing.            */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdio.h"



int main (int argc, char **argv)

  { /* main */
#if defined X11_INCLUDE || defined TERM_INCLUDE
    printf("\n");
    printf("************************************************\n");
    printf("*                                              *\n");
    printf("*    *****      A T T E N T I O N      *****   *\n");
    printf("*                                              *\n");
    printf("*      ESSENTIAL HEADER FILES ARE MISSING      *\n");
    printf("*                                              *\n");
#if defined X11_INCLUDE && defined TERM_INCLUDE
    printf("* The header files for X11 and ncurses are     *\n");
    printf("* missing. You need to install the development *\n");
    printf("* packages of X11 and ncurses.                 *\n");
#elif defined X11_INCLUDE
    printf("* The header files for X11 are missing. You    *\n");
    printf("* need to install the development package of   *\n");
    printf("* X11.                                         *\n");
#elif defined TERM_INCLUDE
    printf("* The header files for ncurses are missing.    *\n");
    printf("* You need to install the development package  *\n");
    printf("* of ncurses.                                  *\n");
#endif
#if defined X11_INCLUDE
    printf("* Search with your package manager for a       *\n");
    printf("* package like libx11-dev or libX11-devel and  *\n");
    printf("* install it.                                  *\n");
#endif
#if defined TERM_INCLUDE
    printf("* Search with your package manager for a       *\n");
    printf("* package like libncurses-devel, ncurses-devel *\n");
    printf("* or libncurses5-dev and install it.           *\n");
#endif
    printf("*                                              *\n");
    printf("* To use your package manager your probably    *\n");
    printf("* need superuser rights. Hopefully you succeed *\n");
    printf("* in installing these packages. Afterwards you *\n");
    printf("* need to start the build from scratch (with   *\n");
    printf("* your make command and your makefile):        *\n");
    printf("*                                              *\n");
    printf("*   make clean                                 *\n");
    printf("*                                              *\n");
    printf("* After the cleanup you can again do:          *\n");
    printf("*                                              *\n");
    printf("*   make depend                                *\n");
    printf("*                                              *\n");
    printf("* Seed7 provides its own header files for X11  *\n");
    printf("* and ncurses so you might succeed in          *\n");
    printf("* proceeding the build without installing the  *\n");
    printf("* development packages. This should only be    *\n");
    printf("* done, if it is not possible to install the   *\n");
    printf("* development packages.                        *\n");
    printf("*                                              *\n");
    printf("************************************************\n");
#endif
    return 0;
  } /* main */

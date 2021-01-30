/********************************************************************/
/*                                                                  */
/*  levelup.c     Increment the revision level in level.h.          */
/*  Copyright (C) 2021  Thomas Mertes                               */
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
/*  File: seed7/src/levelup.c                                       */
/*  Changes: 2021  Thomas Mertes                                    */
/*  Content: Increment the revision level in level.h.               */
/*                                                                  */
/********************************************************************/

#include "stdio.h"



int main (int argc, char *argv[])

  {
    FILE *levelFile;
    char buffer1[16];
    char buffer2[16];
    long currentLevel;

  /* main */
    levelFile = fopen("level.h", "r");
    if (levelFile != NULL) {
      fscanf(levelFile, "%15s %15s %ld\n", buffer1, buffer2, &currentLevel);
      fclose(levelFile);
    } else {
      printf("Create \"level.h\".\n");
      currentLevel = 0;
    } /* if */
    levelFile = fopen("level.h", "w");
    if (levelFile != NULL) {
      currentLevel++;
      fprintf(levelFile, "#define LEVEL %ld\n", currentLevel);
      fclose(levelFile);
      printf("Current level is %ld\n", currentLevel);
    } else {
      printf("*** Cannot write to \"level.h\".\n");
    } /* if */
    return 0;
  } /* main */

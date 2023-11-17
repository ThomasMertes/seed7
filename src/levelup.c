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



static long readLevel (const char *const fileName)

  {
    FILE *levelFile;
    char buffer1[16];
    char buffer2[16];
    long level;

  /* readLevel */
    levelFile = fopen(fileName, "r");
    if (levelFile != NULL) {
      fscanf(levelFile, "%15s %15s %ld\n", buffer1, buffer2, &level);
      fclose(levelFile);
    } else {
      level = -1;
    } /* if */
    return level;
  } /* readLevel */



static void writeLevel (const char *const fileName, long level)

  {
    FILE *levelFile;

  /* writeLevel */
    levelFile = fopen(fileName, "wb");
    if (levelFile != NULL) {
      fprintf(levelFile, "#define LEVEL %ld\n", level);
      fclose(levelFile);
    } else {
      printf("*** Cannot write to \"%s\".\n", fileName);
    } /* if */
  } /* writeLevel */



int main (int argc, char *argv[])

  {
    long releaseLevel;
    long backupLevel;
    long currentLevel;

  /* main */
    releaseLevel = readLevel("level_rl.h");
    backupLevel = readLevel("level_bk.h");
    currentLevel = readLevel("level.h");
    if (releaseLevel != -1) {
      if (releaseLevel != backupLevel) {
        writeLevel("level_bk.h", releaseLevel);
        currentLevel = releaseLevel;
      } else {
        currentLevel++;
      } /* if */
    } else {
      currentLevel++;
    } /* if */
    printf("Current level is %ld\n", currentLevel);
    writeLevel("level.h", currentLevel);
    return 0;
  } /* main */

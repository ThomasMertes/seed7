/********************************************************************/
/*                                                                  */
/*  emc_utl.h     Emscripten utility functions.                     */
/*  Copyright (C) 2023  Thomas Mertes                               */
/*                                                                  */
/*  This file is part of the Seed7 Runtime Library.                 */
/*                                                                  */
/*  The Seed7 Runtime Library is free software; you can             */
/*  redistribute it and/or modify it under the terms of the GNU     */
/*  Lesser General Public License as published by the Free Software */
/*  Foundation; either version 2.1 of the License, or (at your      */
/*  option) any later version.                                      */
/*                                                                  */
/*  The Seed7 Runtime Library is distributed in the hope that it    */
/*  will be useful, but WITHOUT ANY WARRANTY; without even the      */
/*  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR */
/*  PURPOSE.  See the GNU Lesser General Public License for more    */
/*  details.                                                        */
/*                                                                  */
/*  You should have received a copy of the GNU Lesser General       */
/*  Public License along with this program; if not, write to the    */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/emc_utl.h                                       */
/*  Changes: 2023  Thomas Mertes                                    */
/*  Content: Emscripten utility functions.                          */
/*                                                                  */
/********************************************************************/

typedef struct keyData {
    charType key;
    intType clickedX;
    intType clickedY;
    int clickedWindow;
    struct keyData *next;
  } keyDataType;

typedef struct {
    keyDataType *queueIn;
    keyDataType *queueOut;
  } keyQueueType;

typedef void (*tp_DoWaitOrPushKey) (int milliSec);

/* This function synchronizes timAwait with reading characters from */
/* the keyboard. The console keyboard and the graphic keyboard both */
/* provide a doWaitOrPushKey() function.                            */
extern tp_DoWaitOrPushKey doWaitOrPushKey;

void enqueue (keyQueueType *queue, keyDataType *key);
void dequeue (keyQueueType *queue, keyDataType *key);
int mapKeyNameIdToKey (int keyNameId, boolType shiftKey,
    boolType ctrlKey, boolType altKey, boolType keyDown);

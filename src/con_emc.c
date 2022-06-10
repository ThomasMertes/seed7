/********************************************************************/
/*                                                                  */
/*  con_emc.c     Driver for emcc (JavaScript) console access.      */
/*  Copyright (C) 1989 - 2019  Thomas Mertes                        */
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
/*  File: seed7/src/con_emc.c                                       */
/*  Changes: 2015, 2019  Thomas Mertes                              */
/*  Content: Driver for emcc (JavaScript) console access.           */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "emscripten.h"

#include "common.h"
#include "heaputl.h"
#include "fil_rtl.h"
#include "ut8_rtl.h"

#undef EXTERN
#define EXTERN
#include "kbd_drv.h"
#include "con_drv.h"

#define KEY_BUFFER_SIZE 1024

static boolType keybd_initialized = FALSE;
static int keyBuffer[KEY_BUFFER_SIZE];
static int keyBufferReadPos;
static int keyBufferWritePos = 0;



void kbdShut (void)

  { /* kbdShut */
  } /* kbdShut */



static void pushKey (int aKey)

  { /* pushKey */
    if (likely(keyBufferReadPos != (keyBufferWritePos + 1) % KEY_BUFFER_SIZE)) {
      keyBuffer[keyBufferWritePos] = aKey;
      keyBufferWritePos = (keyBufferWritePos + 1) % KEY_BUFFER_SIZE;
    } /* if */
  } /* pushKey */



static int popKey (void)

  {
    int aKey;

  /* popKey */
    if (likely(keyBufferReadPos != keyBufferWritePos)) {
      aKey = keyBuffer[keyBufferReadPos];
      keyBufferReadPos = (keyBufferReadPos + 1) % KEY_BUFFER_SIZE;
    } else {
      aKey = EOF;
    } /* if */
    return aKey;
  } /* popKey */



static int mapKeyByName (char *name, int ctrl, int meta, int shift)

  {
    int aKey;

  /* mapKeyByName */
    if (name[0] >= 'a' && name[0] <= 'z' && name[1] == '\0') {
      if (ctrl) {
        aKey = K_CTL_A + (name[0] - 'a');
      } else if (meta) {
        aKey = K_ALT_A + (name[0] - 'a');
      } else if (shift) {
        aKey = 'A' + (name[0] - 'a');
      } else {
        aKey = name[0];
      } /* if */
    } else if (name[0] >= '0' && name[0] <= '9' && name[1] == '\0') {
      aKey = name[0];
    } else if (name[0] == 'f' && name[1] >= '1' && name[1] <= '9' && name[2] == '\0') {
      if (ctrl) {
        aKey = K_CTL_F1 + (name[1] - '1');
      } else if (meta) {
        aKey = K_ALT_F1 + (name[1] - '1');
      } else if (shift) {
        aKey = K_SFT_F1 + (name[1] - '1');
      } else {
        aKey = K_F1 + (name[1] - '1');
      } /* if */
    } else if (name[0] == 'f' && name[1] == '1' && name[2] >= '0' && name[2] <= '2' && name[3] == '\0') {
      if (ctrl) {
        aKey = K_CTL_F10 + (name[2] - '0');
      } else if (meta) {
        aKey = K_ALT_F10 + (name[2] - '0');
      } else if (shift) {
        aKey = K_SFT_F10 + (name[2] - '0');
      } else {
        aKey = K_F10 + (name[2] - '0');
      } /* if */
    } else if (strcmp(name, "left") == 0) {
      if (ctrl) {
        aKey = K_CTL_LEFT;
      } else {
        aKey = K_LEFT;
      } /* if */
    } else if (strcmp(name, "right") == 0) {
      if (ctrl) {
        aKey = K_CTL_RIGHT;
      } else {
        aKey = K_RIGHT;
      } /* if */
    } else if (strcmp(name, "up") == 0) {
      if (ctrl) {
        aKey = K_CTL_UP;
      } else {
        aKey = K_UP;
      } /* if */
    } else if (strcmp(name, "down") == 0) {
      if (ctrl) {
        aKey = K_CTL_DOWN;
      } else {
        aKey = K_DOWN;
      } /* if */
    } else if (strcmp(name, "home") == 0) {
      if (ctrl) {
        aKey = K_CTL_HOME;
      } else {
        aKey = K_HOME;
      } /* if */
    } else if (strcmp(name, "end") == 0) {
      if (ctrl) {
        aKey = K_CTL_END;
      } else {
        aKey = K_END;
      } /* if */
    } else if (strcmp(name, "pageup") == 0) {
      if (ctrl) {
        aKey = K_CTL_PGUP;
      } else {
        aKey = K_PGUP;
      } /* if */
    } else if (strcmp(name, "pagedown") == 0) {
      if (ctrl) {
        aKey = K_CTL_PGDN;
      } else {
        aKey = K_PGDN;
      } /* if */
    } else if (strcmp(name, "insert") == 0) {
      if (ctrl) {
        aKey = K_CTL_INS;
      } else {
        aKey = K_INS;
      } /* if */
    } else if (strcmp(name, "delete") == 0) {
      if (ctrl) {
        aKey = K_CTL_DEL;
      } else {
        aKey = K_DEL;
      } /* if */
    } else if (strcmp(name, "tab") == 0) {
      if (shift) {
        aKey = K_BACKTAB;
      } else {
        aKey = K_TAB;
      } /* if */
    } else if (strcmp(name, "backspace") == 0) {
      aKey = K_BS;
    } else if (strcmp(name, "enter") == 0 || strcmp(name, "return") == 0) {
      aKey = K_NL;
    } else {
      aKey = '?';
    } /* if */
    return aKey;
  } /* mapKeyByName */



static int mapKeyBySequence (char *sequence, int ctrl, int meta, int shift)

  {
    int aKey;

  /* mapKeyBySequence */
    printf("mapKeyBySequence: %s\n", sequence);
    aKey = EOF;
    return aKey;
  } /* mapKeyBySequence */



static void kbd_init (void)

  { /* kbd_init */
    logFunction(printf("kbd_init\n"););
#ifdef OUT_OF_ORDER
    EM_ASM(
      const readline = require('readline');
      readline.emitKeypressEvents(process.stdin);
      process.stdin.setRawMode(true);
      process.stdin.on('keypress', (str, key) => {
        let aKey;
        if (typeof key.name !== 'undefined') {
          aKey = Module.ccall('mapKeyByName', // name of C function
                              'number', // return type
                              ['string', 'number', 'number', 'number'], // argument types
                              [key.name, key.ctrl, key.meta, key.shift]); // arguments
        } else {
          aKey = Module.ccall('mapKeyBySequence', // name of C function
                              'number', // return type
                              ['string', 'number', 'number', 'number'], // argument types
                              [key.sequence, key.ctrl, key.meta, key.shift]); // arguments
        }
        Module.ccall('pushKey', // name of C function
                     null, // return type
                     ['number'], // argument types
                     aKey); // arguments
      });
    );
#endif
    keybd_initialized = TRUE;
    logFunction(printf("kbd_init -->\n"););
  } /* kbd_init */



boolType kbdKeyPressed (void)

  {
    boolType result;

  /* kbdKeyPressed */
    logFunction(printf("kbdKeyPressed\n"););
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    result = keyBufferReadPos != keyBufferWritePos;
    logFunction(printf("kbdKeyPressed --> %d\n", result););
    return result;
  } /* kbdKeyPressed */



charType kbdGetc (void)

  {
    charType result;

  /* kbdGetc */
    logFunction(printf("kbdGetc\n"););
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    if (kbdKeyPressed()) {
      result = (charType) popKey();
    } else {
      result = (charType) EOF;
    } /* if */
    return result;
  } /* kbdGetc */



charType kbdRawGetc (void)

  { /* kbdRawGetc */
    logFunction(printf("kbdRawGetc\n"););
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    return (charType) EOF;
  } /* kbdRawGetc */



int conHeight (void)

  { /* conHeight */
    return 0;
  } /* conHeight */



int conWidth (void)

  { /* conWidth */
    return 0;
  } /* conWidth */



void conFlush (void)

  { /* conFlush */
  } /* conFlush */



void conCursor (boolType on)

  { /* conCursor */
  } /* conCursor */



void conSetCursor (intType line, intType column)

  { /* conSetCursor */
  } /* conSetCursor */



intType conColumn (void)

  { /* conColumn */
    return 1;
  } /* conColumn */



intType conLine (void)

  { /* conLine */
    return 1;
  } /* conLine */



/**
 *  Writes the string stri to the console at the current position.
 */
void conWrite (const const_striType stri)

  { /* conWrite */
    ut8Write(&stdoutFileRecord, stri);
  } /* conWrite */



void conClear (intType startlin, intType startcol,
    intType stoplin, intType stopcol)

  { /* conClear */
  } /* conClear */



void conUpScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  { /* conUpScroll */
  } /* conUpScroll */



void conDownScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  { /* conDownScroll */
  } /* conDownScroll */



void conLeftScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  { /* conLeftScroll */
  } /* conLeftScroll */



void conRightScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  { /* conRightScroll */
  } /* conRightScroll */



void conShut (void)

  { /* conShut */
  } /* conShut */



int conOpen (void)

  { /* conOpen */
    logFunction(printf("conOpen\n"););
    logFunction(printf("conOpen -->\n"););
    return 1;
  } /* conOpen */

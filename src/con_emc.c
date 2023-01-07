/********************************************************************/
/*                                                                  */
/*  con_emc.c     Driver for emcc (JavaScript) console access.      */
/*  Copyright (C) 2015, 2019, 2023  Thomas Mertes                   */
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
/*  Changes: 2015, 2019, 2023  Thomas Mertes                        */
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
#include "striutl.h"
#include "fil_rtl.h"
#include "ut8_rtl.h"
#include "emc_utl.h"

#undef EXTERN
#define EXTERN
#include "kbd_drv.h"
#include "con_drv.h"


#define TRACE_EVENTS 0
#define WRITE_STRI_BLOCK_SIZE    512

static boolType keybd_initialized = FALSE;
static keyDataType lastKey = {K_NONE, 0, 0, 0, NULL};
static keyQueueType keyQueue = {NULL, NULL};

static void kbdDoWaitOrPushKey (int milliSec);



void kbdShut (void)

  { /* kbdShut */
  } /* kbdShut */



static void kbd_init (void)

  { /* kbd_init */
    logFunction(printf("kbd_init\n"););
    doWaitOrPushKey = &kbdDoWaitOrPushKey;
    EM_ASM({
      const readline = require("readline");
      readline.emitKeypressEvents(process.stdin);
      process.stdin.setRawMode(true);
      mapKeynameToId = new Map([
        ["f1",             1],
        ["f2",             2],
        ["f3",             3],
        ["f4",             4],
        ["f5",             5],
        ["f6",             6],
        ["f7",             7],
        ["f8",             8],
        ["f9",             9],
        ["f10",           10],
        ["f11",           11],
        ["f12",           12],
        ["left",          13],
        ["right",         14],
        ["up",            15],
        ["down",          16],
        ["home",          17],
        ["end",           18],
        ["pageup",        19],
        ["pagedown",      20],
        ["insert",        21],
        ["delete",        22],
        ["enter",         23],
        ["return",        23],
        ["backspace",     24],
        ["tab",           25],
        ["escape",        26],
        ["clear",         35]
      ]);
    });
    keybd_initialized = TRUE;
    logFunction(printf("kbd_init -->\n"););
  } /* kbd_init */



static void addEventPromiseForStdin (void)

  { /* addEventPromiseForStdin */
    logFunction(printf("addEventPromiseForStdin\n"););
    EM_ASM({
      eventPromises.push(new Promise(resolve => {
        function handler (str, key) {
          process.stdin.removeListener("keypress", handler);
          resolve(key);
        }
        process.stdin.on("keypress", handler);
        registerCallback2(handler);
      }));
    });
    logFunction(printf("addEventPromiseForStdin -->\n"););
  } /* addEventPromiseForStdin */



static void setupEventPromises (void)

  { /* setupEventPromises */
    EM_ASM({
      eventPromises = [];
    });
    addEventPromiseForStdin();
  } /* setupEventPromises */



static void freeEventPromises (void)

  { /* freeEventPromises */
    EM_ASM({
      executeCallbacks2();
      eventPromises = [];
    });
  } /* freeEventPromises */



EMSCRIPTEN_KEEPALIVE int decodeKeypress (int keyNameId, int key1, int key2,
    int keyLength, boolType shiftKey, boolType ctrlKey, boolType altKey)

  {
    int pressedKey;

  /* decodeKeypress */
    logFunction(printf("decodeKeypress(%d, %d, %d, %d, %d, %d, %d)\n",
                       keyNameId, key1, key2, keyLength,
                       shiftKey, ctrlKey, altKey););
    if (keyLength == 1) {
      if (key1 < ' ') {
        if (key1 == 13) {
          pressedKey = K_NL;
        } else {
          pressedKey = key1;
        } /* if */
      } else if (key1 == 127 && keyNameId == 24 /* backspace */) {
        pressedKey = K_BS;
      } else if (ctrlKey) {
        if (key1 >= 'A' && key1 <= 'Z') {
          pressedKey = key1 - 'A' + K_CTL_A;
        } else if (key1 >= 'a' && key1 <= 'z') {
          pressedKey = key1 - 'a' + K_CTL_A;
        } else if (key1 >= '0' && key1 <= 57) {
          pressedKey = key1 - '0' + K_CTL_0;
        } else {
          pressedKey = K_UNDEF;
        }
      } else if (altKey) {
        if (key1 >= 'A' && key1 <= 'Z') {
          pressedKey = key1 - 'A' + K_ALT_A;
        } else if (key1 >= 'a' && key1 <= 'z') {
          pressedKey = key1 - 'a' + K_ALT_A;
        } else if (key1 >= '0' && key1 <= 57) {
          pressedKey = key1 - '0' + K_ALT_0;
        } else {
          pressedKey = K_UNDEF;
        } /* if */
      } else {
        pressedKey = key1;
      } /* if */
    } else if (keyLength == 2 && key1 == 27) {
      if (key2 >= 'A' && key2 <= 'Z') {
        pressedKey = key2 - 'A' + K_ALT_A;
      } else if (key2 >= 'a' && key2 <= 'z') {
        pressedKey = key2 - 'a' + K_ALT_A;
      } else if (key2 >= '0' && key2 <= 57) {
        pressedKey = key2 - '0' + K_ALT_0;
      } else {
        pressedKey = mapKeyNameIdToKey(keyNameId, shiftKey, ctrlKey, altKey, TRUE);
      } /* if */
    } else {
      pressedKey = mapKeyNameIdToKey(keyNameId, shiftKey, ctrlKey, altKey, TRUE);
    } /* if */
    logFunction(printf("decodeKeypress(%d, %d, %d, %d, %d, %d, %d) --> %d\n",
                       keyNameId, key1, key2, keyLength,
                       shiftKey, ctrlKey, altKey, pressedKey););
    return pressedKey;
  } /* decodeKeypress */



EM_ASYNC_JS(int, asyncKbdGetc, (void), {
    // console.log("asyncKbdGetc");
    const key = await Promise.any(eventPromises);
    // console.log("after await");
    if (typeof key === "number") {
      return key;
    } else {
#if TRACE_EVENTS
      console.log(key);
#endif
      return Module.ccall("decodeKeypress",
                          "number",
                          ["number", "number", "number", "number",
                           "boolean", "boolean", "boolean"],
                          [mapKeynameToId.get(key.name), key.sequence.charCodeAt(0),
                           key.sequence.charCodeAt(1), key.sequence.length,
                           key.shift, key.ctrl, key.meta,]);
    }
  }); /* asyncKbdGetc */



charType kbdGetc (void)

  {
    charType result;

  /* kbdGetc */
    logFunction(printf("kbdGetc\n"););
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    if (keyQueue.queueOut != NULL) {
      dequeue(&keyQueue, &lastKey);
      result = lastKey.key;
      lastKey.key = K_NONE;
    } else if (lastKey.key != K_NONE) {
      result = lastKey.key;
      lastKey.key = K_NONE;
    } else {
      do {
        setupEventPromises();
        result = (charType) asyncKbdGetc();
        freeEventPromises();
      } while (result == K_NONE);
    } /* if */
    logFunction(printf("kbdGetc --> %d\n", result););
    return result;
  } /* kbdGetc */



static charType waitOrGetc (int milliSec)

  {
    charType result;

  /* waitOrGetc */
    logFunction(printf("waitOrGetc(%d)\n", milliSec););
    setupEventPromises();
    EM_ASM({
      eventPromises.push(new Promise(resolve =>
        setTimeout(() => resolve($0), $1)
      ));
    }, K_NONE,  milliSec);
    result = (charType) asyncKbdGetc();
    freeEventPromises();
    logFunction(printf("waitOrGetc --> %d\n", result););
    return result;
  } /* waitOrGetc */



static void kbdDoWaitOrPushKey (int milliSec)

  { /* kbdDoWaitOrPushKey */
    logFunction(printf("kbdDoWaitOrPushKey(%d)\n", milliSec););
    if (lastKey.key != K_NONE) {
      enqueue(&keyQueue, &lastKey);
    } /* if */
    lastKey.key = waitOrGetc(milliSec);
    if (lastKey.key != K_NONE) {
      enqueue(&keyQueue, &lastKey);
      lastKey.key = K_NONE;
    } /* if */
    logFunction(printf("kbdDoWaitOrPushKey(%d) -->\n", milliSec););
  } /* kbdDoWaitOrPushKey */



boolType kbdInputReady (void)

  {
    boolType inputReady;

  /* kbdInputReady */
    logFunction(printf("kbdInputReady\n"););
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    if (keyQueue.queueOut != NULL || lastKey.key != K_NONE) {
      inputReady = TRUE;
    } else {
      lastKey.key = waitOrGetc(1);
      inputReady = lastKey.key != K_NONE;
    } /* if */
    logFunction(printf("kbdInputReady --> %d\n", inputReady););
    return inputReady;
  } /* kbdInputReady */



charType kbdRawGetc (void)

  { /* kbdRawGetc */
    logFunction(printf("kbdRawGetc\n"););
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    return kbdGetc();
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

  {
    const strElemType *str;
    memSizeType len;
    memSizeType size;
    ucharType stri_buffer[max_utf8_size(WRITE_STRI_BLOCK_SIZE) +
                          NULL_TERMINATION_LEN];
    static int useProcess = -1;

  /* conWrite */
    logFunction(printf("conWrite(\"%s\")\n", striAsUnquotedCStri(stri)););
    if (useProcess == -1) {
      useProcess = EM_ASM_INT({
        if (typeof process !== "undefined") {
          return 1;
        } else {
          return 0;
        }
      });
    } /* if */
    if (useProcess == 0) {
      ut8Write(&stdoutFileRecord, stri);
    } else {
      for (str = stri->mem, len = stri->size; len >= WRITE_STRI_BLOCK_SIZE;
           str += WRITE_STRI_BLOCK_SIZE, len -= WRITE_STRI_BLOCK_SIZE) {
        size = stri_to_utf8(stri_buffer, str, WRITE_STRI_BLOCK_SIZE);
        stri_buffer[size] = '\0';
        EM_ASM({
          let stri = Module.UTF8ToString($0);
          process.stdout.write(stri);
        }, stri_buffer);
      } /* for */
      if (len > 0) {
        size = stri_to_utf8(stri_buffer, str, len);
        stri_buffer[size] = '\0';
        EM_ASM({
          let stri = Module.UTF8ToString($0);
          process.stdout.write(stri);
        }, stri_buffer);
      } /* if */
    } /* if */
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

/********************************************************************/
/*                                                                  */
/*  gkb_emc.c     Keyboard and mouse access using the browser.      */
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
/*  File: seed7/src/gkb_emc.c                                       */
/*  Changes: 2023  Thomas Mertes                                    */
/*  Content: Keyboard and mouse access using the browser.           */
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
#include "data_rtl.h"
#include "hsh_rtl.h"
#include "heaputl.h"
#include "emc_utl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "kbd_drv.h"


#define TRACE_EVENTS 0

static keyDataType lastKey = {K_NONE, 0, 0, 0, NULL};
static keyQueueType keyQueue = {NULL, NULL};

static rtlHashType window_hash = NULL;
intType pointerX = 0;
intType pointerY = 0;

/* The state of a modifier key is TRUE, if the key is currently pressed. */
/* The keyboard state is in capsLockOn, numLockOn and scrollLockOn. */
struct modifierState {
    boolType leftShift;
    boolType rightShift;
    boolType leftControl;
    boolType rightControl;
    boolType leftAlt;
    boolType rightAlt;
    boolType leftSuper;
    boolType rightSuper;
    boolType altGraph;
    boolType capsLock;
    boolType numLock;
    boolType scrollLock;
    boolType capsLockOn;
    boolType numLockOn;
    boolType scrollLockOn;
    boolType useLockState;
    int lockState[3];
  };

static struct modifierState modState = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
                                        FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
                                        FALSE, FALSE, FALSE, FALSE, {0, 0, 0} };
static boolType codeIdPressed[54];
static rtlHashType charPressed;
static boolType mouseKeyPressed[5];

extern boolType ignoreResize (winType aWindow, int width, int height);
extern boolType resize (winType resizeWindow, int width, int height);
extern void setResizeReturnsKey (winType resizeWindow, boolType active);
extern int maxWindowId;



winType find_window (int windowId)

  {
    winType window;

  /* find_window */
    if (window_hash == NULL) {
      window = NULL;
    } else {
      window = (winType) (memSizeType)
          hshIdxDefault0(window_hash,
                         (genericType) (memSizeType) windowId,
                         (intType) windowId,
                         (compareType) &genericCmp);
    } /* if */
    logFunction(printf("find_window(%d) --> " FMT_X_MEM "\n",
                       windowId, (memSizeType) window););
    return window;
  } /* find_window */



void enter_window (winType curr_window, int windowId)

  { /* enter_window */
    logFunction(printf("enter_window(" FMT_X_MEM ", %d)\n",
                       (memSizeType) curr_window, windowId););
    if (window_hash == NULL) {
      window_hash = hshEmpty();
    } /* if */
    (void) hshIdxEnterDefault(window_hash,
                              (genericType) (memSizeType) windowId,
                              (genericType) (memSizeType) curr_window,
                              (intType) windowId);
  } /* enter_window */



void remove_window (int windowId)

  { /* remove_window */
    logFunction(printf("remove_window(%d)\n", windowId););
    /* printf("remove_window(%lx)\n", (unsigned long) windowId); */
    hshExcl(window_hash,
            (genericType) (memSizeType) windowId,
            (intType) windowId,
            (compareType) &genericCmp,
            (destrFuncType) &genericDestr,
            (destrFuncType) &genericDestr);
  } /* remove_window */



static void addEventPromiseForWindowId (int windowId)

  { /* addEventPromiseForWindowId */
    logFunction(printf("addEventPromiseForWindowId(%d)\n", windowId););
    EM_ASM({
      let currentWindow = mapIdToWindow[$0];
      eventPromises.push(new Promise(resolve => {
        function handler (event) {
          currentWindow.removeEventListener("keydown", handler);
          currentWindow.removeEventListener("keyup", handler);
          currentWindow.removeEventListener("mousedown", handler);
          currentWindow.removeEventListener("mouseup", handler);
          currentWindow.removeEventListener("wheel", handler);
          currentWindow.removeEventListener("resize", handler);
          currentWindow.removeEventListener("mousemove", handler);
          resolve(event);
        }
        currentWindow.addEventListener("keydown", handler);
        currentWindow.addEventListener("keyup", handler);
        currentWindow.addEventListener("mousedown", handler);
        currentWindow.addEventListener("mouseup", handler);
        currentWindow.addEventListener("wheel", handler);
        currentWindow.addEventListener("resize", handler);
        currentWindow.addEventListener("mousemove", handler);
        registerCallback(handler);
      }));
    }, windowId);
    logFunction(printf("addEventPromiseForWindowId -->\n"););
  } /* addEventPromiseForWindowId */



static void setupEventPromises (void)

  {
    int windowId;
    winType window;

  /* setupEventPromises */
    logFunction(printf("setupEventPromises()\n"););
    EM_ASM({
      eventPromises = [];
    });
    for (windowId = 1; windowId <= maxWindowId; windowId++) {
      window = find_window(windowId);
      if (window != NULL) {
        addEventPromiseForWindowId(windowId);
      } /* if */
    } /* for */
  } /* setupEventPromises */



static void freeEventPromises (void)

  { /* freeEventPromises */
    logFunction(printf("freeEventPromises()\n"););
    EM_ASM({
      executeCallbacks();
      eventPromises = [];
    });
  } /* freeEventPromises */



void setupEventCallbacksForWindow (int windowId)

  { /* setupEventCallbacksForWindow */
    logFunction(printf("setupEventCallbacksForWindow(%d)\n", windowId););
    EM_ASM({
      if (typeof window !== "undefined" && typeof mapIdToWindow[$0] !== "undefined") {
        let currentWindow = mapIdToWindow[$0];
        currentWindow.addEventListener("contextmenu",
          (event) => event.preventDefault());
        currentWindow.addEventListener("keydown",
          (event) => event.preventDefault());
      }
    }, windowId);
    logFunction(printf("setupEventCallbacksForWindow -->\n"););
  } /* setupEventCallbacksForWindow */



static int mapKeyToCodeId (charType aKey)

  {
    int codeId;

  /* mapKeyToCodeId */
    switch (aKey) {
      case K_F1:  case K_F2:  case K_F3:  case K_F4: case K_F5:
      case K_F6:  case K_F7:  case K_F8:  case K_F9: case K_F10:
      case K_F11: case K_F12:
        codeId = (int) aKey - K_F1 + 1;
        break;
      case K_SFT_F1:  case K_SFT_F2:  case K_SFT_F3:  case K_SFT_F4:
      case K_SFT_F5:  case K_SFT_F6:  case K_SFT_F7:  case K_SFT_F8:
      case K_SFT_F9:  case K_SFT_F10: case K_SFT_F11: case K_SFT_F12:
        codeId = (int) aKey - K_SFT_F1 + 1;
        break;
      case K_CTL_F1:  case K_CTL_F2:  case K_CTL_F3:  case K_CTL_F4:
      case K_CTL_F5:  case K_CTL_F6:  case K_CTL_F7:  case K_CTL_F8:
      case K_CTL_F9:  case K_CTL_F10: case K_CTL_F11: case K_CTL_F12:
        codeId = (int) aKey - K_CTL_F1 + 1;
        break;
      case K_ALT_F1:  case K_ALT_F2:  case K_ALT_F3:  case K_ALT_F4:
      case K_ALT_F5:  case K_ALT_F6:  case K_ALT_F7:  case K_ALT_F8:
      case K_ALT_F9:  case K_ALT_F10: case K_ALT_F11: case K_ALT_F12:
        codeId = (int) aKey - K_ALT_F1 + 1;
        break;
      case K_SFT_LEFT: case K_CTL_LEFT: case K_ALT_LEFT: case K_LEFT:
        codeId = 13;
        break;
      case K_SFT_RIGHT: case K_CTL_RIGHT: case K_ALT_RIGHT: case K_RIGHT:
        codeId = 14;
        break;
      case K_SFT_UP: case K_CTL_UP: case K_ALT_UP: case K_UP:
        codeId = 15;
        break;
      case K_SFT_DOWN: case K_CTL_DOWN: case K_ALT_DOWN: case K_DOWN:
        codeId = 16;
        break;
      case K_SFT_HOME: case K_CTL_HOME: case K_ALT_HOME: case K_HOME:
        codeId = 17;
        break;
      case K_SFT_END: case K_CTL_END: case K_ALT_END: case K_END:
        codeId = 18;
        break;
      case K_SFT_PGUP: case K_CTL_PGUP: case K_ALT_PGUP: case K_PGUP:
        codeId = 19;
        break;
      case K_SFT_PGDN: case K_CTL_PGDN: case K_ALT_PGDN: case K_PGDN:
        codeId = 20;
        break;
      case K_SFT_INS: case K_CTL_INS: case K_ALT_INS: case K_INS:
        codeId = 21;
        break;
      case K_SFT_DEL: case K_CTL_DEL: case K_ALT_DEL: case K_DEL:
        codeId = 22;
        break;
      case K_SFT_NL: case K_CTL_NL: case K_ALT_NL: case K_NL:
        codeId = 23;
        break;
      case K_SFT_BS: case K_CTL_BS: case K_ALT_BS: case K_BS:
        codeId = 24;
        break;
      case K_SFT_TAB: case K_CTL_TAB: case K_ALT_TAB: case K_TAB:
        codeId = 25;
        break;
      case K_SFT_ESC: case K_CTL_ESC: case K_ALT_ESC: case K_ESC:
        codeId = 26;
        break;
      case K_SFT_MENU: case K_CTL_MENU: case K_ALT_MENU: case K_MENU:
        codeId = 27;
        break;
      case K_SFT_PRINT: case K_CTL_PRINT: case K_ALT_PRINT: case K_PRINT:
        codeId = 28;
        break;
      case K_SFT_PAUSE: case K_CTL_PAUSE: case K_ALT_PAUSE: case K_PAUSE:
        codeId = 29;
        break;
      default:
        codeId = 0;
        break;
    } /* switch */
    return codeId;
  } /* mapKeyToCodeId */



static int mapKeyToNumpadCodeId (charType aKey)

  {
    int codeId;

  /* mapKeyToNumpadCodeId */
    switch (aKey) {
      case K_SFT_INS: case K_CTL_INS: case K_ALT_INS: case K_INS:
        codeId = 30;
        break;
      case K_SFT_END: case K_CTL_END: case K_ALT_END: case K_END:
        codeId = 31;
        break;
      case K_SFT_DOWN: case K_CTL_DOWN: case K_ALT_DOWN: case K_DOWN:
        codeId = 32;
        break;
      case K_SFT_PGDN: case K_CTL_PGDN: case K_ALT_PGDN: case K_PGDN:
        codeId = 33;
        break;
      case K_SFT_LEFT: case K_CTL_LEFT: case K_ALT_LEFT: case K_LEFT:
        codeId = 34;
        break;
      case K_SFT_PAD_CENTER: case K_CTL_PAD_CENTER: case K_ALT_PAD_CENTER: case K_PAD_CENTER:
        codeId = 35;
        break;
      case K_SFT_RIGHT: case K_CTL_RIGHT: case K_ALT_RIGHT: case K_RIGHT:
        codeId = 36;
        break;
      case K_SFT_HOME: case K_CTL_HOME: case K_ALT_HOME: case K_HOME:
        codeId = 37;
        break;
      case K_SFT_UP: case K_CTL_UP: case K_ALT_UP: case K_UP:
        codeId = 38;
        break;
      case K_SFT_PGUP: case K_CTL_PGUP: case K_ALT_PGUP: case K_PGUP:
        codeId = 39;
        break;
      case K_SFT_DEL: case K_CTL_DEL: case K_ALT_DEL: case K_DEL:
        codeId = 40;
        break;
      case K_SFT_NL: case K_CTL_NL: case K_ALT_NL: case K_NL:
        codeId = 41;
        break;
      default:
        codeId = 0;
        break;
    } /* switch */
    return codeId;
  } /* mapKeyToNumpadCodeId */



int mapCodeIdToKey (int codeId, boolType shiftKey, boolType ctrlKey,
    boolType altKey, boolType keyDown)

  {
    int aKey;

  /* mapCodeIdToKey */
    switch (codeId) {
      case 42:
        modState.leftShift = keyDown;
        if (!shiftKey) {
          modState.rightShift = keyDown;
        } /* if */
        aKey = K_NONE;
        break;
      case 43:
        modState.rightShift = keyDown;
        if (!shiftKey) {
          modState.leftShift = keyDown;
        } /* if */
        aKey = K_NONE;
        break;
      case 44:
        modState.leftControl = keyDown;
        aKey = K_NONE;
        break;
      case 45:
        modState.rightControl = keyDown;
        aKey = K_NONE;
        break;
      case 46:
        modState.leftAlt = keyDown;
        aKey = K_NONE;
        break;
      case 47:
        modState.rightAlt = keyDown;
        aKey = K_NONE;
        break;
      case 48:
        modState.leftSuper = keyDown;
        aKey = K_NONE;
        break;
      case 49:
        modState.rightSuper = keyDown;
        aKey = K_NONE;
        break;
      case 50:
        modState.altGraph = keyDown;
        aKey = K_NONE;
        break;
      case 51:
        modState.capsLock = keyDown;
        aKey = K_NONE;
        break;
      case 52:
        modState.numLock = keyDown;
        aKey = K_NONE;
        break;
      case 53:
        modState.scrollLock = keyDown;
        aKey = K_NONE;
        break;
      default:
        aKey = mapKeyNameIdToKey(codeId, shiftKey, ctrlKey, altKey, keyDown);
        break;
    } /* switch */
    return aKey;
  } /* mapCodeIdToKey */



EMSCRIPTEN_KEEPALIVE void setModifierState (int modifierNum,
    boolType modifierStateOn, boolType keyDown)

  { /* setModifierState */
    logFunction(printf("setModifierState(%d, %d, %d)\n",
                       modifierNum, modifierStateOn, keyDown););
    if (!modifierStateOn && keyDown) {
      modState.useLockState = TRUE;
      modState.lockState[modifierNum] = 0;
    } /* if */
    if (!modifierStateOn && !keyDown) {
      modState.lockState[modifierNum] = 3;
    } /* if */
    if (modState.useLockState) {
      modState.lockState[modifierNum]++;
      if (modState.lockState[modifierNum] == 1) {
        switch (modifierNum) {
          case 0: modState.capsLockOn  = TRUE; break;
          case 1: modState.numLockOn    = TRUE; break;
          case 2: modState.scrollLockOn = TRUE; break;
        } /* switch */
      } else if (modState.lockState[modifierNum] == 4) {
        switch (modifierNum) {
          case 0: modState.capsLockOn  = FALSE; break;
          case 1: modState.numLockOn    = FALSE; break;
          case 2: modState.scrollLockOn = FALSE; break;
        } /* switch */
        modState.lockState[modifierNum] = 0;
      } /* if */
    } else {
      switch (modifierNum) {
        case 0: modState.capsLockOn  = modifierStateOn; break;
        case 1: modState.numLockOn    = modifierStateOn; break;
        case 2: modState.scrollLockOn = modifierStateOn; break;
      } /* switch */
    } /* if */
  } /* setModifierState */



EMSCRIPTEN_KEEPALIVE int decodeMousemoveEvent (int clientX, int clientY)

  {
    int result = K_NONE;

  /* decodeMousemoveEvent */
    logFunction(printf("decodeMousemoveEvent(%d, %d)\n",
                       clientX, clientY););
    pointerX = clientX;
    pointerY = clientY;
    logFunction(printf("decodeMousemoveEvent(%d, %d) --> %d\n",
                       clientX, clientY, result););
    return result;
  } /* decodeMousemoveEvent */



EMSCRIPTEN_KEEPALIVE int decodeKeydownEvent (int codeId, boolType deadKey,
    int key1, int keyLength, boolType shiftKey, boolType ctrlKey, boolType altKey)

  {
    int pressedKey;

  /* decodeKeydownEvent */
    logFunction(printf("decodeKeydownEvent(%d, %d, %d, %d, %d, %d, %d)\n",
                       codeId, deadKey, key1, keyLength,
                       shiftKey, ctrlKey, altKey););
    if (keyLength == 1) {
      if (ctrlKey) {
        if (key1 >= 'A' && key1 <= 'Z') {
          pressedKey = key1 - 'A' + K_CTL_A;
          key1 += 'a' - 'A';
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
          key1 += 'a' - 'A';
        } else if (key1 >= 'a' && key1 <= 'z') {
          pressedKey = key1 - 'a' + K_ALT_A;
        } else if (key1 >= '0' && key1 <= 57) {
          pressedKey = key1 - '0' + K_ALT_0;
        } else {
          pressedKey = K_UNDEF;
        }
      } else {
        pressedKey = key1;
        if (key1 >= 'A' && key1 <= 'Z') {
          key1 += 'a' - 'A';
        } /* if */
      } /* if */
      (void) hshIdxEnterDefault(charPressed,
                                (genericType) key1,
                                (genericType) 1,
                                (intType) key1);
    } else if (deadKey) {
      pressedKey = K_NONE;
    } else {
      pressedKey = mapCodeIdToKey(codeId, shiftKey, ctrlKey, altKey, TRUE);
      if (pressedKey != K_NONE) {
        codeIdPressed[codeId] = TRUE;
      } /* if */
    } /* if */
    logFunction(printf("decodeKeydownEvent(%d, %d, %d, %d, %d, %d, %d) --> %d\n",
                       codeId, deadKey, key1, keyLength,
                       shiftKey, ctrlKey, altKey, pressedKey););
    return pressedKey;
  } /* decodeKeydownEvent */



EMSCRIPTEN_KEEPALIVE int decodeKeyupEvent (int codeId, int key1,
    int keyLength, boolType shiftKey, boolType ctrlKey, boolType altKey)

  {
    int releasedKey;

  /* decodeKeyupEvent */
    logFunction(printf("decodeKeyupEvent(%d, %d, %d, %d, %d, %d)\n",
                       codeId, key1, keyLength, shiftKey, ctrlKey, altKey););
    if (keyLength == 1) {
      if (ctrlKey) {
        if (key1 >= 'A' && key1 <= 'Z') {
          releasedKey = key1 - 'A' + K_CTL_A;
          key1 += 'a' - 'A';
        } else if (key1 >= 'a' && key1 <= 'z') {
          releasedKey = key1 - 'a' + K_CTL_A;
        } else if (key1 >= '0' && key1 <= 57) {
          releasedKey = key1 - '0' + K_CTL_0;
        } else {
          releasedKey = K_UNDEF;
        }
      } else if (altKey) {
        if (key1 >= 'A' && key1 <= 'Z') {
          releasedKey = key1 - 'A' + K_ALT_A;
          key1 += 'a' - 'A';
        } else if (key1 >= 'a' && key1 <= 'z') {
          releasedKey = key1 - 'a' + K_ALT_A;
        } else if (key1 >= '0' && key1 <= 57) {
          releasedKey = key1 - '0' + K_ALT_0;
        } else {
          releasedKey = K_UNDEF;
        }
      } else {
        releasedKey = key1;
        if (key1 >= 'A' && key1 <= 'Z') {
          key1 += 'a' - 'A';
        } /* if */
      } /* if */
      hshExcl(charPressed,
              (genericType) key1,
              (intType) key1,
              (compareType) &genericCmp,
              (destrFuncType) &genericDestr,
              (destrFuncType) &genericDestr);
    } else {
      releasedKey = mapCodeIdToKey(codeId, shiftKey, ctrlKey, altKey, FALSE);
      if (releasedKey != K_NONE) {
        codeIdPressed[codeId] = FALSE;
      } /* if */
    } /* if */
    logFunction(printf("decodeKeyupEvent(%d, %d, %d, %d, %d, %d) --> %d\n",
                       codeId, key1, keyLength, shiftKey, ctrlKey, altKey, K_NONE););
    return K_NONE;
  } /* decodeKeyupEvent */



EMSCRIPTEN_KEEPALIVE int decodeMousedownEvent (int windowId, int button,
    int clientX, int clientY, boolType shiftKey, boolType ctrlKey, boolType altKey)

  {
    int result;

  /* decodeMousedownEvent */
    logFunction(printf("decodeMousedownEvent(%d, %d, %d, %d, %d, %d, %d)\n",
                       windowId, button, clientX, clientY,
                       shiftKey, ctrlKey, altKey););
    lastKey.buttonX = clientX;
    lastKey.buttonY = clientY;
    lastKey.buttonWindow = windowId;
    if (button < 5) {
      mouseKeyPressed[button] = TRUE;
    } /* if */
    switch (button) {
      case 0:
        if (shiftKey) {
          result = K_SFT_MOUSE1;
        } else if (ctrlKey) {
          result = K_CTL_MOUSE1;
        } else if (altKey) {
          result = K_ALT_MOUSE1;
        } else {
          result = K_MOUSE1;
        } /* if */
        break;
      case 1:
        if (shiftKey) {
          result = K_SFT_MOUSE2;
        } else if (ctrlKey) {
          result = K_CTL_MOUSE2;
        } else if (altKey) {
          result = K_ALT_MOUSE2;
        } else {
          result = K_MOUSE2;
        } /* if */
        break;
      case 2:
        if (shiftKey) {
          result = K_SFT_MOUSE3;
        } else if (ctrlKey) {
          result = K_CTL_MOUSE3;
        } else if (altKey) {
          result = K_ALT_MOUSE3;
        } else {
          result = K_MOUSE3;
        } /* if */
        break;
      case 3:
        if (shiftKey) {
          result = K_SFT_MOUSE_BACK;
        } else if (ctrlKey) {
          result = K_CTL_MOUSE_BACK;
        } else if (altKey) {
          result = K_ALT_MOUSE_BACK;
        } else {
          result = K_MOUSE_BACK;
        } /* if */
        break;
      case 4:
        if (shiftKey) {
          result = K_SFT_MOUSE_FWD;
        } else if (ctrlKey) {
          result = K_CTL_MOUSE_FWD;
        } else if (altKey) {
          result = K_ALT_MOUSE_FWD;
        } else {
          result = K_MOUSE_FWD;
        } /* if */
        break;
      default:
        result = K_UNDEF;
        break;
    } /* switch */
    logFunction(printf("decodeMousedownEvent(%d, %d, %d, %d, %d, %d, %d) --> %d\n",
                       windowId, button, clientX, clientY,
                       shiftKey, ctrlKey, altKey, result););
    return result;
  } /* decodeMousedownEvent */



EMSCRIPTEN_KEEPALIVE int decodeMouseupEvent (int button)

  { /* decodeMouseupEvent */
    logFunction(printf("decodeMouseupEvent(%d)\n",
                       button););
    if (button < 5) {
      mouseKeyPressed[button] = FALSE;
    } /* if */
    logFunction(printf("decodeMouseupEvent(%d) --> %d\n",
                       button, K_NONE););
    return K_NONE;
  } /* decodeMouseupEvent */



EMSCRIPTEN_KEEPALIVE int decodeWheelEvent (int windowId, int deltaY,
    int clientX, int clientY, boolType shiftKey, boolType ctrlKey, boolType altKey)

  {
    int result;

  /* decodeWheelEvent */
    logFunction(printf("decodeWheelEvent(%d, %d, %d, %d, %d, %d, %d)\n",
                       windowId, deltaY, clientX, clientY,
                       shiftKey, ctrlKey, altKey););
    lastKey.buttonX = clientX;
    lastKey.buttonY = clientY;
    lastKey.buttonWindow = windowId;
    if (deltaY < 0) {
      if (shiftKey) {
        result = K_SFT_MOUSE4;
      } else if (ctrlKey) {
        result = K_CTL_MOUSE4;
      } else if (altKey) {
        result = K_ALT_MOUSE4;
      } else {
        result = K_MOUSE4;
      } /* if */
    } else if (deltaY > 0) {
      if (shiftKey) {
        result = K_SFT_MOUSE5;
      } else if (ctrlKey) {
        result = K_CTL_MOUSE5;
      } else if (altKey) {
        result = K_ALT_MOUSE5;
      } else {
        result = K_MOUSE5;
      } /* if */
    } else {
      result = K_NONE;
    } /* if */
    logFunction(printf("decodeWheelEvent(%d, %d, %d, %d, %d, %d, %d) --> %d\n",
                       windowId, deltaY, clientX, clientY,
                       shiftKey, ctrlKey, altKey, result););
    return result;
  } /* decodeWheelEvent */



EMSCRIPTEN_KEEPALIVE int decodeResizeEvent (int windowId, int width, int height)

  {
    winType window;
    int result;

  /* decodeResizeEvent */
    logFunction(printf("decodeResizeEvent(%d, %d, %d)\n",
                       windowId, width, height););
    window = find_window(windowId);
    if (ignoreResize(window, width, height)) {
      result = K_NONE;
    } else {
      if (resize(window, width, height)) {
        result = K_RESIZE;
        lastKey.buttonWindow = windowId;
      } else {
        result = K_NONE;
      } /* if */
    } /* if */
    logFunction(printf("decodeResizeEvent(%d, %d, %d) --> %d\n",
                       windowId, width, height, result););
    return result;
  } /* decodeResizeEvent */



EM_ASYNC_JS(int, asyncGkbdGetc, (void), {
    // console.log("asyncGkbdGetc");
    const event = await Promise.any(eventPromises);
    // console.log("after await");
    // console.log(event);
    if (event.type === "mousemove") {
#if TRACE_EVENTS
      console.log(event);
#endif
      return Module.ccall("decodeMousemoveEvent", "number",
                          ["number", "number"], [event.clientX, event.clientY]);
    } else if (event.type === "keydown") {
#if TRACE_EVENTS
      console.log(event);
#endif
      if (event.code === "CapsLock") {
        Module.ccall("setModifierState", null,
                     ["number", "boolean", "boolean"],
                     [0, event.getModifierState("CapsLock"), 1]);
      } else if (event.code === "NumLock") {
        Module.ccall("setModifierState", null,
                     ["number", "boolean", "boolean"],
                     [1, event.getModifierState("NumLock"), 1]);
      } else if (event.code === "ScrollLock") {
        Module.ccall("setModifierState", null,
                     ["number", "boolean", "boolean"],
                     [2, event.getModifierState("ScrollLock"), 1]);
      }
      return Module.ccall("decodeKeydownEvent", "number",
                          ["number", "boolean", "number", "number",
                           "boolean", "boolean", "boolean"],
                          [mapKeyboardEventCodeToId.get(event.code), event.key === "Dead",
                           event.key.charCodeAt(0), event.key.length,
                           event.shiftKey, event.ctrlKey, event.altKey]);
    } else if (event.type === "keyup") {
#if TRACE_EVENTS
      console.log(event);
#endif
      if (event.code === "CapsLock") {
        Module.ccall("setModifierState", null,
                     ["number", "boolean", "boolean"],
                     [0, event.getModifierState("CapsLock"), 0]);
      } else if (event.code === "NumLock") {
        Module.ccall("setModifierState", null,
                     ["number", "boolean", "boolean"],
                     [1, event.getModifierState("NumLock"), 0]);
      } else if (event.code === "ScrollLock") {
        Module.ccall("setModifierState", null,
                     ["number", "boolean", "boolean"],
                     [2, event.getModifierState("ScrollLock"), 0]);
      }
      return Module.ccall("decodeKeyupEvent", "number",
                          ["number", "number", "number", "boolean", "boolean", "boolean"],
                          [mapKeyboardEventCodeToId.get(event.code), event.key.charCodeAt(0),
                           event.key.length, event.shiftKey, event.ctrlKey, event.altKey]);
    } else if (event.type === "mousedown") {
#if TRACE_EVENTS
      console.log(event);
#endif
      return Module.ccall("decodeMousedownEvent", "number",
                          ["number", "number", "number", "number", "boolean", "boolean", "boolean"],
                          [mapCanvasToId.get(event.target), event.button, event.clientX, event.clientY,
                           event.shiftKey, event.ctrlKey, event.altKey]);
    } else if (event.type === "mouseup") {
#if TRACE_EVENTS
      console.log(event);
#endif
      return Module.ccall("decodeMouseupEvent", "number",
                          ["number", "boolean", "boolean", "boolean"],
                          [event.button, event.shiftKey, event.ctrlKey, event.altKey]);
    } else if (event.type === "wheel") {
#if TRACE_EVENTS
      console.log(event);
#endif
      return Module.ccall("decodeWheelEvent", "number",
                          ["number", "number", "number", "number", "boolean", "boolean", "boolean"],
                          [mapCanvasToId.get(event.target), event.deltaY, event.clientX, event.clientY,
                           event.shiftKey, event.ctrlKey, event.altKey]);
    } else if (event.type === "resize") {
#if TRACE_EVENTS
      console.log(event);
#endif
      return Module.ccall("decodeResizeEvent", "number",
                          ["number", "number", "number"],
                          [mapWindowToId.get(event.target), event.target.innerWidth, event.target.innerHeight]);
    } else {
      return event;
    }
  }); /* asyncGkbdGetc */



charType gkbGetc (void)

  {
    charType result;

  /* gkbGetc */
    logFunction(printf("gkbGetc\n"););
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
        result = (charType) asyncGkbdGetc();
        freeEventPromises();
      } while (result == K_NONE);
    } /* if */
    logFunction(printf("gkbGetc --> %d\n", result););
    return result;
  } /* gkbGetc */



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
    result = (charType) asyncGkbdGetc();
    freeEventPromises();
    logFunction(printf("waitOrGetc --> %d\n", result););
    return result;
  } /* waitOrGetc */



static void gkbDoWaitOrPushKey (int milliSec)

  { /* gkbDoWaitOrPushKey */
    logFunction(printf("gkbDoWaitOrPushKey(%d)\n", milliSec););
    if (lastKey.key != K_NONE) {
      enqueue(&keyQueue, &lastKey);
    } /* if */
    lastKey.key = waitOrGetc(milliSec);
    if (lastKey.key != K_NONE) {
      enqueue(&keyQueue, &lastKey);
      lastKey.key = K_NONE;
    } /* if */
    logFunction(printf("gkbDoWaitOrPushKey(%d) -->\n", milliSec););
  } /* gkbDoWaitOrPushKey */



boolType gkbInputReady (void)

  {
    boolType inputReady;

  /* gkbInputReady */
    logFunction(printf("gkbInputReady\n"););
    if (keyQueue.queueOut != NULL || lastKey.key != K_NONE) {
      inputReady = TRUE;
    } else {
      lastKey.key = waitOrGetc(1);
      inputReady = lastKey.key != K_NONE;
    } /* if */
    logFunction(printf("gkbInputReady --> %d\n", inputReady););
    return inputReady;
  } /* gkbInputReady */



void synchonizeTimAwaitWithGraphicKeyboard (void)
  { /* synchonizeTimAwaitWithGraphicKeyboard */
    doWaitOrPushKey = &gkbDoWaitOrPushKey;
  } /* synchonizeTimAwaitWithGraphicKeyboard */



void gkbInitKeyboard (void)

  { /* gkbInitKeyboard */
    logFunction(printf("gkbInitKeyboard()\n"););
    memset(codeIdPressed, 0, sizeof(codeIdPressed));
    charPressed = hshEmpty();
    memset(mouseKeyPressed, 0, sizeof(mouseKeyPressed));
    EM_ASM({
      mapKeyboardEventCodeToId = new Map([
        ["F1",             1],
        ["F2",             2],
        ["F3",             3],
        ["F4",             4],
        ["F5",             5],
        ["F6",             6],
        ["F7",             7],
        ["F8",             8],
        ["F9",             9],
        ["F10",           10],
        ["F11",           11],
        ["F12",           12],
        ["ArrowLeft",     13],
        ["ArrowRight",    14],
        ["ArrowUp",       15],
        ["ArrowDown",     16],
        ["Home",          17],
        ["End",           18],
        ["PageUp",        19],
        ["PageDown",      20],
        ["Insert",        21],
        ["Delete",        22],
        ["Enter",         23],
        ["Backspace",     24],
        ["Tab",           25],
        ["Escape",        26],
        ["ContextMenu",   27],
        ["PrintScreen",   28],
        ["Pause",         29],
        ["Numpad0",       30],
        ["Numpad1",       31],
        ["Numpad2",       32],
        ["Numpad3",       33],
        ["Numpad4",       34],
        ["Numpad5",       35],
        ["Numpad6",       36],
        ["Numpad7",       37],
        ["Numpad8",       38],
        ["Numpad9",       39],
        ["NumpadDecimal", 40],
        ["NumpadEnter",   41],
        ["ShiftLeft",     42],
        ["ShiftRight",    43],
        ["ControlLeft",   44],
        ["ControlRight",  45],
        ["AltLeft",       46],
        ["AltRight",      47],
        ["MetaLeft",      48],
        ["OSLeft",        48],
        ["MetaRight",     49],
        ["OSRight",       49],
        ["AltGraph",      50],
        ["CapsLock",      51],
        ["NumLock",       52],
        ["ScrollLock",    53]
      ]);
    });
  } /* gkbInitKeyboard */



boolType gkbButtonPressed (charType button)

  {
    charType ch1 = 0;
    charType ch2 = 0;
    charType ch3 = 0;
    int codeId;
    boolType buttonPressed;

  /* gkbButtonPressed */
    logFunction(printf("gkbButtonPressed(%04x)\n", button););
    switch (button) {
      case K_SHIFT:
        buttonPressed = modState.leftShift ||
                        modState.rightShift;
        break;
      case K_LEFT_SHIFT:
        buttonPressed = modState.leftShift;
        break;
      case K_RIGHT_SHIFT:
        buttonPressed = modState.rightShift;
        break;
      case K_CONTROL:
        buttonPressed = modState.leftControl ||
                        modState.rightControl;
        break;
      case K_LEFT_CONTROL:
        buttonPressed = modState.leftControl;
        break;
      case K_RIGHT_CONTROL:
        buttonPressed = modState.rightControl;
        break;
      case K_ALT:
        buttonPressed = modState.leftAlt ||
                        modState.rightAlt;
        break;
      case K_LEFT_ALT:
        buttonPressed = modState.leftAlt;
        break;
      case K_RIGHT_ALT:
        buttonPressed = modState.rightAlt;
        break;
      case K_SUPER:
        buttonPressed = modState.leftSuper ||
                        modState.rightSuper;
        break;
      case K_LEFT_SUPER:
        buttonPressed = modState.leftSuper;
        break;
      case K_RIGHT_SUPER:
        buttonPressed = modState.rightSuper;
        break;
      case K_SHIFT_LOCK:
        buttonPressed = modState.capsLock;
        break;
      case K_SHIFT_LOCK_ON:
        buttonPressed = modState.capsLockOn;
        break;
      case K_NUM_LOCK:
        buttonPressed = modState.numLock;
        break;
      case K_NUM_LOCK_ON:
        buttonPressed = modState.numLockOn;
        break;
      case K_SCROLL_LOCK:
        buttonPressed = modState.scrollLock;
        break;
      case K_SCROLL_LOCK_ON:
        buttonPressed = modState.scrollLockOn;
        break;
      case K_SFT_MOUSE1: case K_CTL_MOUSE1: case K_ALT_MOUSE1: case K_MOUSE1:
        buttonPressed = mouseKeyPressed[0];
        break;
      case K_SFT_MOUSE2: case K_CTL_MOUSE2: case K_ALT_MOUSE2: case K_MOUSE2:
        buttonPressed = mouseKeyPressed[1];
        break;
      case K_SFT_MOUSE3: case K_CTL_MOUSE3: case K_ALT_MOUSE3: case K_MOUSE3:
        buttonPressed = mouseKeyPressed[2];
        break;
      case K_SFT_MOUSE_BACK: case K_CTL_MOUSE_BACK: case K_ALT_MOUSE_BACK: case K_MOUSE_BACK:
        buttonPressed = mouseKeyPressed[3];
        break;
      case K_SFT_MOUSE_FWD: case K_CTL_MOUSE_FWD: case K_ALT_MOUSE_FWD: case K_MOUSE_FWD:
        buttonPressed = mouseKeyPressed[4];
        break;
      default:
        buttonPressed = hshContains(charPressed,
                                    (genericType) button,
                                    (intType) button,
                                    (compareType) &genericCmp);
        if (!buttonPressed) {
          switch (button) {
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
            case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
            case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
            case 's': case 't': case 'u': case 'v': case 'w': case 'x':
            case 'y': case 'z':
              ch1 = button - 'a' + 'A';
              ch2 = button - 'a' + K_CTL_A;
              ch3 = button - 'a' + K_ALT_A;
              break;
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
            case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
            case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
            case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
            case 'Y': case 'Z':
              ch1 = button - 'A' + 'a';
              ch2 = button - 'A' + K_CTL_A;
              ch3 = button - 'A' + K_ALT_A;
              break;
            case K_CTL_A: case K_CTL_B: case K_CTL_C: case K_CTL_D:
            case K_CTL_E: case K_CTL_F: case K_CTL_G:
                                        case K_CTL_K: case K_CTL_L:
            case K_CTL_M: case K_CTL_N: case K_CTL_O: case K_CTL_P:
            case K_CTL_Q: case K_CTL_R: case K_CTL_S: case K_CTL_T:
            case K_CTL_U: case K_CTL_V: case K_CTL_W: case K_CTL_X:
            case K_CTL_Y: case K_CTL_Z:
              ch1 = button - K_CTL_A + 'a';
              ch2 = button - K_CTL_A + 'A';
              ch3 = button - K_CTL_A + K_ALT_A;
              break;
            case K_ALT_A: case K_ALT_B: case K_ALT_C: case K_ALT_D:
            case K_ALT_E: case K_ALT_F: case K_ALT_G: case K_ALT_H:
            case K_ALT_I: case K_ALT_J: case K_ALT_K: case K_ALT_L:
            case K_ALT_M: case K_ALT_N: case K_ALT_O: case K_ALT_P:
            case K_ALT_Q: case K_ALT_R: case K_ALT_S: case K_ALT_T:
            case K_ALT_U: case K_ALT_V: case K_ALT_W: case K_ALT_X:
            case K_ALT_Y: case K_ALT_Z:
              ch1 = button - K_ALT_A + 'a';
              ch2 = button - K_ALT_A + 'A';
              ch3 = button - K_ALT_A + K_CTL_A;
              break;
          } /* switch */
          if (ch1 != 0) {
            buttonPressed = hshContains(charPressed,
                                        (genericType) ch1,
                                        (intType) ch1,
                                        (compareType) &genericCmp);
          } /* if */
          if (!buttonPressed && ch2 != 0) {
            buttonPressed = hshContains(charPressed,
                                        (genericType) ch2,
                                        (intType) ch2,
                                        (compareType) &genericCmp);
          } /* if */
          if (!buttonPressed && ch3 != 0) {
            buttonPressed = hshContains(charPressed,
                                        (genericType) ch3,
                                        (intType) ch3,
                                        (compareType) &genericCmp);
          } /* if */
          if (!buttonPressed) {
            codeId = mapKeyToCodeId(button);
            if (codeId != 0) {
              buttonPressed = codeIdPressed[codeId];
            } /* if */
            if (!buttonPressed) {
              codeId = mapKeyToNumpadCodeId(button);
              if (codeId != 0) {
                buttonPressed = codeIdPressed[codeId];
              } /* if */
            } /* if */
          } /* if */
        } /* if */
        break;
    } /* switch */
    logFunction(printf("gkbButtonPressed --> %d\n", buttonPressed););
    return buttonPressed;
  } /* gkbButtonPressed */



charType gkbRawGetc (void)

  { /* gkbRawGetc */
    return gkbGetc();
  } /* gkbRawGetc */



void gkbSelectInput (winType aWindow, charType aKey, boolType active)

  { /* gkbSelectInput */
    if (aKey == K_RESIZE) {
      setResizeReturnsKey(aWindow, active);
    } else if (aKey != K_CLOSE) {
      raise_error(RANGE_ERROR);
    } /* if */
  } /* gkbSelectInput */



winType gkbWindow (void)

  {
    winType result;

  /* gkbWindow */
    logFunction(printf("gkbWindow\n"););
    result = find_window(lastKey.buttonWindow);
    if (result != NULL) {
      result->usage_count++;
    } /* if */
    logFunction(printf("gkbWindow -> " FMT_U_MEM "\n", (memSizeType) result););
    return result;
  } /* gkbWindow */



intType gkbButtonXpos (void)

  { /* gkbButtonXpos */
    logFunction(printf("gkbButtonXpos -> " FMT_D "\n", lastKey.buttonX););
    return lastKey.buttonX;
  } /* gkbButtonXpos */



intType gkbButtonYpos (void)

  { /* gkbButtonYpos */
    logFunction(printf("gkbButtonYpos -> " FMT_D "\n", lastKey.buttonY););
    return lastKey.buttonY;
  } /* gkbButtonYpos */

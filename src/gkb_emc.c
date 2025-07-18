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
#include <emscripten/console.h>
#include <emscripten/wasm_worker.h>

#include "common.h"
#include "data_rtl.h"
#include "os_decls.h"
#include "hsh_rtl.h"
#include "heaputl.h"
#include "emc_utl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "kbd_drv.h"


#define TRACE_EVENTS 0

/* KEY_CLOSE can be sent earlier or later. There are two posibilities: */
/* Immediately after clicking the close button (X) of a window: */
#define AT_X_BUTTON_PRESS  1
/* After clicking the confirmation button "Stay on page": */
#define AFTER_CONFIRMATION 2

/* The moment to send KEY_CLOSE can be 1 or 2 as explained above. */
#define MOMENT_TO_SEND_KEY_CLOSE 2

/* Clicking the close button (X) of a browser window closes the window */
/* or triggers a popup. The popup is created by the browser and cannot */
/* be avoided. The popup offers the choice between "Stay on page" and  */
/* "Leave page". The state of a window regarding the close button (X)  */
/* and the popup is maintained with the closePopupState. The window    */
/* can be in three states:                                             */
/* 0: The close button has not been pressed hence no popup is active.  */
/* 1: The popup is active and the user has not decided yet.            */
/* 2: The user made an unknown decision and the popup has been closed. */
#define NO_CLOSE_POPUP_ACTIVE     0
#define CLOSE_POPUP_ACTIVE        1
#define CLOSE_POPUP_DECISION_MADE 2
/* Unfortunately there is no straightforward way to identify the users */
/* decision ("Stay on page" or "Leave page"). Some indicators are used */
/* to identify the users decision:                                     */
/* 1. An input event for a window in the CLOSE_POPUP_DECISION_MADE     */
/*    state indicates that the window still exists. In this case the   */
/*    choice must have been "Stay on page". The input events           */
/*    "mousemove", "mousedown" and "wheel" are checked for this.       */
/* 2. If a window receives the focus (event "focus") all windows in    */
/*    state CLOSE_POPUP_DECISION_MADE are checked if they still exist. */
/*    If any such window does not exist the users choice must have     */
/*    been "Leave page".                                               */
/* 3. If a window in the CLOSE_POPUP_DECISION_MADE state is closed the */
/*    choice must have been "Leave page". The "visibilitychange" event */
/*    is triggered if a window closes. Note that "visibilitychange" is */
/*    sent in all situations where the visibility of a window changes  */
/*    (e.g. for minimize as well).                                     */
/* 4. The deprecated "unload" event might be sent if the window is     */
/*    being unloaded. This indicates the choice "Leave page" as well.  */
/* If the choice was "Stay on page" the function setClosePopupState    */
/* switches the window state from CLOSE_POPUP_DECISION_MADE to         */
/* NO_CLOSE_POPUP_ACTIVE.                                              */
/* If the choice was "Leave page" the program is terminated with       */
/* os_exit(). This is done with the exitOrException flag since calling */
/* os_exit() from an async function triggers an error.                 */
static boolType exitOrException = FALSE;

/* If copyWindow() fails GRAPHIC_ERROR is raised. This happens if just */
/* one popup is allowed. The exception is raised with the              */
/* exitOrException flag and emc_err_info since raising an exception    */
/* from an async function triggers an error.                           */
static errInfoType emc_err_info = OKAY_NO_ERROR;

static winType windowForNewTabCheck = NULL;

/* Wasm workers require that the HTTP header from the server contains: */
/*   Cross-Origin-Opener-Policy: same-origin                           */
/*   Cross-Origin-Embedder-Policy: require-corp                        */
/* Without these headers window.crossOriginIsolated is false and the   */
/* function setTimeout() will be used as fallback.                     */
#define USE_WASM_WORKERS 0

static keyDataType lastKey = {K_NONE, 0, 0, 0, NULL};
static keyQueueType keyQueue = {NULL, NULL};

static rtlHashType window_hash = NULL;
intType pointerX = 0;
intType pointerY = 0;
boolType touchInSubWindow = FALSE;
#if USE_WASM_WORKERS
emscripten_wasm_worker_t worker = 0;
em_promise_t timeoutPromise;
unsigned int timeoutPromiseNumber = 0;
unsigned int timeoutPromiseCounter = 0;
#endif

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
static boolType codeIdPressed[KEY_ID_LAST_VALUE + 1];
static rtlHashType charPressed;
static boolType mouseKeyPressed[5];
static boolType closePopupActive = FALSE;

typedef struct stateElementStruct *stateElementType;

typedef struct stateElementStruct {
    int windowId;
    int closePopupState;
    stateElementType next;
  } stateElementRecord;

static stateElementType closePopupStateList = NULL;

extern int getCloseAction (winType actual_window);
extern boolType ignoreResize (winType aWindow, int width, int height);
extern boolType resize (winType resizeWindow, int width, int height);
extern void setResizeReturnsKey (winType resizeWindow, boolType active);
extern void drwSetCloseAction (winType actual_window, intType closeAction);
extern boolType windowExists (int windowId);
extern int copyWindow (int windowId);
extern boolType windowIsInNewTab (winType currentWindow);
extern void moveWindowToDocumentTab (winType currentWindow);
extern intType getWindowLeftPos (const const_winType aWindow);
extern intType getWindowTopPos (const const_winType aWindow);
extern boolType isSubWindow (winType aWindow);
extern boolType isTab (winType aWindow);
extern intType timMicroSec (void);
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
    logFunction0(printf("find_window(%d) --> " FMT_X_MEM "\n",
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
    if (window_hash != NULL) {
      hshExcl(window_hash,
              (genericType) (memSizeType) windowId,
              (intType) windowId,
              (compareType) &genericCmp,
              (destrFuncType) &genericDestr,
              (destrFuncType) &genericDestr);
    } /* if */
  } /* remove_window */



/**
 *  Get the closePopupState of a window.
 *  Note that the closePopupStateList does not include windows in the
 *  state NO_CLOSE_POPUP_ACTIVE. This way non existing windows
 *  are also in the state NO_CLOSE_POPUP_ACTIVE.
 */
static int getClosePopupState (int windowId)

  {
    stateElementType stateElement;
    int state = NO_CLOSE_POPUP_ACTIVE;

  /* getClosePopupState */
    logFunction(printf("getClosePopupState(%d)\n", windowId););
    stateElement = closePopupStateList;
    while (stateElement != NULL && stateElement->windowId != windowId) {
      stateElement = stateElement->next;
    } /* while */
    if (stateElement != NULL) {
      state = stateElement->closePopupState;
    } /* if */
    logFunction(printf("getClosePopupState(%d) --> %d\n",
                       windowId, state););
    return state;
  } /* getClosePopupState */



/**
 *  Set the closePopupState of a window.
 *  Note that the closePopupStateList does not include windows in the
 *  state NO_CLOSE_POPUP_ACTIVE. As soon as a window is set to
 *  NO_CLOSE_POPUP_ACTIVE it is removed from the list.
 */
void setClosePopupState (int windowId, int state)

  {
    stateElementType stateElement;
    stateElementType *stateElementAddr;

  /* setClosePopupState */
    logFunction(printf("setClosePopupState(%d, %d)\n", windowId, state););
    if (state != NO_CLOSE_POPUP_ACTIVE) {
      stateElement = closePopupStateList;
      while (stateElement != NULL && stateElement->windowId != windowId) {
        stateElement = stateElement->next;
      } /* while */
      if (stateElement != NULL) {
        stateElement->closePopupState = state;
      } else {
        stateElement = (stateElementType) malloc(sizeof(stateElementRecord));
        if (stateElement != NULL) {
          stateElement->windowId = windowId;
          stateElement->closePopupState = state;
          stateElement->next = closePopupStateList;
          closePopupStateList = stateElement;
        } /* if */
      } /* if */
    } else {
      stateElementAddr = &closePopupStateList;
      while (*stateElementAddr != NULL && (*stateElementAddr)->windowId != windowId) {
        stateElementAddr = &(*stateElementAddr)->next;
      } /* while */
      if (*stateElementAddr != NULL) {
        stateElement = *stateElementAddr;
        *stateElementAddr = stateElement->next;
        free(stateElement);
      } /* if */
    } /* if */
    logFunction(printf("setClosePopupState(%d, %d) -->\n",
                       windowId, state););
  } /* setClosePopupState */



/**
 *  Determines if there is still any close popup active.
 *  This function is called upon receiving a windows input event
 *  ("mousemove", "mousedown" and "wheel"). After the user decided on
 *  the close popup (the window state is CLOSE_POPUP_DECISION_MADE)
 *  the users decision is still unknown. An input event indicates that
 *  this window still exists and the user decided for "Stay on page".
 *  In this case the window state is set to NO_CLOSE_POPUP_ACTIVE.
 *  @return TRUE if there is any close popup active.
 */
boolType isClosePopupActive (int actualWindowId)

  {
    stateElementType stateElement;
    int state;
    boolType decidedForStayOnPage = FALSE;
    boolType isActive = FALSE;

  /* isClosePopupActive */
    logFunction(printf("isClosePopupActive(%d)\n", actualWindowId););
    stateElement = closePopupStateList;
    while (stateElement != NULL) {
      state = stateElement->closePopupState;
      logMessage(printf("window %d: %d\n", stateElement->windowId,
                        state););
      if (stateElement->windowId == actualWindowId) {
        if (state == CLOSE_POPUP_DECISION_MADE) {
          decidedForStayOnPage = TRUE;
        } else if (state != NO_CLOSE_POPUP_ACTIVE) {
          isActive = TRUE;
        } /* if */
      } else if (state != NO_CLOSE_POPUP_ACTIVE) {
        isActive = TRUE;
      } /* if */
      stateElement = stateElement->next;
    } /* while */
    if (decidedForStayOnPage) {
      setClosePopupState(actualWindowId, NO_CLOSE_POPUP_ACTIVE);
    } /* if */
    logFunction(printf("isClosePopupActive --> %d\n", isActive););
    return isActive;
  } /* isClosePopupActive */



/**
 *  Checks if any window in the CLOSE_POPUP_DECISION_MADE state does not exist.
 *  In this case the users decision on the close popup was "Leave page".
 *  @return TRUE if "Leave page" has been decided on a window.
 */
boolType leavePageWasPressed (void)

  {
    stateElementType stateElement;
    int state;
    boolType okay = FALSE;

  /* leavePageWasPressed */
    logFunction(printf("leavePageWasPressed()\n"););
    stateElement = closePopupStateList;
    while (stateElement != NULL) {
      state = stateElement->closePopupState;
      logMessage(printf("window %d: %d\n", stateElement->windowId,
                 state););
      if (state == CLOSE_POPUP_DECISION_MADE &&
          !windowExists(stateElement->windowId)) {
        okay = TRUE;
      } /* if */
      stateElement = stateElement->next;
    } /* while */
    logFunction(printf("leavePageWasPressed --> %d\n", okay););
    return okay;
  } /* leavePageWasPressed */



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
          currentWindow.removeEventListener("beforeunload", handler);
          currentWindow.removeEventListener("focus", handler);
          currentWindow.removeEventListener("visibilitychange", handler);
          currentWindow.removeEventListener("unload", handler);
          currentWindow.removeEventListener("touchstart", handler);
          currentWindow.removeEventListener("touchend", handler);
          currentWindow.removeEventListener("touchcancel", handler);
          currentWindow.removeEventListener("touchmove", handler);
          resolve(event);
        }
        currentWindow.addEventListener("keydown", handler);
        currentWindow.addEventListener("keyup", handler);
        currentWindow.addEventListener("mousedown", handler);
        currentWindow.addEventListener("mouseup", handler);
        currentWindow.addEventListener("wheel", handler);
        currentWindow.addEventListener("resize", handler);
        currentWindow.addEventListener("mousemove", handler);
        currentWindow.addEventListener("beforeunload", handler);
        currentWindow.addEventListener("focus", handler);
        currentWindow.addEventListener("visibilitychange", handler);
        currentWindow.addEventListener("unload", handler);
        currentWindow.addEventListener("touchstart", handler, {passive: false});
        currentWindow.addEventListener("touchend", handler);
        currentWindow.addEventListener("touchcancel", handler);
        currentWindow.addEventListener("touchmove", handler, {passive: false});
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
        codeId = (int) aKey - K_F1 + KEY_ID_F1;
        break;
      case K_SFT_F1:  case K_SFT_F2:  case K_SFT_F3:  case K_SFT_F4:
      case K_SFT_F5:  case K_SFT_F6:  case K_SFT_F7:  case K_SFT_F8:
      case K_SFT_F9:  case K_SFT_F10: case K_SFT_F11: case K_SFT_F12:
        codeId = (int) aKey - K_SFT_F1 + KEY_ID_F1;
        break;
      case K_CTL_F1:  case K_CTL_F2:  case K_CTL_F3:  case K_CTL_F4:
      case K_CTL_F5:  case K_CTL_F6:  case K_CTL_F7:  case K_CTL_F8:
      case K_CTL_F9:  case K_CTL_F10: case K_CTL_F11: case K_CTL_F12:
        codeId = (int) aKey - K_CTL_F1 + KEY_ID_F1;
        break;
      case K_ALT_F1:  case K_ALT_F2:  case K_ALT_F3:  case K_ALT_F4:
      case K_ALT_F5:  case K_ALT_F6:  case K_ALT_F7:  case K_ALT_F8:
      case K_ALT_F9:  case K_ALT_F10: case K_ALT_F11: case K_ALT_F12:
        codeId = (int) aKey - K_ALT_F1 + KEY_ID_F1;
        break;
      case K_SFT_LEFT: case K_CTL_LEFT: case K_ALT_LEFT: case K_LEFT:
        codeId = KEY_ID_ArrowLeft;
        break;
      case K_SFT_RIGHT: case K_CTL_RIGHT: case K_ALT_RIGHT: case K_RIGHT:
        codeId = KEY_ID_ArrowRight;
        break;
      case K_SFT_UP: case K_CTL_UP: case K_ALT_UP: case K_UP:
        codeId = KEY_ID_ArrowUp;
        break;
      case K_SFT_DOWN: case K_CTL_DOWN: case K_ALT_DOWN: case K_DOWN:
        codeId = KEY_ID_ArrowDown;
        break;
      case K_SFT_HOME: case K_CTL_HOME: case K_ALT_HOME: case K_HOME:
        codeId = KEY_ID_Home;
        break;
      case K_SFT_END: case K_CTL_END: case K_ALT_END: case K_END:
        codeId = KEY_ID_End;
        break;
      case K_SFT_PGUP: case K_CTL_PGUP: case K_ALT_PGUP: case K_PGUP:
        codeId = KEY_ID_PageUp;
        break;
      case K_SFT_PGDN: case K_CTL_PGDN: case K_ALT_PGDN: case K_PGDN:
        codeId = KEY_ID_PageDown;
        break;
      case K_SFT_INS: case K_CTL_INS: case K_ALT_INS: case K_INS:
        codeId = KEY_ID_Insert;
        break;
      case K_SFT_DEL: case K_CTL_DEL: case K_ALT_DEL: case K_DEL:
        codeId = KEY_ID_Delete;
        break;
      case K_SFT_NL: case K_CTL_NL: case K_ALT_NL: case K_NL:
        codeId = KEY_ID_Enter;
        break;
      case K_SFT_BS: case K_CTL_BS: case K_ALT_BS: case K_BS:
        codeId = KEY_ID_Backspace;
        break;
      case K_SFT_TAB: case K_CTL_TAB: case K_ALT_TAB: case K_TAB:
        codeId = KEY_ID_Tab;
        break;
      case K_SFT_ESC: case K_CTL_ESC: case K_ALT_ESC: case K_ESC:
        codeId = KEY_ID_Escape;
        break;
      case K_SFT_MENU: case K_CTL_MENU: case K_ALT_MENU: case K_MENU:
        codeId = KEY_ID_ContextMenu;
        break;
      case K_SFT_PRINT: case K_CTL_PRINT: case K_ALT_PRINT: case K_PRINT:
        codeId = KEY_ID_PrintScreen;
        break;
      case K_SFT_PAUSE: case K_CTL_PAUSE: case K_ALT_PAUSE: case K_PAUSE:
        codeId = KEY_ID_Pause;
        break;
      default:
        codeId = KEY_ID_NO_KEY;
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
        codeId = KEY_ID_Numpad0;
        break;
      case K_SFT_END: case K_CTL_END: case K_ALT_END: case K_END:
        codeId = KEY_ID_Numpad1;
        break;
      case K_SFT_DOWN: case K_CTL_DOWN: case K_ALT_DOWN: case K_DOWN:
        codeId = KEY_ID_Numpad2;
        break;
      case K_SFT_PGDN: case K_CTL_PGDN: case K_ALT_PGDN: case K_PGDN:
        codeId = KEY_ID_Numpad3;
        break;
      case K_SFT_LEFT: case K_CTL_LEFT: case K_ALT_LEFT: case K_LEFT:
        codeId = KEY_ID_Numpad4;
        break;
      case K_SFT_PAD_CENTER: case K_CTL_PAD_CENTER: case K_ALT_PAD_CENTER: case K_PAD_CENTER:
        codeId = KEY_ID_Numpad5;
        break;
      case K_SFT_RIGHT: case K_CTL_RIGHT: case K_ALT_RIGHT: case K_RIGHT:
        codeId = KEY_ID_Numpad6;
        break;
      case K_SFT_HOME: case K_CTL_HOME: case K_ALT_HOME: case K_HOME:
        codeId = KEY_ID_Numpad7;
        break;
      case K_SFT_UP: case K_CTL_UP: case K_ALT_UP: case K_UP:
        codeId = KEY_ID_Numpad8;
        break;
      case K_SFT_PGUP: case K_CTL_PGUP: case K_ALT_PGUP: case K_PGUP:
        codeId = KEY_ID_Numpad9;
        break;
      case K_SFT_DEL: case K_CTL_DEL: case K_ALT_DEL: case K_DEL:
        codeId = KEY_ID_NumpadDecimal;
        break;
      case K_SFT_NL: case K_CTL_NL: case K_ALT_NL: case K_NL:
        codeId = KEY_ID_NumpadEnter;
        break;
      default:
        codeId = KEY_ID_NO_KEY;
        break;
    } /* switch */
    return codeId;
  } /* mapKeyToNumpadCodeId */



int mapCodeIdToKey (int codeId, boolType shiftKey, boolType ctrlKey,
    boolType altKey, boolType keyDown)

  {
    int aKey;

  /* mapCodeIdToKey */
    logFunction(printf("mapCodeIdToKey(%d, %d, %d, %d, %d)\n",
                       codeId, shiftKey, ctrlKey, altKey, keyDown););
    switch (codeId) {
      case KEY_ID_ShiftLeft:
        modState.leftShift = keyDown;
        aKey = K_NONE;
        break;
      case KEY_ID_ShiftRight:
        modState.rightShift = keyDown;
        aKey = K_NONE;
        break;
      case KEY_ID_ControlLeft:
        modState.leftControl = keyDown;
        aKey = K_NONE;
        break;
      case KEY_ID_ControlRight:
        modState.rightControl = keyDown;
        aKey = K_NONE;
        break;
      case KEY_ID_AltLeft:
        modState.leftAlt = keyDown;
        aKey = K_NONE;
        break;
      case KEY_ID_AltRight:
        modState.rightAlt = keyDown;
        aKey = K_NONE;
        break;
      case KEY_ID_MetaLeft:
        modState.leftSuper = keyDown;
        aKey = K_NONE;
        break;
      case KEY_ID_MetaRight:
        modState.rightSuper = keyDown;
        aKey = K_NONE;
        break;
      case KEY_ID_AltGraph:
        modState.altGraph = keyDown;
        aKey = K_NONE;
        break;
      case KEY_ID_CapsLock:
        modState.capsLock = keyDown;
        aKey = K_NONE;
        break;
      case KEY_ID_NumLock:
        modState.numLock = keyDown;
        aKey = K_NONE;
        break;
      case KEY_ID_ScrollLock:
        modState.scrollLock = keyDown;
        aKey = K_NONE;
        break;
      default:
        aKey = mapKeyNameIdToKey(codeId, shiftKey, ctrlKey, altKey, keyDown);
        break;
    } /* switch */
    logFunction(printf("mapCodeIdToKey --> %d\n", aKey););
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



EMSCRIPTEN_KEEPALIVE int decodeMousemoveEvent (int windowId,
    int clientX, int clientY)

  {
    int result = K_NONE;

  /* decodeMousemoveEvent */
    logFunction(printf("decodeMousemoveEvent(%d, %d, %d)\n",
                       windowId, clientX, clientY););
    pointerX = clientX;
    pointerY = clientY;
    if (closePopupActive) {
      closePopupActive = isClosePopupActive(windowId);
    } /* if */
    logFunction(printf("decodeMousemoveEvent(%d, %d, %d) --> %d\n",
                       windowId, clientX, clientY, result););
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
        } else if ((key1 >= ' ' && key1 <= '~') || key1 >= 160) {
          pressedKey = key1;
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
        } else if ((key1 >= ' ' && key1 <= '~') || key1 >= 160) {
          releasedKey = key1;
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
    lastKey.clickedX = clientX;
    lastKey.clickedY = clientY;
    lastKey.clickedWindow = windowId;
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
    if (closePopupActive) {
      closePopupActive = isClosePopupActive(windowId);
    } /* if */
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
    lastKey.clickedX = clientX;
    lastKey.clickedY = clientY;
    lastKey.clickedWindow = windowId;
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
    if (closePopupActive) {
      closePopupActive = isClosePopupActive(windowId);
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
      windowForNewTabCheck = window;
    } else {
      if (resize(window, width, height)) {
        result = K_RESIZE;
        lastKey.clickedWindow = windowId;
      } else {
        result = K_NONE;
      } /* if */
    } /* if */
    logFunction(printf("decodeResizeEvent(%d, %d, %d) --> %d\n",
                       windowId, width, height, result););
    return result;
  } /* decodeResizeEvent */



EMSCRIPTEN_KEEPALIVE int decodeBeforeunloadEvent (int windowId, int eventPhase)

  {
    winType window;
    int result = K_NONE;

  /* decodeBeforeunloadEvent */
    logFunction(printf("decodeBeforeunloadEvent(%d (closePopupState=%d))\n",
                       windowId, getClosePopupState(windowId)););
    if (eventPhase == 2) {
      window = find_window(windowId);
      logMessage(printf("close action: %d\n", getCloseAction(window)););
      switch (getCloseAction(window)) {
        case CLOSE_BUTTON_CLOSES_PROGRAM:
          exitOrException = TRUE;
          emc_err_info = OKAY_NO_ERROR;
          break;
        case CLOSE_BUTTON_RETURNS_KEY:
          closePopupActive = TRUE;
          setClosePopupState(windowId, CLOSE_POPUP_ACTIVE);
#if MOMENT_TO_SEND_KEY_CLOSE == AT_X_BUTTON_PRESS
          result = K_CLOSE;
#endif
          lastKey.clickedWindow = windowId;
          break;
        case CLOSE_BUTTON_RAISES_EXCEPTION:
          closePopupActive = TRUE;
          setClosePopupState(windowId, CLOSE_POPUP_ACTIVE);
          exitOrException = TRUE;
          emc_err_info = GRAPHIC_ERROR;
#if MOMENT_TO_SEND_KEY_CLOSE == AT_X_BUTTON_PRESS
          result = K_CLOSE;
#endif
          break;
      } /* switch */
    } /* if */
    logFunction(printf("decodeBeforeunloadEvent(%d (closePopupState=%d)) --> %d\n",
                       windowId, getClosePopupState(windowId), result););
    return result;
  } /* decodeBeforeunloadEvent */



EMSCRIPTEN_KEEPALIVE int decodeFocusEvent (int windowId)

  {
    int result = K_NONE;

  /* decodeFocusEvent */
    logFunction(printf("decodeFocusEvent(%d (closePopupState=%d))\n",
                       windowId, getClosePopupState(windowId)););
    if (closePopupActive) {
      if (leavePageWasPressed()) {
        exitOrException = TRUE;
        emc_err_info = OKAY_NO_ERROR;
      } /* if */
      if (getClosePopupState(windowId) == CLOSE_POPUP_ACTIVE) {
        setClosePopupState(windowId, CLOSE_POPUP_DECISION_MADE);
#if MOMENT_TO_SEND_KEY_CLOSE == AFTER_CONFIRMATION
        result = K_CLOSE;
#endif
      } /* if */
    } /* if */
    logFunction(printf("decodeFocusEvent(%d (closePopupState=%d)) --> %d\n",
                       windowId, getClosePopupState(windowId), result););
    return result;
  } /* decodeFocusEvent */



EMSCRIPTEN_KEEPALIVE int decodeVisibilitychange (int windowId)

  {
    int newWindowId;
    int result = K_NONE;

  /* decodeVisibilitychange */
    logFunction(printf("decodeVisibilitychange(%d (closePopupState=%d))\n",
                        windowId, getClosePopupState(windowId)););
    if (closePopupActive) {
      switch (getClosePopupState(windowId)) {
        case CLOSE_POPUP_ACTIVE:
          /* There is no popup and the window is closed directly.      */
          /* In this case a copied window replaces the closing window. */
          newWindowId = copyWindow(windowId);
          if (newWindowId == 0) {
            exitOrException = TRUE;
            emc_err_info = GRAPHIC_ERROR;
          } else {
            lastKey.clickedWindow = newWindowId;
            windowId = newWindowId;
          } /* if */
#if MOMENT_TO_SEND_KEY_CLOSE == AFTER_CONFIRMATION
          result = K_CLOSE;
#endif
          break;
        case CLOSE_POPUP_DECISION_MADE:
          exitOrException = TRUE;
          emc_err_info = OKAY_NO_ERROR;
          break;
      } /* switch */
    } /* if */
    logFunction(printf("decodeVisibilitychange(%d (closePopupState=%d)) --> %d\n",
                        windowId, getClosePopupState(windowId), result););
    return result;
  } /* decodeVisibilitychange */



EMSCRIPTEN_KEEPALIVE int decodeUnloadEvent (int windowId)

  { /* decodeUnloadEvent */
    logFunction(printf("decodeUnloadEvent(%d (closePopupState=%d))\n",
                       windowId, getClosePopupState(windowId)););
    if (closePopupActive) {
      switch (getClosePopupState(windowId)) {
        case CLOSE_POPUP_ACTIVE:
          /* There is no popup and the window is closed directly. */
          break;
        case CLOSE_POPUP_DECISION_MADE:
          exitOrException = TRUE;
          emc_err_info = OKAY_NO_ERROR;
          break;
      } /* switch */
    } /* if */
    return K_NONE;
  } /* decodeUnloadEvent */



EMSCRIPTEN_KEEPALIVE int decodeTouchstartEvent (int windowId,
    int clientX, int clientY, boolType shiftKey, boolType ctrlKey, boolType altKey)

  {
    winType window;
    int result = K_NONE;

  /* decodeTouchstartEvent */
    logFunction(printf("decodeTouchstartEvent(%d, %d, %d, %d, %d, %d)\n",
                       windowId, clientX, clientY, shiftKey, ctrlKey, altKey););
    window = find_window(windowId);
    if (isSubWindow(window)) {
      lastKey.clickedX = clientX;
      lastKey.clickedY = clientY;
      pointerX = clientX;
      pointerY = clientY;
      lastKey.clickedWindow = windowId;
      if (shiftKey) {
        result = K_SFT_MOUSE1;
      } else if (ctrlKey) {
        result = K_CTL_MOUSE1;
      } else if (altKey) {
        result = K_ALT_MOUSE1;
      } else {
        result = K_MOUSE1;
      } /* if */
      touchInSubWindow = TRUE;
    } /* if */
    mouseKeyPressed[0] = TRUE;
    logFunction(printf("decodeTouchstartEvent(%d, %d, %d, %d, %d, %d) --> %d\n",
                       windowId, clientX, clientY, shiftKey, ctrlKey, altKey, result););
    return result;
  } /* decodeTouchstartEvent */



EMSCRIPTEN_KEEPALIVE int decodeTouchendEvent (int windowId)

  { /* decodeTouchendEvent */
    logFunction(printf("decodeTouchendEvent(%d)\n",
                       windowId););
    mouseKeyPressed[0] = FALSE;
    touchInSubWindow = FALSE;
    return K_NONE;
  } /* decodeTouchendEvent */



EMSCRIPTEN_KEEPALIVE int decodeTouchcancelEvent (int windowId)

  { /* decodeTouchcancelEvent */
    logFunction(printf("decodeTouchcancelEvent(%d)\n",
                       windowId););
    mouseKeyPressed[0] = FALSE;
    touchInSubWindow = FALSE;
    return K_NONE;
  } /* decodeTouchcancelEvent */



EMSCRIPTEN_KEEPALIVE int decodeTouchmoveEvent (int clientX, int clientY)

  { /* decodeTouchmoveEvent */
    logFunction(printf("decodeTouchmoveEvent(%d, %d)\n",
                       clientX, clientY););
    if (touchInSubWindow) {
      pointerX = clientX;
      pointerY = clientY;
    } /* if */
    logFunction(printf("decodeTouchmoveEvent(%d, %d, %d) --> %d\n",
                       clientX, clientY, touchInSubWindow););
    return touchInSubWindow;
  } /* decodeTouchmoveEvent */



EM_ASYNC_JS(int, asyncGkbdGetc, (void), {
    // console.log("asyncGkbdGetc");
    const event = await Promise.any(eventPromises);
    // if (event !== 1114511) {
    //   console.log("after await");
    //   console.log(event);
    // }
    if (event.type === "touchmove") {
#if TRACE_EVENTS
      console.log(event);
#endif
      if (event.touches.length === 1) {
        if (Module.ccall("decodeTouchmoveEvent", "number",
                         ["number", "number"],
                         [event.touches[0].clientX, event.touches[0].clientY])) {
          event.preventDefault();
        }
      }
      return 1114511;
    } else if (event.type === "mousemove") {
#if TRACE_EVENTS
      console.log(event);
#endif
      return Module.ccall("decodeMousemoveEvent", "number",
                          ["number", "number", "number"],
                          [mapCanvasToId.get(event.target), event.clientX, event.clientY]);
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
    } else if (event.type === "beforeunload") {
#if TRACE_EVENTS
      console.log(event);
#endif
      event.returnValue = true;
      event.preventDefault();
      return Module.ccall("decodeBeforeunloadEvent", "number",
                          ["number", "number"],
                          [mapCanvasToId.get(event.target.activeElement.firstChild), event.eventPhase]);
    } else if (event.type === "focus") {
#if TRACE_EVENTS
      console.log(event);
#endif
      return Module.ccall("decodeFocusEvent", "number",
                          ["number"],
                          [mapWindowToId.get(event.target)]);
    } else if (event.type === "visibilitychange") {
#if TRACE_EVENTS
      console.log(event);
#endif
      event.preventDefault();
      return Module.ccall("decodeVisibilitychange", "number",
                          ["number"],
                          [mapCanvasToId.get(event.target.activeElement.firstChild)]);
    } else if (event.type === "unload") {
#if TRACE_EVENTS
      console.log(event);
#endif
      return Module.ccall("decodeUnloadEvent", "number", ["number"],
                          [mapCanvasToId.get(event.target.activeElement.firstChild)]);
    } else if (event.type === "touchstart") {
#if TRACE_EVENTS
      console.log(event);
#endif
      if (event.touches.length === 1) {
        let aKey = Module.ccall("decodeTouchstartEvent", "number",
                                ["number", "number", "number", "boolean", "boolean", "boolean"],
                                [mapCanvasToId.get(event.target),
                                 event.touches[0].clientX, event.touches[0].clientY,
                                 event.shiftKey, event.ctrlKey, event.altKey]);
        if (aKey !== 1114511) {
          event.preventDefault();
        }
        return aKey;
      } else {
        return 1114511;
      }
    } else if (event.type === "touchend") {
#if TRACE_EVENTS
      console.log(event);
#endif
      return Module.ccall("decodeTouchendEvent", "number",
                          ["number"],
                          [mapCanvasToId.get(event.target)]);
    } else if (event.type === "touchcancel") {
#if TRACE_EVENTS
      console.log(event);
#endif
      return Module.ccall("decodeTouchcancelEvent", "number",
                          ["number"],
                          [mapCanvasToId.get(event.target)]);
    } else {
      return event;
    }
  }); /* asyncGkbdGetc */



static boolType timerIsThrottled (void)

  {
    intType timeBefore;
    intType timeAfter;
    boolType isThrottled;

  /* timerIsThrottled */
    logFunction(printf("timerIsThrottled()\n"););
    timeBefore = timMicroSec();
    EM_ASM({
      eventPromises = [];
      eventPromises.push(new Promise(resolve =>
        setTimeout(() => resolve($0), $1)
      ));
    }, K_NONE, 1);
    asyncGkbdGetc();
    freeEventPromises();
    timeAfter = timMicroSec();
    /* printf("time diff: " FMT_U "\n", timeAfter - timeBefore); */
    isThrottled = timeAfter - timeBefore >= 20000;
    logFunction(printf("timerIsThrottled() --> %d\n", isThrottled););
    return isThrottled;
  } /* timerIsThrottled */



static void checkIfWindowIsInNewTab (winType currentWindow)

  { /* checkIfWindowIsInNewTab */
    logFunction(printf("checkIfWindowIsInNewTab(" FMT_U_MEM ")\n",
                        (memSizeType) currentWindow););
    if (!isTab(currentWindow)
#if USE_WASM_WORKERS
        && worker == 0
#endif
    ) {
      if (windowIsInNewTab(currentWindow) || timerIsThrottled()) {
        moveWindowToDocumentTab(currentWindow);
      } /* if */
    } /* if */
    logFunction(printf("checkIfWindowIsInNewTab(" FMT_U_MEM ") -->\n",
                        (memSizeType) currentWindow););
  } /* checkIfWindowIsInNewTab */



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
        if (windowForNewTabCheck != NULL) {
          checkIfWindowIsInNewTab(windowForNewTabCheck);
          windowForNewTabCheck = NULL;
        } /* if */
      } while (result == K_NONE && !exitOrException);
    } /* if */
    if (unlikely(exitOrException)) {
      if (emc_err_info == OKAY_NO_ERROR) {
        os_exit(0);
      } else {
        exitOrException = FALSE;
        raise_error(emc_err_info);
      } /* if */
    } /* if */
    logFunction(printf("gkbGetc --> %d\n", result););
    return result;
  } /* gkbGetc */



#if USE_WASM_WORKERS
static void timeOver (int workerPromiseNumber)

  { /* timeOver */
    logMessage(emscripten_console_log("timeOver()"););
    if (timeoutPromiseNumber == (unsigned int) workerPromiseNumber) {
      logMessage(emscripten_console_log("timeoutPromiseNumber okay"););
      emscripten_promise_resolve(timeoutPromise, EM_PROMISE_FULFILL,
                                 /* value: */ (void*) K_NONE);
    } /* if */
    logMessage(emscripten_console_log("timeOver() -->"););
  } /* timeOver */



/**
 *  Timeout worker function to be executed in a worker thread.
 */
static void timeoutWorker (int workerPromiseNumber, int microseconds)

  { /* timeoutWorker */
    logMessage(emscripten_console_log("timeoutWorker()"););
    emscripten_wasm_worker_sleep((int64_t) microseconds * 1000);
    emscripten_wasm_worker_post_function_vi(EMSCRIPTEN_WASM_WORKER_ID_PARENT,
                                            timeOver, workerPromiseNumber);
    logMessage(emscripten_console_log("timeoutWorker() -->"););
  } /* timeoutWorker */



static void setupTimeoutPromise (int microseconds)

  { /* setupTimeoutPromise */
    logFunction(printf("setupTimeoutPromise(%d)\n", microseconds););
    if (worker == 0) {
      EM_ASM({
        eventPromises.push(new Promise(resolve =>
          setTimeout(() => resolve($0), $1)
        ));
      }, K_NONE, microseconds / 1000);
    } else {
      timeoutPromise = emscripten_promise_create();
      EM_ASM({
        eventPromises.push(getPromise($0));
      }, (int) (void *) timeoutPromise);
      timeoutPromiseCounter++;
      timeoutPromiseNumber = timeoutPromiseCounter;
      emscripten_wasm_worker_post_function_vii(worker, timeoutWorker,
                                               (int) timeoutPromiseNumber,
                                               microseconds);
    } /* if */
    logFunction(printf("setupTimeoutPromise(%d) -->\n", microseconds););
  } /* setupTimeoutPromise */



static void freeTimeoutPromise (void)

  { /* freeTimeoutPromise */
    logFunction(printf("freeTimeoutPromise()\n"););
    if (worker != 0) {
      emscripten_promise_destroy(timeoutPromise);
      timeoutPromiseNumber = 0;
    } /* if */
  } /* freeTimeoutPromise */
#endif



static charType waitOrGetc (int milliSec)

  {
    charType result;

  /* waitOrGetc */
    logFunction(printf("waitOrGetc(%d)\n", milliSec););
    setupEventPromises();
#if USE_WASM_WORKERS
    setupTimeoutPromise(milliSec * 1000);
#else
    EM_ASM({
      eventPromises.push(new Promise(resolve =>
        setTimeout(() => resolve($0), $1)
      ));
    }, K_NONE, milliSec);
#endif
    result = (charType) asyncGkbdGetc();
    freeEventPromises();
#if USE_WASM_WORKERS
    freeTimeoutPromise();
#endif
    if (windowForNewTabCheck != NULL) {
      checkIfWindowIsInNewTab(windowForNewTabCheck);
      windowForNewTabCheck = NULL;
    } /* if */
    if (unlikely(exitOrException)) {
      if (emc_err_info == OKAY_NO_ERROR) {
        os_exit(0);
      } else {
        exitOrException = FALSE;
        raise_error(emc_err_info);
      } /* if */
    } /* if */
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



void synchronizeTimAwaitWithGraphicKeyboard (void)
  { /* synchronizeTimAwaitWithGraphicKeyboard */
    doWaitOrPushKey = &gkbDoWaitOrPushKey;
  } /* synchronizeTimAwaitWithGraphicKeyboard */



#if USE_WASM_WORKERS
static void terminateWorker (void)

  { /* terminateWorker */
    logFunction(printf("terminateWorker()\n"););
    emscripten_terminate_wasm_worker(worker);
    logFunction(printf("terminateWorker() -->\n"););
  } /* terminateWorker */



static void startWorker (void)

  {
    int crossOriginIsolated;

  /* startWorker */
    logFunction(printf("startWorker()\n"););
    crossOriginIsolated = EM_ASM_INT({
      if (typeof window !== "undefined") {
        return window.crossOriginIsolated;
      } else {
        return 0;
      }
    });
    if (crossOriginIsolated) {
      worker = emscripten_malloc_wasm_worker(/*stackSize: */1024);
      if (likely(worker != 0)) {
        os_atexit(terminateWorker);
      } /* if */
    } /* if */
    logFunction(printf("startWorker() -->\n"););
  } /* startWorker */
#endif



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
        ["F13",           13],
        ["F14",           14],
        ["F15",           15],
        ["F16",           16],
        ["F17",           17],
        ["F18",           18],
        ["F19",           19],
        ["F20",           20],
        ["F21",           21],
        ["F22",           22],
        ["F23",           23],
        ["F24",           24],
        ["ArrowLeft",     25],
        ["ArrowRight",    26],
        ["ArrowUp",       27],
        ["ArrowDown",     28],
        ["Home",          29],
        ["End",           30],
        ["PageUp",        31],
        ["PageDown",      32],
        ["Insert",        33],
        ["Delete",        34],
        ["Enter",         35],
        ["Backspace",     36],
        ["Tab",           37],
        ["Escape",        38],
        ["ContextMenu",   39],
        ["PrintScreen",   40],
        ["Pause",         41],
        ["Numpad0",       42],
        ["Numpad1",       43],
        ["Numpad2",       44],
        ["Numpad3",       45],
        ["Numpad4",       46],
        ["Numpad5",       47],
        ["Numpad6",       48],
        ["Numpad7",       49],
        ["Numpad8",       50],
        ["Numpad9",       51],
        ["NumpadDecimal", 52],
        ["NumpadEnter",   53],
        ["KeyA",          54],
        ["KeyB",          55],
        ["KeyC",          56],
        ["KeyD",          57],
        ["KeyE",          58],
        ["KeyF",          59],
        ["KeyG",          60],
        ["KeyH",          61],
        ["KeyI",          62],
        ["KeyJ",          63],
        ["KeyK",          64],
        ["KeyL",          65],
        ["KeyM",          66],
        ["KeyN",          67],
        ["KeyO",          68],
        ["KeyP",          69],
        ["KeyQ",          70],
        ["KeyR",          71],
        ["KeyS",          72],
        ["KeyT",          73],
        ["KeyU",          74],
        ["KeyV",          75],
        ["KeyW",          76],
        ["KeyX",          77],
        ["KeyY",          78],
        ["KeyZ",          79],
        ["ShiftLeft",     80],
        ["ShiftRight",    81],
        ["ControlLeft",   82],
        ["ControlRight",  83],
        ["AltLeft",       84],
        ["AltRight",      85],
        ["MetaLeft",      86],
        ["OSLeft",        86],
        ["MetaRight",     87],
        ["OSRight",       87],
        ["AltGraph",      88],
        ["CapsLock",      89],
        ["NumLock",       90],
        ["ScrollLock",    91]
      ]);
    });
#if USE_WASM_WORKERS
    startWorker();
#endif
  } /* gkbInitKeyboard */



void gkbCloseKeyboard (void)

  { /* gkbCloseKeyboard */
    if (window_hash != NULL) {
      freeGenericHash(window_hash);
      window_hash = NULL;
    } /* if */
  } /* gkbCloseKeyboard */



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
            if (codeId != KEY_ID_NO_KEY) {
              buttonPressed = codeIdPressed[codeId];
            } /* if */
            if (!buttonPressed) {
              codeId = mapKeyToNumpadCodeId(button);
              if (codeId != KEY_ID_NO_KEY) {
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
    } else if (aKey == K_CLOSE) {
      if (active) {
        drwSetCloseAction(aWindow, CLOSE_BUTTON_RETURNS_KEY);
      } else {
        drwSetCloseAction(aWindow, CLOSE_BUTTON_CLOSES_PROGRAM);
      } /* if */
    } else {
      raise_error(RANGE_ERROR);
    } /* if */
  } /* gkbSelectInput */



intType gkbClickedXpos (void)

  {
    winType window;
    intType xPos;

  /* gkbClickedXpos */
    logFunction(printf("gkbClickedXpos\n"););
    xPos = lastKey.clickedX;
    window = find_window(lastKey.clickedWindow);
    if (window != NULL) {
      xPos -= getWindowLeftPos(window);
    } /* if */
    logFunction(printf("gkbClickedXpos -> " FMT_D "\n", xPos););
    return xPos;
  } /* gkbClickedXpos */



intType gkbClickedYpos (void)

  {
    winType window;
    intType yPos;

  /* gkbClickedYpos */
    logFunction(printf("gkbClickedYpos\n"););
    yPos = lastKey.clickedY;
    window = find_window(lastKey.clickedWindow);
    if (window != NULL) {
      yPos -= getWindowTopPos(window);
    } /* if */
    logFunction(printf("gkbClickedYpos -> " FMT_D "\n", yPos););
    return yPos;
  } /* gkbClickedYpos */



winType gkbWindow (void)

  {
    winType result;

  /* gkbWindow */
    logFunction(printf("gkbWindow\n"););
    result = find_window(lastKey.clickedWindow);
    if (result != NULL && result->usage_count != 0) {
      result->usage_count++;
    } /* if */
    logFunction(printf("gkbWindow -> " FMT_U_MEM " (usage=" FMT_U ")\n",
                       (memSizeType) result,
                       result != NULL ? result->usage_count : (uintType) 0););
    return result;
  } /* gkbWindow */

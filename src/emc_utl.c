/********************************************************************/
/*                                                                  */
/*  emc_utl.c     Emscripten utility functions.                     */
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
/*  File: seed7/src/emc_utl.c                                       */
/*  Changes: 2023  Thomas Mertes                                    */
/*  Content: Emscripten utility functions.                          */
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
#include "rtl_err.h"
#include "kbd_drv.h"
#include "emc_utl.h"


typedef void (*tp_atExitFunction) (void);

typedef struct atExitData {
    tp_atExitFunction function;
    struct atExitData *next;
  } atExitDataType;

static atExitDataType *atExitLifoStack = NULL;

/* This function synchronizes timAwait with reading characters from */
/* the keyboard. The console keyboard and the graphic keyboard both */
/* provide a doWaitOrPushKey() function.                            */
tp_DoWaitOrPushKey doWaitOrPushKey = NULL;

typedef int (*tp_startMain) (int argc, char **argv);



int registerExitFunction (tp_atExitFunction function)

  {
    atExitDataType *atExitElement;

  /* registerExitFunction */
    logFunction(printf("registerExitFunction()\n"););
    atExitElement = malloc(sizeof(atExitDataType));
    if (unlikely(atExitElement == NULL)) {
      raise_error(MEMORY_ERROR);
    } else {
      atExitElement->function = function;
      atExitElement->next = atExitLifoStack;
      atExitLifoStack = atExitElement;
    } /* if */
    return 0;
  } /* registerExitFunction */



void doExit (int returnCode)

  {
    atExitDataType *atExitElement;
    atExitDataType *nextElement;

  /* doExit */
    logFunction(printf("doExit(%d)\n", returnCode););
    atExitElement = atExitLifoStack;
    while (atExitElement != NULL) {
      atExitElement->function();
      nextElement = atExitElement->next;
      free(atExitElement);
      atExitElement = nextElement;
    } /* if */
    atExitLifoStack = NULL;
    logFunction(printf("doExit() -->\n"););
    EM_ASM({
      if (reloadPageFunction !== null) {
        reloadPageFunction();
      }
    });
    emscripten_force_exit(returnCode);
  } /* doExit */



static boolType startMainButtonPresent (void)

  {
    boolType buttonPresent;

  /* startMainButtonPresent */
    logFunction(printf("startMainButtonPresent\n"););
    buttonPresent = EM_ASM_INT({
      let buttonPresent = 0;
      if (typeof document !== "undefined") {
        let elements = document.getElementsByName("startMain");
        if (typeof elements !== "undefined") {
          let currentButton = elements[0];
          if (typeof currentButton !== "undefined") {
            buttonPresent = 1;
          }
        }
      }
      return buttonPresent;
    });
    logFunction(printf("startMainButtonPresent --> %d\n", buttonPresent););
    return buttonPresent;
  } /* startMainButtonPresent */



static void addEventPromiseForStartButton (void)

  { /* addEventPromiseForStartButton */
    logFunction(printf("addEventPromiseForStartButton\n"););
    EM_ASM({
      let elements = document.getElementsByName("startMain");
      let currentButton = elements[0];
      eventPromises.push(new Promise(resolve => {
        function handler (event) {
          currentButton.removeEventListener("click", handler);
          resolve(event);
        }
        currentButton.addEventListener("click", handler);
        registerCallback(handler);
      }));
    });
    logFunction(printf("addEventPromiseForStartButton -->\n"););
  } /* addEventPromiseForStartButton */



static void setupEventPromises (void)

  { /* setupEventPromises */
    logFunction(printf("setupEventPromises()\n"););
    EM_ASM({
      eventPromises = [];
    });
    addEventPromiseForStartButton();
  } /* setupEventPromises */



static void freeEventPromises (void)

  { /* freeEventPromises */
    logFunction(printf("freeEventPromises()\n"););
    EM_ASM({
      executeCallbacks();
      eventPromises = [];
    });
  } /* freeEventPromises */



EM_ASYNC_JS(void, asyncButtonClick, (void), {
    // console.log("asyncButtonClick");
    const event = await Promise.any(eventPromises);
    // console.log("after await");
    // console.log(event);
    if (event.type === "click") {
#if TRACE_EVENTS
      console.log(event);
#endif
    }
  }); /* asyncButtonClick */



static void awaitButtonClick (void)

  { /* awaitButtonClick */
    logFunction(printf("awaitButtonClick\n"););
    setupEventPromises();
    asyncButtonClick();
    freeEventPromises();
    logFunction(printf("awaitButtonClick -->\n"););
  } /* awaitButtonClick */



int executeStartMainOnButtonClick (tp_startMain startMain,
    int argc, char **argv)

  { /* executeStartMainOnButtonClick */
    logFunction(printf("executeStartMainOnButtonClick\n"););
    if (startMainButtonPresent()) {
      awaitButtonClick();
    } /* if */
    return startMain(argc, argv);
  } /* executeStartMainOnButtonClick */



void enqueue (keyQueueType *queue, keyDataType *key)

  {
    keyDataType *newElement;

  /* enqueue */
    logFunction(printf("enqueue(%d)\n", key->key););
    newElement = malloc(sizeof(keyDataType));
    if (unlikely(newElement == NULL)) {
      raise_error(MEMORY_ERROR);
    } else {
      memcpy(newElement, key, sizeof(keyDataType));
      if (queue->queueIn == NULL) {
        queue->queueIn = newElement;
        queue->queueOut = newElement;
      } else {
        queue->queueIn->next = newElement;
        queue->queueIn = newElement;
      } /* if */
    } /* if */
    logFunction(printf("enqueue(%d) -->\n", key->key););
  } /* enqueue */



void dequeue (keyQueueType *queue, keyDataType *key)

  {
    keyDataType *nextElement;

  /* dequeue */
    logFunction(printf("dequeue\n"););
    key->key = queue->queueOut->key;
    key->clickedX = queue->queueOut->clickedX;
    key->clickedY = queue->queueOut->clickedY;
    key->clickedWindow = queue->queueOut->clickedWindow;
    nextElement = queue->queueOut->next;
    free(queue->queueOut);
    queue->queueOut = nextElement;
    if (queue->queueOut == NULL) {
      queue->queueIn = NULL;
    } /* if */
    logFunction(printf("dequeue --> %d\n", key->key););
  } /* dequeue */



/**
 *  Map a keyNameId and a modifier key state to a character.
 *  The JavaScript interfaces for console and browser keyboards both
 *  use a key name (string) to identify function and cursor keys. These
 *  strings are mapped with mapKeynameToId and mapKeyboardEventCodeToId
 *  to a keyNameId. A keyNameId is just used in the Seed7 drivers for
 *  Enscripten.
 */
int mapKeyNameIdToKey (int keyNameId, boolType shiftKey,
    boolType ctrlKey, boolType altKey, boolType keyDown)

  {
    int aKey;

  /* mapKeyNameIdToKey */
    logFunction(printf("mapKeyNameIdToKey(%d, %d, %d, %d, %d)\n",
                       keyNameId, shiftKey, ctrlKey, altKey, keyDown););
    switch (keyNameId) {
      case KEY_ID_F1:
      case KEY_ID_F2:
      case KEY_ID_F3:
      case KEY_ID_F4:
      case KEY_ID_F5:
      case KEY_ID_F6:
      case KEY_ID_F7:
      case KEY_ID_F8:
      case KEY_ID_F9:
      case KEY_ID_F10:
      case KEY_ID_F11:
      case KEY_ID_F12:
        if (shiftKey) {
          aKey = keyNameId - 1 + K_SFT_F1;
        } else if (ctrlKey) {
          aKey = keyNameId - 1 + K_CTL_F1;
        } else if (altKey) {
          aKey = keyNameId - 1 + K_ALT_F1;
        } else {
          aKey = keyNameId - 1 + K_F1;
        }
        break;
      case KEY_ID_F13:
      case KEY_ID_F14:
      case KEY_ID_F15:
      case KEY_ID_F16:
      case KEY_ID_F17:
      case KEY_ID_F18:
      case KEY_ID_F19:
      case KEY_ID_F20:
      case KEY_ID_F21:
      case KEY_ID_F22:
      case KEY_ID_F23:
      case KEY_ID_F24:
        aKey = K_UNDEF;
        break;
      case KEY_ID_ArrowLeft:
        if (shiftKey) {
          aKey = K_SFT_LEFT;
        } else if (ctrlKey) {
          aKey = K_CTL_LEFT;
        } else if (altKey) {
          aKey = K_ALT_LEFT;
        } else {
          aKey = K_LEFT;
        }
        break;
      case KEY_ID_ArrowRight:
        if (shiftKey) {
          aKey = K_SFT_RIGHT;
        } else if (ctrlKey) {
          aKey = K_CTL_RIGHT;
        } else if (altKey) {
          aKey = K_ALT_RIGHT;
        } else {
          aKey = K_RIGHT;
        }
        break;
      case KEY_ID_ArrowUp:
        if (shiftKey) {
          aKey = K_SFT_UP;
        } else if (ctrlKey) {
          aKey = K_CTL_UP;
        } else if (altKey) {
          aKey = K_ALT_UP;
        } else {
          aKey = K_UP;
        }
        break;
      case KEY_ID_ArrowDown:
        if (shiftKey) {
          aKey = K_SFT_DOWN;
        } else if (ctrlKey) {
          aKey = K_CTL_DOWN;
        } else if (altKey) {
          aKey = K_ALT_DOWN;
        } else {
          aKey = K_DOWN;
        }
        break;
      case KEY_ID_Home:
        if (shiftKey) {
          aKey = K_SFT_HOME;
        } else if (ctrlKey) {
          aKey = K_CTL_HOME;
        } else if (altKey) {
          aKey = K_ALT_HOME;
        } else {
          aKey = K_HOME;
        }
        break;
      case KEY_ID_End:
        if (shiftKey) {
          aKey = K_SFT_END;
        } else if (ctrlKey) {
          aKey = K_CTL_END;
        } else if (altKey) {
          aKey = K_ALT_END;
        } else {
          aKey = K_END;
        }
        break;
      case KEY_ID_PageUp:
        if (shiftKey) {
          aKey = K_SFT_PGUP;
        } else if (ctrlKey) {
          aKey = K_CTL_PGUP;
        } else if (altKey) {
          aKey = K_ALT_PGUP;
        } else {
          aKey = K_PGUP;
        }
        break;
      case KEY_ID_PageDown:
        if (shiftKey) {
          aKey = K_SFT_PGDN;
        } else if (ctrlKey) {
          aKey = K_CTL_PGDN;
        } else if (altKey) {
          aKey = K_ALT_PGDN;
        } else {
          aKey = K_PGDN;
        }
        break;
      case KEY_ID_Insert:
        if (shiftKey) {
          aKey = K_SFT_INS;
        } else if (ctrlKey) {
          aKey = K_CTL_INS;
        } else if (altKey) {
          aKey = K_ALT_INS;
        } else {
          aKey = K_INS;
        }
        break;
      case KEY_ID_Delete:
        if (shiftKey) {
          aKey = K_SFT_DEL;
        } else if (ctrlKey) {
          aKey = K_CTL_DEL;
        } else if (altKey) {
          aKey = K_ALT_DEL;
        } else {
          aKey = K_DEL;
        }
        break;
      case KEY_ID_Enter:
        if (shiftKey) {
          aKey = K_SFT_NL;
        } else if (ctrlKey) {
          aKey = K_CTL_NL;
        } else if (altKey) {
          aKey = K_ALT_NL;
        } else {
          aKey = K_NL;
        }
        break;
      case KEY_ID_Backspace:
        if (shiftKey) {
          aKey = K_SFT_BS;
        } else if (ctrlKey) {
          aKey = K_CTL_BS;
        } else if (altKey) {
          aKey = K_ALT_BS;
        } else {
          aKey = K_BS;
        }
        break;
      case KEY_ID_Tab:
        if (shiftKey) {
          aKey = K_SFT_TAB;
        } else if (ctrlKey) {
          aKey = K_CTL_TAB;
        } else if (altKey) {
          aKey = K_ALT_TAB;
        } else {
          aKey = K_TAB;
        }
        break;
      case KEY_ID_Escape:
        if (shiftKey) {
          aKey = K_SFT_ESC;
        } else if (ctrlKey) {
          aKey = K_CTL_ESC;
        } else if (altKey) {
          aKey = K_ALT_ESC;
        } else {
          aKey = K_ESC;
        }
        break;
      case KEY_ID_ContextMenu:
        if (shiftKey) {
          aKey = K_SFT_MENU;
        } else if (ctrlKey) {
          aKey = K_CTL_MENU;
        } else if (altKey) {
          aKey = K_ALT_MENU;
        } else {
          aKey = K_MENU;
        }
        break;
      case KEY_ID_PrintScreen:
        if (shiftKey) {
          aKey = K_SFT_PRINT;
        } else if (ctrlKey) {
          aKey = K_CTL_PRINT;
        } else if (altKey) {
          aKey = K_ALT_PRINT;
        } else {
          aKey = K_PRINT;
        }
        break;
      case KEY_ID_Pause:
        if (shiftKey) {
          aKey = K_SFT_PAUSE;
        } else if (ctrlKey) {
          aKey = K_CTL_PAUSE;
        } else if (altKey) {
          aKey = K_ALT_PAUSE;
        } else {
          aKey = K_PAUSE;
        }
        break;
      case KEY_ID_Numpad0:
        if (shiftKey) {
          aKey = K_SFT_INS;
        } else if (ctrlKey) {
          aKey = K_CTL_INS;
        } else if (altKey) {
          aKey = K_ALT_INS;
        } else {
          aKey = K_INS;
        }
        break;
      case KEY_ID_Numpad1:
        if (shiftKey) {
          aKey = K_SFT_END;
        } else if (ctrlKey) {
          aKey = K_CTL_END;
        } else if (altKey) {
          aKey = K_ALT_END;
        } else {
          aKey = K_END;
        }
        break;
      case KEY_ID_Numpad2:
        if (shiftKey) {
          aKey = K_SFT_DOWN;
        } else if (ctrlKey) {
          aKey = K_CTL_DOWN;
        } else if (altKey) {
          aKey = K_ALT_DOWN;
        } else {
          aKey = K_DOWN;
        }
        break;
      case KEY_ID_Numpad3:
        if (shiftKey) {
          aKey = K_SFT_PGDN;
        } else if (ctrlKey) {
          aKey = K_CTL_PGDN;
        } else if (altKey) {
          aKey = K_ALT_PGDN;
        } else {
          aKey = K_PGDN;
        }
        break;
      case KEY_ID_Numpad4:
        if (shiftKey) {
          aKey = K_SFT_LEFT;
        } else if (ctrlKey) {
          aKey = K_CTL_LEFT;
        } else if (altKey) {
          aKey = K_ALT_LEFT;
        } else {
          aKey = K_LEFT;
        }
        break;
      case KEY_ID_Numpad5:
        if (shiftKey) {
          aKey = K_SFT_PAD_CENTER;
        } else if (ctrlKey) {
          aKey = K_CTL_PAD_CENTER;
        } else if (altKey) {
          aKey = K_ALT_PAD_CENTER;
        } else {
          aKey = K_PAD_CENTER;
        }
        break;
      case KEY_ID_Numpad6:
        if (shiftKey) {
          aKey = K_SFT_RIGHT;
        } else if (ctrlKey) {
          aKey = K_CTL_RIGHT;
        } else if (altKey) {
          aKey = K_ALT_RIGHT;
        } else {
          aKey = K_RIGHT;
        }
        break;
      case KEY_ID_Numpad7:
        if (shiftKey) {
          aKey = K_SFT_HOME;
        } else if (ctrlKey) {
          aKey = K_CTL_HOME;
        } else if (altKey) {
          aKey = K_ALT_HOME;
        } else {
          aKey = K_HOME;
        }
        break;
      case KEY_ID_Numpad8:
        if (shiftKey) {
          aKey = K_SFT_UP;
        } else if (ctrlKey) {
          aKey = K_CTL_UP;
        } else if (altKey) {
          aKey = K_ALT_UP;
        } else {
          aKey = K_UP;
        }
        break;
      case KEY_ID_Numpad9:
        if (shiftKey) {
          aKey = K_SFT_PGUP;
        } else if (ctrlKey) {
          aKey = K_CTL_PGUP;
        } else if (altKey) {
          aKey = K_ALT_PGUP;
        } else {
          aKey = K_PGUP;
        }
        break;
      case KEY_ID_NumpadDecimal:
        if (shiftKey) {
          aKey = K_SFT_DEL;
        } else if (ctrlKey) {
          aKey = K_CTL_DEL;
        } else if (altKey) {
          aKey = K_ALT_DEL;
        } else {
          aKey = K_DEL;
        }
        break;
      case KEY_ID_NumpadEnter:
        if (shiftKey) {
          aKey = K_SFT_NL;
        } else if (ctrlKey) {
          aKey = K_CTL_NL;
        } else if (altKey) {
          aKey = K_ALT_NL;
        } else {
          aKey = K_NL;
        }
        break;
      default:
        aKey = K_UNDEF;
        break;
    } /* switch */
    logFunction(printf("mapKeyNameIdToKey --> %d\n", aKey););
    return aKey;
  } /* mapKeyNameIdToKey */

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

#define KEY_ID_NO_KEY         0

#define KEY_ID_F1             1
#define KEY_ID_F2             2
#define KEY_ID_F3             3
#define KEY_ID_F4             4
#define KEY_ID_F5             5
#define KEY_ID_F6             6
#define KEY_ID_F7             7
#define KEY_ID_F8             8
#define KEY_ID_F9             9
#define KEY_ID_F10           10
#define KEY_ID_F11           11
#define KEY_ID_F12           12
#define KEY_ID_F13           13
#define KEY_ID_F14           14
#define KEY_ID_F15           15
#define KEY_ID_F16           16
#define KEY_ID_F17           17
#define KEY_ID_F18           18
#define KEY_ID_F19           19
#define KEY_ID_F20           20
#define KEY_ID_F21           21
#define KEY_ID_F22           22
#define KEY_ID_F23           23
#define KEY_ID_F24           24
#define KEY_ID_ArrowLeft     25
#define KEY_ID_ArrowRight    26
#define KEY_ID_ArrowUp       27
#define KEY_ID_ArrowDown     28
#define KEY_ID_Home          29
#define KEY_ID_End           30
#define KEY_ID_PageUp        31
#define KEY_ID_PageDown      32
#define KEY_ID_Insert        33
#define KEY_ID_Delete        34
#define KEY_ID_Enter         35
#define KEY_ID_Backspace     36
#define KEY_ID_Tab           37
#define KEY_ID_Escape        38
#define KEY_ID_ContextMenu   39
#define KEY_ID_PrintScreen   40
#define KEY_ID_Pause         41
#define KEY_ID_Numpad0       42
#define KEY_ID_Numpad1       43
#define KEY_ID_Numpad2       44
#define KEY_ID_Numpad3       45
#define KEY_ID_Numpad4       46
#define KEY_ID_Numpad5       47
#define KEY_ID_Numpad6       48
#define KEY_ID_Numpad7       49
#define KEY_ID_Numpad8       50
#define KEY_ID_Numpad9       51
#define KEY_ID_NumpadDecimal 52
#define KEY_ID_NumpadEnter   53
#define KEY_ID_KeyA          54
#define KEY_ID_KeyB          55
#define KEY_ID_KeyC          56
#define KEY_ID_KeyD          57
#define KEY_ID_KeyE          58
#define KEY_ID_KeyF          59
#define KEY_ID_KeyG          60
#define KEY_ID_KeyH          61
#define KEY_ID_KeyI          62
#define KEY_ID_KeyJ          63
#define KEY_ID_KeyK          64
#define KEY_ID_KeyL          65
#define KEY_ID_KeyM          66
#define KEY_ID_KeyN          67
#define KEY_ID_KeyO          68
#define KEY_ID_KeyP          69
#define KEY_ID_KeyQ          70
#define KEY_ID_KeyR          71
#define KEY_ID_KeyS          72
#define KEY_ID_KeyT          73
#define KEY_ID_KeyU          74
#define KEY_ID_KeyV          75
#define KEY_ID_KeyW          76
#define KEY_ID_KeyX          77
#define KEY_ID_KeyY          78
#define KEY_ID_KeyZ          79
#define KEY_ID_ShiftLeft     80
#define KEY_ID_ShiftRight    81
#define KEY_ID_ControlLeft   82
#define KEY_ID_ControlRight  83
#define KEY_ID_AltLeft       84
#define KEY_ID_AltRight      85
#define KEY_ID_MetaLeft      86
#define KEY_ID_MetaRight     87
#define KEY_ID_AltGraph      88
#define KEY_ID_CapsLock      89
#define KEY_ID_NumLock       90
#define KEY_ID_ScrollLock    91

#define KEY_ID_LAST_VALUE    91


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

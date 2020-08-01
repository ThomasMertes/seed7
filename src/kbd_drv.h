/********************************************************************/
/*                                                                  */
/*  kbd_drv.h     Prototypes for keyboard access functions.         */
/*  Copyright (C) 1989 - 2005  Thomas Mertes                        */
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
/*  File: seed7/src/kbd_drv.h                                       */
/*  Changes: 1990, 1991, 1992, 1993, 1994, 2005  Thomas Mertes      */
/*  Content: Prototypes for keyboard access functions.              */
/*                                                                  */
/********************************************************************/

#define K_CTL_A        1
#define K_CTL_B        2
#define K_CTL_C        3
#define K_CTL_D        4
#define K_CTL_E        5
#define K_CTL_F        6
#define K_CTL_G        7
#define K_CTL_H        8
#define K_BS           8
#define K_CTL_I        9
#define K_TAB          9
#define K_CTL_J       10
#define K_NL          10
#define K_CTL_K       11
#define K_CTL_L       12
#define K_CTL_M       13
#define K_CTL_N       14
#define K_CTL_O       15
#define K_CTL_P       16
#define K_CTL_Q       17
#define K_CTL_R       18
#define K_CTL_S       19
#define K_CTL_T       20
#define K_CTL_U       21
#define K_CTL_V       22
#define K_CTL_W       23
#define K_CTL_X       24
#define K_CTL_Y       25
#define K_CTL_Z       26
#define K_ESC         27

/* KEYCODE_BASE is beyond the range of Unicode characters */
#define KEYCODE_BASE 0x110000

#define K_ALT_A      (KEYCODE_BASE +   1)
#define K_ALT_B      (KEYCODE_BASE +   2)
#define K_ALT_C      (KEYCODE_BASE +   3)
#define K_ALT_D      (KEYCODE_BASE +   4)
#define K_ALT_E      (KEYCODE_BASE +   5)
#define K_ALT_F      (KEYCODE_BASE +   6)
#define K_ALT_G      (KEYCODE_BASE +   7)
#define K_ALT_H      (KEYCODE_BASE +   8)
#define K_ALT_I      (KEYCODE_BASE +   9)
#define K_ALT_J      (KEYCODE_BASE +  10)
#define K_ALT_K      (KEYCODE_BASE +  11)
#define K_ALT_L      (KEYCODE_BASE +  12)
#define K_ALT_M      (KEYCODE_BASE +  13)
#define K_ALT_N      (KEYCODE_BASE +  14)
#define K_ALT_O      (KEYCODE_BASE +  15)
#define K_ALT_P      (KEYCODE_BASE +  16)
#define K_ALT_Q      (KEYCODE_BASE +  17)
#define K_ALT_R      (KEYCODE_BASE +  18)
#define K_ALT_S      (KEYCODE_BASE +  19)
#define K_ALT_T      (KEYCODE_BASE +  20)
#define K_ALT_U      (KEYCODE_BASE +  21)
#define K_ALT_V      (KEYCODE_BASE +  22)
#define K_ALT_W      (KEYCODE_BASE +  23)
#define K_ALT_X      (KEYCODE_BASE +  24)
#define K_ALT_Y      (KEYCODE_BASE +  25)
#define K_ALT_Z      (KEYCODE_BASE +  26)
#define K_ALT_0      (KEYCODE_BASE +  48)
#define K_ALT_1      (KEYCODE_BASE +  49)
#define K_ALT_2      (KEYCODE_BASE +  50)
#define K_ALT_3      (KEYCODE_BASE +  51)
#define K_ALT_4      (KEYCODE_BASE +  52)
#define K_ALT_5      (KEYCODE_BASE +  53)
#define K_ALT_6      (KEYCODE_BASE +  54)
#define K_ALT_7      (KEYCODE_BASE +  55)
#define K_ALT_8      (KEYCODE_BASE +  56)
#define K_ALT_9      (KEYCODE_BASE +  57)
#define K_F1         (KEYCODE_BASE +  64)
#define K_F2         (KEYCODE_BASE +  65)
#define K_F3         (KEYCODE_BASE +  66)
#define K_F4         (KEYCODE_BASE +  67)
#define K_F5         (KEYCODE_BASE +  68)
#define K_F6         (KEYCODE_BASE +  69)
#define K_F7         (KEYCODE_BASE +  70)
#define K_F8         (KEYCODE_BASE +  71)
#define K_F9         (KEYCODE_BASE +  72)
#define K_F10        (KEYCODE_BASE +  73)
#define K_F11        (KEYCODE_BASE +  74)
#define K_F12        (KEYCODE_BASE +  75)
#define K_SFT_F1     (KEYCODE_BASE +  80)
#define K_SFT_F2     (KEYCODE_BASE +  81)
#define K_SFT_F3     (KEYCODE_BASE +  82)
#define K_SFT_F4     (KEYCODE_BASE +  83)
#define K_SFT_F5     (KEYCODE_BASE +  84)
#define K_SFT_F6     (KEYCODE_BASE +  85)
#define K_SFT_F7     (KEYCODE_BASE +  86)
#define K_SFT_F8     (KEYCODE_BASE +  87)
#define K_SFT_F9     (KEYCODE_BASE +  88)
#define K_SFT_F10    (KEYCODE_BASE +  89)
#define K_SFT_F11    (KEYCODE_BASE +  90)
#define K_SFT_F12    (KEYCODE_BASE +  91)
#define K_CTL_F1     (KEYCODE_BASE +  96)
#define K_CTL_F2     (KEYCODE_BASE +  97)
#define K_CTL_F3     (KEYCODE_BASE +  98)
#define K_CTL_F4     (KEYCODE_BASE +  99)
#define K_CTL_F5     (KEYCODE_BASE + 100)
#define K_CTL_F6     (KEYCODE_BASE + 101)
#define K_CTL_F7     (KEYCODE_BASE + 102)
#define K_CTL_F8     (KEYCODE_BASE + 103)
#define K_CTL_F9     (KEYCODE_BASE + 104)
#define K_CTL_F10    (KEYCODE_BASE + 105)
#define K_CTL_F11    (KEYCODE_BASE + 106)
#define K_CTL_F12    (KEYCODE_BASE + 107)
#define K_ALT_F1     (KEYCODE_BASE + 112)
#define K_ALT_F2     (KEYCODE_BASE + 113)
#define K_ALT_F3     (KEYCODE_BASE + 114)
#define K_ALT_F4     (KEYCODE_BASE + 115)
#define K_ALT_F5     (KEYCODE_BASE + 116)
#define K_ALT_F6     (KEYCODE_BASE + 117)
#define K_ALT_F7     (KEYCODE_BASE + 118)
#define K_ALT_F8     (KEYCODE_BASE + 119)
#define K_ALT_F9     (KEYCODE_BASE + 120)
#define K_ALT_F10    (KEYCODE_BASE + 121)
#define K_ALT_F11    (KEYCODE_BASE + 122)
#define K_ALT_F12    (KEYCODE_BASE + 123)
#define K_NULCHAR    (KEYCODE_BASE + 144)
#define K_BACKTAB    (KEYCODE_BASE + 145)
#define K_LEFT       (KEYCODE_BASE + 160)
#define K_RIGHT      (KEYCODE_BASE + 161)
#define K_UP         (KEYCODE_BASE + 162)
#define K_DOWN       (KEYCODE_BASE + 163)
#define K_HOME       (KEYCODE_BASE + 164)
#define K_END        (KEYCODE_BASE + 165)
#define K_PGUP       (KEYCODE_BASE + 166)
#define K_PGDN       (KEYCODE_BASE + 167)
#define K_INS        (KEYCODE_BASE + 168)
#define K_DEL        (KEYCODE_BASE + 169)
#define K_PAD_CENTER (KEYCODE_BASE + 170)
#define K_CTL_LEFT   (KEYCODE_BASE + 224)
#define K_CTL_RIGHT  (KEYCODE_BASE + 225)
#define K_CTL_UP     (KEYCODE_BASE + 226)
#define K_CTL_DOWN   (KEYCODE_BASE + 227)
#define K_CTL_HOME   (KEYCODE_BASE + 228)
#define K_CTL_END    (KEYCODE_BASE + 229)
#define K_CTL_PGUP   (KEYCODE_BASE + 230)
#define K_CTL_PGDN   (KEYCODE_BASE + 231)
#define K_CTL_INS    (KEYCODE_BASE + 232)
#define K_CTL_DEL    (KEYCODE_BASE + 233)
#define K_SCRLUP     (KEYCODE_BASE + 234)
#define K_SCRLDN     (KEYCODE_BASE + 235)
#define K_INSLN      (KEYCODE_BASE + 236)
#define K_DELLN      (KEYCODE_BASE + 237)
#define K_ERASE      (KEYCODE_BASE + 238)
#define K_CTL_NL     (KEYCODE_BASE + 239)
#define K_NULLCMD    (KEYCODE_BASE + 244)
#define K_REDRAW     (KEYCODE_BASE + 245)
#define K_NEWWINDOW  (KEYCODE_BASE + 246)
#define K_MOUSE1     (KEYCODE_BASE + 247)
#define K_MOUSE2     (KEYCODE_BASE + 248)
#define K_MOUSE3     (KEYCODE_BASE + 249)
#define K_MOUSE4     (KEYCODE_BASE + 250)
#define K_MOUSE5     (KEYCODE_BASE + 251)
#define K_UNDEF      (KEYCODE_BASE + 255)
#define K_NONE       (KEYCODE_BASE + 256)

/*
    MOUSE1ON,  MOUSE1OFF, MOUSE2ON,  MOUSE2OFF, MOUSE3ON,  MOUSE3OFF,
    CHARACTER, CTL_BS,
*/


chartype kbdGetc (void);
booltype kbdKeyPressed (void);
chartype kbdRawGetc (void);
void kbdShut (void);

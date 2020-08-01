/********************************************************************/
/*                                                                  */
/*  con_con.c     Driver for conio console access.                  */
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
/*  File: seed7/src/con_con.c                                       */
/*  Changes: 1994  Thomas Mertes                                    */
/*  Content: Driver for conio console access.                       */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "conio.h"

#include "common.h"
#include "con_drv.h"
#include "kbd_drv.h"


/* #define atexit(x) */

#define SCRHEIGHT 25
#define SCRWIDTH 80

#define black 0
#define green 2
#define lightgray 7
#define darkgray 8
#define yellow 14
#define white 15

static char currentattribute;
static char outbuffer[SCRWIDTH];
static boolType console_initialized = FALSE;
static boolType cursor_on = FALSE;


#ifdef MAP_TO_ISO
static char MAP[] = {
/*   0 */ '\000','\001','\002','\003','\004','\005','\006','\007','\010','\011',
/*  10 */ '\012','\013','\014','\015','\016','\017','\020','\021','\022','\023',
/*  20 */ '\024','\025','\026','\027','\030','\031','\032','\033','\034','\035',
/*  30 */ '\036','\037',' ',   '!',   '\"',  '#',   '$',   '%',   '&',   '\'',
/*  40 */ '(',   ')',   '*',   '+',   ',',   '-',   '.',   '/',   '0',   '1',
/*  50 */ '2',   '3',   '4',   '5',   '6',   '7',   '8',   '9',   ':',   ';',
/*  60 */ '<',   '=',   '>',   '?',   '@',   'A',   'B',   'C',   'D',   'E',
/*  70 */ 'F',   'G',   'H',   'I',   'J',   'K',   'L',   'M',   'N',   'O',
/*  80 */ 'P',   'Q',   'R',   'S',   'T',   'U',   'V',   'W',   'X',   'Y',
/*  90 */ 'Z',   '[',   '\\',  ']',   '^',   '_',   '`',   'a',   'b',   'c',
/* 100 */ 'd',   'e',   'f',   'g',   'h',   'i',   'j',   'k',   'l',   'm',
/* 110 */ 'n',   'o',   'p',   'q',   'r',   's',   't',   'u',   'v',   'w',
/* 120 */ 'x',   'y',   'z',   '{',   '|',   '}',   '~',   '#',   ' ',   ' ',
/* 130 */ ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',
/* 140 */ ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',
/* 150 */ ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',
/* 160 */ ' ',   '≠',   'õ',   'ú',   '',   'ù',   '|',   '',   '\"',  '?',
/* 170 */ '¶',   'Æ',   '™',   '-',   '?',   '?',   '¯',   'Ò',   '˝',   '?',
/* 180 */ '\'',  'Ê',   '',   '˘',   ',',   '?',   'ß',   'Ø',   '¨',   '´',
/* 190 */ '?',   '®',   '?',   '?',   '?',   '?',   'é',   'è',   'í',   'Ä',
/* 200 */ '?',   'ê',   '?',   '?',   '?',   '?',   '?',   '?',   '?',   '•',
/* 210 */ '?',   '?',   '?',   '?',   'ô',   '?',   '?',   '?',   '?',   '?',
/* 220 */ 'ö',   '?',   '?',   '·',   'Ö',   '†',   'É',   '?',   'Ñ',   'Ü',
/* 230 */ 'ë',   'á',   'ä',   'Ç',   'à',   'â',   'ç',   '°',   'å',   'ã',
/* 240 */ 'Î',   '§',   'ï',   '¢',   'ì',   '?',   'î',   'ˆ',   '?',   'ó',
/* 250 */ '£',   'ñ',   'Å',   '?',   '?',   'ò'};
#endif


static int MAP_KEY[] = {
/*   0 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_NULCHAR,  K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/*   8 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_BACKTAB,
/*  16 */ K_ALT_Q,  K_ALT_W,   K_ALT_E,  K_ALT_R,    K_ALT_T, K_ALT_Y, K_ALT_U, K_ALT_I,
/*  24 */ K_ALT_O,  K_ALT_P,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_ALT_A, K_ALT_S,
/*  32 */ K_ALT_D,  K_ALT_F,   K_ALT_G,  K_ALT_H,    K_ALT_J, K_ALT_K, K_ALT_L, K_UNDEF,
/*  40 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_ALT_Z, K_ALT_X, K_ALT_C, K_ALT_V,
/*  48 */ K_ALT_B,  K_ALT_N,   K_ALT_M,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/*  56 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_F1,       K_F2, K_F3, K_F4, K_F5,
/*  64 */ K_F6,     K_F7,      K_F8,     K_F9,       K_F10, K_UNDEF, K_UNDEF, K_HOME,
/*  72 */ K_UP,     K_PGUP,    K_UNDEF,  K_LEFT,     K_UNDEF, K_RIGHT, K_UNDEF, K_END,
/*  80 */ K_DOWN,   K_PGDN,    K_INS,    K_DEL,      K_SFT_F1, K_SFT_F2, K_SFT_F3, K_SFT_F4,
/*  88 */ K_SFT_F5, K_SFT_F6,  K_SFT_F7, K_SFT_F8,   K_SFT_F9, K_SFT_F10, K_CTL_F1, K_CTL_F2,
/*  96 */ K_CTL_F3, K_CTL_F4,  K_CTL_F5, K_CTL_F6,   K_CTL_F7, K_CTL_F8, K_CTL_F9, K_CTL_F10,
/* 104 */ K_ALT_F1, K_ALT_F2,  K_ALT_F3, K_ALT_F4,   K_ALT_F5, K_ALT_F6, K_ALT_F7, K_ALT_F8,
/* 112 */ K_ALT_F9, K_ALT_F10, K_UNDEF,  K_CTL_LEFT, K_CTL_RIGHT, K_CTL_END, K_CTL_PGDN, K_CTL_HOME,
/* 120 */ K_ALT_1,  K_ALT_2,   K_ALT_3,  K_ALT_4,    K_ALT_5, K_ALT_6, K_ALT_7, K_ALT_8,
/* 128 */ K_ALT_9,  K_ALT_0,   K_UNDEF,  K_UNDEF,    K_CTL_PGUP, K_UNDEF, K_UNDEF, K_UNDEF,
/* 136 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 144 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 152 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 160 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_CTL_INS, K_CTL_DEL, K_UNDEF,
/* 168 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 176 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 184 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 192 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 200 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 208 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 216 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 224 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 232 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 240 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
/* 248 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF};



void kbdShut (void)

  { /* kbdShut */
  } /* kbdShut */



boolType kbdKeyPressed (void)

  { /* kbdKeyPressed */
    return kbhit();
  } /* kbdKeyPressed */



charType kbdGetc (void)

  {
    charType key;

  /* kbdGetc */
    key = getch();
    if (key == 0) {
      key = getch();
      key = MAP_KEY[key];
    } /* if */
    if (key == 13) {
      key = 10;
    } /* if */
#ifdef OUT_OF_ORDER
      switch (key) {
        case 59: key = K_F1;     break;
        case 60: key = K_F2;     break;
        case 61: key = K_F3;     break;
        case 62: key = K_F4;     break;
        case 63: key = K_F5;     break;
        case 64: key = K_F6;     break;
        case 65: key = K_F7;     break;
        case 66: key = K_F8;     break;
        case 67: key = K_F9;     break;
        case 68: key = K_F10;    break;
        case 71: key = K_HOME;   break;
        case 72: key = K_UP;     break;
        case 73: key = K_PGUP;   break;
        case 75: key = K_LEFT;   break;
        case 77: key = K_RIGHT;  break;
        case 79: key = K_END;    break;
        case 80: key = K_DOWN;   break;
        case 81: key = K_PGDN;   break;
        case 84: key = K_SFT_F1; break;
        case 85: key = K_SFT_F2; break;
        case 86: key = K_SFT_F3; break;
        case 87: key = K_SFT_F4; break;
        case 88: key = K_SFT_F5; break;
        case 89: key = K_SFT_F6; break;
        case 90: key = K_SFT_F7; break;
        case 91: key = K_SFT_F8; break;
        case 92: key = K_SFT_F9; break;
        case 93: key = K_SFT_F10;break;
    } /* if */
#endif
    return key;
  } /* kbdGetc */



charType kbdRawGetc (void)

  { /* kbdRawGetc */
    return kbdGetc();
  } /* kbdRawGetc */



static void con_beep (void)

  { /* con_beep */
    fputc('\007', stderr);
  } /* con_beep */



static void con_setcolor (intType foreground, intType background)

  { /* con_setcolor */
    currentattribute = (char) (foreground + 16 * (background % 8));
  } /* con_setcolor */



static void con_standardcolour (void)

  { /* con_standardcolour */
    con_setcolor(lightgray, black);
  } /* con_standardcolour */



static void con_normalcolour (void)

  { /* con_normalcolour */
    con_setcolor(lightgray, black);
  } /* con_normalcolour */



static void con_setfont (char *fontname)

  { /* con_setfont */
  } /* con_setfont */



intType textheight (void)

  { /* textheight */
    return 1;
  } /* textheight */



intType textwidth (striType stri,
    intType startcol, intType stopcol)

  { /* textwidth */
    return stopcol + 1 - startcol;
  } /* textwidth */



void textcolumns (striType stri, intType striwidth,
    intType * cols, intType *rest)

  { /* textcolumns */
    *cols = striwidth;
    *rest = 0;
  } /* textcolumns */



int conHeight (void)

  { /* conHeight */
    return SCRHEIGHT;
  } /* conHeight */



int conWidth (void)

  { /* conWidth */
    return SCRWIDTH;
  } /* conWidth */



void conFlush (void)

  { /* conFlush */
  } /* conFlush */



void conCursor (boolType on)

  { /* conCursor */
    cursor_on = on;
    if (on) {
      _setcursortype(_SOLIDCURSOR);
    } else {
      _setcursortype(_NOCURSOR);
    } /* if */
  } /* conCursor */



void conSetCursor (intType lin, intType col)

  /* Moves the system curser to the given place of the console.     */
  /* When no system cursor exists this procedure can be replaced by */
  /* a dummy procedure.                                             */

  { /* conSetCursor */
    gotoxy(col, lin);
  } /* conSetCursor */



void conText (intType lin, intType col, ustriType stri,
memSizeType length)

  /* This function writes the string stri to the console at the     */
  /* position (lin, col). The position (lin, col) must be a legal   */
  /* position of the console. The string stri is not allowed to go  */
  /* beyond the right border of the console. All console output     */
  /* must be done with this function.                               */

  { /* conText */
    memcpy(outbuffer, stri, length);
    outbuffer[length] = '\0';
    conSetCursor(lin, col);
    cputs(outbuffer);
  } /* conText */



void conClear (intType startlin, intType startcol,
    intType stoplin, intType stopcol)

  /* Clears the area described by startlin, stoplin, startcol and   */
  /* stopcol.                                                       */

  { /* conClear */
    clrscr();
  } /* conClear */



void conUpScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  /* Scrolls the area inside startlin, startcol, stoplin and        */
  /* stopcol upward by count lines. The upper count lines of the    */
  /* area are overwritten. At the lower end of the area blank lines */
  /* are inserted. Nothing is changed outside the area.             */

  { /* conUpScroll */
  } /* conUpScroll */



void conDownScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  /* Scrolls the area inside startlin, startcol, stoplin and        */
  /* stopcol downward by count lines. The lower count lines of the  */
  /* area are overwritten. At the upper end of the area blank lines */
  /* are inserted. Nothing is changed outside the area.             */

  { /* conDownScroll */
  } /* conDownScroll */



void conLeftScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  /* Scrolls the area inside startlin, startcol, stoplin and        */
  /* stopcol leftward by count lines. The left count lines of the   */
  /* area are overwritten. At the right end of the area blank lines */
  /* are inserted. Nothing is changed outside the area.             */

  { /* conLeftScroll */
  } /* conLeftScroll */



void conRightScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  /* Scrolls the area inside startlin, startcol, stoplin and        */
  /* stopcol rightward by count lines. The right count lines of the */
  /* area are overwritten. At the left end of the area blank lines  */
  /* are inserted. Nothing is changed outside the area.             */

  { /* conRightScroll */
  } /* conRightScroll */



void conShut (void)

  { /* conShut */
    if (console_initialized) {
      con_standardcolour();
      conCursor(TRUE);
      conClear(1, 1, 25, 80);
      conSetCursor(1, 24);
      console_initialized = FALSE;
    } /* if */
  } /* conShut */



int conOpen (void)

  /* Initializes and clears the console.                            */

  { /* conOpen */
    con_normalcolour();
    conClear(1, 1, 25, 80);
    conCursor(FALSE);
    console_initialized = TRUE;
    atexit(conShut);
    return 1;
  } /* conOpen */

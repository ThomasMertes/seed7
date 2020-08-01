/********************************************************************/
/*                                                                  */
/*  con_dos.c     Driver for dos console access.                    */
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
/*  File: seed7/src/con_dos.c                                       */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Driver for dos console access.                         */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "dos.h"

#include "common.h"
#include "kbd_drv.h"

#undef EXTERN
#define EXTERN
#include "con_drv.h"

#define atexit(x)


#define SCRHEIGHT 25
#define SCRWIDTH 80

#define black 0
#define green 2
#define lightgray 7
#define darkgray 8
#define yellow 14
#define white 15

typedef struct {
  char character, attribute;
} cellType;

typedef cellType lineOfScreenType[SCRWIDTH];

typedef struct {
    cellType screen[SCRHEIGHT][SCRWIDTH];
  } screenType;

static boolType monochrom;
static screenType *current_screen;
static char currentattribute;
static lineOfScreenType outbuffer;
static boolType console_initialized = FALSE;
static boolType cursor_on = FALSE;

static unsigned char cursor_startline;
static unsigned char cursor_endline;


static charType map_from_437[] = {
/*   0 */    0,    1,    2,    3,    4,    5,    6,    7,    8,    9,
/*  10 */   10,   11,   12,   13,   14,   15,   16,   17,   18,   19,
/*  20 */   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,
/*  30 */   30,   31,  ' ',  '!',  '"',  '#',  '$',  '%',  '&', '\'',
/*  40 */  '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',  '0',  '1',
/*  50 */  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  ':',  ';',
/*  60 */  '<',  '=',  '>',  '?',  '@',  'A',  'B',  'C',  'D',  'E',
/*  70 */  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
/*  80 */  'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',
/*  90 */  'Z',  '[', '\\',  ']',  '^',  '_',  '`',  'a',  'b',  'c',
/* 100 */  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',
/* 110 */  'n',  'o',  'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
/* 120 */  'x',  'y',  'z',  '{',  '|',  '}',  '~',  127,  199,  252,
/* 130 */  233,  226,  228,  224,  229,  231,  234,  235,  232,  239,
/* 140 */  238,  236,  196,  197,  201,  230,  198,  244,  246,  242,
/* 150 */  251,  249,  255,  214,  220,  162,  163,  165, 8359,  402,
/* 160 */  225,  237,  243,  250,  241,  209,  170,  186,  191, 8976,
/* 170 */  172,  189,  188,  161,  171,  187, 9617, 9618, 9619, 9474,
/* 180 */ 9508, 9569, 9570, 9558, 9557, 9571, 9553, 9559, 9565, 9564,
/* 190 */ 9563, 9488, 9492, 9524, 9516, 9500, 9472, 9532, 9566, 9567,
/* 200 */ 9562, 9556, 9577, 9574, 9568, 9552, 9580, 9575, 9576, 9572,
/* 210 */ 9573, 9561, 9560, 9554, 9555, 9579, 9578, 9496, 9484, 9608,
/* 220 */ 9604, 9612, 9616, 9600,  945,  223,  915,  960,  931,  963,
/* 230 */  181,  964,  934,  920,  937,  948, 8734,  966,  949, 8745,
/* 240 */ 8801,  177, 8805, 8804, 8992, 8993,  247, 8776,  176, 8729,
/* 250 */  183, 8730, 8319,  178, 9632,  160};


static int map_key[] = {
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
/* 128 */ K_ALT_9,  K_ALT_0,   K_UNDEF,  K_UNDEF,    K_CTL_PGUP, K_UNDEF, K_CTL_PGUP, K_UNDEF,
/* 136 */ K_UNDEF,  K_UNDEF,   K_UNDEF,  K_UNDEF,    K_UNDEF, K_CTL_UP, K_UNDEF, K_UNDEF,
/* 144 */ K_UNDEF,  K_CTL_DOWN, K_CTL_INS, K_CTL_DEL, K_UNDEF, K_UNDEF, K_UNDEF, K_UNDEF,
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
    return (char) bdos(0xB, 0, 0) & 1;
  } /* kbdKeyPressed */



charType kbdGetc (void)

  {
    union REGS r;
    charType key;

  /* kbdGetc */
    r.h.ah = (unsigned char) 0;
    int86(0x16, &r, &r);
    key = (charType) r.h.al;
    if (key == 0) {
      key = map_key[r.h.ah];
    } else {
      key = map_from_437[key & 0xFF];
    } /* if */
    if (key == 13) {
      key = 10;
    } /* if */
    return key;
  } /* kbdGetc */



charType kbdRawGetc (void)

  { /* kbdRawGetc */
    return kbdGetc();
  } /* kbdRawGetc */



void snd_beep (void)

  { /* snd_beep */
    fputc('\007', stderr);
  } /* snd_beep */



void setcolour (intType foreground, intType background)

  { /* setcolour */
    currentattribute = (char) (foreground + 16 * (background % 8));
  } /* setcolour */



void standardcolour (void)

  { /* standardcolour */
    setcolour(lightgray, black);
  } /* standardcolour */



void normalcolour (void)

  { /* normalcolour */
    setcolour(lightgray, black);
  } /* normalcolour */



void setfont (char *fontname)

  { /* setfont */
  } /* setfont */



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

  {
    union REGS r;

  /* conCursor */
    cursor_on = on;
    if (on) {
      r.h.ah = (unsigned char) 1; /* set cursor type */
      r.h.ch = (unsigned char) cursor_startline;
      r.h.cl = (unsigned char) cursor_endline;
      int86(0x10, &r, &r);
    } else {
      r.h.ah = (unsigned char) 1; /* set cursor type */
      r.h.ch = (unsigned char) 32;
      r.h.cl = (unsigned char) 0;
      int86(0x10, &r, &r);
    } /* if */
  } /* conCursor */



void conSetCursor (intType line, intType column)

  /* Moves the system curser to the given place of the console.     */
  /* When no system cursor exists this procedure can be replaced by */
  /* a dummy procedure.                                             */

  {
    union REGS r;

  /* conSetCursor */
    r.h.ah = (unsigned char) 2; /* cursor addressing function */
    r.h.dh = (unsigned char) (line - 1);
    r.h.dl = (unsigned char) (column - 1);
    r.h.bh = (unsigned char) 0; /* video page */
    int86(0x10, &r, &r);
  } /* conSetCursor */



void conText (intType lin, intType col, ustriType stri,
memSizeType length)

  /* This function writes the string stri to the console at the     */
  /* position (lin, col). The position (lin, col) must be a legal   */
  /* position of the console. The string stri is not allowed to go  */
  /* beyond the right border of the console. All console output     */
  /* must be done with this function.                               */

  {
    intType pos;

  /* conText */
    for (pos = 0; pos < length; pos++) {
#ifdef MAP_TO_ISO
      outbuffer[pos].character = MAP[stri[pos] & 0xFF];
#else
      outbuffer[pos].character = stri[pos];
#endif
      outbuffer[pos].attribute = currentattribute;
    } /* for */
    memcpy(&current_screen->screen[lin - 1][col - 1],
      outbuffer, 2 * length);
  } /* conText */



void conClear (intType startlin, intType startcol,
    intType stoplin, intType stopcol)

  /* Clears the area described by startlin, stoplin, startcol and   */
  /* stopcol.                                                       */

  {
    union REGS r;

  /* conClear */
    r.h.ah = (unsigned char) 6; /* scroll up code */
    r.h.al = (unsigned char) 0; /* clear screen code */
    r.h.ch = (unsigned char) (startlin - 1);
    r.h.cl = (unsigned char) (startcol - 1);
    r.h.dh = (unsigned char) (stoplin - 1);
    r.h.dl = (unsigned char) (stopcol - 1);
    r.h.bh = (unsigned char) currentattribute; /* blank line colour */
    int86(0x10, &r, &r);
  } /* conClear */



void conUpScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  /* Scrolls the area inside startlin, startcol, stoplin and        */
  /* stopcol upward by count lines. The upper count lines of the    */
  /* area are overwritten. At the lower end of the area blank lines */
  /* are inserted. Nothing is changed outside the area.             */

  {
    union REGS r;

  /* conUpScroll */
    r.h.ah = (unsigned char) 6; /* scroll up code */
    r.h.al = (unsigned char) count;
    r.h.ch = (unsigned char) (startlin - 1);
    r.h.cl = (unsigned char) (startcol - 1);
    r.h.dh = (unsigned char) (stoplin - 1);
    r.h.dl = (unsigned char) (stopcol - 1);
    r.h.bh = (unsigned char) 7; /* blank line is black */
    int86(0x10, &r, &r);
  } /* conUpScroll */



void conDownScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  /* Scrolls the area inside startlin, startcol, stoplin and        */
  /* stopcol downward by count lines. The lower count lines of the  */
  /* area are overwritten. At the upper end of the area blank lines */
  /* are inserted. Nothing is changed outside the area.             */

  {
    union REGS r;

  /* conDownScroll */
    r.h.ah = (unsigned char) 7; /* scroll down code */
    r.h.al = (unsigned char) count;
    r.h.ch = (unsigned char) (startlin - 1);
    r.h.cl = (unsigned char) (startcol - 1);
    r.h.dh = (unsigned char) (stoplin - 1);
    r.h.dl = (unsigned char) (stopcol - 1);
    r.h.bh = (unsigned char) 7; /* blank line is black */
    int86(0x10, &r, &r);
  } /* conDownScroll */



void conLeftScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  /* Scrolls the area inside startlin, startcol, stoplin and        */
  /* stopcol leftward by count lines. The left count lines of the   */
  /* area are overwritten. At the right end of the area blank lines */
  /* are inserted. Nothing is changed outside the area.             */

  {
    int line;
    int num_bytes;
    char *destination;
    char *source;

  /* conLeftScroll */
    if (count > 0) {
      num_bytes = 2 * (stopcol - startcol - count + 1);
      source = (char *) &current_screen->
          screen[startlin - 1][startcol + count - 1];
      destination = (char *) &current_screen->
          screen[startlin - 1][startcol - 1];
      for (line = startlin; line <= stoplin; line++) {
        memmove(destination, source, num_bytes);
        source += 160;
        destination += 160;
      } /* for */
    } /* if */
  } /* conLeftScroll */



void conRightScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  /* Scrolls the area inside startlin, startcol, stoplin and        */
  /* stopcol rightward by count lines. The right count lines of the */
  /* area are overwritten. At the left end of the area blank lines  */
  /* are inserted. Nothing is changed outside the area.             */

  {
    int line;
    int num_bytes;
    char *destination;
    char *source;

  /* conRightScroll */
    if (count > 0) {
      num_bytes = 2 * (stopcol - startcol - count + 1);
      source = (char *) &current_screen->
          screen[startlin - 1][startcol - 1];
      destination = (char *) &current_screen->
          screen[startlin - 1][startcol + count - 1];
      for (line = startlin; line <= stoplin; line++) {
        memmove(destination, source, num_bytes);
        source += 160;
        destination += 160;
      } /* for */
    } /* if */
  } /* conRightScroll */



void conShut (void)

  { /* conShut */
    if (console_initialized) {
      standardcolour();
      conCursor(TRUE);
      conClear(1, 1, 25, 80);
      conSetCursor(1, 24);
      console_initialized = FALSE;
    } /* if */
  } /* conShut */



int conOpen (void)

  /* Initializes and clears the console.                            */

  {
    union REGS r;

  /* conOpen */
    normalcolour();
    conClear(1, 1, 25, 80);

    /* Lowlevel request to find out the video state */
    r.h.ah = (unsigned char) 15; /* read video state */
    int86(0x10, &r, &r);
    if (r.h.al == 7) {
      monochrom = TRUE;
      current_screen = (screenType *) 0xB0000000;
    } else {
      monochrom = FALSE;
      current_screen = (screenType *) 0xB8000000;
    } /* if */
    r.h.ah = (unsigned char) 3; /* read cursor position */
    r.h.bh = (unsigned char) 0;
    int86(0x10, &r, &r);
    cursor_startline = r.h.ch;
    cursor_endline = r.h.cl;
    conCursor(FALSE);
    console_initialized = TRUE;
    atexit(conShut);
    return 1;
  } /* conOpen */

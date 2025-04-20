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

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "dos.h"

#include "common.h"
#include "striutl.h"
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


static charType map_key[] = {
/*   0 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_NULCHAR,   K_UNDEF,
/*   5 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/*  10 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/*  15 */ K_BACKTAB,   K_ALT_Q,     K_ALT_W,     K_ALT_E,     K_ALT_R,
/*  20 */ K_ALT_T,     K_ALT_Y,     K_ALT_U,     K_ALT_I,     K_ALT_O,
/*  25 */ K_ALT_P,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/*  30 */ K_ALT_A,     K_ALT_S,     K_ALT_D,     K_ALT_F,     K_ALT_G,
/*  35 */ K_ALT_H,     K_ALT_J,     K_ALT_K,     K_ALT_L,     K_UNDEF,
/*  40 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_ALT_Z,
/*  45 */ K_ALT_X,     K_ALT_C,     K_ALT_V,     K_ALT_B,     K_ALT_N,
/*  50 */ K_ALT_M,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/*  55 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_F1,
/*  60 */ K_F2,        K_F3,        K_F4,        K_F5,        K_F6,
/*  65 */ K_F7,        K_F8,        K_F9,        K_F10,       K_UNDEF,
/*  70 */ K_UNDEF,     K_HOME,      K_UP,        K_PGUP,      K_UNDEF,
/*  75 */ K_LEFT,      K_UNDEF,     K_RIGHT,     K_UNDEF,     K_END,
/*  80 */ K_DOWN,      K_PGDN,      K_INS,       K_DEL,       K_SFT_F1,
/*  85 */ K_SFT_F2,    K_SFT_F3,    K_SFT_F4,    K_SFT_F5,    K_SFT_F6,
/*  90 */ K_SFT_F7,    K_SFT_F8,    K_SFT_F9,    K_SFT_F10,   K_CTL_F1,
/*  95 */ K_CTL_F2,    K_CTL_F3,    K_CTL_F4,    K_CTL_F5,    K_CTL_F6,
/* 100 */ K_CTL_F7,    K_CTL_F8,    K_CTL_F9,    K_CTL_F10,   K_ALT_F1,
/* 105 */ K_ALT_F2,    K_ALT_F3,    K_ALT_F4,    K_ALT_F5,    K_ALT_F6,
/* 110 */ K_ALT_F7,    K_ALT_F8,    K_ALT_F9,    K_ALT_F10,   K_UNDEF,
/* 115 */ K_CTL_LEFT,  K_CTL_RIGHT, K_CTL_END,   K_CTL_PGDN,  K_CTL_HOME,
/* 120 */ K_ALT_1,     K_ALT_2,     K_ALT_3,     K_ALT_4,     K_ALT_5,
/* 125 */ K_ALT_6,     K_ALT_7,     K_ALT_8,     K_ALT_9,     K_ALT_0,
/* 130 */ K_UNDEF,     K_UNDEF,     K_CTL_PGUP,  K_F11,       K_F12,
/* 135 */ K_SFT_F11,   K_SFT_F12,   K_CTL_F11,   K_CTL_F12,   K_ALT_F11,
/* 140 */ K_ALT_F12,   K_CTL_UP,    K_UNDEF,     K_UNDEF,     K_UNDEF,
/* 145 */ K_CTL_DOWN,  K_CTL_INS,   K_CTL_DEL,   K_UNDEF,     K_UNDEF,
/* 150 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/* 155 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/* 160 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/* 165 */ K_CTL_INS,   K_CTL_DEL,   K_UNDEF,     K_UNDEF,     K_UNDEF,
/* 170 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/* 175 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/* 180 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/* 185 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/* 190 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/* 195 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/* 200 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/* 205 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/* 210 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/* 215 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/* 220 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/* 225 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/* 230 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/* 235 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/* 240 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/* 245 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/* 250 */ K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,     K_UNDEF,
/* 255 */ K_UNDEF};



void kbdShut (void)

  { /* kbdShut */
  } /* kbdShut */



boolType kbdInputReady (void)

  { /* kbdInputReady */
    return (char) bdos(0xB, 0, 0) & 1;
  } /* kbdInputReady */



charType kbdGetc (void)

  {
    union REGS r;
    charType key;

  /* kbdGetc */
    logFunction(printf("kbdGetc\n"););
    r.h.ah = (unsigned char) 0;
    int86(0x16, &r, &r);
    key = (charType) r.h.al;
    if (key == 0) {
      key = map_key[r.h.ah];
    } else {
      key = mapFromCodePage((unsigned char) (key & 0xFF));
    } /* if */
    if (key == 13) {
      key = 10;
    } /* if */
    logFunction(printf("kbdGetc --> %d\n", key););
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



void textcolumns (striType stri, intType striwidth,
    intType *cols, intType *rest)

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



/**
 *  Moves the system cursor to the given place of the console.
 *  If no system cursor exists this function can be replaced by
 *  a dummy function.
 */
void conSetCursor (intType line, intType column)

  {
    union REGS r;

  /* conSetCursor */
    r.h.ah = (unsigned char) 2; /* cursor addressing function */
    r.h.dh = (unsigned char) (line - 1);
    r.h.dl = (unsigned char) (column - 1);
    r.h.bh = (unsigned char) 0; /* video page */
    int86(0x10, &r, &r);
  } /* conSetCursor */



/**
 *  This function writes the string stri to the console at the
 *  position (lin, col). The position (lin, col) must be a legal
 *  position of the console. The string stri is not allowed to go
 *  beyond the right border of the console. All console output
 *  must be done with this function.
 */
void conText (intType lin, intType col, ustriType stri,
memSizeType length)

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



/**
 *  Clears the area described by startlin, stoplin, startcol and stopcol.
 */
void conClear (intType startlin, intType startcol,
    intType stoplin, intType stopcol)

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



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol upward by numLines lines. The upper numLines lines of the
 *  area are overwritten. At the lower end of the area blank lines
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that numLines is greater or equal 1.
 */
void conUpScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType numLines)

  {
    union REGS r;

  /* conUpScroll */
    r.h.ah = (unsigned char) 6; /* scroll up code */
    r.h.al = (unsigned char) numLines;
    r.h.ch = (unsigned char) (startlin - 1);
    r.h.cl = (unsigned char) (startcol - 1);
    r.h.dh = (unsigned char) (stoplin - 1);
    r.h.dl = (unsigned char) (stopcol - 1);
    r.h.bh = (unsigned char) 7; /* blank line is black */
    int86(0x10, &r, &r);
  } /* conUpScroll */



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol downward by numLines lines. The lower numLines lines of the
 *  area are overwritten. At the upper end of the area blank lines
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that numLines is greater or equal 1.
 */
void conDownScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType numLines)

  {
    union REGS r;

  /* conDownScroll */
    r.h.ah = (unsigned char) 7; /* scroll down code */
    r.h.al = (unsigned char) numLines;
    r.h.ch = (unsigned char) (startlin - 1);
    r.h.cl = (unsigned char) (startcol - 1);
    r.h.dh = (unsigned char) (stoplin - 1);
    r.h.dl = (unsigned char) (stopcol - 1);
    r.h.bh = (unsigned char) 7; /* blank line is black */
    int86(0x10, &r, &r);
  } /* conDownScroll */



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol leftward by numCols columns. The left numCols columns of the
 *  area are overwritten. At the right end of the area blank columns
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that numCols is greater or equal 1.
 */
void conLeftScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType numCols)

  {
    int line;
    int num_bytes;
    char *destination;
    char *source;

  /* conLeftScroll */
    if (numCols > 0) {
      num_bytes = 2 * (stopcol - startcol - numCols + 1);
      source = (char *) &current_screen->
          screen[startlin - 1][startcol + numCols - 1];
      destination = (char *) &current_screen->
          screen[startlin - 1][startcol - 1];
      for (line = startlin; line <= stoplin; line++) {
        memmove(destination, source, num_bytes);
        source += 160;
        destination += 160;
      } /* for */
    } /* if */
  } /* conLeftScroll */



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol rightward by numCols columns. The right numCols columns of the
 *  area are overwritten. At the left end of the area blank columns
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that numCols is greater or equal 1.
 */
void conRightScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType numCols)

  {
    int line;
    int num_bytes;
    char *destination;
    char *source;

  /* conRightScroll */
    if (numCols > 0) {
      num_bytes = 2 * (stopcol - startcol - numCols + 1);
      source = (char *) &current_screen->
          screen[startlin - 1][startcol - 1];
      destination = (char *) &current_screen->
          screen[startlin - 1][startcol + numCols - 1];
      for (line = startlin; line <= stoplin; line++) {
        memmove(destination, source, num_bytes);
        source += 160;
        destination += 160;
      } /* for */
    } /* if */
  } /* conRightScroll */



void conShut (void)

  { /* conShut */
    logFunction(printf("conShut\n"););
    if (console_initialized) {
      standardcolour();
      conCursor(TRUE);
      conClear(1, 1, 25, 80);
      conSetCursor(1, 24);
      console_initialized = FALSE;
    } /* if */
    logFunction(printf("conShut -->\n"););
  } /* conShut */



/**
 *  Initializes and clears the console.
 */
int conOpen (void)

  {
    union REGS r;

  /* conOpen */
    logFunction(printf("conOpen\n"););
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
    logFunction(printf("conOpen -->\n"););
    return 1;
  } /* conOpen */

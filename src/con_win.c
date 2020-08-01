/********************************************************************/
/*                                                                  */
/*  con_win.c     Driver for windows console access                 */
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
/*  File: seed7/src/con_win.c                                       */
/*  Changes: 2005  Thomas Mertes                                    */
/*  Content: Driver for windows console access                      */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "windows.h"
#include "conio.h"

#include "common.h"
#include "con_drv.h"
#include "kbd_drv.h"


/* #define atexit(x) */

#define SCRHEIGHT 25
#define SCRWIDTH 80

static char currentattribute;
static booltype console_initialized = FALSE;
static booltype cursor_on = FALSE;


static chartype map_char[] = {
/*   0 */ 0000, 0001, 0002, 0003, 0004, 0005, 0006, 0007, 0010, 0011,
/*  10 */ 0012, 0013, 0014, 0015, 0016, 0017, 0020, 0021, 0022, 0023,
/*  20 */ 0024, 0025, 0026, 0027, 0030, 0031, 0032, 0033, 0034, 0035,
/*  30 */ 0036, 0037, ' ',  '!',  '\"', '#',  '$',  '%',  '&',  '\'',
/*  40 */ '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',  '0',  '1',
/*  50 */ '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  ':',  ';',
/*  60 */ '<',  '=',  '>',  '?',  '@',  'A',  'B',  'C',  'D',  'E',
/*  70 */ 'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
/*  80 */ 'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',
/*  90 */ 'Z',  '[',  '\\', ']',  '^',  '_',  '`',  'a',  'b',  'c',
/* 100 */ 'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',
/* 110 */ 'n',  'o',  'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
/* 120 */ 'x',  'y',  'z',  '{',  '|',  '}',  '~',  0177, 0307, 0374,
/* 130 */ 0351, 0342, 0344, 0340, 0345, 0347, 0352, 0353, 0350, 0357,
/* 140 */ 0356, 0354, 0304, 0305, 0311, 0346, 0306, 0364, 0366, 0362,
/* 150 */ 0373, 0371, 0377, 0326, 0334, 0370, 0243, 0330, 0327, '?',
/* 160 */ 0341, 0355, 0363, 0372, 0361, 0321, 0252, 0272, 0277, 0256,
/* 170 */ 0254, 0275, 0274, 0241, 0253, 0273, '?',  '?',  '?',  '?',
/* 180 */ '?',  0301, 0302, 0300, 0251, '?',  '?',  '?',  '?',  0242,
/* 190 */ 0245, '?',  '?',  '?',  '?',  '?',  '?',  '?',  0343, 0303,
/* 200 */ '?',  '?',  '?',  '?',  '?',  '?',  '?',  0244, 0360, 0320,
/* 210 */ 0312, 0313, 0310, 0271, 0315, 0316, 0317, '?',  '?',  '?',
/* 220 */ '?',  0246, 0314, '?',  0363, 0337, 0324, 0322, 0365, 0325,
/* 230 */ 0265, 0376, 0336, 0332, 0333, 0331, 0375, 0335, 0257, 0264,
/* 240 */ 0255, 0261, '?',  0276, 0266, 0247, 0367, 0270, 0260, 0250,
/* 250 */ 0267, 0271, 0263, 0262, '?',  0240};


static chartype map_key[] = {
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



#ifdef ANSI_C

void kbdShut (void)
#else

void kbdShut ()
#endif

  { /* kbdShut */
  } /* kbdShut */



#ifdef ANSI_C

booltype kbdKeyPressed (void)
#else

booltype kbdKeyPressed ()
#endif

  { /* kbdKeyPressed */
    return(kbhit());
  } /* kbdKeyPressed */



#ifdef ANSI_C

chartype kbdGetc (void)
#else

chartype kbdGetc ()
#endif

  {
    int key;
    chartype result;

  /* kbdGetc */
    key = getch();
    if (key == 0) {
      key = getch();
      result = map_key[key];
    } else if (key == 224) {
      key = getch();
      result = map_key[key];
    } else if (key >= 0 && key <= 255) {
      result = map_char[key];
    } else {
      result = map_char[key & 0xFF];
    } /* if */
    if (result == 13) {
      result = 10;
    } /* if */
    return(result);
  } /* kbdGetc */



#ifdef ANSI_C

chartype kbdRawGetc (void)
#else

chartype kbdRawGetc ()
#endif

  { /* kbdRawGetc */
    return(kbdGetc());
  } /* kbdRawGetc */



#ifdef ANSI_C

static void con_beep (void)
#else

static void con_beep ()
#endif

  { /* con_beep */
    fputc('\007', stderr);
  } /* con_beep */



#ifdef ANSI_C

static void con_setcolor (inttype foreground, inttype background)
#else

static void con_setcolor (foreground, background)
inttype foreground;
inttype background;
#endif

  { /* con_setcolor */
    currentattribute = (char) (foreground + 16 * (background % 8));
  } /* con_setcolor */



#ifdef ANSI_C

static void con_standardcolour (void)
#else

static void con_standardcolour ()
#endif

  { /* con_standardcolour */
    con_setcolor(lightgray, black);
  } /* con_standardcolour */



#ifdef ANSI_C

static void con_normalcolour (void)
#else

static void con_normalcolour ()
#endif

  { /* con_normalcolour */
    con_setcolor(lightgray, black);
  } /* con_normalcolour */



#ifdef ANSI_C

static void con_setfont (char *fontname)
#else

static void con_setfont (fontname)
char *fontname;
#endif

  { /* con_setfont */
  } /* con_setfont */



#ifdef ANSI_C

inttype textheight (void)
#else

inttype textheight ()
#endif

  { /* textheight */
    return 1;
  } /* textheight */



#ifdef ANSI_C

inttype textwidth (stritype stri,
    inttype startcol, inttype stopcol)
#else

inttype textwidth (stri, startcol, stopcol)
stritype stri;
inttype startcol;
inttype stopcol;
#endif

  { /* textwidth */
    return(stopcol + 1 - startcol);
  } /* textwidth */



#ifdef ANSI_C

void textcolumns (stritype stri, inttype striwidth,
    inttype * cols, inttype *rest)
#else

void textcolumns (stri, striwidth, cols, rest)
stritype stri;
inttype striwidth;
inttype *cols;
inttype *rest;
#endif

  { /* textcolumns */
    *cols = striwidth;
    *rest = 0;
  } /* textcolumns */



#ifdef ANSI_C

int conHeight (void)
#else

int conHeight ()
#endif

  {
    HANDLE hConsole;
    CONSOLE_SCREEN_BUFFER_INFO con_info;

  /* conHeight */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      if (GetConsoleScreenBufferInfo(hConsole, &con_info)) {
        return(con_info.dwSize.Y);
      } else {
        /* printf("GetConsoleScreenBufferInfo(%d, & ) ==> Error %d\n",
            hConsole, GetLastError()); */
        return(SCRHEIGHT);
      } /* if */
    } else {
      /* printf("GetStdHandle(STD_OUTPUT_HANDLE) ==> %d / Error %d\n",
          hConsole, GetLastError()); */
      return(SCRHEIGHT);
    } /* if */
  } /* conHeight */



#ifdef ANSI_C

int conWidth (void)
#else

int conWidth ()
#endif

  {
    HANDLE hConsole;
    CONSOLE_SCREEN_BUFFER_INFO con_info;

  /* conWidth */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      if (GetConsoleScreenBufferInfo(hConsole, &con_info)) {
        return(con_info.dwSize.X);
      } else {
        /* printf("GetConsoleScreenBufferInfo(%d, & ) ==> Error %d\n",
            hConsole, GetLastError()); */
        return(SCRWIDTH);
      } /* if */
    } else {
      /* printf("GetStdHandle(STD_OUTPUT_HANDLE) ==> %d / Error %d\n",
          hConsole, GetLastError()); */
      return(SCRWIDTH);
    } /* if */
  } /* conWidth */



#ifdef ANSI_C

void conFlush (void)
#else

void conFlush ()
#endif

  { /* conFlush */
  } /* conFlush */



#ifdef ANSI_C

void conCursor (booltype on)
#else

void conCursor (on)
booltype on;
#endif

  { /* conCursor */
    cursor_on = on;
#ifdef OUT_OF_ORDER
    if (on) {
      _setcursortype(_SOLIDCURSOR);
    } else {
      _setcursortype(_NOCURSOR);
    } /* if */
#endif
  } /* conCursor */



#ifdef ANSI_C

void conSetCursor (inttype lin, inttype col)
#else

void conSetCursor (lin, col)
inttype lin;
inttype col;
#endif

  /* Moves the system curser to the given place of the console.     */
  /* When no system cursor exists this procedure can be replaced by */
  /* a dummy procedure.                                             */

  {
    HANDLE hConsole;
    COORD position;

  /* conSetCursor */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      position.X = col - 1;
      position.Y = lin - 1;
      if (!SetConsoleCursorPosition(hConsole, position)) {
        /* printf("SetConsoleCursorPosition(%d, (%d, %d)) ==> Error %d\n",
            hConsole, col - 1, lin - 1, GetLastError()); */
      } /* if */
    } else {
      /* printf("GetStdHandle(STD_OUTPUT_HANDLE) ==> %d / Error %d\n",
          hConsole, GetLastError()); */
    } /* if */
  } /* conSetCursor */



#ifdef ANSI_C

void conText (inttype lin, inttype col, ustritype stri,
memsizetype length)
#else

void conText (lin, col, stri, length)
inttype lin;
inttype col;
ustritype stri;
memsizetype length;
#endif

  /* This function writes the string stri to the console at the     */
  /* position (lin, col). The position (lin, col) must be a legal   */
  /* position of the console. The string stri is not allowed to go  */
  /* beyond the right border of the console. All console output     */
  /* must be done with this function.                               */

  {
    HANDLE hConsole;
    COORD position;
    DWORD numchars;

  /* conText */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      position.X = col - 1;
      position.Y = lin - 1;
      if (SetConsoleCursorPosition(hConsole, position)) {
        WriteConsole(hConsole, stri, length, &numchars, NULL);
      } else {
        /* printf("SetConsoleCursorPosition(%d, (%d, %d)) ==> Error %d\n",
            hConsole, col - 1, lin - 1, GetLastError()); */
      } /* if */
    } else {
      /* printf("GetStdHandle(STD_OUTPUT_HANDLE) ==> %d / Error %d\n",
          hConsole, GetLastError()); */
    } /* if */
  } /* conText */



#ifdef ANSI_C

void conClear (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol)
#else

void conClear (startlin, startcol, stoplin, stopcol)
inttype startlin;
inttype startcol;
inttype stoplin;
inttype stopcol;
#endif

  /* Clears the area described by startlin, stoplin, startcol and   */
  /* stopcol.                                                       */

  {
    HANDLE hConsole;
    COORD position;
    DWORD numchars;

  /* conClear */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    position.X = startcol - 1;
    position.Y = startlin - 1;
    while (position.Y < stoplin) {
      FillConsoleOutputCharacter(hConsole, (TCHAR) ' ',
          stopcol - startcol + 1, position, &numchars);
      position.Y++;
    } /* while */
  } /* conClear */



#ifdef ANSI_C

void conUpScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count)
#else

void conUpScroll (startlin, startcol, stoplin, stopcol, count)
inttype startlin;
inttype startcol;
inttype stoplin;
inttype stopcol;
inttype count;
#endif

  /* Scrolls the area inside startlin, startcol, stoplin and        */
  /* stopcol upward by count lines. The upper count lines of the    */
  /* area are overwritten. At the lower end of the area blank lines */
  /* are inserted. Nothing is changed outside the area.             */

  {
    HANDLE hConsole;
    SMALL_RECT scrollRect;
    COORD destOrigin;
    CHAR_INFO fillChar;

  /* conUpScroll */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      scrollRect.Left   = startcol - 1;
      scrollRect.Top    = startlin + count - 1;
      scrollRect.Right  = stopcol - 1;
      scrollRect.Bottom = stoplin - 1;
      destOrigin.X = startcol - 1;
      destOrigin.Y = startlin - 1;
      memset(&fillChar, 0, sizeof(CHAR_INFO));
      fillChar.Char.AsciiChar = ' ';
      ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, destOrigin, &fillChar);
    } else {
      /* printf("GetStdHandle(STD_OUTPUT_HANDLE) ==> %d / Error %d\n",
          hConsole, GetLastError()); */
    } /* if */
  } /* conUpScroll */



#ifdef ANSI_C

void conDownScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count)
#else

void conDownScroll (startlin, startcol, stoplin, stopcol, count)
inttype startlin;
inttype startcol;
inttype stoplin;
inttype stopcol;
inttype count;
#endif

  /* Scrolls the area inside startlin, startcol, stoplin and        */
  /* stopcol downward by count lines. The lower count lines of the  */
  /* area are overwritten. At the upper end of the area blank lines */
  /* are inserted. Nothing is changed outside the area.             */

  {
    HANDLE hConsole;
    SMALL_RECT scrollRect;
    COORD destOrigin;
    CHAR_INFO fillChar;

  /* conDownScroll */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      scrollRect.Left   = startcol - 1;
      scrollRect.Top    = startlin - 1;
      scrollRect.Right  = stopcol - 1;
      scrollRect.Bottom = stoplin - count - 1;
      destOrigin.X = startcol - 1;
      destOrigin.Y = startlin + count - 1;
      memset(&fillChar, 0, sizeof(CHAR_INFO));
      fillChar.Char.AsciiChar = ' ';
      ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, destOrigin, &fillChar);
    } else {
      /* printf("GetStdHandle(STD_OUTPUT_HANDLE) ==> %d / Error %d\n",
          hConsole, GetLastError()); */
    } /* if */
  } /* conDownScroll */



#ifdef ANSI_C

void conLeftScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count)
#else

void conLeftScroll (startlin, startcol, stoplin, stopcol, count)
inttype startlin;
inttype startcol;
inttype stoplin;
inttype stopcol;
inttype count;
#endif

  /* Scrolls the area inside startlin, startcol, stoplin and        */
  /* stopcol leftward by count lines. The left count lines of the   */
  /* area are overwritten. At the right end of the area blank lines */
  /* are inserted. Nothing is changed outside the area.             */

  {
    HANDLE hConsole;
    SMALL_RECT scrollRect;
    COORD destOrigin;
    CHAR_INFO fillChar;

  /* conLeftScroll */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      scrollRect.Left   = startcol + count - 1;
      scrollRect.Top    = startlin - 1;
      scrollRect.Right  = stopcol - 1;
      scrollRect.Bottom = stoplin - 1;
      destOrigin.X = startcol - 1;
      destOrigin.Y = startlin - 1;
      memset(&fillChar, 0, sizeof(CHAR_INFO));
      fillChar.Char.AsciiChar = ' ';
      ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, destOrigin, &fillChar);
    } else {
      /* printf("GetStdHandle(STD_OUTPUT_HANDLE) ==> %d / Error %d\n",
          hConsole, GetLastError()); */
    } /* if */
  } /* conLeftScroll */



#ifdef ANSI_C

void conRightScroll (inttype startlin, inttype startcol,
    inttype stoplin, inttype stopcol, inttype count)
#else

void conRightScroll (startlin, startcol, stoplin, stopcol, count)
inttype startlin;
inttype startcol;
inttype stoplin;
inttype stopcol;
inttype count;
#endif

  /* Scrolls the area inside startlin, startcol, stoplin and        */
  /* stopcol rightward by count lines. The right count lines of the */
  /* area are overwritten. At the left end of the area blank lines  */
  /* are inserted. Nothing is changed outside the area.             */

  {
    HANDLE hConsole;
    SMALL_RECT scrollRect;
    COORD destOrigin;
    CHAR_INFO fillChar;

  /* conRightScroll */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      scrollRect.Left   = startcol - 1;
      scrollRect.Top    = startlin - 1;
      scrollRect.Right  = stopcol - count - 1;
      scrollRect.Bottom = stoplin - 1;
      destOrigin.X = startcol + count - 1;
      destOrigin.Y = startlin - 1;
      memset(&fillChar, 0, sizeof(CHAR_INFO));
      fillChar.Char.AsciiChar = ' ';
      ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, destOrigin, &fillChar);
    } else {
      /* printf("GetStdHandle(STD_OUTPUT_HANDLE) ==> %d / Error %d\n",
          hConsole, GetLastError()); */
    } /* if */
  } /* conRightScroll */



#ifdef ANSI_C

void conShut (void)
#else

void conShut ()
#endif

  { /* conShut */
    if (console_initialized) {
      con_standardcolour();
      conCursor(TRUE);
      conClear(1, 1, conHeight(), conWidth());
      conSetCursor(1, 24);
      console_initialized = FALSE;
    } /* if */
  } /* conShut */



#ifdef ANSI_C

int conOpen (void)
#else

int conOpen ()
#endif

  /* Initializes and clears the console.                            */

  { /* conOpen */
    con_normalcolour();
    conClear(1, 1, conHeight(), conWidth());
    conCursor(FALSE);
    console_initialized = TRUE;
    atexit(conShut);
    return(1);
  } /* conOpen */

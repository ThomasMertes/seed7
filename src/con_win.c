/********************************************************************/
/*                                                                  */
/*  con_win.c     Driver for windows console access.                */
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
/*  Content: Driver for windows console access.                     */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "windows.h"
#include "wchar.h"
#include "conio.h"

#include "common.h"
#include "heaputl.h"
#include "striutl.h"
#include "ut8_rtl.h"
#include "rtl_err.h"
#include "con_drv.h"
#include "kbd_drv.h"


#ifdef C_PLUS_PLUS

extern "C" wint_t _getwch (void);

#else

extern wint_t _getwch (void);

#endif


#define SCRHEIGHT 25
#define SCRWIDTH 80
#define WRITE_STRI_BLOCK_SIZE 256

static boolType keybd_initialized = FALSE;
static DWORD saved_console_input_mode;

static char currentattribute;
static boolType console_initialized = FALSE;
static boolType cursor_on = FALSE;


#ifdef OS_GETCH_READS_BYTES
static charType map_char[] = {
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
#endif


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



static void kbd_init (void)

  {
    HANDLE hConsole;

  /* kbd_init */
    logFunction(printf("kbd_init\n"););
    hConsole = GetStdHandle(STD_INPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      if (!GetConsoleMode(hConsole, &saved_console_input_mode)) {
        logError(printf("kbd_init: GetConsoleMode(hConsole, *) failed:\n"
                        "Error=%d\n", GetLastError());
                 fflush(stdout););
      } else {
#ifdef OUT_OF_ORDER
        if (saved_console_input_mode & ENABLE_ECHO_INPUT) { printf("ECHO_INPUT\n"); }
        /* if (saved_console_input_mode & ENABLE_INSERT_MODE) { printf("INSERT_MODE\n"); } */
        if (saved_console_input_mode & ENABLE_LINE_INPUT) { printf("LINE_INPUT\n"); }
        if (saved_console_input_mode & ENABLE_MOUSE_INPUT) { printf("MOUSE_INPUT\n"); }
        if (saved_console_input_mode & ENABLE_PROCESSED_INPUT) { printf("PROCESSED_INPUT\n"); }
        /* if (saved_console_input_mode & ENABLE_QUICK_EDIT_MODE) { printf("QUICK_EDIT_MODE\n"); } */
        if (saved_console_input_mode & ENABLE_WINDOW_INPUT) { printf("WINDOW_INPUT\n"); }
#endif
        SetConsoleMode(hConsole, saved_console_input_mode & (DWORD) ~ENABLE_PROCESSED_INPUT);
        keybd_initialized = TRUE;
      } /* if */
    } /* if */
    logFunction(printf("kbd_init -->\n"););
  } /* kbd_init */



void kbdShut (void)

  {
    HANDLE hConsole;

  /* kbdShut */
    if (keybd_initialized) {
      hConsole = GetStdHandle(STD_INPUT_HANDLE);
      if (hConsole != INVALID_HANDLE_VALUE) {
        SetConsoleMode(hConsole, saved_console_input_mode);
      } /* if */
    } /* if */
  } /* kbdShut */



boolType kbdKeyPressed (void)

  { /* kbdKeyPressed */
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    return kbhit();
  } /* kbdKeyPressed */



charType kbdGetc (void)

  {
    intType key;
    charType result;

  /* kbdGetc */
    logFunction(printf("kbdGetc\n"););
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    key = os_getch();
    if (key == 0 || key == 224) {
      /* printf("key [%ld, ", key); */
      key = os_getch();
      result = map_key[key];
      /* printf("%ld] -> %lu ", key, result); */
    } else {
#ifdef OS_GETCH_READS_BYTES
      result = map_char[(uintType) key & 0xFF];
#else
      result = (charType) key;
#endif
    } /* if */
    if (result == 13) {
      result = 10;
    } /* if */
    logFunction(printf("kbdGetc --> %d\n", result););
    return result;
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

  {
    HANDLE hConsole;
    CONSOLE_SCREEN_BUFFER_INFO con_info;

  /* conHeight */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      if (GetConsoleScreenBufferInfo(hConsole, &con_info)) {
        return con_info.dwSize.Y;
      } else {
        /* printf("GetConsoleScreenBufferInfo(%d, & ) --> Error %d\n",
            hConsole, GetLastError()); */
        return SCRHEIGHT;
      } /* if */
    } else {
      /* printf("GetStdHandle(STD_OUTPUT_HANDLE) --> %d / Error %d\n",
          hConsole, GetLastError()); */
      return SCRHEIGHT;
    } /* if */
  } /* conHeight */



int conWidth (void)

  {
    HANDLE hConsole;
    CONSOLE_SCREEN_BUFFER_INFO con_info;

  /* conWidth */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
      if (GetConsoleScreenBufferInfo(hConsole, &con_info)) {
        return con_info.dwSize.X;
      } else {
        /* printf("GetConsoleScreenBufferInfo(%d, & ) --> Error %d\n",
            hConsole, GetLastError()); */
        return SCRWIDTH;
      } /* if */
    } else {
      /* printf("GetStdHandle(STD_OUTPUT_HANDLE) --> %d / Error %d\n",
          hConsole, GetLastError()); */
      return SCRWIDTH;
    } /* if */
  } /* conWidth */



void conFlush (void)

  { /* conFlush */
  } /* conFlush */



void conCursor (boolType on)

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



/**
 *  Moves the system curser to the given place of the console.
 *  When no system cursor exists this procedure can be replaced by
 *  a dummy procedure.
 */
void conSetCursor (intType line, intType column)

  {
    HANDLE hConsole;
    COORD position;

  /* conSetCursor */
    if (unlikely(line <= 0 || column <= 0)) {
      raise_error(RANGE_ERROR);
    } else if (line <= INT16TYPE_MAX && column <= INT16TYPE_MAX) {
      if (console_initialized) {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
          position.X = (int16Type) (column - 1);
          position.Y = (int16Type) (line - 1);
          if (!SetConsoleCursorPosition(hConsole, position)) {
            /* printf("SetConsoleCursorPosition(%d, (%d, %d)) --> Error %d\n",
                hConsole, column - 1, line - 1, GetLastError()); */
          } /* if */
        } else {
          /* printf("GetStdHandle(STD_OUTPUT_HANDLE) --> %d / Error %d\n",
              hConsole, GetLastError()); */
        } /* if */
      } /* if */
    } /* if */
  } /* conSetCursor */



static void doWriteConsole (HANDLE hConsole, const const_striType stri)

  {
    wcharType wstri_buffer[2 * WRITE_STRI_BLOCK_SIZE];
    wstriType wstri;
    wstriType wstri_part;
    memSizeType wstri_size;
    errInfoType err_info = OKAY_NO_ERROR;
    DWORD numchars;

  /* doWriteConsole */
    /* fprintf(stderr, "doWriteConsole(%lx, ...)", (unsigned long) hConsole); */
    if (stri->size <= WRITE_STRI_BLOCK_SIZE) {
      wstri_size = stri_to_utf16(wstri_buffer, stri->mem, stri->size, &err_info);
      if (unlikely(err_info != OKAY_NO_ERROR)) {
        raise_error(RANGE_ERROR);
      } else {
        WriteConsoleW(hConsole, wstri_buffer, (DWORD) wstri_size, &numchars, NULL);
      } /* if */
    } else {
      if (unlikely(stri->size > (MAX_WSTRI_LEN + 1) / 2 ||
                   !ALLOC_WSTRI(wstri, stri->size * 2 - 1))) {
        raise_error(MEMORY_ERROR);
      } else {
        wstri_size = stri_to_utf16(wstri, stri->mem, stri->size, &err_info);
        if (unlikely(err_info != OKAY_NO_ERROR)) {
          raise_error(RANGE_ERROR);
        } else {
          wstri_part = wstri;
          /* Writing may fail for lengths above 26000 to 32000 */
          while (wstri_size > 25000) {
            WriteConsoleW(hConsole, wstri_part, 25000, &numchars, NULL);
            wstri_part = &wstri_part[25000];
            wstri_size -= 25000;
          } /* while */
          WriteConsoleW(hConsole, wstri_part, (DWORD) wstri_size, &numchars, NULL);
        } /* if */
        UNALLOC_WSTRI(wstri, stri->size * 2 - 1);
      } /* if */
    } /* if */
  } /* doWriteConsole */



/**
 *  Writes the string stri to the console at the current position.
 */
void conWrite (const const_striType stri)

  {
    HANDLE hConsole;
    DWORD mode;

  /* conWrite */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE &&
        GetFileType(hConsole) == FILE_TYPE_CHAR &&
        GetConsoleMode(hConsole, &mode) != 0) {
      /* hConsole refers to a real console */
      doWriteConsole(hConsole, stri);
    } else {
      /* The output has been redirected */
      ut8Write(stdout, stri);
    } /* if */
  } /* conWrite */



/**
 *  Clears the area described by startlin, stoplin, startcol and stopcol.
 */
void conClear (intType startlin, intType startcol,
    intType stoplin, intType stopcol)

  {
    HANDLE hConsole;
    COORD position;
    DWORD numchars;

  /* conClear */
    if (unlikely(startlin <= 0 || startcol <= 0 ||
                 stoplin < startlin || stopcol < startcol)) {
      raise_error(RANGE_ERROR);
    } else if (startlin <= INT16TYPE_MAX && startcol <= INT16TYPE_MAX) {
      if (stoplin > INT16TYPE_MAX) {
        stoplin = INT16TYPE_MAX;
      } /* if */
      if (stopcol > INT16TYPE_MAX) {
        stopcol = INT16TYPE_MAX;
      } /* if */
      hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
      position.X = (int16Type) (startcol - 1);
      position.Y = (int16Type) (startlin - 1);
      while (position.Y < (int16Type) stoplin) {
        FillConsoleOutputCharacter(hConsole, (TCHAR) ' ',
            (unsigned int) (stopcol - startcol + 1), position, &numchars);
        position.Y++;
      } /* while */
    } /* if */
  } /* conClear */



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol upward by count lines. The upper count lines of the
 *  area are overwritten. At the lower end of the area blank lines
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that count is greater or equal 1.
 */
void conUpScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  {
    HANDLE hConsole;
    SMALL_RECT scrollRect;
    COORD destOrigin;
    CHAR_INFO fillChar;

  /* conUpScroll */
    if (unlikely(startlin <= 0 || startcol <= 0 ||
                 stoplin < startlin || stopcol < startcol)) {
      raise_error(RANGE_ERROR);
    } else if (startlin <= INT16TYPE_MAX && startcol <= INT16TYPE_MAX) {
      if (count > stoplin - startlin + 1) {
        conClear(startlin, startcol, stoplin, stopcol);
      } else {
        if (stoplin > INT16TYPE_MAX) {
          stoplin = INT16TYPE_MAX;
        } /* if */
        if (stopcol > INT16TYPE_MAX) {
          stopcol = INT16TYPE_MAX;
        } /* if */
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
          scrollRect.Left   = (int16Type) (startcol - 1);
          scrollRect.Top    = (int16Type) (startlin + count - 1);
          scrollRect.Right  = (int16Type) (stopcol - 1);
          scrollRect.Bottom = (int16Type) (stoplin - 1);
          destOrigin.X = (int16Type) (startcol - 1);
          destOrigin.Y = (int16Type) (startlin - 1);
          memset(&fillChar, 0, sizeof(CHAR_INFO));
          fillChar.Char.AsciiChar = ' ';
          ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, destOrigin, &fillChar);
        } else {
          /* printf("GetStdHandle(STD_OUTPUT_HANDLE) --> %d / Error %d\n",
              hConsole, GetLastError()); */
        } /* if */
      } /* if */
    } /* if */
  } /* conUpScroll */



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol downward by count lines. The lower count lines of the
 *  area are overwritten. At the upper end of the area blank lines
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that count is greater or equal 1.
 */
void conDownScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  {
    HANDLE hConsole;
    SMALL_RECT scrollRect;
    COORD destOrigin;
    CHAR_INFO fillChar;

  /* conDownScroll */
    if (unlikely(startlin <= 0 || startcol <= 0 ||
                 stoplin < startlin || stopcol < startcol)) {
      raise_error(RANGE_ERROR);
    } else if (startlin <= INT16TYPE_MAX && startcol <= INT16TYPE_MAX) {
      if (count > stoplin - startlin + 1) {
        conClear(startlin, startcol, stoplin, stopcol);
      } else {
        if (stoplin > INT16TYPE_MAX) {
          stoplin = INT16TYPE_MAX;
        } /* if */
        if (stopcol > INT16TYPE_MAX) {
          stopcol = INT16TYPE_MAX;
        } /* if */
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
          scrollRect.Left   = (int16Type) (startcol - 1);
          scrollRect.Top    = (int16Type) (startlin - 1);
          scrollRect.Right  = (int16Type) (stopcol - 1);
          scrollRect.Bottom = (int16Type) (stoplin - count - 1);
          destOrigin.X = (int16Type) (startcol - 1);
          destOrigin.Y = (int16Type) (startlin + count - 1);
          memset(&fillChar, 0, sizeof(CHAR_INFO));
          fillChar.Char.AsciiChar = ' ';
          ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, destOrigin, &fillChar);
        } else {
          /* printf("GetStdHandle(STD_OUTPUT_HANDLE) --> %d / Error %d\n",
              hConsole, GetLastError()); */
        } /* if */
      } /* if */
    } /* if */
  } /* conDownScroll */



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol leftward by count columns. The left count columns of the
 *  area are overwritten. At the right end of the area blank columns
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that count is greater or equal 1.
 */
void conLeftScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  {
    HANDLE hConsole;
    SMALL_RECT scrollRect;
    COORD destOrigin;
    CHAR_INFO fillChar;

  /* conLeftScroll */
    if (unlikely(startlin <= 0 || startcol <= 0 ||
                 stoplin < startlin || stopcol < startcol)) {
      raise_error(RANGE_ERROR);
    } else if (startlin <= INT16TYPE_MAX && startcol <= INT16TYPE_MAX) {
      if (count > stopcol - startcol + 1) {
        conClear(startlin, startcol, stoplin, stopcol);
      } else {
        if (stoplin > INT16TYPE_MAX) {
          stoplin = INT16TYPE_MAX;
        } /* if */
        if (stopcol > INT16TYPE_MAX) {
          stopcol = INT16TYPE_MAX;
        } /* if */
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
          scrollRect.Left   = (int16Type) (startcol + count - 1);
          scrollRect.Top    = (int16Type) (startlin - 1);
          scrollRect.Right  = (int16Type) (stopcol - 1);
          scrollRect.Bottom = (int16Type) (stoplin - 1);
          destOrigin.X = (int16Type) (startcol - 1);
          destOrigin.Y = (int16Type) (startlin - 1);
          memset(&fillChar, 0, sizeof(CHAR_INFO));
          fillChar.Char.AsciiChar = ' ';
          ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, destOrigin, &fillChar);
        } else {
          /* printf("GetStdHandle(STD_OUTPUT_HANDLE) --> %d / Error %d\n",
              hConsole, GetLastError()); */
        } /* if */
      } /* if */
    } /* if */
  } /* conLeftScroll */



/**
 *  Scrolls the area inside startlin, startcol, stoplin and
 *  stopcol rightward by count columns. The right count columns of the
 *  area are overwritten. At the left end of the area blank columns
 *  are inserted. Nothing is changed outside the area.
 *  The calling function assures that count is greater or equal 1.
 */
void conRightScroll (intType startlin, intType startcol,
    intType stoplin, intType stopcol, intType count)

  {
    HANDLE hConsole;
    SMALL_RECT scrollRect;
    COORD destOrigin;
    CHAR_INFO fillChar;

  /* conRightScroll */
    if (unlikely(startlin <= 0 || startcol <= 0 ||
                 stoplin < startlin || stopcol < startcol)) {
      raise_error(RANGE_ERROR);
    } else if (startlin <= INT16TYPE_MAX && startcol <= INT16TYPE_MAX) {
      if (count > stopcol - startcol + 1) {
        conClear(startlin, startcol, stoplin, stopcol);
      } else {
        if (stoplin > INT16TYPE_MAX) {
          stoplin = INT16TYPE_MAX;
        } /* if */
        if (stopcol > INT16TYPE_MAX) {
          stopcol = INT16TYPE_MAX;
        } /* if */
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
          scrollRect.Left   = (int16Type) (startcol - 1);
          scrollRect.Top    = (int16Type) (startlin - 1);
          scrollRect.Right  = (int16Type) (stopcol - count - 1);
          scrollRect.Bottom = (int16Type) (stoplin - 1);
          destOrigin.X = (int16Type) (startcol + count - 1);
          destOrigin.Y = (int16Type) (startlin - 1);
          memset(&fillChar, 0, sizeof(CHAR_INFO));
          fillChar.Char.AsciiChar = ' ';
          ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, destOrigin, &fillChar);
        } else {
          /* printf("GetStdHandle(STD_OUTPUT_HANDLE) --> %d / Error %d\n",
              hConsole, GetLastError()); */
        } /* if */
      } /* if */
    } /* if */
  } /* conRightScroll */



void conShut (void)

  { /* conShut */
    logFunction(printf("conShut\n"););
    if (console_initialized) {
      con_standardcolour();
      conCursor(TRUE);
      conClear(1, 1, conHeight(), conWidth());
      conSetCursor(1, 24);
      console_initialized = FALSE;
    } /* if */
    logFunction(printf("conShut -->\n"););
  } /* conShut */



/**
 *  Initializes and clears the console.
 */
int conOpen (void)

  { /* conOpen */
    logFunction(printf("conOpen\n"););
    con_normalcolour();
    conClear(1, 1, conHeight(), conWidth());
    conCursor(FALSE);
    console_initialized = TRUE;
    atexit(conShut);
    logFunction(printf("conOpen -->\n"););
    return 1;
  } /* conOpen */

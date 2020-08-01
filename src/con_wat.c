/********************************************************************/
/*                                                                  */
/*  con_wat.c     Driver for watcom and djgpp console access.       */
/*  Copyright (C) 1989 - 2018  Thomas Mertes                        */
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
/*  File: seed7/src/con_wat.c                                       */
/*  Changes: 1994, 2018  Thomas Mertes                              */
/*  Content: Driver for watcom and djgpp console access.            */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "dos.h"
#include "conio.h"
#include "termios.h"
#include "errno.h"

#include "common.h"
#include "os_decls.h"
#include "heaputl.h"
#include "striutl.h"
#include "ut8_rtl.h"
#include "rtl_err.h"
#include "kbd_drv.h"

#undef EXTERN
#define EXTERN
#include "con_drv.h"

/* #define atexit(x) */

#define int86(a,b,c) int386(a,b,c)


#define SCRHEIGHT 25
#define SCRWIDTH 80

#define black 0
#define green 2
#define lightgray 7
#define darkgray 8
#define yellow 14
#define white 15

static struct termios term_descr;
static struct termios term_bak;
static boolType keybd_initialized = FALSE;

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
static boolType console_initialized = FALSE;
static boolType cursor_on = FALSE;

static unsigned char cursor_startline;
static unsigned char cursor_endline;


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



/**
 *  Determine if two termios structs are equal.
 *  Comparing with memcmp does not work correctly.
 *  Struct termios has data at and after &c_cc[NCCS].
 *  Therefore memcmp sees differences, even if the
 *  official fields of struct termios are equal.
 *  @return TRUE if the thow termios structs are equal,
 *          FALSE otherwise.
 */
static boolType term_descr_equal (struct termios *term_descr1, struct termios *term_descr2)

  {
    int pos;
    boolType equal;

  /* term_descr_equal */
    equal = term_descr1->c_iflag == term_descr2->c_iflag &&
            term_descr1->c_oflag == term_descr2->c_oflag &&
            term_descr1->c_cflag == term_descr2->c_cflag &&
            term_descr1->c_lflag == term_descr2->c_lflag;
    for (pos = 0; pos < NCCS; pos++) {
      if (term_descr1->c_cc[pos] != term_descr2->c_cc[pos]) {
        equal = FALSE;
      } /* if */
    } /* for */
    return equal;
  } /* term_descr_equal */



/**
 *  Change the terminal attributes to 'new_term_descr'.
 *  The function tcsetattr() returns success if any of the
 *  requested changes could be successfully carried out.
 *  If doing multiple changes it is necessary to check
 *  with tcgetattr(), that all changes have been performed
 *  successfully.
 *  @return TRUE if the change of the attributes was successful,
 *          FALSE otherwise.
 */
static boolType tcset_term_descr (int file_no, struct termios *new_term_descr)

  {
    struct termios term_descr_check;
    int trial = 0;
    boolType succeeded = FALSE;

  /* tcset_term_descr */
    do {
      trial++;
      if (tcsetattr(file_no, TCSANOW, new_term_descr) == 0 &&
          tcgetattr(file_no, &term_descr_check) == 0 &&
          term_descr_equal(new_term_descr, &term_descr_check)) {
        succeeded = TRUE;
      } /* if */
    } while (!succeeded && trial < 10);
    /* show_term_descr(new_term_descr);
       show_term_descr(&term_descr_check); */
    /* printf("trial=%d\n", trial); */
    return succeeded;
  } /* tcset_term_descr */



void kbdShut (void)

  { /* kbdShut */
    if (keybd_initialized) {
      tcset_term_descr(fileno(stdin), &term_bak);
      keybd_initialized = FALSE;
    } /* if */
  } /* kbdShut */



static void kbd_init (void)

  {
    int file_no;

  /* kbd_init */
    file_no = fileno(stdin);
    if (tcgetattr(0, &term_descr) != 0) {
      printf("kbd_init: tcgetattr(%d, ...) failed:\n"
             "errno=%d\nerror: %s\n",
             file_no, errno, strerror(errno));
    } else {
      /* show_term_descr(&term_descr); */
      memcpy(&term_bak, &term_descr, sizeof(struct termios));
      term_descr.c_cc[VINTR] = (cc_t) -1;
      if (!tcset_term_descr(file_no, &term_descr)) {
        printf("kbd_init: tcsetattr(%d, VINTR=-1) failed:\n"
               "errno=%d\nerror: %s\n",
               file_no, errno, strerror(errno));
        /* show_term_descr(&term_descr); */
      } else {
        keybd_initialized = TRUE;
        atexit(kbdShut);
        fflush(stdout);
      } /* if */
    } /* if */
  } /* kbd_init */



boolType kbdKeyPressed (void)

  { /* kbdKeyPressed */
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    return (char) bdos(0xB, 0, 0) & 1;
  } /* kbdKeyPressed */



charType kbdGetc (void)

  {
    union REGS r;
    charType key;

  /* kbdGetc */
    logFunction(printf("kbdGetc\n"););
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
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
 *  Moves the system curser to the given place of the console.
 *  If no system cursor exists this procedure can be replaced by
 *  a dummy procedure.
 */
void conSetCursor (intType line, intType column)

  {
    union REGS r;

  /* conSetCursor */
    if (line <= 0 || column <= 0) {
      raise_error(RANGE_ERROR);
    } else if (line <= UINT8TYPE_MAX && column <= UINT8TYPE_MAX) {
      r.h.ah = (unsigned char) 2; /* cursor addressing function */
      r.h.dh = (unsigned char) (line - 1);
      r.h.dl = (unsigned char) (column - 1);
      r.h.bh = (unsigned char) 0; /* video page */
      int86(0x10, &r, &r);
    } /* if */
  } /* conSetCursor */



intType conColumn (void)

  {
    union REGS r;
    intType column;

  /* conColumn */
    r.h.ah = (unsigned char) 3; /* get cursor position and shape */
    r.h.bh = (unsigned char) 0; /* video page */
    int86(0x10, &r, &r);
    column = (intType) r.h.dl + 1;
    return column;
  } /* conColumn */



intType conLine (void)

  {
    union REGS r;
    intType line;

  /* conLine */
    r.h.ah = (unsigned char) 3; /* get cursor position and shape */
    r.h.bh = (unsigned char) 0; /* video page */
    int86(0x10, &r, &r);
    line = (intType) r.h.dh + 1;
    return line;
  } /* conLine */



static void doCPuts (cstriType cstri)

  {
    char *nlPos;

  /* doCPuts */
    while ((nlPos = strchr(cstri, '\n')) != NULL) {
      *nlPos = '\0';
      cputs(cstri);
      *nlPos = '\n';
      cputs("\r\n");
      cstri = &nlPos[1];
    } /* while */
    cputs(cstri);
  } /* doCPuts */



static void doWriteConsole (const const_striType stri)

  {
    os_striType os_stri;
    errInfoType err_info = OKAY_NO_ERROR;

  /* doWriteConsole */
    os_stri = stri_to_os_stri(stri, &err_info);
    if (unlikely(os_stri == NULL)) {
      raise_error(err_info);
    } else {
      doCPuts((cstriType) os_stri);
      os_stri_free(os_stri);
    } /* if */
  } /* doWriteConsole */



/**
 *  Writes the string stri to the console at the current position.
 *  The string stri is not allowed to go beyond the right border of
 *  the console.
 */
void conWrite (const const_striType stri)

  {
    os_fstat_struct stat_buf;

  /* conWrite */
    if (os_fstat(1, &stat_buf) == 0 && (int) stat_buf.st_dev < 0) {
      /* stdout refers to a real console */
      doWriteConsole(stri);
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
    union REGS r;

  /* conClear */
    if (startlin <= 0 || startcol <= 0 ||
        stoplin < startlin || stopcol < startcol) {
      raise_error(RANGE_ERROR);
    } else if (startlin <= UINT8TYPE_MAX && startcol <= UINT8TYPE_MAX) {
      if (stoplin > UINT8TYPE_MAX) {
        stoplin = UINT8TYPE_MAX;
      } /* if */
      if (stopcol > UINT8TYPE_MAX) {
        stopcol = UINT8TYPE_MAX;
      } /* if */
      r.h.ah = (unsigned char) 6; /* scroll up code */
      r.h.al = (unsigned char) 0; /* clear screen code */
      r.h.ch = (unsigned char) (startlin - 1);
      r.h.cl = (unsigned char) (startcol - 1);
      r.h.dh = (unsigned char) (stoplin - 1);
      r.h.dl = (unsigned char) (stopcol - 1);
      r.h.bh = (unsigned char) currentattribute; /* blank line colour */
      int86(0x10, &r, &r);
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
    union REGS r;

  /* conUpScroll */
    if (startlin <= 0 || startcol <= 0 ||
        stoplin < startlin || stopcol < startcol) {
      raise_error(RANGE_ERROR);
    } else if (startlin <= UINT8TYPE_MAX && startcol <= UINT8TYPE_MAX) {
      if (count > stoplin - startlin + 1) {
        conClear(startlin, startcol, stoplin, stopcol);
      } else {
        if (stoplin > UINT8TYPE_MAX) {
          stoplin = UINT8TYPE_MAX;
        } /* if */
        if (stopcol > UINT8TYPE_MAX) {
          stopcol = UINT8TYPE_MAX;
        } /* if */
        r.h.ah = (unsigned char) 6; /* scroll up code */
        r.h.al = (unsigned char) count;
        r.h.ch = (unsigned char) (startlin - 1);
        r.h.cl = (unsigned char) (startcol - 1);
        r.h.dh = (unsigned char) (stoplin - 1);
        r.h.dl = (unsigned char) (stopcol - 1);
        r.h.bh = (unsigned char) 7; /* blank line is black */
        int86(0x10, &r, &r);
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
    union REGS r;

  /* conDownScroll */
    if (startlin <= 0 || startcol <= 0 ||
        stoplin < startlin || stopcol < startcol) {
      raise_error(RANGE_ERROR);
    } else if (startlin <= UINT8TYPE_MAX && startcol <= UINT8TYPE_MAX) {
      if (count > stoplin - startlin + 1) {
        conClear(startlin, startcol, stoplin, stopcol);
      } else {
        if (stoplin > UINT8TYPE_MAX) {
          stoplin = UINT8TYPE_MAX;
        } /* if */
        if (stopcol > UINT8TYPE_MAX) {
          stopcol = UINT8TYPE_MAX;
        } /* if */
        r.h.ah = (unsigned char) 7; /* scroll down code */
        r.h.al = (unsigned char) count;
        r.h.ch = (unsigned char) (startlin - 1);
        r.h.cl = (unsigned char) (startcol - 1);
        r.h.dh = (unsigned char) (stoplin - 1);
        r.h.dl = (unsigned char) (stopcol - 1);
        r.h.bh = (unsigned char) 7; /* blank line is black */
        int86(0x10, &r, &r);
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
    int line;
    int num_bytes;
    char *destination;
    char *source;

  /* conLeftScroll */
    if (startlin <= 0 || startcol <= 0 ||
        stoplin < startlin || stopcol < startcol) {
      raise_error(RANGE_ERROR);
    } else if (startlin <= SCRHEIGHT && startcol <= SCRWIDTH) {
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
    int line;
    int num_bytes;
    char *destination;
    char *source;

  /* conRightScroll */
    if (startlin <= 0 || startcol <= 0 ||
        stoplin < startlin || stopcol < startcol) {
      raise_error(RANGE_ERROR);
    } else if (startlin <= SCRHEIGHT && startcol <= SCRWIDTH) {
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

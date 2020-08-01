/********************************************************************/
/*                                                                  */
/*  kbd_poll.c    Driver for terminfo keyboard access using poll()  */
/*  Copyright (C) 1989 - 2010  Thomas Mertes                        */
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
/*  File: seed7/src/kbd_poll.c                                      */
/*  Changes: 1994, 2006, 2010  Thomas Mertes                        */
/*  Content: Driver for terminfo keyboard access                    */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"

#ifdef USE_MYUNISTD_H
#include "myunistd.h"
#else
#include "unistd.h"
#endif

#include "common.h"
#include "striutl.h"

#ifdef INCL_CURSES_BEFORE_TERM
/* The following include is necessary for RM Machines. */
#include "curses.h"
#endif

#include "termios.h"

#ifdef USE_TERMINFO
#ifdef INCL_NCURSES_TERM
#include "ncurses/term.h"
#else
#include "term.h"
#endif
#endif

#include "poll.h"
#include "errno.h"

#include "trm_drv.h"

#ifdef USE_TERMCAP
#include "cap_def.h"
#endif

#include "con_drv.h"

#undef EXTERN
#define EXTERN
#include "kbd_drv.h"


#undef TRACE_FKEYS


/* #define atexit(x) */


static booltype utf8_mode = FALSE;
static booltype key_buffer_filled = FALSE;
static int last_key;
extern booltype changes;
static struct termios term_descr;
static struct termios term_bak;

static booltype keybd_initialized = FALSE;


#define SIZE_KEY_TABLE 97

static const_cstritype key_table[SIZE_KEY_TABLE];
static char erase_ch[2];

static chartype key_code[SIZE_KEY_TABLE] = {
    K_BS,          0,             0,             0,             K_DEL,
    K_DELLN,       K_DOWN,        K_INS,         0,             0,
    K_HOME,        K_F1,          K_F2,          K_F3,          K_F4,
    K_F5,          K_F6,          K_F7,          K_F8,          K_F9,
    K_F10,         K_SFT_F1,      K_SFT_F2,      K_SFT_F3,      K_SFT_F4,
    K_SFT_F5,      K_SFT_F6,      K_SFT_F7,      K_SFT_F8,      K_SFT_F9,
    K_SFT_F10,     0,             0,             0,             0,
    0,             0,             0,             0,             0,
    0,             0,             K_INS,         K_INSLN,       K_LEFT,
    K_END,         K_PGDN,        K_PGUP,        K_RIGHT,       K_SCRLDN,
    K_SCRLUP,      0,             K_UP,          K_END,         K_BACKTAB,
    K_PAD_CENTER,  K_HOME,        K_END,         K_ESC,
    K_ALT_A,       K_ALT_B,       K_ALT_C,       K_ALT_D,       K_ALT_E,
    K_ALT_F,       K_ALT_G,       K_ALT_H,       K_ALT_I,       K_ALT_J,
    K_ALT_K,       K_ALT_L,       K_ALT_M,       K_ALT_N,       K_ALT_O,
    K_ALT_P,       K_ALT_Q,       K_ALT_R,       K_ALT_S,       K_ALT_T,
    K_ALT_U,       K_ALT_V,       K_ALT_W,       K_ALT_X,       K_ALT_Y,
    K_ALT_Z,       K_ALT_0,       K_ALT_1,       K_ALT_2,       K_ALT_3,
    K_ALT_4,       K_ALT_5,       K_ALT_6,       K_ALT_7,       K_ALT_8,
    K_ALT_9,
    K_BS,          K_DEL};



#ifdef ANSI_C

static booltype read_char_if_present (char *ch)
#else

static booltype read_char_if_present (ch)
char *ch;
#endif

  {
    struct pollfd poll_fds[1];
    int poll_result;
    booltype result;

  /* read_char_if_present */
    poll_fds[0].fd = fileno(stdin);
    poll_fds[0].events = POLLIN | POLLPRI;
    poll_result = poll(poll_fds, 1, 1000 /* milliseconds */);
    if (poll_result == 1) {
      /* printf("poll_fds[0].events = %04X\n", poll_fds[0].events);
         printf("poll_fds[0].revents = %04X\n", poll_fds[0].revents); */
      fread(ch, 1, 1, stdin);
      result = TRUE;
    } else {
      result = FALSE;
    } /* if */
#ifdef TRACE_FKEYS
    printf("read_char_if_present: ch=%u, poll_result=%d, result=%s\n",
        *ch, poll_result, result ? "TRUE" : "FALSE");
#endif
    return result;
  } /* read_char_if_present */



#ifdef ANSI_C

static chartype read_utf8_key (ustritype ustri, int ustri_len)
#else

static chartype read_utf8_key (ustri, ustri_len)
ustritype ustri;
int ustri_len;
#endif

  {
    size_t len;
    strelemtype stri[6];
    memsizetype dest_len;

  /* read_utf8_key */
    if (ustri[0] <= 0x7F) {
      if (ustri_len == 1) {
        return ustri[0];
      } else { /* ustri_len >= 2 */
        last_key = ustri[1];
        key_buffer_filled = TRUE;
        return K_UNDEF;
      } /* if */
    } else if ((ustri[0] & 0xE0) == 0xC0) {
      len = 2;
    } else if ((ustri[0] & 0xF0) == 0xE0) {
      len = 3;
    } else if ((ustri[0] & 0xF8) == 0xF0) {
      len = 4;
    } else if ((ustri[0] & 0xFC) == 0xF8) {
      len = 5;
    } else if ((ustri[0] & 0xFC) == 0xFC) {
      len = 6;
    } else {
      if (ustri_len == 1) {
        return ustri[0];
      } else { /* ustri_len >= 2 */
        last_key = ustri[1];
        key_buffer_filled = TRUE;
        return K_UNDEF;
      } /* if */
    } /* if */
    if (ustri_len == 2 && (ustri[1] & 0xC0) != 0x80) {
      last_key = ustri[1];
      key_buffer_filled = TRUE;
      return ustri[0];
    } else {
      while (ustri_len < len) {
        if (read_char_if_present(&ustri[ustri_len])) {
          ustri[ustri_len + 1] = '\0';
        } else {
          ustri[ustri_len] = '\0';
        } /* if */
        if (ustri[ustri_len] == '\0') {
          if (ustri_len == 1) {
            return ustri[0];
          } else {
            return K_UNDEF;
          } /* if */
        } else if ((ustri[ustri_len] & 0xC0) != 0x80) {
          last_key = ustri[ustri_len];
          key_buffer_filled = TRUE;
          if (ustri_len == 1) {
            return ustri[0];
          } else {
            return K_UNDEF;
          } /* if */
        } /* if */
        ustri_len++;
      } /* while */
      if (utf8_to_stri(stri, &dest_len, ustri, len) == 0 &&
          dest_len == 1) {
        return stri[0];
      } else {
        return K_UNDEF;
      } /* if */
    } /* if */
  } /* read_utf8_key */



#ifdef ANSI_C

static void utf8_init (void)
#else

static void utf8_init ()
#endif

  {
    char *s;

  /* utf8_init */
    if (((s = getenv("LC_ALL"))   && *s) ||
        ((s = getenv("LC_CTYPE")) && *s) ||
        ((s = getenv("LANG"))     && *s)) {
      if (strstr(s, "UTF-8") || strstr(s, "utf8")) {
        utf8_mode = TRUE;
      } /* if */
    } /* if */
  } /* utf8_init */



#ifdef ANSI_C

static void key_table_init (void)
#else

static void key_table_init ()
#endif

  {
    int number;
    int num2;

  /* key_table_init */
    /* fprintf(stderr, "keypad_xmit=\"%s\"\n", keypad_xmit); */
    putcontrol(keypad_xmit); /* keypad_transmit_mode */
    if (key_backspace != NULL && strcmp(key_backspace, "\177") != 0) {
      key_table[ 0] = key_backspace;
    } else {
      key_table[ 0] = NULL;
    } /* if */
    /* printf("key_backspace %d\n", key_backspace[0]); */
    key_table[ 1] = key_catab;
    key_table[ 2] = key_clear;
    key_table[ 3] = key_ctab;
    key_table[ 4] = key_dc;
    key_table[ 5] = key_dl;
    key_table[ 6] = key_down;
    key_table[ 7] = key_eic;
    key_table[ 8] = key_eol;
    key_table[ 9] = key_eos;
    key_table[10] = key_home;
    key_table[11] = key_f1;
    key_table[12] = key_f2;
    key_table[13] = key_f3;
    key_table[14] = key_f4;
    key_table[15] = key_f5;
    key_table[16] = key_f6;
    key_table[17] = key_f7;
    key_table[18] = key_f8;
    key_table[19] = key_f9;
    key_table[20] = key_f10;
    key_table[21] = key_f11;
    key_table[22] = key_f12;
    key_table[23] = key_f13;
    key_table[24] = key_f14;
    key_table[25] = key_f15;
    key_table[26] = key_f16;
    key_table[27] = key_f17;
    key_table[28] = key_f18;
    key_table[29] = key_f19;
    key_table[30] = key_f20;
    key_table[31] = key_f21;
    key_table[32] = key_f22;
    key_table[33] = key_f23;
    key_table[34] = key_f24;
    key_table[35] = key_f25;
    key_table[36] = key_f26;
    key_table[37] = key_f27;
    key_table[38] = key_f28;
    key_table[39] = key_f29;
    key_table[40] = key_f30;
    key_table[41] = key_f0;
    key_table[42] = key_ic;
    key_table[43] = key_il;
    key_table[44] = key_left;
    key_table[45] = key_ll;
    key_table[46] = key_npage;
    key_table[47] = key_ppage;
    key_table[48] = key_right;
    key_table[49] = key_sf;
    key_table[50] = key_sr;
    key_table[51] = key_stab;
    key_table[52] = key_up;
    key_table[53] = key_end;
    key_table[54] = key_btab;
    key_table[55] = key_b2;
    key_table[56] = key_find;
    key_table[57] = key_select;
    key_table[58] = "\033";
    key_table[59] = "\033a";
    key_table[60] = "\033b";
    key_table[61] = "\033c";
    key_table[62] = "\033d";
    key_table[63] = "\033e";
    key_table[64] = "\033f";
    key_table[65] = "\033g";
    key_table[66] = "\033h";
    key_table[67] = "\033i";
    key_table[68] = "\033j";
    key_table[69] = "\033k";
    key_table[70] = "\033l";
    key_table[71] = "\033m";
    key_table[72] = "\033n";
    key_table[73] = "\033o";
    key_table[74] = "\033p";
    key_table[75] = "\033q";
    key_table[76] = "\033r";
    key_table[77] = "\033s";
    key_table[78] = "\033t";
    key_table[79] = "\033u";
    key_table[80] = "\033v";
    key_table[81] = "\033w";
    key_table[82] = "\033x";
    key_table[83] = "\033y";
    key_table[84] = "\033z";
    key_table[85] = "\0330";
    key_table[86] = "\0331";
    key_table[87] = "\0332";
    key_table[88] = "\0333";
    key_table[89] = "\0334";
    key_table[90] = "\0335";
    key_table[91] = "\0336";
    key_table[92] = "\0337";
    key_table[93] = "\0338";
    key_table[94] = "\0339";
    /* If the erase character of the terminal device is different   */
    /* from the one defined in the terminfo/termcap database it is  */
    /* defined additional here.                                     */
    if (erase_ch[0] != '\010' && erase_ch[0] != '\177' &&
        key_backspace != NULL && strcmp(erase_ch, key_backspace) != 0 &&
        key_dc != NULL && strcmp(erase_ch, key_dc) != 0) {
      key_table[95] = erase_ch;
      /* printf("set erase ch 1\n"); */
    } else if (key_table[0] == NULL && erase_ch[0] == '\177') {
      key_table[95] = erase_ch;
      /* printf("set erase ch 2\n"); */
    } else {
      key_table[95] = NULL;
    } /* if */
    key_table[96] = "\177";
    /* Some function key definitions start with a printable         */
    /* character. This makes absolutly no sense and confuses the    */
    /* function key recognition. Therefore such definitions are     */
    /* thrown out here.                                             */
    for (number = 0; number < SIZE_KEY_TABLE; number++) {
#ifdef TRACE_FKEYS
      if (key_table[number] != NULL) {
        fprintf(stderr, "key%d=\"%s\"\n", number, key_table[number]);
      } else {
        fprintf(stderr, "key%d=NULL\n", number);
      } /* if */
#endif
      if (key_table[number] != NULL &&
          key_table[number][0] >= ' ' && key_table[number][0] <= '~') {
        /* fprintf(stderr, "key%d=\"%s\" thrown out\n",
            number, key_table[number]); */
        key_table[number] = NULL;
      } /* if */
    } /* for */
    /* Sometimes there are double entrys in the function key        */
    /* definitions. This makes absolutly no sense and confuses the  */
    /* function key recognition. Therefore such definitions are     */
    /* thrown out here.                                             */
    for (number = 0; number < SIZE_KEY_TABLE; number++) {
      for (num2 = number + 1; num2 < SIZE_KEY_TABLE; num2++) {
        if (key_table[number] != NULL && key_table[num2] != NULL &&
            strcmp(key_table[number], key_table[num2]) == 0) {
          /* fprintf(stderr, "key%d=kex%d=\"%s\"\n",
              number, num2, key_table[number]); */
          key_table[num2] = NULL;
        } /* if */
      } /* for */
    } /* for */
  } /* key_table_init */



#ifdef ANSI_C

void kbdShut (void)
#else

void kbdShut ()
#endif

  { /* kbdShut */
    if (keybd_initialized) {
      tcsetattr(fileno(stdin), TCSANOW, &term_bak);
      if (caps_initialized) {
        /* fprintf(stderr, "keypad_local=\"%s\"\n", keypad_local); */
        putcontrol(keypad_local); /* out of keypad transmit mode */
      } /* if */
      keybd_initialized = FALSE;
    } /* if */
  } /* kbdShut */



#ifdef ANSI_C

static void kbd_init (void)
#else

static void kbd_init ()
#endif

  { /* kbd_init */
    if (!caps_initialized) {
      getcaps();
    } /* if */
    if (tcgetattr(fileno(stdin), &term_descr) != 0) {
      printf("kbd_init: tcgetattr(%d, ...) failed, errno=%d\n",
          fileno(stdin), errno);
      printf("EBADF=%d  EINTR=%d  EINVAL=%d  ENOTTY=%d  EIO=%d\n",
          EBADF, EINTR, EINVAL, ENOTTY, EIO);
    } else {
      memcpy(&term_bak, &term_descr, sizeof(struct termios));
      erase_ch[0] = (char) term_descr.c_cc[VERASE];
      erase_ch[1] = '\0';
      /* printf("erase_ch %d\n", erase_ch[0]); */
      term_descr.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL | ICANON);
      term_descr.c_cc[VINTR] = (cc_t) -1;
      term_descr.c_cc[VQUIT] = (cc_t) -1;
      term_descr.c_cc[VSTOP] = (cc_t) -1;
#ifdef VSTART
      term_descr.c_cc[VSTART] = (cc_t) -1;
#endif
#ifdef VSUSP
      term_descr.c_cc[VSUSP] = (cc_t) -1;
#endif
      term_descr.c_cc[VMIN] = 1;
      term_descr.c_cc[VTIME] = 0;
      if (tcsetattr(fileno(stdin), TCSANOW, &term_descr) != 0) {
        printf("kbd_init: tcsetattr(%d, VMIN=1) failed, errno=%d\n",
            fileno(stdin), errno);
        printf("EBADF=%d  EINTR=%d  EINVAL=%d  ENOTTY=%d  EIO=%d\n",
            EBADF, EINTR, EINVAL, ENOTTY, EIO);
      } else {
        keybd_initialized = TRUE;
        atexit(kbdShut);
        if (getcaps()) {
          key_table_init();
        } /* if */
        utf8_init();
        fflush(stdout);
      } /* if */
    } /* if */
  } /* kbd_init */



#ifdef ANSI_C

booltype kbdKeyPressed (void)
#else

booltype kbdKeyPressed ()
#endif

  {
    struct pollfd poll_fds[1];
    booltype result;

  /* kbdKeyPressed */
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    if (key_buffer_filled) {
      result = TRUE;
    } else {
      if (changes) {
        conFlush();
      } /* if */
      poll_fds[0].fd = fileno(stdin);
      poll_fds[0].events = POLLIN | POLLPRI;
      if (poll(poll_fds, 1, 0) == 1) {
        result = TRUE;
      } else {
        result = FALSE;
      } /* if */
    } /* if */
    return result;
  } /* kbdKeyPressed */



#ifdef ANSI_C

chartype kbdGetc (void)
#else

chartype kbdGetc ()
#endif

  {
    char buffer[10];
    int fread_result;
    int exact_match;
    int number;
    size_t len;
    int key_number;
    chartype result;

  /* kbdGetc */
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    if (changes) {
      conFlush();
    } /* if */
    fread_result = read(fileno(stdin), &buffer, 10);
#ifdef TRACE_FKEYS
    printf("kbdGetc: fread_result = %u, buffer: ", fread_result);
    for (number = 0; number < fread_result; number++) {
      if (buffer[number] == '\\') {
        printf("\\\\");
      } else if (buffer[number] >= ' ' && buffer[number] <= '~') {
        printf("%c", buffer[number]);
      } else {
        printf("\\%u\\", buffer[number]);
      } /* if */
    } /* if */
    printf("\n");
#endif
    key_number = 0;
    exact_match = 0;
    for (number = 0; number < SIZE_KEY_TABLE; number++) {
      if (key_table[number] != NULL) {
        len = strlen(key_table[number]);
        if (fread_result == len &&
            memcmp(key_table[number], buffer, len) == 0) {
          exact_match++;
          key_number = number;
        } /* if */
      } /* if */
    } /* for */
    if (exact_match == 1) {
      result = key_code[key_number];
    } else {
      if (utf8_mode) {
        result = read_utf8_key((ustritype) buffer, fread_result);
      } else {
        if (fread_result == 1) {
          result = buffer[0];
        } else { /* if (fread_result >= 2) { */
          result = K_UNDEF;
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* kbdGetc */



#ifdef ANSI_C

chartype kbdRawGetc (void)
#else

chartype kbdRawGetc ()
#endif

  {
    chartype result;

  /* kbdRawRead */
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    if (key_buffer_filled) {
      key_buffer_filled = FALSE;
      result = ((chartype) last_key) & 0xFF;
    } else {
      if (changes) {
        conFlush();
      } /* if */
      result = getc(stdin);
    } /* if */
/*  fprintf(stderr, "<%d>", result); */
    return result;
  } /* kbdRawRead */

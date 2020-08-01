/********************************************************************/
/*                                                                  */
/*  kbd_poll.c    Driver for terminfo keyboard access using poll(). */
/*  Copyright (C) 1989 - 2013  Thomas Mertes                        */
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
/*  Changes: 1994, 2006, 2010, 2013  Thomas Mertes                  */
/*  Content: Driver for terminfo keyboard access using poll().      */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"

#ifdef UNISTD_H_PRESENT
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

#define KEY_BUFFER_SIZE 20

static booltype utf8_mode = FALSE;
static uchartype key_buffer[KEY_BUFFER_SIZE];
static memsizetype key_buffer_size = 0;
extern booltype changes;
static struct termios term_descr;
static struct termios term_bak;

static booltype keybd_initialized = FALSE;


#define SIZE_KEY_TABLE 130

static const_cstritype key_table[SIZE_KEY_TABLE];
static char erase_ch[2];

static chartype key_code[SIZE_KEY_TABLE] = {
/*   0 */ K_BS,         K_BS,         K_BACKTAB,    K_PAD_CENTER, 0,
/*   5 */ 0,            0,            K_DEL,        K_DELLN,      K_DOWN,
/*  10 */ K_INS,        K_END,        K_NL,         0,            0,
/*  15 */ K_HOME,       0,            K_F1,         K_F2,         K_F3,
/*  20 */ K_F4,         K_F5,         K_F6,         K_F7,         K_F8,
/*  25 */ K_F9,         K_F10,        K_F11,        K_F12,        K_SFT_F1,
/*  30 */ K_SFT_F2,     K_SFT_F3,     K_SFT_F4,     K_SFT_F5,     K_SFT_F6,
/*  35 */ K_SFT_F7,     K_SFT_F8,     K_SFT_F9,     K_SFT_F10,    K_SFT_F11,
/*  40 */ K_SFT_F12,    K_CTL_F1,     K_CTL_F2,     K_CTL_F3,     K_CTL_F4,
/*  45 */ K_CTL_F5,     K_CTL_F6,     K_CTL_F7,     K_CTL_F8,     K_CTL_F9,
/*  50 */ K_CTL_F10,    K_CTL_F11,    K_CTL_F12,    0,            0,
/*  55 */ 0,            0,            0,            0,            0,
/*  60 */ 0,            0,            0,            0,            0,
/*  65 */ K_ALT_F1,     K_ALT_F2,     K_ALT_F3,     K_ALT_F4,     K_ALT_F5,
/*  70 */ K_ALT_F6,     K_ALT_F7,     K_ALT_F8,     K_ALT_F9,     K_ALT_F10,
/*  75 */ K_ALT_F11,    K_ALT_F12,    0,            0,            0,
/*  80 */ K_HOME,       K_INS,        K_INSLN,      K_LEFT,       K_END,
/*  85 */ K_PGDN,       K_PGUP,       K_RIGHT,      K_END,        K_SCRLDN,
/*  90 */ K_SCRLUP,     0,            K_UP,         K_ALT_A,      K_ALT_B,
/*  95 */ K_ALT_C,      K_ALT_D,      K_ALT_E,      K_ALT_F,      K_ALT_G,
/* 100 */ K_ALT_H,      K_ALT_I,      K_ALT_J,      K_ALT_K,      K_ALT_L,
/* 105 */ K_ALT_M,      K_ALT_N,      K_ALT_O,      K_ALT_P,      K_ALT_Q,
/* 110 */ K_ALT_R,      K_ALT_S,      K_ALT_T,      K_ALT_U,      K_ALT_V,
/* 115 */ K_ALT_W,      K_ALT_X,      K_ALT_Y,      K_ALT_Z,      K_ALT_0,
/* 120 */ K_ALT_1,      K_ALT_2,      K_ALT_3,      K_ALT_4,      K_ALT_5,
/* 125 */ K_ALT_6,      K_ALT_7,      K_ALT_8,      K_ALT_9,      K_DEL};



#ifdef OUT_OF_ORDER
static void show_term_descr (struct termios *curr_term_descr)

  {
    int pos;

  /* show_term_descr */
    printf("c_iflag=%x\n", curr_term_descr->c_iflag);      /* input modes */
    printf("c_oflag=%x\n", curr_term_descr->c_oflag);      /* output modes */
    printf("c_cflag=%x\n", curr_term_descr->c_cflag);      /* control modes */
    printf("c_lflag=%x\n", curr_term_descr->c_lflag);      /* local modes */
    for (pos = 0; pos < NCCS; pos++) {
      printf("%d ", curr_term_descr->c_cc[pos]);
    } /* for */
    printf("\n");
    printf("ECHO=%d\n", (curr_term_descr->c_lflag & ECHO) != 0);
    printf("ECHOE=%d\n", (curr_term_descr->c_lflag & ECHOE) != 0);
    printf("ECHOK=%d\n", (curr_term_descr->c_lflag & ECHOK) != 0);
    printf("ECHONL=%d\n", (curr_term_descr->c_lflag & ECHONL) != 0);
    printf("ICANON=%d\n", (curr_term_descr->c_lflag & ICANON) != 0);
    printf("VINTR=%d\n", curr_term_descr->c_cc[VINTR]);
    printf("VQUIT=%d\n", curr_term_descr->c_cc[VQUIT]);
    printf("VSTOP=%d\n", curr_term_descr->c_cc[VSTOP]);
#ifdef VSTART
    printf("VSTART=%d\n", curr_term_descr->c_cc[VSTART]);
#endif
#ifdef VSUSP
    printf("VSUSP=%d\n", curr_term_descr->c_cc[VSUSP]);
#endif
    printf("VMIN=%d\n", curr_term_descr->c_cc[VMIN]);
    printf("VTIME=%d\n", curr_term_descr->c_cc[VTIME]);
  } /* show_term_descr */
#endif



static booltype term_descr_equal (struct termios *term_descr1, struct termios *term_descr2)

  {
    int pos;
    booltype equal;

  /* term_descr_equal */
    /* Comparing with memcmp does not work correctly.    */
    /* Struct termios has data at and after &c_cc[NCCS]. */
    /* Therefore memcmp sees differences, even when the  */
    /* official fields of struct termios are equal.      */
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



static booltype tcset_term_descr (int file_no, struct termios *new_term_descr)

  {
    struct termios term_descr_check;
    int trial = 0;
    booltype succeeded = FALSE;

  /* tcset_term_descr */
    /* The function tcsetattr() returns success if any of the */
    /* requested changes could be successfully carried out.   */
    /* When doing multiple changes it is necessary to check   */
    /* with tcgetattr(), that all changes have been performed */
    /* successfully.                                          */
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



static booltype read_char_if_present (uchartype *ch)

  {
    int file_no;
    struct pollfd poll_fds[1];
    int poll_result;
    booltype result;

  /* read_char_if_present */
    file_no = fileno(stdin);
    poll_fds[0].fd = file_no;
    poll_fds[0].events = POLLIN | POLLPRI;
    poll_result = poll(poll_fds, 1, 1000 /* milliseconds */);
    if (poll_result == 1) {
      /* printf("poll_fds[0].events = %04X\n", poll_fds[0].events);
         printf("poll_fds[0].revents = %04X\n", poll_fds[0].revents); */
      result = read(file_no, ch, 1) == 1;
    } else {
      result = FALSE;
    } /* if */
#ifdef TRACE_FKEYS
    printf("read_char_if_present: ch=%u, poll_result=%d, result=%s\n",
        *ch, poll_result, result ? "TRUE" : "FALSE");
#endif
    return result;
  } /* read_char_if_present */



static chartype read_utf8_key (void)

  {
    size_t len;
    strelemtype stri[6];
    memsizetype dest_len;
    chartype key;

  /* read_utf8_key */
    if (key_buffer[0] <= 0x7F) {
      key = key_buffer[0];
      if (key_buffer_size >= 2) {
        memmove(key_buffer, &key_buffer[1], key_buffer_size - 1);
      } /* if */
      key_buffer_size--;
      return key;
    } else if ((key_buffer[0] & 0xE0) == 0xC0) {
      len = 2;
    } else if ((key_buffer[0] & 0xF0) == 0xE0) {
      len = 3;
    } else if ((key_buffer[0] & 0xF8) == 0xF0) {
      len = 4;
    } else if ((key_buffer[0] & 0xFC) == 0xF8) {
      len = 5;
    } else if ((key_buffer[0] & 0xFC) == 0xFC) {
      len = 6;
    } else {
      key = key_buffer[0];
      if (key_buffer_size >= 2) {
        memmove(key_buffer, &key_buffer[1], key_buffer_size - 1);
      } /* if */
      key_buffer_size--;
      return key;
    } /* if */
    while (key_buffer_size < len) {
      if (read_char_if_present(&key_buffer[key_buffer_size])) {
        key_buffer[key_buffer_size + 1] = '\0';
      } else {
        key_buffer[key_buffer_size] = '\0';
      } /* if */
      if ((key_buffer[key_buffer_size] & 0xC0) != 0x80) {
        key = key_buffer[0];
        if (key_buffer_size >= 2) {
          memmove(key_buffer, &key_buffer[1], key_buffer_size - 1);
        } /* if */
        key_buffer_size--;
        return key;
      } /* if */
      key_buffer_size++;
    } /* while */
    if (utf8_to_stri(stri, &dest_len, key_buffer, len) == 0 && dest_len == 1) {
      key = stri[0];
      if (key_buffer_size > len) {
        memmove(key_buffer, &key_buffer[len], key_buffer_size - len);
      } /* if */
      key_buffer_size -= len;
      return key;
    } else {
      key = key_buffer[0];
      if (key_buffer_size >= 2) {
        memmove(key_buffer, &key_buffer[1], key_buffer_size - 1);
      } /* if */
      key_buffer_size--;
      return key;
    } /* if */
  } /* read_utf8_key */



static void utf8_init (void)

  {
    char *s;

  /* utf8_init */
    if (((s = getenv("LC_ALL"))   && *s) ||
        ((s = getenv("LC_CTYPE")) && *s) ||
        ((s = getenv("LANG"))     && *s)) {
      if (strstr(s, "UTF-8") || strstr(s, "utf8")) {
        utf8_mode = TRUE;
      } /* if */
    } else {
      utf8_mode = TRUE;
    } /* if */
  } /* utf8_init */



static void key_table_init (void)

  {
    int number;
    int num2;

  /* key_table_init */
    /* fprintf(stderr, "keypad_xmit=\"%s\"\n", keypad_xmit); */
    putcontrol(keypad_xmit); /* keypad_transmit_mode */
    key_table[ 0] = erase_ch;   /* K_BS */
    /* printf("erase_ch %d\n", erase_ch[0]); */
    if (key_backspace != NULL && strcmp(key_backspace, erase_ch) != 0) {
      /* If the backspace character defined in the terminfo/termcap */
      /* database is different from the erase character defined by  */
      /* the terminal device it is defined additionally as K_BS.    */
      key_table[ 1] = key_backspace; /* K_BS */
    } else {
      key_table[ 1] = NULL;
    } /* if */
    /* printf("key_backspace %d\n", key_backspace[0]); */
    key_table[ 2] = key_btab;   /* K_BACKTAB */
    key_table[ 3] = key_b2;     /* K_PAD_CENTER */
    key_table[ 4] = key_catab;
    key_table[ 5] = key_clear;
    key_table[ 6] = key_ctab;
    key_table[ 7] = key_dc;     /* K_DEL */
    key_table[ 8] = key_dl;     /* K_DELLN */
    key_table[ 9] = key_down;   /* K_DOWN */
    key_table[10] = key_eic;    /* K_INS */
    key_table[11] = key_end;    /* K_END */
    key_table[12] = key_enter;  /* K_NL */
    key_table[13] = key_eol;
    key_table[14] = key_eos;
    key_table[15] = key_home;   /* K_HOME */
    key_table[16] = key_f0;
    key_table[17] = key_f1;     /* K_F1 */
    key_table[18] = key_f2;     /* K_F2 */
    key_table[19] = key_f3;     /* K_F3 */
    key_table[20] = key_f4;     /* K_F4 */
    key_table[21] = key_f5;     /* K_F5 */
    key_table[22] = key_f6;     /* K_F6 */
    key_table[23] = key_f7;     /* K_F7 */
    key_table[24] = key_f8;     /* K_F8 */
    key_table[25] = key_f9;     /* K_F9 */
    key_table[26] = key_f10;    /* K_F10 */
    key_table[27] = key_f11;    /* K_F11 */
    key_table[28] = key_f12;    /* K_F12 */
    key_table[29] = key_f13;    /* K_SFT_F1 */
    key_table[30] = key_f14;    /* K_SFT_F2 */
    key_table[31] = key_f15;    /* K_SFT_F3 */
    key_table[32] = key_f16;    /* K_SFT_F4 */
    key_table[33] = key_f17;    /* K_SFT_F5 */
    key_table[34] = key_f18;    /* K_SFT_F6 */
    key_table[35] = key_f19;    /* K_SFT_F7 */
    key_table[36] = key_f20;    /* K_SFT_F8 */
    key_table[37] = key_f21;    /* K_SFT_F9 */
    key_table[38] = key_f22;    /* K_SFT_F10 */
    key_table[39] = key_f23;    /* K_SFT_F11 */
    key_table[40] = key_f24;    /* K_SFT_F12 */
    key_table[41] = key_f25;    /* K_CTL_F1 */
    key_table[42] = key_f26;    /* K_CTL_F2 */
    key_table[43] = key_f27;    /* K_CTL_F3 */
    key_table[44] = key_f28;    /* K_CTL_F4 */
    key_table[45] = key_f29;    /* K_CTL_F5 */
    key_table[46] = key_f30;    /* K_CTL_F6 */
    key_table[47] = key_f31;    /* K_CTL_F7 */
    key_table[48] = key_f32;    /* K_CTL_F8 */
    key_table[49] = key_f33;    /* K_CTL_F9 */
    key_table[50] = key_f34;    /* K_CTL_F10 */
    key_table[51] = key_f35;    /* K_CTL_F11 */
    key_table[52] = key_f36;    /* K_CTL_F12 */
    key_table[53] = key_f37;
    key_table[54] = key_f38;
    key_table[55] = key_f39;
    key_table[56] = key_f40;
    key_table[57] = key_f41;
    key_table[58] = key_f42;
    key_table[59] = key_f43;
    key_table[60] = key_f44;
    key_table[61] = key_f45;
    key_table[62] = key_f46;
    key_table[63] = key_f47;
    key_table[64] = key_f48;
    key_table[65] = key_f49;    /* K_ALT_F1 */
    key_table[66] = key_f50;    /* K_ALT_F2 */
    key_table[67] = key_f51;    /* K_ALT_F3 */
    key_table[68] = key_f52;    /* K_ALT_F4 */
    key_table[69] = key_f53;    /* K_ALT_F5 */
    key_table[70] = key_f54;    /* K_ALT_F6 */
    key_table[71] = key_f55;    /* K_ALT_F7 */
    key_table[72] = key_f56;    /* K_ALT_F8 */
    key_table[73] = key_f57;    /* K_ALT_F9 */
    key_table[74] = key_f58;    /* K_ALT_F10 */
    key_table[75] = key_f59;    /* K_ALT_F11 */
    key_table[76] = key_f60;    /* K_ALT_F12 */
    key_table[77] = key_f61;
    key_table[78] = key_f62;
    key_table[79] = key_f63;
    key_table[80] = key_find;   /* K_HOME */
    key_table[81] = key_ic;     /* K_INS */
    key_table[82] = key_il;     /* K_INSLN */
    key_table[83] = key_left;   /* K_LEFT */
    key_table[84] = key_ll;     /* K_END */
    key_table[85] = key_npage;  /* K_PGDN */
    key_table[86] = key_ppage;  /* K_PGUP */
    key_table[87] = key_right;  /* K_RIGHT */
    key_table[88] = key_select; /* K_END */
    key_table[89] = key_sf;     /* K_SCRLDN */
    key_table[90] = key_sr;     /* K_SCRLUP */
    key_table[91] = key_stab;
    key_table[92] = key_up;     /* K_UP */
    key_table[93] = "\033a";    /* K_ALT_A */
    key_table[94] = "\033b";    /* K_ALT_B */
    key_table[95] = "\033c";    /* K_ALT_C */
    key_table[96] = "\033d";    /* K_ALT_D */
    key_table[97] = "\033e";    /* K_ALT_E */
    key_table[98] = "\033f";    /* K_ALT_F */
    key_table[99] = "\033g";    /* K_ALT_G */
    key_table[100] = "\033h";    /* K_ALT_H */
    key_table[101] = "\033i";   /* K_ALT_I */
    key_table[102] = "\033j";   /* K_ALT_J */
    key_table[103] = "\033k";   /* K_ALT_K */
    key_table[104] = "\033l";   /* K_ALT_L */
    key_table[105] = "\033m";   /* K_ALT_M */
    key_table[106] = "\033n";   /* K_ALT_N */
    key_table[107] = "\033o";   /* K_ALT_O */
    key_table[108] = "\033p";   /* K_ALT_P */
    key_table[109] = "\033q";   /* K_ALT_Q */
    key_table[110] = "\033r";   /* K_ALT_R */
    key_table[111] = "\033s";   /* K_ALT_S */
    key_table[112] = "\033t";   /* K_ALT_T */
    key_table[113] = "\033u";   /* K_ALT_U */
    key_table[114] = "\033v";   /* K_ALT_V */
    key_table[115] = "\033w";   /* K_ALT_W */
    key_table[116] = "\033x";   /* K_ALT_X */
    key_table[117] = "\033y";   /* K_ALT_Y */
    key_table[118] = "\033z";   /* K_ALT_Z */
    key_table[119] = "\0330";   /* K_ALT_0 */
    key_table[120] = "\0331";   /* K_ALT_1 */
    key_table[121] = "\0332";   /* K_ALT_2 */
    key_table[122] = "\0333";   /* K_ALT_3 */
    key_table[123] = "\0334";   /* K_ALT_4 */
    key_table[124] = "\0335";   /* K_ALT_5 */
    key_table[125] = "\0336";   /* K_ALT_6 */
    key_table[126] = "\0337";   /* K_ALT_7 */
    key_table[127] = "\0338";   /* K_ALT_8 */
    key_table[128] = "\0339";   /* K_ALT_9 */
    key_table[129] = "\177";    /* K_DEL */
    /* Some function key definitions start with a printable         */
    /* character. This makes absolutly no sense and confuses the    */
    /* function key recognition. Therefore such definitions are     */
    /* thrown out here.                                             */
    for (number = 0; number < SIZE_KEY_TABLE; number++) {
#ifdef TRACE_FKEYS
      if (key_table[number] != NULL) {
        const_cstritype ch_ptr;

        fprintf(stderr, "key%d=\"", number);
        ch_ptr = key_table[number];
        while (*ch_ptr != '\0') {
          if (*ch_ptr <= 31) {
            fprintf(stderr, "^%c", *ch_ptr + '@');
          } else if (*ch_ptr == '^' || *ch_ptr == '\\') {
            fprintf(stderr, "%c%c", *ch_ptr, *ch_ptr);
          } else if (*ch_ptr <= 126) {
            fprintf(stderr, "%c", *ch_ptr);
          } else {
            fprintf(stderr, "\\%3o", *ch_ptr);
          } /* if */
          ch_ptr++;
        } /* while */
        fprintf(stderr, "\"\n");
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



void kbdShut (void)

  { /* kbdShut */
    if (keybd_initialized) {
      tcset_term_descr(fileno(stdin), &term_bak);
      if (caps_initialized) {
        /* fprintf(stderr, "keypad_local=\"%s\"\n", keypad_local); */
        putcontrol(keypad_local); /* out of keypad transmit mode */
      } /* if */
      keybd_initialized = FALSE;
    } /* if */
  } /* kbdShut */



static void kbd_init (void)

  {
    int file_no;

  /* kbd_init */
    if (!caps_initialized) {
      getcaps();
    } /* if */
    file_no = fileno(stdin);
    if (tcgetattr(file_no, &term_descr) != 0) {
      printf("kbd_init: tcgetattr(%d, ...) failed, errno=%d\n",
          file_no, errno);
      printf("EBADF=%d  EINTR=%d  EINVAL=%d  ENOTTY=%d  EIO=%d\n",
          EBADF, EINTR, EINVAL, ENOTTY, EIO);
    } else {
      memcpy(&term_bak, &term_descr, sizeof(struct termios));
      erase_ch[0] = (char) term_descr.c_cc[VERASE];
      erase_ch[1] = '\0';
      /* printf("erase_ch %d\n", erase_ch[0]); */
      term_descr.c_lflag &= (unsigned int) ~(ECHO | ECHOE | ECHOK | ECHONL | ICANON);
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
      if (!tcset_term_descr(file_no, &term_descr)) {
        printf("kbd_init: tcsetattr(%d, VMIN=1) failed, errno=%d\n",
            file_no, errno);
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



booltype kbdKeyPressed (void)

  {
    struct pollfd poll_fds[1];
    booltype result;

  /* kbdKeyPressed */
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    if (key_buffer_size > 0) {
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



chartype kbdGetc (void)

  {
    memsizetype read_result;
    int exact_match_count;
    int exact_matched_key;
    int partial_match_count;
    size_t partial_match_len;
    int partial_matched_key;
    int number;
    size_t len;
    chartype result;

  /* kbdGetc */
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    if (changes) {
      conFlush();
    } /* if */
    if (key_buffer_size == 0) {
      read_result = (memsizetype) read(fileno(stdin), &key_buffer, KEY_BUFFER_SIZE);
      /* printf("kbdGetc: read_result=%ld", read_result); */
      if (read_result == 0 || read_result == (memsizetype) (-1)) {
        return (chartype) EOF;
      } else {
        key_buffer_size = read_result;
      } /* if */
    } /* if */
#ifdef TRACE_FKEYS
    printf("key_buffer:\n");
    for (number = 0; number < key_buffer_size; number++) {
      if (key_buffer[number] == '\\') {
        printf("\\\\");
      } else if (key_buffer[number] >= ' ' && key_buffer[number] <= '~') {
        printf("%c", key_buffer[number]);
      } else {
        printf("\\%u\\", key_buffer[number]);
      } /* if */
    } /* if */
    printf("\n");
#endif
    exact_match_count = 0;
    exact_matched_key = 0;
    partial_match_len = 0;
    partial_match_count = 0;
    partial_matched_key = 0;
    for (number = 0; number < SIZE_KEY_TABLE; number++) {
      if (key_table[number] != NULL) {
        len = strlen(key_table[number]);
        if (key_buffer_size >= len &&
            memcmp(key_table[number], key_buffer, len) == 0) {
          if (key_buffer_size == len) {
            exact_match_count++;
            exact_matched_key = number;
          } else if (len > partial_match_len) {
            partial_match_len = len;
            partial_match_count = 1;
            partial_matched_key = number;
          } else if (len == partial_match_len) {
            partial_match_count++;
            partial_matched_key = number;
          } /* if */
        } /* if */
      } /* if */
    } /* for */
    if (exact_match_count == 1) {
      /* Exact match means:                     */
      /* All characters in key_buffer are used. */
      key_buffer_size = 0;
      result = key_code[exact_matched_key];
    } else if (partial_match_count == 1) {
      /* Partial match means:                       */
      /* Not all characters in key_buffer are used. */
      len = strlen(key_table[partial_matched_key]);
      /* Preserve the characters of the next key(s). */
      memmove(key_buffer, &key_buffer[len], key_buffer_size - len);
      key_buffer_size -= len;
      result = key_code[partial_matched_key];
    } else if (key_buffer[0] == '\033') {
      if (key_buffer_size == 1) {
        /* It is assumed that read() reads all characters of   */
        /* an encoded key. Therefore a single escape character */
        /* cannot be part of a larger escape sequence. When    */
        /* escape sequences can be ripped apart this function  */
        /* will not work correct.                              */
        result = K_ESC;
        key_buffer_size = 0;
      } else if (key_buffer[1] == '\033') {
        /* It is assumed that no key encoding starts with two */
        /* escape characters.                                 */
        result = K_ESC;
        /* Preserve the characters of the next key(s). */
        memmove(key_buffer, &key_buffer[1], key_buffer_size - 1);
        key_buffer_size--;
      } else {
        /* The check for key encodings did not find an unique result. */
        result = K_UNDEF;
        key_buffer_size = 0;
      } /* if */
    } else {
      if (utf8_mode) {
        result = read_utf8_key();
      } else {
        result = key_buffer[0];
        if (key_buffer_size >= 2) {
          /* Preserve the characters of the next key(s). */
          memmove(key_buffer, &key_buffer[1], key_buffer_size - 1);
        } /* if */
        key_buffer_size--;
      } /* if */
    } /* if */
    return result;
  } /* kbdGetc */



chartype kbdRawGetc (void)

  {
    uchartype ch;
    chartype result;

  /* kbdRawRead */
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    if (key_buffer_size > 0) {
      result = key_buffer[0];
      if (key_buffer_size >= 2) {
        memmove(key_buffer, &key_buffer[1], key_buffer_size - 1);
      } /* if */
      key_buffer_size--;
    } else {
      if (changes) {
        conFlush();
      } /* if */
      if (read(fileno(stdin), &ch, 1) != 1) {
        result = (chartype) EOF;
      } else {
        result = (chartype) ch;
      } /* if */
    } /* if */
/*  fprintf(stderr, "<%d>", result); */
    return result;
  } /* kbdRawRead */

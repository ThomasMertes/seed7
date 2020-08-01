/********************************************************************/
/*                                                                  */
/*  kbd_inf.c     Driver for terminfo keyboard access               */
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
/*  Free Software Foundation, Inc., 59 Temple Place, Suite 330,     */
/*  Boston, MA 02111-1307 USA                                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/kbd_inf.c                                       */
/*  Changes: 1994  Thomas Mertes                                    */
/*  Content: Driver for terminfo keyboard access                    */
/*                                                                  */
/********************************************************************/

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"

#include "version.h"
#include "inf_conf.h"


#ifdef INCL_CURSES_BEFORE_TERM
/* The following include is necessary for RM Machines. */
#include "curses.h"
#endif

#include "termios.h"

#ifndef USE_TERMCAP
#ifdef INCL_NCURSES_TERM
#include "ncurses/term.h"
#else
#include "term.h"
#endif
#endif

#include "common.h"
#include "trm_drv.h"

#ifdef USE_TERMCAP
#include "cap_def.h"
#endif

#include "scr_drv.h"

#undef EXTERN
#define EXTERN
#include "kbd_drv.h"


/* #define atexit(x) */


static booltype key_buffer_filled = FALSE;
static int last_key;
extern booltype changes;
static struct termios term_descr;
static struct termios term_bak;

static booltype keybd_initialized = FALSE;


#define SIZE_KEY_TABLE 97

static char *key_table[SIZE_KEY_TABLE];
static char erase_ch[2];

#ifdef OUT_OF_ORDER
char *escape = "\033";

static char **key_table[SIZE_KEY_TABLE] = {
    &key_backspace, &key_catab,     &key_clear,     &key_ctab,      &key_dc,
    &key_dl,        &key_down,      &key_eic,       &key_eol,       &key_eos,
    &key_f0,        &key_f1,        &key_f2,        &key_f3,        &key_f4,
    &key_f5,        &key_f6,        &key_f7,        &key_f8,        &key_f9,
    &key_f10,       &key_f11,       &key_f12,       &key_f13,       &key_f14,
    &key_f15,       &key_f16,       &key_f17,       &key_f18,       &key_f19,
    &key_f20,       &key_f21,       &key_f22,       &key_f23,       &key_f24,
    &key_f25,       &key_f26,       &key_f27,       &key_f28,       &key_f29,
    &key_f30,       &key_home,      &key_ic,        &key_il,        &key_left,
    &key_ll,        &key_npage,     &key_ppage,     &key_right,     &key_sf,
    &key_sr,        &key_stab,      &key_up,        &key_end,       &escape};
#endif

static int key_code[SIZE_KEY_TABLE] = {
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

static chartype read_f_key (chartype actual_char)
#else

static chartype read_f_key (actual_char)
chartype actual_char;
#endif

  {
    char in_buffer[101];
    static char last_partial_match[101];
    static time_t last_partial_time = 0;
    int pos;
    int exact_match;
    int partial_match;
    int number;
    int len;
    int key_number;

  /* read_f_key */
    if (last_partial_time != 0) {
      if (time(NULL) - last_partial_time < 5) {
        /* If the previous call of read_f_key was at most 5 seconds */
        /* ago and the function key was recognised with a partial   */
        /* match and a timeout the following check is done. It is   */
        /* checked if the new character together with the           */
        /* characters from the partial match of the previous call   */
        /* of read_f_key are a possible begin of a function key. In */
        /* this case it was wrong that the previous call of         */
        /* read_f_key submits the function key found with a partial */
        /* match and a timeout. Hopefully the function key did not  */
        /* damage something. Normally the function key deliverd in  */
        /* such a case is the escape key. For that reason escape    */
        /* should not start any dangerous function. Terminating a   */
        /* function with escape should be safe. The following       */
        /* actions are done to avoid further damage. As long as     */
        /* characters are already typed in they are read and        */
        /* discarded. When no more characters are present in the    */
        /* input queue the function returns K_NULLCMD. The          */
        /* K_NULLCMD should do nothing in every application.        */
        /* fprintf(stderr, "<possible garbage keys>"); */
        pos = strlen(last_partial_match);
        last_partial_match[pos] = (char) actual_char;
        last_partial_match[pos + 1] = '\0';
        pos++;
        key_number = 0;
        exact_match = 0;
        partial_match = 0;
        for (number = 0; number < SIZE_KEY_TABLE; number++) {
          if (key_table[number] != NULL) {
            len = strlen(key_table[number]);
            if (pos <= len) {
              if (strncmp(key_table[number], last_partial_match,
                  (SIZE_TYPE) pos) == 0) {
                if (pos == len) {
                  exact_match++;
                  key_number = number;
                } else {
                  partial_match++;
                } /* if */
              } /* if */
            } /* if */
          } /* if */
        } /* for */
        if (exact_match != 0 || partial_match != 0) {
          term_descr.c_cc[VMIN] = 0;
          term_descr.c_cc[VTIME] = 0;
          tcsetattr(fileno(stdin), TCSANOW, &term_descr);
          while (fread(&last_partial_match[pos], 1, 1, stdin) == 1) ;
          term_descr.c_cc[VMIN] = 1;
          term_descr.c_cc[VTIME] = 0;
          tcsetattr(fileno(stdin), TCSANOW, &term_descr);
          last_partial_time = 0;
          return(K_NULLCMD);
        } /* if */
      } /* if */
      last_partial_time = 0;
    } /* if */
    in_buffer[0] = (char) actual_char;
    in_buffer[1] = '\0';
    pos = 1;
    do {
      key_number = 0;
      exact_match = 0;
      partial_match = 0;
      for (number = 0; number < SIZE_KEY_TABLE; number++) {
        if (key_table[number] != NULL) {
          len = strlen(key_table[number]);
          if (pos <= len) {
            if (strncmp(key_table[number], in_buffer,
                (SIZE_TYPE) pos) == 0) {
              if (pos == len) {
                exact_match++;
                key_number = number;
              } else {
                partial_match++;
              } /* if */
            } /* if */
          } /* if */
        } /* if */
      } /* for */
#ifdef TRACE_FKEYS
      { char *cha = in_buffer;
        printf("key match \"");
        while (*cha != 0) {
          if (*cha == '\"' || *cha == '\\') {
            printf("\%c", *cha);
          } else if (*cha >= ' ' && *cha <= '~') {
            printf("%c", *cha);
          } else {
            printf("\\%d\\", (int) *cha);
          } /* if */
          cha++;
        } /* while */
        printf("\"");
        printf("exact %d partial %d\n", exact_match, partial_match);
      }
#endif
      if (exact_match == 0) {
        if (partial_match != 0) {
          in_buffer[pos] = getc(stdin);
          in_buffer[pos + 1] = '\0';
        } /* if */
      } else {
        if (partial_match != 0) {
          term_descr.c_cc[VMIN] = 0;
          term_descr.c_cc[VTIME] = 10;
          tcsetattr(fileno(stdin), TCSANOW, &term_descr);
          if (fread(&in_buffer[pos], 1, 1, stdin) == 1) {
            in_buffer[pos + 1] = '\0';
          } else {
            strcpy(last_partial_match, in_buffer);
            last_partial_time = time(NULL);
            partial_match = 0;
          } /* if */
          term_descr.c_cc[VMIN] = 1;
          term_descr.c_cc[VTIME] = 0;
          tcsetattr(fileno(stdin), TCSANOW, &term_descr);
        } /* if */
      } /* if */
      pos++;
    } while (partial_match != 0 && pos < 100);
    if (exact_match == 1) {
      return(key_code[key_number]);
    } else {
      if (pos == 2) {
        return(actual_char);
      } else if (pos == 3) {
        last_key = in_buffer[1];
        key_buffer_filled = TRUE;
        return(actual_char);
      } else {
        return(K_UNDEF);
      } /* if */
    } /* if */
  } /* read_f_key */



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

  {
    int number;
    int num2;

  /* kbd_init */
    if (!caps_initialized) {
      getcaps();
    } /* if */
    tcgetattr(fileno(stdin), &term_descr);
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
    tcsetattr(fileno(stdin), TCSANOW, &term_descr);
    keybd_initialized = TRUE;
    atexit(kbdShut);
    if (getcaps()) {
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
    } /* if */
  } /* kbd_init */



#ifdef ANSI_C

booltype kbdKeyPressed (void)
#else

booltype kbdKeyPressed ()
#endif

  {
    booltype result;
    char buffer;

  /* kbdKeyPressed */
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    if (key_buffer_filled) {
      result = TRUE;
    } else {
      if (changes) {
        scrFlush();
      } /* if */
      term_descr.c_cc[VMIN] = 0;
      term_descr.c_cc[VTIME] = 0;
      tcsetattr(fileno(stdin), TCSANOW, &term_descr);
      if (fread(&buffer, 1, 1, stdin) == 1) {
        result = TRUE;
        last_key = buffer;
        key_buffer_filled = TRUE;
      } else {
        result = FALSE;
      } /* if */
      term_descr.c_cc[VMIN] = 1;
      term_descr.c_cc[VTIME] = 0;
      tcsetattr(fileno(stdin), TCSANOW, &term_descr);
    } /* if */
    return(result);
  } /* kbdKeyPressed */



#ifdef ANSI_C

chartype kbdGetc (void)
#else

chartype kbdGetc ()
#endif

  {
    chartype result;

  /* kbdGetc */
    if (!keybd_initialized) {
      kbd_init();
    } /* if */
    if (key_buffer_filled) {
      key_buffer_filled = FALSE;
      result = last_key;
    } else {
      if (changes) {
        scrFlush();
      } /* if */
      result = getc(stdin);
    } /* if */
    result = read_f_key(result);
/*  fprintf(stderr, "<%d>", result); */
    return(result);
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
      result = last_key;
    } else {
      if (changes) {
        scrFlush();
      } /* if */
      result = getc(stdin);
    } /* if */
/*  fprintf(stderr, "<%d>", result); */
    return(result);
  } /* kbdRawRead */

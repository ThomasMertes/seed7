/********************************************************************/
/*                                                                  */
/*  fwd_term.h    Forward calls to a shared terminfo library.       */
/*  Copyright (C) 1989 - 2019  Thomas Mertes                        */
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
/*  File: seed7/src/fwd_term.h                                      */
/*  Changes: 2019  Thomas Mertes                                    */
/*  Content: Forward calls to a shared terminfo library.            */
/*                                                                  */
/********************************************************************/

#ifndef EXTERN
#define EXTERN extern
#endif

extern int setupterm(const char *term, int filedes, int *errret);
extern char *tgoto (const char *cap, int col, int row);
extern int tputs (const char *str, int affcnt, int (*putc)(int));

extern int tgetent (char *bp, const char *name);
extern int tgetflag (const char *id);
extern int tgetnum (const char *id);
extern char *tgetstr (const char *id, char **area);

EXTERN int columns;
EXTERN int lines;

EXTERN int auto_right_margin;
EXTERN int ceol_standout_glitch;
EXTERN int move_insert_mode;

EXTERN char *carriage_return;
EXTERN char *clear_screen;
EXTERN char *clr_eol;
EXTERN char *cursor_address;
EXTERN char *cursor_down;
EXTERN char *cursor_invisible;
EXTERN char *cursor_left;
EXTERN char *cursor_normal;
EXTERN char *cursor_right;
EXTERN char *cursor_up;
EXTERN char *cursor_visible;
EXTERN char *delete_character;
EXTERN char *delete_line;
EXTERN char *enter_ca_mode;
EXTERN char *enter_delete_mode;
EXTERN char *enter_insert_mode;
EXTERN char *enter_reverse_mode;
EXTERN char *enter_standout_mode;
EXTERN char *exit_ca_mode;
EXTERN char *exit_delete_mode;
EXTERN char *exit_insert_mode;
EXTERN char *exit_standout_mode;
EXTERN char *insert_character;
EXTERN char *insert_line;
EXTERN char *pad_char;
EXTERN char *parm_delete_line;
EXTERN char *parm_insert_line;

EXTERN char *keypad_local;
EXTERN char *keypad_xmit;
EXTERN char *key_b2;
EXTERN char *key_backspace;
EXTERN char *key_btab;
EXTERN char *key_catab;
EXTERN char *key_clear;
EXTERN char *key_ctab;
EXTERN char *key_dc;
EXTERN char *key_dl;
EXTERN char *key_down;
EXTERN char *key_eic;
EXTERN char *key_end;
EXTERN char *key_enter;
EXTERN char *key_eol;
EXTERN char *key_eos;
EXTERN char *key_find;
EXTERN char *key_home;
EXTERN char *key_ic;
EXTERN char *key_il;
EXTERN char *key_left;
EXTERN char *key_ll;
EXTERN char *key_npage;
EXTERN char *key_ppage;
EXTERN char *key_right;
EXTERN char *key_select;
EXTERN char *key_sf;
EXTERN char *key_sr;
EXTERN char *key_stab;
EXTERN char *key_up;

EXTERN char *key_f0;
EXTERN char *key_f1;
EXTERN char *key_f2;
EXTERN char *key_f3;
EXTERN char *key_f4;
EXTERN char *key_f5;
EXTERN char *key_f6;
EXTERN char *key_f7;
EXTERN char *key_f8;
EXTERN char *key_f9;
EXTERN char *key_f10;
EXTERN char *key_f11;
EXTERN char *key_f12;
EXTERN char *key_f13;
EXTERN char *key_f14;
EXTERN char *key_f15;
EXTERN char *key_f16;
EXTERN char *key_f17;
EXTERN char *key_f18;
EXTERN char *key_f19;
EXTERN char *key_f20;
EXTERN char *key_f21;
EXTERN char *key_f22;
EXTERN char *key_f23;
EXTERN char *key_f24;
EXTERN char *key_f25;
EXTERN char *key_f26;
EXTERN char *key_f27;
EXTERN char *key_f28;
EXTERN char *key_f29;
EXTERN char *key_f30;
EXTERN char *key_f31;
EXTERN char *key_f32;
EXTERN char *key_f33;
EXTERN char *key_f34;
EXTERN char *key_f35;
EXTERN char *key_f36;
EXTERN char *key_f37;
EXTERN char *key_f38;
EXTERN char *key_f39;
EXTERN char *key_f40;
EXTERN char *key_f41;
EXTERN char *key_f42;
EXTERN char *key_f43;
EXTERN char *key_f44;
EXTERN char *key_f45;
EXTERN char *key_f46;
EXTERN char *key_f47;
EXTERN char *key_f48;
EXTERN char *key_f49;
EXTERN char *key_f50;
EXTERN char *key_f51;
EXTERN char *key_f52;
EXTERN char *key_f53;
EXTERN char *key_f54;
EXTERN char *key_f55;
EXTERN char *key_f56;
EXTERN char *key_f57;
EXTERN char *key_f58;
EXTERN char *key_f59;
EXTERN char *key_f60;
EXTERN char *key_f61;
EXTERN char *key_f62;
EXTERN char *key_f63;

typedef struct term {
    int dummy;
  } TERMINAL;


#ifndef HAS_TERMIOS_H

/* c_cc characters */
#define VINTR  0
#define VQUIT  1
#define VERASE 2
#define VKILL  3
#define VEOF   4
#define VTIME  5
#define VMIN   6
#define VSWTC  7
#define VSTART 8
#define VSTOP  9
#define VSUSP 10

/* c_lflag bits */
#define ICANON  0000002
#define ECHO    0000010
#define ECHOE   0000020
#define ECHOK   0000040
#define ECHONL  0000100

/* tcsetattr() actions */
#define TCSANOW    0
#define TCSADRAIN  1
#define TCSAFLUSH  2

typedef unsigned char cc_t;
typedef unsigned int  speed_t;
typedef unsigned int  tcflag_t;

#define NCCS 32

struct termios {
    tcflag_t c_iflag;
    tcflag_t c_oflag;
    tcflag_t c_cflag;
    tcflag_t c_lflag;
    cc_t c_line;
    cc_t c_cc[NCCS];
    speed_t c_ispeed;
    speed_t c_ospeed;
  };

extern int tcgetattr (int fd, struct termios *termios_p);

extern int tcsetattr (int fd, int actions,
                      const struct termios *termios_p);

#endif

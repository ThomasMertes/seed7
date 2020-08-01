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

extern char *tgoto (const char *cap, int col, int row);
extern int tputs (const char *str, int affcnt, int (*putc)(int));
extern int setupterm(const char *term, int filedes, int *errret);

int columns;
int lines;

int auto_right_margin;
int ceol_standout_glitch;
int move_insert_mode;

char *carriage_return;
char *clear_screen;
char *clr_eol;
char *cursor_address;
char *cursor_down;
char *cursor_invisible;
char *cursor_left;
char *cursor_normal;
char *cursor_right;
char *cursor_up;
char *cursor_visible;
char *delete_character;
char *delete_line;
char *enter_ca_mode;
char *enter_delete_mode;
char *enter_insert_mode;
char *enter_reverse_mode;
char *enter_standout_mode;
char *exit_ca_mode;
char *exit_delete_mode;
char *exit_insert_mode;
char *exit_standout_mode;
char *insert_character;
char *insert_line;
char *pad_char;
char *parm_delete_line;
char *parm_insert_line;

char *keypad_local;
char *keypad_xmit;
char *key_b2;
char *key_backspace;
char *key_btab;
char *key_catab;
char *key_clear;
char *key_ctab;
char *key_dc;
char *key_dl;
char *key_down;
char *key_eic;
char *key_end;
char *key_enter;
char *key_eol;
char *key_eos;
char *key_find;
char *key_home;
char *key_ic;
char *key_il;
char *key_left;
char *key_ll;
char *key_npage;
char *key_ppage;
char *key_right;
char *key_select;
char *key_sf;
char *key_sr;
char *key_stab;
char *key_up;

char *key_f0;
char *key_f1;
char *key_f2;
char *key_f3;
char *key_f4;
char *key_f5;
char *key_f6;
char *key_f7;
char *key_f8;
char *key_f9;
char *key_f10;
char *key_f11;
char *key_f12;
char *key_f13;
char *key_f14;
char *key_f15;
char *key_f16;
char *key_f17;
char *key_f18;
char *key_f19;
char *key_f20;
char *key_f21;
char *key_f22;
char *key_f23;
char *key_f24;
char *key_f25;
char *key_f26;
char *key_f27;
char *key_f28;
char *key_f29;
char *key_f30;
char *key_f31;
char *key_f32;
char *key_f33;
char *key_f34;
char *key_f35;
char *key_f36;
char *key_f37;
char *key_f38;
char *key_f39;
char *key_f40;
char *key_f41;
char *key_f42;
char *key_f43;
char *key_f44;
char *key_f45;
char *key_f46;
char *key_f47;
char *key_f48;
char *key_f49;
char *key_f50;
char *key_f51;
char *key_f52;
char *key_f53;
char *key_f54;
char *key_f55;
char *key_f56;
char *key_f57;
char *key_f58;
char *key_f59;
char *key_f60;
char *key_f61;
char *key_f62;
char *key_f63;

typedef struct term {
    int dummy;
  } TERMINAL;

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

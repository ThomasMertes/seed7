/********************************************************************/
/*                                                                  */
/*  cap_def.h     Define terminfo vars for termcap screen access.    */
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
/*  File: seed7/src/cap_def.h                                       */
/*  Changes: 1994  Thomas Mertes                                    */
/*  Content: Define terminfo vars for termcap screen access.        */
/*                                                                  */
/********************************************************************/

EXTERN char *insert_line;            /* al insert line */
EXTERN booltype auto_right_margin;   /* am automatic margins (warp around) */
EXTERN char *BC;                     /* backspace (if not BS) */
EXTERN char *clr_eos;                /* cd clear to end of display */
EXTERN char *clr_eol;                /* ce clear to end of line */
EXTERN char *clear_screen;           /* cl clear screen */
EXTERN int columns;                  /* co number of columns */
EXTERN char *cursor_address;         /* cm cursor motion */
EXTERN char *carriage_return;        /* cr cursor return */
EXTERN char *delete_character;       /* dc delete character */
EXTERN char *delete_line;            /* dl delete line */
EXTERN char *enter_delete_mode;      /* dm enter delete mode (not used now) */
EXTERN char *cursor_down;            /* do cursor down */
EXTERN char *exit_delete_mode;       /* ed end delete mode (not used now) */
EXTERN char *exit_insert_mode;       /* ei end insert mode */
EXTERN booltype hard_copy;           /* hc hardcopy terminal (not used now) */
EXTERN char *insert_character;       /* ic insert character */
EXTERN char *enter_insert_mode;      /* im enter insert mode */
EXTERN char *IP;                     /* post-insert mode */
EXTERN char *cursor_left;            /* le cursor left (not used now) */
EXTERN int lines;                    /* li number of lines */
EXTERN booltype move_insert_mode;    /* mi safe to move while in insert mode */
EXTERN char *cursor_right;           /* nd cursor right (not used now) */
EXTERN char single_pad_char;         /* pc pad character */
EXTERN char *pad_char;               /* pc pad character */
EXTERN char *cursor_normal;          /* ve cursor normal */
EXTERN char *exit_standout_mode;     /* se end standout mode (not used now) */
EXTERN char *enter_standout_mode;    /* so enter standout mode (not used now) */
EXTERN char *exit_ca_mode;           /* te end cursor addressing mode */
EXTERN char *enter_ca_mode;          /* ti enter cursor addressing mode */
EXTERN char *cursor_up;              /* up cursor up (not used now) */
EXTERN char *cursor_visible;         /* vs makes cursor very visible */
EXTERN char *cursor_invisible;       /* vi makes cursor invisible */
EXTERN char *enter_reverse_mode;     /* mr turn on reverse video mode */
EXTERN booltype ceol_standout_glitch;/* xs standout not erased by overwriting */
EXTERN char *parm_insert_line;       /* AL add #1 new blank lines (PG*) */
EXTERN char *parm_delete_line;       /* DL delete #1 lines (PG*) */
EXTERN char *keypad_local;           /* ke out of keypad transmit mode */
EXTERN char *keypad_xmit;            /* ks put terminal in keypad transmit mode */
EXTERN char *key_backspace;          /* kb sent by backspace key */
EXTERN char *key_btab;               /* kB sent by back-tab key */
EXTERN char *key_b2;                 /* K2 sent by center of keypad */
EXTERN char *key_catab;              /* ka sent by clear-all-tabs key */
EXTERN char *key_clear;              /* kC sent by clear screen or erase key */
EXTERN char *key_ctab;               /* kt sent by clear tab key */
EXTERN char *key_dc;                 /* kD sent by delete character key */
EXTERN char *key_dl;                 /* kL sent by delete line key */
EXTERN char *key_down;               /* kd sent by terminal down arrow key */
EXTERN char *key_eic;                /* kM sent by rmir or smir in insert mode */
EXTERN char *key_end;                /* @7 sent by end key */
EXTERN char *key_enter;              /* @8 sent by enter/send key */
EXTERN char *key_eol;                /* kE sent by clear-to-end-of-line key */
EXTERN char *key_eos;                /* kS sent by clear-to-end-of-screen key */
EXTERN char *key_f0;                 /* k0 sent by function key f0 */
EXTERN char *key_f1;                 /* k1 sent by function key f1 */
EXTERN char *key_f2;                 /* k2 sent by function key f2 */
EXTERN char *key_f3;                 /* k3 sent by function key f3 */
EXTERN char *key_f4;                 /* k4 sent by function key f4 */
EXTERN char *key_f5;                 /* k5 sent by function key f5 */
EXTERN char *key_f6;                 /* k6 sent by function key f6 */
EXTERN char *key_f7;                 /* k7 sent by function key f7 */
EXTERN char *key_f8;                 /* k8 sent by function key f8 */
EXTERN char *key_f9;                 /* k9 sent by function key f9 */
EXTERN char *key_f10;                /* k; sent by function key f10 */
EXTERN char *key_f11;                /* F1 sent by function key f11 */
EXTERN char *key_f12;                /* F2 sent by function key f12 */
EXTERN char *key_f13;                /* F3 sent by function key f13 */
EXTERN char *key_f14;                /* F4 sent by function key f14 */
EXTERN char *key_f15;                /* F5 sent by function key f15 */
EXTERN char *key_f16;                /* F6 sent by function key f16 */
EXTERN char *key_f17;                /* F7 sent by function key f17 */
EXTERN char *key_f18;                /* F8 sent by function key f18 */
EXTERN char *key_f19;                /* F9 sent by function key f19 */
EXTERN char *key_f20;                /* FA sent by function key f20 */
EXTERN char *key_f21;                /* FB sent by function key f21 */
EXTERN char *key_f22;                /* FC sent by function key f22 */
EXTERN char *key_f23;                /* FD sent by function key f23 */
EXTERN char *key_f24;                /* FE sent by function key f24 */
EXTERN char *key_f25;                /* FF sent by function key f25 */
EXTERN char *key_f26;                /* FG sent by function key f26 */
EXTERN char *key_f27;                /* FH sent by function key f27 */
EXTERN char *key_f28;                /* FI sent by function key f28 */
EXTERN char *key_f29;                /* FJ sent by function key f29 */
EXTERN char *key_f30;                /* FK sent by function key f30 */
EXTERN char *key_f31;                /* FL sent by function key f31 */
EXTERN char *key_f32;                /* FM sent by function key f32 */
EXTERN char *key_f33;                /* FN sent by function key f33 */
EXTERN char *key_f34;                /* FO sent by function key f34 */
EXTERN char *key_f35;                /* FP sent by function key f35 */
EXTERN char *key_f36;                /* FQ sent by function key f36 */
EXTERN char *key_f37;                /* FR sent by function key f37 */
EXTERN char *key_f38;                /* FS sent by function key f38 */
EXTERN char *key_f39;                /* FT sent by function key f39 */
EXTERN char *key_f40;                /* FU sent by function key f40 */
EXTERN char *key_f41;                /* FV sent by function key f41 */
EXTERN char *key_f42;                /* FW sent by function key f42 */
EXTERN char *key_f43;                /* FX sent by function key f43 */
EXTERN char *key_f44;                /* FY sent by function key f44 */
EXTERN char *key_f45;                /* FZ sent by function key f45 */
EXTERN char *key_f46;                /* Fa sent by function key f46 */
EXTERN char *key_f47;                /* Fb sent by function key f47 */
EXTERN char *key_f48;                /* Fc sent by function key f48 */
EXTERN char *key_f49;                /* Fd sent by function key f49 */
EXTERN char *key_f50;                /* Fe sent by function key f50 */
EXTERN char *key_f51;                /* Ff sent by function key f51 */
EXTERN char *key_f52;                /* Fg sent by function key f52 */
EXTERN char *key_f53;                /* Fh sent by function key f53 */
EXTERN char *key_f54;                /* Fi sent by function key f54 */
EXTERN char *key_f55;                /* Fj sent by function key f55 */
EXTERN char *key_f56;                /* Fk sent by function key f56 */
EXTERN char *key_f57;                /* Fl sent by function key f57 */
EXTERN char *key_f58;                /* Fm sent by function key f58 */
EXTERN char *key_f59;                /* Fn sent by function key f59 */
EXTERN char *key_f60;                /* Fo sent by function key f60 */
EXTERN char *key_f61;                /* Fp sent by function key f61 */
EXTERN char *key_f62;                /* Fq sent by function key f62 */
EXTERN char *key_f63;                /* Fr sent by function key f63 */
EXTERN char *key_find;               /* @0 sent by find key */
EXTERN char *key_home;               /* kh sent by home key */
EXTERN char *key_ic;                 /* kI sent by ins char/enter ins mode key */
EXTERN char *key_il;                 /* kA sent by insert line key */
EXTERN char *key_left;               /* kl sent by terminal left arrow key */
EXTERN char *key_ll;                 /* kH sent by home-down key */
EXTERN char *key_npage;              /* kN sent by next-page key */
EXTERN char *key_ppage;              /* kP sent by previous-page key */
EXTERN char *key_right;              /* kr sent by terminal right arrow key */
EXTERN char *key_select;             /* *6 sent by select key */
EXTERN char *key_sf;                 /* kF sent by scroll-forward/down key */
EXTERN char *key_sr;                 /* kR sent by scroll-backward/up key */
EXTERN char *key_stab;               /* kT sent by set-tab key */
EXTERN char *key_up;                 /* ku sent by terminal up arrow key */

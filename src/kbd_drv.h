/********************************************************************/
/*                                                                  */
/*  kbd_drv.h     Prototypes for keyboard access functions.         */
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
/*  File: seed7/src/kbd_drv.h                                       */
/*  Changes: 1990, 1991, 1992, 1993, 1994, 2005  Thomas Mertes      */
/*  Content: Prototypes for keyboard access functions.              */
/*                                                                  */
/********************************************************************/

#define K_CTL_A        1
#define K_CTL_B        2
#define K_CTL_C        3
#define K_CTL_D        4
#define K_CTL_E        5
#define K_CTL_F        6
#define K_CTL_G        7
#define K_CTL_H        8
#define K_BS           8
#define K_CTL_I        9
#define K_TAB          9
#define K_CTL_J       10
#define K_NL          10
#define K_CTL_K       11
#define K_CTL_L       12
#define K_CTL_M       13
#define K_CTL_N       14
#define K_CTL_O       15
#define K_CTL_P       16
#define K_CTL_Q       17
#define K_CTL_R       18
#define K_CTL_S       19
#define K_CTL_T       20
#define K_CTL_U       21
#define K_CTL_V       22
#define K_CTL_W       23
#define K_CTL_X       24
#define K_CTL_Y       25
#define K_CTL_Z       26
#define K_ESC         27
#define K_ALT_A      257
#define K_ALT_B      258
#define K_ALT_C      259
#define K_ALT_D      260
#define K_ALT_E      261
#define K_ALT_F      262
#define K_ALT_G      263
#define K_ALT_H      264
#define K_ALT_I      265
#define K_ALT_J      266
#define K_ALT_K      267
#define K_ALT_L      268
#define K_ALT_M      269
#define K_ALT_N      270
#define K_ALT_O      271
#define K_ALT_P      272
#define K_ALT_Q      273
#define K_ALT_R      274
#define K_ALT_S      275
#define K_ALT_T      276
#define K_ALT_U      277
#define K_ALT_V      278
#define K_ALT_W      279
#define K_ALT_X      280
#define K_ALT_Y      281
#define K_ALT_Z      282
#define K_ALT_0      304
#define K_ALT_1      305
#define K_ALT_2      306
#define K_ALT_3      307
#define K_ALT_4      308
#define K_ALT_5      309
#define K_ALT_6      310
#define K_ALT_7      311
#define K_ALT_8      312
#define K_ALT_9      313
#define K_F1         320
#define K_F2         321
#define K_F3         322
#define K_F4         323
#define K_F5         324
#define K_F6         325
#define K_F7         326
#define K_F8         327
#define K_F9         328
#define K_F10        329
#define K_SFT_F1     336
#define K_SFT_F2     337
#define K_SFT_F3     338
#define K_SFT_F4     339
#define K_SFT_F5     340
#define K_SFT_F6     341
#define K_SFT_F7     342
#define K_SFT_F8     343
#define K_SFT_F9     344
#define K_SFT_F10    345
#define K_CTL_F1     352
#define K_CTL_F2     353
#define K_CTL_F3     354
#define K_CTL_F4     355
#define K_CTL_F5     356
#define K_CTL_F6     357
#define K_CTL_F7     358
#define K_CTL_F8     359
#define K_CTL_F9     360
#define K_CTL_F10    361
#define K_ALT_F1     368
#define K_ALT_F2     369
#define K_ALT_F3     370
#define K_ALT_F4     371
#define K_ALT_F5     372
#define K_ALT_F6     373
#define K_ALT_F7     374
#define K_ALT_F8     375
#define K_ALT_F9     376
#define K_ALT_F10    377
#define K_NULCHAR    400
#define K_BACKTAB    401
#define K_LEFT       416
#define K_RIGHT      417
#define K_UP         418
#define K_DOWN       419
#define K_HOME       420
#define K_END        421
#define K_PGUP       422
#define K_PGDN       423
#define K_INS        424
#define K_DEL        425
#define K_PAD_CENTER 426
#define K_CTL_LEFT   480
#define K_CTL_RIGHT  481
#define K_CTL_UP     482
#define K_CTL_DOWN   483
#define K_CTL_HOME   484
#define K_CTL_END    485
#define K_CTL_PGUP   486
#define K_CTL_PGDN   487
#define K_CTL_INS    488
#define K_CTL_DEL    489
#define K_SCRLUP     490
#define K_SCRLDN     491
#define K_INSLN      492
#define K_DELLN      493
#define K_ERASE      494
#define K_CTL_NL     495
#define K_NULLCMD    500
#define K_REDRAW     501
#define K_NEWWINDOW  502
#define K_MOUSE1     503
#define K_MOUSE2     504
#define K_MOUSE3     505
#define K_MOUSE4     506
#define K_MOUSE5     507
#define K_UNDEF      511
#define K_NONE       512

/*
    MOUSE1ON,  MOUSE1OFF, MOUSE2ON,  MOUSE2OFF, MOUSE3ON,  MOUSE3OFF,
    CHARACTER, CTL_BS,
*/


#ifdef ANSI_C

chartype kbdGetc (void);
booltype kbdKeyPressed (void);
chartype kbdRawGetc (void);
void kbdShut (void);

#else

chartype kbdGetc ();
booltype kbdKeyPressed ();
chartype kbdRawGetc ();
void kbdShut ();

#endif

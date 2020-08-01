/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000  Thomas Mertes                        */
/*                                                                  */
/*  This program is free software; you can redistribute it and/or   */
/*  modify it under the terms of the GNU General Public License as  */
/*  published by the Free Software Foundation; either version 2 of  */
/*  the License, or (at your option) any later version.             */
/*                                                                  */
/*  This program is distributed in the hope that it will be useful, */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   */
/*  GNU General Public License for more details.                    */
/*                                                                  */
/*  You should have received a copy of the GNU General Public       */
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/kbdlib.h                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions to do keyboard input.            */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype kbd_busy_getc  (listtype arguments);
objecttype kbd_getc       (listtype arguments);
objecttype kbd_gets       (listtype arguments);
objecttype kbd_keypressed (listtype arguments);
objecttype kbd_line_read  (listtype arguments);
objecttype kbd_raw_getc   (listtype arguments);
objecttype kbd_word_read  (listtype arguments);

#else

objecttype kbd_busy_getc ();
objecttype kbd_getc ();
objecttype kbd_gets ();
objecttype kbd_keypressed ();
objecttype kbd_line_read ();
objecttype kbd_raw_getc ();
objecttype kbd_word_read ();

#endif

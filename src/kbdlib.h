/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/kbdlib.h                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions to do keyboard input.            */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype kbd_busy_getc (listtype);
objecttype kbd_getc (listtype);
objecttype kbd_gets (listtype);
objecttype kbd_keypressed (listtype);
objecttype kbd_line_read (listtype);
objecttype kbd_raw_getc (listtype);
objecttype kbd_word_read (listtype);

#else

objecttype kbd_busy_getc ();
objecttype kbd_getc ();
objecttype kbd_gets ();
objecttype kbd_keypressed ();
objecttype kbd_line_read ();
objecttype kbd_raw_getc ();
objecttype kbd_word_read ();

#endif

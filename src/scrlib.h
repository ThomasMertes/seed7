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
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/scrlib.h                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions to do text screen output.        */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype scr_clear  (listtype arguments);
objecttype scr_cursor (listtype arguments);
objecttype scr_flush  (listtype arguments);
objecttype scr_height (listtype arguments);
objecttype scr_h_scl  (listtype arguments);
objecttype scr_open   (listtype arguments);
objecttype scr_setpos (listtype arguments);
objecttype scr_v_scl  (listtype arguments);
objecttype scr_width  (listtype arguments);
objecttype scr_write  (listtype arguments);

#else

objecttype scr_clear ();
objecttype scr_cursor ();
objecttype scr_flush ();
objecttype scr_height ();
objecttype scr_h_scl ();
objecttype scr_open ();
objecttype scr_setpos ();
objecttype scr_v_scl ();
objecttype scr_width ();
objecttype scr_write ();

#endif

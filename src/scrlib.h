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
/*  File: seed7/src/scrlib.h                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions to do text screen output.        */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype scr_clear (listtype);
objecttype scr_cursor (listtype);
objecttype scr_flush (listtype);
objecttype scr_height (listtype);
objecttype scr_h_scl (listtype);
objecttype scr_open (listtype);
objecttype scr_setpos (listtype);
objecttype scr_v_scl (listtype);
objecttype scr_width (listtype);
objecttype scr_write (listtype);

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

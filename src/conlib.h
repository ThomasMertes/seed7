/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2011  Thomas Mertes                        */
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
/*  File: seed7/src/conlib.h                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions to do text console output.       */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype con_clear  (listtype arguments);
objecttype con_cursor (listtype arguments);
objecttype con_flush  (listtype arguments);
objecttype con_height (listtype arguments);
objecttype con_h_scl  (listtype arguments);
objecttype con_open   (listtype arguments);
objecttype con_setpos (listtype arguments);
objecttype con_v_scl  (listtype arguments);
objecttype con_width  (listtype arguments);
objecttype con_write  (listtype arguments);

#else

objecttype con_clear ();
objecttype con_cursor ();
objecttype con_flush ();
objecttype con_height ();
objecttype con_h_scl ();
objecttype con_open ();
objecttype con_setpos ();
objecttype con_v_scl ();
objecttype con_width ();
objecttype con_write ();

#endif

/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2005  Thomas Mertes                        */
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
/*  File: seed7/src/ut8lib.h                                        */
/*  Changes: 2005  Thomas Mertes                                    */
/*  Content: All Primitive actions for the UTF-8 file type.         */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype ut8_getc      (listtype arguments);
objecttype ut8_gets      (listtype arguments);
objecttype ut8_line_read (listtype arguments);
objecttype ut8_seek      (listtype arguments);
objecttype ut8_word_read (listtype arguments);
objecttype ut8_write     (listtype arguments);

#else

objecttype ut8_getc ();
objecttype ut8_gets ();
objecttype ut8_line_read ();
objecttype ut8_seek ();
objecttype ut8_word_read ();
objecttype ut8_write ();

#endif

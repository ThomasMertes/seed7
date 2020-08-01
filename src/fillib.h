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
/*  File: seed7/src/fillib.h                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions for the primitive file type.     */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype fil_big_lng (listtype);
objecttype fil_big_seek (listtype);
objecttype fil_big_tell (listtype);
objecttype fil_close (listtype);
objecttype fil_cpy (listtype);
objecttype fil_create (listtype);
objecttype fil_empty (listtype);
objecttype fil_eof (listtype);
objecttype fil_eq (listtype);
objecttype fil_err (listtype);
objecttype fil_flush (listtype);
objecttype fil_getc (listtype);
objecttype fil_gets (listtype);
objecttype fil_in (listtype);
objecttype fil_lit (listtype);
objecttype fil_lng (listtype);
objecttype fil_line_read (listtype);
objecttype fil_ne (listtype);
objecttype fil_open (listtype);
objecttype fil_out (listtype);
objecttype fil_popen (listtype);
objecttype fil_seek (listtype);
objecttype fil_tell (listtype);
objecttype fil_value (listtype);
objecttype fil_word_read (listtype);
objecttype fil_write (listtype);

#else

objecttype fil_big_lng ();
objecttype fil_big_seek ();
objecttype fil_big_tell ();
objecttype fil_close ();
objecttype fil_cpy ();
objecttype fil_create ();
objecttype fil_empty ();
objecttype fil_eof ();
objecttype fil_eq ();
objecttype fil_err ();
objecttype fil_flush ();
objecttype fil_getc ();
objecttype fil_gets ();
objecttype fil_in ();
objecttype fil_lit();
objecttype fil_lng();
objecttype fil_line_read ();
objecttype fil_ne ();
objecttype fil_open ();
objecttype fil_out ();
objecttype fil_popen ();
objecttype fil_seek ();
objecttype fil_tell ();
objecttype fil_value ();
objecttype fil_word_read ();
objecttype fil_write ();

#endif

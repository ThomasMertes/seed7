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
/*  File: seed7/src/fillib.h                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions for the primitive file type.     */
/*                                                                  */
/********************************************************************/

objectType fil_big_lng          (listType arguments);
objectType fil_big_seek         (listType arguments);
objectType fil_big_tell         (listType arguments);
objectType fil_close            (listType arguments);
objectType fil_cpy              (listType arguments);
objectType fil_create           (listType arguments);
objectType fil_destr            (listType arguments);
objectType fil_empty            (listType arguments);
objectType fil_eof              (listType arguments);
objectType fil_eq               (listType arguments);
objectType fil_err              (listType arguments);
objectType fil_flush            (listType arguments);
objectType fil_getc             (listType arguments);
objectType fil_gets             (listType arguments);
objectType fil_has_next         (listType arguments);
objectType fil_in               (listType arguments);
objectType fil_input_ready      (listType arguments);
objectType fil_line_read        (listType arguments);
objectType fil_lit              (listType arguments);
objectType fil_lng              (listType arguments);
objectType fil_ne               (listType arguments);
objectType fil_open             (listType arguments);
objectType fil_open_null_device (listType arguments);
objectType fil_out              (listType arguments);
objectType fil_pclose           (listType arguments);
objectType fil_pipe             (listType arguments);
objectType fil_popen            (listType arguments);
objectType fil_print            (listType arguments);
objectType fil_seek             (listType arguments);
objectType fil_seekable         (listType arguments);
objectType fil_setbuf           (listType arguments);
objectType fil_tell             (listType arguments);
objectType fil_truncate         (listType arguments);
objectType fil_value            (listType arguments);
objectType fil_word_read        (listType arguments);
objectType fil_write            (listType arguments);

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
/*  File: seed7/src/drwlib.h                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions to do graphic output.            */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype gkb_busy_getc (listtype);
objecttype gkb_getc (listtype);
objecttype gkb_keypressed (listtype);
objecttype gkb_line_read (listtype);
objecttype gkb_raw_getc (listtype);
objecttype gkb_stri_read (listtype);
objecttype gkb_word_read (listtype);
objecttype gkb_xpos (listtype);
objecttype gkb_ypos (listtype);
objecttype drw_arc (listtype);
objecttype drw_arc2 (listtype);
objecttype drw_background (listtype);
objecttype drw_circle (listtype);
objecttype drw_clear (listtype);
objecttype drw_color (listtype);
objecttype drw_copyarea (listtype);
objecttype drw_cpy (listtype);
objecttype drw_create (listtype);
objecttype drw_destr (listtype);
objecttype drw_empty (listtype);
objecttype drw_eq (listtype);
objecttype drw_farcchord (listtype);
objecttype drw_farcpieslice (listtype);
objecttype drw_fcircle (listtype);
objecttype drw_fellipse (listtype);
objecttype drw_flush (listtype);
objecttype drw_get (listtype);
objecttype drw_height (listtype);
objecttype drw_image (listtype);
objecttype drw_line (listtype);
objecttype drw_ne (listtype);
objecttype drw_new_pixmap (listtype);
objecttype drw_open (listtype);
objecttype drw_parc (listtype);
objecttype drw_pcircle (listtype);
objecttype drw_pfarcchord (listtype);
objecttype drw_pfarcpieslice (listtype);
objecttype drw_pfcircle (listtype);
objecttype drw_pfellipse (listtype);
objecttype drw_pline (listtype);
objecttype drw_point (listtype);
objecttype drw_ppoint (listtype);
objecttype drw_prect (listtype);
objecttype drw_put (listtype);
objecttype drw_rect (listtype);
objecttype drw_rgbcol (listtype);
objecttype drw_rot (listtype);
objecttype drw_scale (listtype);
objecttype drw_text (listtype);
objecttype drw_width (listtype);

#else

objecttype gkb_busy_getc ();
objecttype gkb_getc ();
objecttype gkb_keypressed ();
objecttype gkb_line_read ();
objecttype gkb_raw_getc ();
objecttype gkb_stri_read ();
objecttype gkb_word_read ();
objecttype gkb_xpos ();
objecttype gkb_ypos ();
objecttype drw_arc ();
objecttype drw_arc2 ();
objecttype drw_background ();
objecttype drw_circle ();
objecttype drw_clear ();
objecttype drw_color ();
objecttype drw_copyarea ();
objecttype drw_cpy ();
objecttype drw_create ();
objecttype drw_destr ();
objecttype drw_empty ();
objecttype drw_eq ();
objecttype drw_farcchord ();
objecttype drw_farcpieslice ();
objecttype drw_fcircle ();
objecttype drw_fellipse ();
objecttype drw_flush ();
objecttype drw_get ();
objecttype drw_height ();
objecttype drw_image ();
objecttype drw_line ();
objecttype drw_ne ();
objecttype drw_new_pixmap ();
objecttype drw_open ();
objecttype drw_parc ();
objecttype drw_pcircle ();
objecttype drw_pfarcchord ();
objecttype drw_pfarcpieslice ();
objecttype drw_pfcircle ();
objecttype drw_pfellipse ();
objecttype drw_pline ();
objecttype drw_point ();
objecttype drw_ppoint ();
objecttype drw_prect ();
objecttype drw_put ();
objecttype drw_rect ();
objecttype drw_rgbcol ();
objecttype drw_rot ();
objecttype drw_scale ();
objecttype drw_text ();
objecttype drw_width ();

#endif

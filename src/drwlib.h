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
/*  File: seed7/src/drwlib.h                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions to do graphic output.            */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype gkb_busy_getc           (listtype arguments);
objecttype gkb_button_pressed      (listtype arguments);
objecttype gkb_button_xpos         (listtype arguments);
objecttype gkb_button_ypos         (listtype arguments);
objecttype gkb_getc                (listtype arguments);
objecttype gkb_gets                (listtype arguments);
objecttype gkb_keypressed          (listtype arguments);
objecttype gkb_line_read           (listtype arguments);
objecttype gkb_raw_getc            (listtype arguments);
objecttype gkb_window              (listtype arguments);
objecttype gkb_word_read           (listtype arguments);
objecttype drw_arc                 (listtype arguments);
objecttype drw_arc2                (listtype arguments);
objecttype drw_background          (listtype arguments);
objecttype drw_circle              (listtype arguments);
objecttype drw_clear               (listtype arguments);
objecttype drw_cmp                 (listtype arguments);
objecttype drw_color               (listtype arguments);
objecttype drw_copyarea            (listtype arguments);
objecttype drw_cpy                 (listtype arguments);
objecttype drw_create              (listtype arguments);
objecttype drw_destr               (listtype arguments);
objecttype drw_empty               (listtype arguments);
objecttype drw_eq                  (listtype arguments);
objecttype drw_farcchord           (listtype arguments);
objecttype drw_farcpieslice        (listtype arguments);
objecttype drw_fcircle             (listtype arguments);
objecttype drw_fellipse            (listtype arguments);
objecttype drw_flush               (listtype arguments);
objecttype drw_fpolyLine           (listtype arguments);
objecttype drw_genPointList        (listtype arguments);
objecttype drw_get                 (listtype arguments);
objecttype drw_hashcode            (listtype arguments);
objecttype drw_height              (listtype arguments);
objecttype drw_image               (listtype arguments);
objecttype drw_line                (listtype arguments);
objecttype drw_ne                  (listtype arguments);
objecttype drw_new_pixmap          (listtype arguments);
objecttype drw_open                (listtype arguments);
objecttype drw_open_sub_window     (listtype arguments);
objecttype drw_parc                (listtype arguments);
objecttype drw_pcircle             (listtype arguments);
objecttype drw_pfarcchord          (listtype arguments);
objecttype drw_pfarcpieslice       (listtype arguments);
objecttype drw_pfcircle            (listtype arguments);
objecttype drw_pfellipse           (listtype arguments);
objecttype drw_pline               (listtype arguments);
objecttype drw_point               (listtype arguments);
objecttype drw_pointer_xpos        (listtype arguments);
objecttype drw_pointer_ypos        (listtype arguments);
objecttype drw_polyLine            (listtype arguments);
objecttype drw_ppoint              (listtype arguments);
objecttype drw_prect               (listtype arguments);
objecttype drw_put                 (listtype arguments);
objecttype drw_rect                (listtype arguments);
objecttype drw_rgbcol              (listtype arguments);
objecttype drw_rot                 (listtype arguments);
objecttype drw_scale               (listtype arguments);
objecttype drw_setContent          (listtype arguments);
objecttype drw_setPos              (listtype arguments);
objecttype drw_setTransparentColor (listtype arguments);
objecttype drw_text                (listtype arguments);
objecttype drw_toBottom            (listtype arguments);
objecttype drw_toTop               (listtype arguments);
objecttype drw_width               (listtype arguments);
objecttype drw_xpos                (listtype arguments);
objecttype drw_ypos                (listtype arguments);

#else

objecttype gkb_busy_getc ();
objecttype gkb_button_pressed ();
objecttype gkb_button_xpos ();
objecttype gkb_button_ypos ();
objecttype gkb_getc ();
objecttype gkb_gets ();
objecttype gkb_keypressed ();
objecttype gkb_line_read ();
objecttype gkb_raw_getc ();
objecttype gkb_window ();
objecttype gkb_word_read ();
objecttype drw_arc ();
objecttype drw_arc2 ();
objecttype drw_background ();
objecttype drw_circle ();
objecttype drw_clear ();
objecttype drw_cmp ();
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
objecttype drw_fpolyLine ();
objecttype drw_genPointList ();
objecttype drw_get ();
objecttype drw_hashcode ();
objecttype drw_height ();
objecttype drw_image ();
objecttype drw_line ();
objecttype drw_ne ();
objecttype drw_new_pixmap ();
objecttype drw_open ();
objecttype drw_open_sub_window ();
objecttype drw_parc ();
objecttype drw_pcircle ();
objecttype drw_pfarcchord ();
objecttype drw_pfarcpieslice ();
objecttype drw_pfcircle ();
objecttype drw_pfellipse ();
objecttype drw_pline ();
objecttype drw_point ();
objecttype drw_pointer_xpos ();
objecttype drw_pointer_ypos ();
objecttype drw_polyLine ();
objecttype drw_ppoint ();
objecttype drw_prect ();
objecttype drw_put ();
objecttype drw_rect ();
objecttype drw_rgbcol ();
objecttype drw_rot ();
objecttype drw_scale ();
objecttype drw_setContent ();
objecttype drw_setPos ();
objecttype drw_setTransparentColor ();
objecttype drw_text ();
objecttype drw_toBottom ();
objecttype drw_toTop ();
objecttype drw_width ();
objecttype drw_xpos ();
objecttype drw_ypos ();

#endif

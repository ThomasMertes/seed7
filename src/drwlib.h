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
/*  File: seed7/src/drwlib.h                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: All primitive actions to do graphic output.            */
/*                                                                  */
/********************************************************************/

objectType gkb_busy_getc              (listType arguments);
objectType gkb_button_pressed         (listType arguments);
objectType gkb_button_xpos            (listType arguments);
objectType gkb_button_ypos            (listType arguments);
objectType gkb_getc                   (listType arguments);
objectType gkb_gets                   (listType arguments);
objectType gkb_keypressed             (listType arguments);
objectType gkb_line_read              (listType arguments);
objectType gkb_raw_getc               (listType arguments);
objectType gkb_select_input           (listType arguments);
objectType gkb_window                 (listType arguments);
objectType gkb_word_read              (listType arguments);
objectType drw_arc                    (listType arguments);
objectType drw_arc2                   (listType arguments);
objectType drw_background             (listType arguments);
objectType drw_border                 (listType arguments);
objectType drw_capture                (listType arguments);
objectType drw_circle                 (listType arguments);
objectType drw_clear                  (listType arguments);
objectType drw_cmp                    (listType arguments);
objectType drw_color                  (listType arguments);
objectType drw_convPointList          (listType arguments);
objectType drw_copyarea               (listType arguments);
objectType drw_cpy                    (listType arguments);
objectType drw_create                 (listType arguments);
objectType drw_destr                  (listType arguments);
objectType drw_empty                  (listType arguments);
objectType drw_eq                     (listType arguments);
objectType drw_farcchord              (listType arguments);
objectType drw_farcpieslice           (listType arguments);
objectType drw_fcircle                (listType arguments);
objectType drw_fellipse               (listType arguments);
objectType drw_flush                  (listType arguments);
objectType drw_fpolyLine              (listType arguments);
objectType drw_genPointList           (listType arguments);
objectType drw_get                    (listType arguments);
objectType drw_getImage               (listType arguments);
objectType drw_getImagePixel          (listType arguments);
objectType drw_getPixel               (listType arguments);
objectType drw_getPixelArray          (listType arguments);
objectType drw_get_pixmap_from_pixels (listType arguments);
objectType drw_hashcode               (listType arguments);
objectType drw_height                 (listType arguments);
objectType drw_line                   (listType arguments);
objectType drw_ne                     (listType arguments);
objectType drw_new_pixmap             (listType arguments);
objectType drw_open                   (listType arguments);
objectType drw_open_sub_window        (listType arguments);
objectType drw_parc                   (listType arguments);
objectType drw_pcircle                (listType arguments);
objectType drw_pfarc                  (listType arguments);
objectType drw_pfarcchord             (listType arguments);
objectType drw_pfarcpieslice          (listType arguments);
objectType drw_pfcircle               (listType arguments);
objectType drw_pfellipse              (listType arguments);
objectType drw_pixelToRgb             (listType arguments);
objectType drw_pline                  (listType arguments);
objectType drw_point                  (listType arguments);
objectType drw_pointer_xpos           (listType arguments);
objectType drw_pointer_ypos           (listType arguments);
objectType drw_polyLine               (listType arguments);
objectType drw_ppoint                 (listType arguments);
objectType drw_prect                  (listType arguments);
objectType drw_put                    (listType arguments);
objectType drw_put_scaled             (listType arguments);
objectType drw_rect                   (listType arguments);
objectType drw_rgbcol                 (listType arguments);
objectType drw_rot                    (listType arguments);
objectType drw_screen_height          (listType arguments);
objectType drw_screen_width           (listType arguments);
objectType drw_scale                  (listType arguments);
objectType drw_setCloseAction         (listType arguments);
objectType drw_setContent             (listType arguments);
objectType drw_setCursorVisible       (listType arguments);
objectType drw_setPos                 (listType arguments);
objectType drw_setTransparentColor    (listType arguments);
objectType drw_setWindowName          (listType arguments);
objectType drw_text                   (listType arguments);
objectType drw_toBottom               (listType arguments);
objectType drw_toTop                  (listType arguments);
objectType drw_value                  (listType arguments);
objectType drw_width                  (listType arguments);
objectType drw_xpos                   (listType arguments);
objectType drw_ypos                   (listType arguments);

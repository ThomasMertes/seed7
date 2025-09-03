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

objectType gkb_button_pressed            (listType arguments);
objectType gkb_clicked_xpos              (listType arguments);
objectType gkb_clicked_ypos              (listType arguments);
objectType gkb_getc                      (listType arguments);
objectType gkb_gets                      (listType arguments);
objectType gkb_input_ready               (listType arguments);
objectType gkb_line_read                 (listType arguments);
objectType gkb_raw_getc                  (listType arguments);
objectType gkb_select_input              (listType arguments);
objectType gkb_window                    (listType arguments);
objectType gkb_word_read                 (listType arguments);
objectType drw_border                    (listType arguments);
objectType drw_capture                   (listType arguments);
objectType drw_clear                     (listType arguments);
objectType drw_cmp                       (listType arguments);
objectType drw_color                     (listType arguments);
objectType drw_conv_point_list           (listType arguments);
objectType drw_copyarea                  (listType arguments);
objectType drw_cpy                       (listType arguments);
objectType drw_create                    (listType arguments);
objectType drw_destr                     (listType arguments);
objectType drw_empty                     (listType arguments);
objectType drw_eq                        (listType arguments);
objectType drw_flush                     (listType arguments);
objectType drw_fpoly_line                (listType arguments);
objectType drw_gen_point_list            (listType arguments);
objectType drw_get_image_pixel           (listType arguments);
objectType drw_get_pixel                 (listType arguments);
objectType drw_get_pixel_array           (listType arguments);
objectType drw_get_pixel_data            (listType arguments);
objectType drw_get_pixel_data_from_array (listType arguments);
objectType drw_get_pixmap                (listType arguments);
objectType drw_get_pixmap_from_pixels    (listType arguments);
objectType drw_hashcode                  (listType arguments);
objectType drw_height                    (listType arguments);
objectType drw_ne                        (listType arguments);
objectType drw_new_pixmap                (listType arguments);
objectType drw_open                      (listType arguments);
objectType drw_open_sub_window           (listType arguments);
objectType drw_parc                      (listType arguments);
objectType drw_pcircle                   (listType arguments);
objectType drw_pfarc                     (listType arguments);
objectType drw_pfarcchord                (listType arguments);
objectType drw_pfarcpieslice             (listType arguments);
objectType drw_pfcircle                  (listType arguments);
objectType drw_pfellipse                 (listType arguments);
objectType drw_pixel_to_rgb              (listType arguments);
objectType drw_pline                     (listType arguments);
objectType drw_pointer_xpos              (listType arguments);
objectType drw_pointer_ypos              (listType arguments);
objectType drw_poly_line                 (listType arguments);
objectType drw_ppoint                    (listType arguments);
objectType drw_prect                     (listType arguments);
objectType drw_put                       (listType arguments);
objectType drw_put_scaled                (listType arguments);
objectType drw_rgbcol                    (listType arguments);
objectType drw_rot                       (listType arguments);
objectType drw_screen_height             (listType arguments);
objectType drw_screen_width              (listType arguments);
objectType drw_scale                     (listType arguments);
objectType drw_set_content               (listType arguments);
objectType drw_set_cursor_visible        (listType arguments);
objectType drw_set_pointer_pos           (listType arguments);
objectType drw_set_pos                   (listType arguments);
objectType drw_set_size                  (listType arguments);
objectType drw_set_transparent_color     (listType arguments);
objectType drw_set_window_name           (listType arguments);
objectType drw_text                      (listType arguments);
objectType drw_to_bottom                 (listType arguments);
objectType drw_to_top                    (listType arguments);
objectType drw_value                     (listType arguments);
objectType drw_width                     (listType arguments);
objectType drw_xpos                      (listType arguments);
objectType drw_ypos                      (listType arguments);

objectType plt_bstring                   (listType arguments);
objectType plt_cmp                       (listType arguments);
objectType plt_cpy                       (listType arguments);
objectType plt_create                    (listType arguments);
objectType plt_destr                     (listType arguments);
objectType plt_empty                     (listType arguments);
objectType plt_eq                        (listType arguments);
objectType plt_hashcode                  (listType arguments);
objectType plt_ne                        (listType arguments);
objectType plt_point_list                (listType arguments);
objectType plt_value                     (listType arguments);

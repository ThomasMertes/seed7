/********************************************************************/
/*                                                                  */
/*  x11_rend.h    X11 xRender extension interfaces used by Seed7.   */
/*  Copyright (C) 1989 - 2021  Thomas Mertes                        */
/*                                                                  */
/*  This file is part of the Seed7 Runtime Library.                 */
/*                                                                  */
/*  The Seed7 Runtime Library is free software; you can             */
/*  redistribute it and/or modify it under the terms of the GNU     */
/*  Lesser General Public License as published by the Free Software */
/*  Foundation; either version 2.1 of the License, or (at your      */
/*  option) any later version.                                      */
/*                                                                  */
/*  The Seed7 Runtime Library is distributed in the hope that it    */
/*  will be useful, but WITHOUT ANY WARRANTY; without even the      */
/*  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR */
/*  PURPOSE.  See the GNU Lesser General Public License for more    */
/*  details.                                                        */
/*                                                                  */
/*  You should have received a copy of the GNU Lesser General       */
/*  Public License along with this program; if not, write to the    */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/x11_x.h                                         */
/*  Changes: 2019  Thomas Mertes                                    */
/*  Content: X11 interfaces used by Seed7.                          */
/*                                                                  */
/********************************************************************/

#define PictOpOver  3

#define CPSubwindowMode    (1 << 8)

#define XDoubleToFixed(f)    ((XFixed) ((f) * 65536))

typedef struct { int dumme; } XRenderPictFormat;

typedef XID Picture;

typedef int XFixed;

typedef struct {
    XFixed matrix[3][3];
  } XTransform;

typedef struct {
    int     repeat;
    Picture alpha_map;
    int     alpha_x_origin;
    int     alpha_y_origin;
    int     clip_x_origin;
    int     clip_y_origin;
    Pixmap  clip_mask;
    Bool    graphics_exposures;
    int     subwindow_mode;
    int     poly_edge;
    int     poly_mode;
    Atom    dither;
    Bool    component_alpha;
} XRenderPictureAttributes;


extern void XRenderComposite (Display *display, int op, Picture src,
                              Picture mask, Picture dst, int src_x, int src_y,
                              int mask_x, int mask_y, int dst_x, int dst_y,
                              unsigned int width, unsigned int height);
extern Picture XRenderCreatePicture (Display *display,
                                     Drawable drawable,
                                     const XRenderPictFormat *format,
                                     unsigned long valuemask,
                                     const XRenderPictureAttributes *attributes);
extern XRenderPictFormat *XRenderFindVisualFormat (Display *display,
                                                   const Visual *visual);
extern void XRenderFreePicture (Display *display, Picture picture);
extern void XRenderSetPictureTransform (Display *display,
                                        Picture picture,
                                        XTransform *transform);

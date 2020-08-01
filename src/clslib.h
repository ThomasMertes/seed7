/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2002  Thomas Mertes                        */
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
/*  File: seed7/src/arrlib.h                                        */
/*  Changes: 1993, 1994, 2002  Thomas Mertes                        */
/*  Content: All primitive actions for structure types.             */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype cls_conv2 (listtype);
objecttype cls_cpy (listtype);
objecttype cls_cpy2 (listtype);
objecttype cls_create (listtype);
objecttype cls_create2 (listtype);
objecttype cls_eq (listtype);
objecttype cls_ne (listtype);
objecttype cls_select (listtype);

#else

objecttype cls_conv2 ();
objecttype cls_cpy ();
objecttype cls_cpy2 ();
objecttype cls_create ();
objecttype cls_create2 ();
objecttype cls_eq ();
objecttype cls_ne ();
objecttype cls_select ();

#endif

/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2008  Thomas Mertes                        */
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
/*  File: seed7/src/itflib.h                                        */
/*  Changes: 1993, 1994, 2002, 2008  Thomas Mertes                  */
/*  Content: All primitive actions for interface types.             */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype itf_cmp          (listtype arguments);
objecttype itf_conv2        (listtype arguments);
objecttype itf_cpy          (listtype arguments);
objecttype itf_cpy2         (listtype arguments);
objecttype itf_create       (listtype arguments);
objecttype itf_create2      (listtype arguments);
objecttype itf_eq           (listtype arguments);
objecttype itf_hashcode     (listtype arguments);
objecttype itf_ne           (listtype arguments);
objecttype itf_select       (listtype arguments);
objecttype itf_to_heap      (listtype arguments);
objecttype itf_to_interface (listtype arguments);

#else

objecttype itf_cmp ();
objecttype itf_conv2 ();
objecttype itf_cpy ();
objecttype itf_cpy2 ();
objecttype itf_create ();
objecttype itf_create2 ();
objecttype itf_eq ();
objecttype itf_hashcode ();
objecttype itf_ne ();
objecttype itf_select ();
objecttype itf_to_heap ();
objecttype itf_to_interface ();

#endif

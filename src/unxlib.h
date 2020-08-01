/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/unxlib.h                                        */
/*  Changes: 1994  Thomas Mertes                                    */
/*  Content: All primitive actions for files and directorys.        */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype unx_cd (listtype);
objecttype unx_cp (listtype);
objecttype unx_lng (listtype);
objecttype unx_ls (listtype);
objecttype unx_mkdir (listtype);
objecttype unx_mv (listtype);
objecttype unx_pwd (listtype);
objecttype unx_rm (listtype);
objecttype unx_sh (listtype);
objecttype unx_sleep (listtype);

#else

objecttype unx_cd ();
objecttype unx_cp ();
objecttype unx_lng ();
objecttype unx_ls ();
objecttype unx_mkdir ();
objecttype unx_mv ();
objecttype unx_pwd ();
objecttype unx_rm ();
objecttype unx_sh ();
objecttype unx_sleep ();

#endif

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
/*  File: seed7/src/cmdlib.h                                        */
/*  Changes: 1994  Thomas Mertes                                    */
/*  Content: Primitive actions for various commands.                */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype cmd_chdir (listtype);
objecttype cmd_copy (listtype);
objecttype cmd_getcwd (listtype);
objecttype cmd_lng (listtype);
objecttype cmd_ls (listtype);
objecttype cmd_mkdir (listtype);
objecttype cmd_move (listtype);
objecttype cmd_remove (listtype);
objecttype cmd_sh (listtype);
objecttype cmd_sleep (listtype);

#else

objecttype cmd_chdir ();
objecttype cmd_copy ();
objecttype cmd_getcwd ();
objecttype cmd_lng ();
objecttype cmd_ls ();
objecttype cmd_mkdir ();
objecttype cmd_move ();
objecttype cmd_remove ();
objecttype cmd_sh ();
objecttype cmd_sleep ();

#endif

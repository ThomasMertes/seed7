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
/*  License along with this program; if not, write to the           */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Library                                                 */
/*  File: seed7/src/cmdlib.h                                        */
/*  Changes: 1994  Thomas Mertes                                    */
/*  Content: Primitive actions for various commands.                */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype cmd_big_filesize (listtype);
objecttype cmd_chdir (listtype);
objecttype cmd_clone_file (listtype);
objecttype cmd_config_value (listtype);
objecttype cmd_copy_file (listtype);
objecttype cmd_filemode (listtype);
objecttype cmd_filesize (listtype);
objecttype cmd_filetype (listtype);
objecttype cmd_filetype_sl (listtype);
objecttype cmd_getcwd (listtype);
objecttype cmd_get_atime (listtype);
objecttype cmd_get_ctime (listtype);
objecttype cmd_get_mtime (listtype);
objecttype cmd_ls (listtype);
objecttype cmd_mkdir (listtype);
objecttype cmd_move (listtype);
objecttype cmd_readlink (listtype);
objecttype cmd_remove (listtype);
objecttype cmd_remove_any_file (listtype);
objecttype cmd_set_atime (listtype);
objecttype cmd_set_filemode (listtype);
objecttype cmd_set_mtime (listtype);
objecttype cmd_shell (listtype);
objecttype cmd_symlink (listtype);

#else

objecttype cmd_big_filesize ();
objecttype cmd_chdir ();
objecttype cmd_clone_file ();
objecttype cmd_config_value ();
objecttype cmd_copy_file ();
objecttype cmd_filemode ();
objecttype cmd_filesize ();
objecttype cmd_filetype ();
objecttype cmd_filetype_sl ();
objecttype cmd_getcwd ();
objecttype cmd_get_atime ();
objecttype cmd_get_ctime ();
objecttype cmd_get_mtime ();
objecttype cmd_ls ();
objecttype cmd_mkdir ();
objecttype cmd_move ();
objecttype cmd_readlink ();
objecttype cmd_remove ();
objecttype cmd_remove_any_file ();
objecttype cmd_set_atime ();
objecttype cmd_set_filemode ();
objecttype cmd_set_mtime ();
objecttype cmd_shell ();
objecttype cmd_symlink ();

#endif

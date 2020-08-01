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

objecttype cmd_big_filesize    (listtype arguments);
objecttype cmd_chdir           (listtype arguments);
objecttype cmd_clone_file      (listtype arguments);
objecttype cmd_config_value    (listtype arguments);
objecttype cmd_copy_file       (listtype arguments);
objecttype cmd_filemode        (listtype arguments);
objecttype cmd_filesize        (listtype arguments);
objecttype cmd_filetype        (listtype arguments);
objecttype cmd_filetype_sl     (listtype arguments);
objecttype cmd_getcwd          (listtype arguments);
objecttype cmd_getenv          (listtype arguments);
objecttype cmd_get_atime       (listtype arguments);
objecttype cmd_get_ctime       (listtype arguments);
objecttype cmd_get_mtime       (listtype arguments);
objecttype cmd_ls              (listtype arguments);
objecttype cmd_mkdir           (listtype arguments);
objecttype cmd_move            (listtype arguments);
objecttype cmd_pipe2           (listtype arguments);
objecttype cmd_readlink        (listtype arguments);
objecttype cmd_remove          (listtype arguments);
objecttype cmd_remove_any_file (listtype arguments);
objecttype cmd_setenv          (listtype arguments);
objecttype cmd_set_atime       (listtype arguments);
objecttype cmd_set_filemode    (listtype arguments);
objecttype cmd_set_mtime       (listtype arguments);
objecttype cmd_shell           (listtype arguments);
objecttype cmd_symlink         (listtype arguments);
objecttype cmd_to_os_path      (listtype arguments);

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
objecttype cmd_getenv ();
objecttype cmd_get_atime ();
objecttype cmd_get_ctime ();
objecttype cmd_get_mtime ();
objecttype cmd_ls ();
objecttype cmd_mkdir ();
objecttype cmd_move ();
objecttype cmd_pipe2 ();
objecttype cmd_readlink ();
objecttype cmd_remove ();
objecttype cmd_remove_any_file ();
objecttype cmd_setenv ();
objecttype cmd_set_atime ();
objecttype cmd_set_filemode ();
objecttype cmd_set_mtime ();
objecttype cmd_shell ();
objecttype cmd_symlink ();
objecttype cmd_to_os_path ();

#endif

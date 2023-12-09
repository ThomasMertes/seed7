/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
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
/*  Content: Primitive actions for various system commands.         */
/*                                                                  */
/********************************************************************/

objectType cmd_big_filesize         (listType arguments);
objectType cmd_chdir                (listType arguments);
objectType cmd_clone_file           (listType arguments);
objectType cmd_config_value         (listType arguments);
objectType cmd_copy_file            (listType arguments);
objectType cmd_environment          (listType arguments);
objectType cmd_filemode             (listType arguments);
objectType cmd_filesize             (listType arguments);
objectType cmd_filetype             (listType arguments);
objectType cmd_filetype_sl          (listType arguments);
objectType cmd_final_path           (listType arguments);
objectType cmd_getcwd               (listType arguments);
objectType cmd_getenv               (listType arguments);
objectType cmd_get_atime            (listType arguments);
objectType cmd_get_ctime            (listType arguments);
objectType cmd_get_group            (listType arguments);
objectType cmd_get_group_of_symlink (listType arguments);
objectType cmd_get_mtime            (listType arguments);
objectType cmd_get_mtime_of_symlink (listType arguments);
objectType cmd_get_owner            (listType arguments);
objectType cmd_get_owner_of_symlink (listType arguments);
objectType cmd_get_search_path      (listType arguments);
objectType cmd_home_dir             (listType arguments);
objectType cmd_ls                   (listType arguments);
objectType cmd_mkdir                (listType arguments);
objectType cmd_move                 (listType arguments);
objectType cmd_read_link            (listType arguments);
objectType cmd_read_link_absolute   (listType arguments);
objectType cmd_remove_file          (listType arguments);
objectType cmd_remove_tree          (listType arguments);
objectType cmd_setenv               (listType arguments);
objectType cmd_set_atime            (listType arguments);
objectType cmd_set_filemode         (listType arguments);
objectType cmd_set_group            (listType arguments);
objectType cmd_set_group_of_symlink (listType arguments);
objectType cmd_set_mtime            (listType arguments);
objectType cmd_set_mtime_of_symlink (listType arguments);
objectType cmd_set_owner            (listType arguments);
objectType cmd_set_owner_of_symlink (listType arguments);
objectType cmd_set_search_path      (listType arguments);
objectType cmd_shell                (listType arguments);
objectType cmd_shell_escape         (listType arguments);
objectType cmd_symlink              (listType arguments);
objectType cmd_to_os_path           (listType arguments);
objectType cmd_unsetenv             (listType arguments);

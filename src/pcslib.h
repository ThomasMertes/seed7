/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2014  Thomas Mertes                        */
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
/*  File: seed7/src/pcslib.h                                        */
/*  Changes: 2014  Thomas Mertes                                    */
/*  Content: All primitive actions for the process type.            */
/*                                                                  */
/********************************************************************/

objectType pcs_child_stderr (listType arguments);
objectType pcs_child_stdin  (listType arguments);
objectType pcs_child_stdout (listType arguments);
objectType pcs_cmp          (listType arguments);
objectType pcs_cpy          (listType arguments);
objectType pcs_create       (listType arguments);
objectType pcs_destr        (listType arguments);
objectType pcs_empty        (listType arguments);
objectType pcs_eq           (listType arguments);
objectType pcs_exit_value   (listType arguments);
objectType pcs_hashcode     (listType arguments);
objectType pcs_is_alive     (listType arguments);
objectType pcs_kill         (listType arguments);
objectType pcs_ne           (listType arguments);
objectType pcs_pipe2        (listType arguments);
objectType pcs_pty          (listType arguments);
objectType pcs_start        (listType arguments);
objectType pcs_str          (listType arguments);
objectType pcs_value        (listType arguments);
objectType pcs_wait_for     (listType arguments);

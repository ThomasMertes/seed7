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
/*  File: seed7/src/sqllib.h                                        */
/*  Changes: 2013, 2014  Thomas Mertes                              */
/*  Content: All primitive actions for database access.             */
/*                                                                  */
/********************************************************************/

objectType sql_bind_bigint       (listType arguments);
objectType sql_bind_bigrat       (listType arguments);
objectType sql_bind_bool         (listType arguments);
objectType sql_bind_bstri        (listType arguments);
objectType sql_bind_duration     (listType arguments);
objectType sql_bind_float        (listType arguments);
objectType sql_bind_int          (listType arguments);
objectType sql_bind_null         (listType arguments);
objectType sql_bind_stri         (listType arguments);
objectType sql_bind_time         (listType arguments);
objectType sql_close             (listType arguments);
objectType sql_column_bigint     (listType arguments);
objectType sql_column_bigrat     (listType arguments);
objectType sql_column_bool       (listType arguments);
objectType sql_column_bstri      (listType arguments);
objectType sql_column_duration   (listType arguments);
objectType sql_column_float      (listType arguments);
objectType sql_column_int        (listType arguments);
objectType sql_column_stri       (listType arguments);
objectType sql_column_time       (listType arguments);
objectType sql_commit            (listType arguments);
objectType sql_cpy_db            (listType arguments);
objectType sql_cpy_stmt          (listType arguments);
objectType sql_create_db         (listType arguments);
objectType sql_create_stmt       (listType arguments);
objectType sql_destr_db          (listType arguments);
objectType sql_destr_stmt        (listType arguments);
objectType sql_empty_db          (listType arguments);
objectType sql_empty_stmt        (listType arguments);
objectType sql_execute           (listType arguments);
objectType sql_fetch             (listType arguments);
objectType sql_is_null           (listType arguments);
objectType sql_open              (listType arguments);
objectType sql_prepare           (listType arguments);
objectType sql_stmt_column_count (listType arguments);
objectType sql_stmt_column_name  (listType arguments);

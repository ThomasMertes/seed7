/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2013  Thomas Mertes                        */
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
/*  Module: Analyzer - Libraries                                    */
/*  File: seed7/src/libpath.h                                       */
/*  Changes: 1990 - 1994, 2013  Thomas Mertes                       */
/*  Content: Procedures to manage the include library search path.  */
/*                                                                  */
/********************************************************************/

void find_include_file (const_stritype include_file_name, errinfotype *err_info);
void append_to_lib_path (const_stritype path, errinfotype *err_info);
void init_lib_path (const_stritype source_file_name,
    const const_rtlArraytype seed7_libraries, errinfotype *err_info);
void free_lib_path (void);

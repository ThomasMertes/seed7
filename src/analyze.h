/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2000  Thomas Mertes                        */
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
/*  Module: Analyzer - Main                                         */
/*  File: seed7/src/analyze.h                                       */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: Main procedure of the analyzing phase.                 */
/*                                                                  */
/********************************************************************/

progtype analyze_file (const const_stritype source_file_argument, uinttype options,
    const const_rtlArraytype seed7_libraries, const const_stritype prot_file_name,
    errinfotype *err_info);
progtype analyze (const const_stritype source_file_argument, uinttype options,
    const const_rtlArraytype libraryDirs, const const_stritype prot_file_name);
progtype analyze_string (const const_stritype input_string, uinttype options,
    const const_rtlArraytype seed7_libraries, const const_stritype prot_file_name,
    errinfotype *err_info);

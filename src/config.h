/********************************************************************/
/*                                                                  */
/*  config.h      Configuration settings.                           */
/*  Copyright (C) 1989 - 2012  Thomas Mertes                        */
/*                                                                  */
/*  This file is part of the Seed7 Runtime Library.                 */
/*                                                                  */
/*  The Seed7 Runtime Library is free software; you can             */
/*  redistribute it and/or modify it under the terms of the GNU     */
/*  Lesser General Public License as published by the Free Software */
/*  Foundation; either version 2.1 of the License, or (at your      */
/*  option) any later version.                                      */
/*                                                                  */
/*  The Seed7 Runtime Library is distributed in the hope that it    */
/*  will be useful, but WITHOUT ANY WARRANTY; without even the      */
/*  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR */
/*  PURPOSE.  See the GNU Lesser General Public License for more    */
/*  details.                                                        */
/*                                                                  */
/*  You should have received a copy of the GNU Lesser General       */
/*  Public License along with this program; if not, write to the    */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/config.h                                        */
/*  Changes: 2012  Thomas Mertes                                    */
/*  Content: Configuration settings.                                */
/*                                                                  */
/********************************************************************/

#define WITH_FLOAT
#define WITH_REFERENCE
#define WITH_DRAW
#undef  WITH_STATISTIC
#define WITH_COMPILATION_INFO
#define WITH_TYPE_CHECK
#undef  WITH_ACTION_CHECK
#define WITH_PROTOCOL
#define USE_ALTERNATE_NEXT_CHARACTER
#define USE_INFILE_BUFFER
#define USE_CHUNK_ALLOCS
#undef  USE_ALTERNATE_CHUNK_ALLOCS
#define USE_VARIABLE_FORMATS
#undef  USE_MAXIMUM_MALLOC_CHECK
#define DO_HEAPSIZE_COMPUTATION
#undef  DO_HEAP_STATISTIC
#undef  DO_HEAP_CHECK
#undef  DO_HEAP_LOG
#undef  CHECK_STACK
#define CHECKED_STACK_SIZE_LIMIT 0x800000
#define DIALOG_IN_SIGNAL_HANDLER
#define WITH_STRI_CAPACITY
#define ALLOW_STRITYPE_SLICES
#define ALLOW_BSTRITYPE_SLICES
#define WITH_STRI_FREELIST
#define WITH_BIGINT_FREELIST
#define WITH_OBJECT_FREELIST
#define WITH_LIST_FREELIST
#define WITH_NODE_FREELIST
#define WITH_FILE_FREELIST
#define STACK_LIKE_ALLOC_FOR_OS_STRI
#define INTTYPE_SIZE 64
#define CHECK_INT_OVERFLOW
#define BITSETTYPE_SIZE INTTYPE_SIZE
#define FLOATTYPE_DOUBLE
#define LOG_FUNCTIONS_EVERYWHERE 0
#define VERBOSE_EXCEPTIONS_EVERYWHERE 0

#if OS_PATH_HAS_DRIVE_LETTERS
#define MAP_ABSOLUTE_PATH_TO_DRIVE_LETTERS
#define FORBID_DRIVE_LETTERS
#define EMULATE_ROOT_CWD
#endif

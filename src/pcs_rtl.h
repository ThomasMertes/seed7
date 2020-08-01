/********************************************************************/
/*                                                                  */
/*  pcs_rtl.h     Platform idependent process handling functions.   */
/*  Copyright (C) 1989 - 2014  Thomas Mertes                        */
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
/*  File: seed7/src/pcs_rtl.h                                       */
/*  Changes: 2014  Thomas Mertes                                    */
/*  Content: Platform idependent process handling functions.        */
/*                                                                  */
/********************************************************************/

intType pcsCmpGeneric (const genericType value1, const genericType value2);
void pcsCpy (processType *const process_to, const processType process_from);
void pcsCpyGeneric (genericType *const dest, const genericType source);
processType pcsCreate (const processType process_from);
genericType pcsCreateGeneric (const genericType from_value);
void pcsDestr (const processType old_process);
void pcsDestrGeneric (const genericType old_value);

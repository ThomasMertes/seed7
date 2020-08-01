/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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
/*  Module: General                                                 */
/*  File: seed7/src/syvarutl.h                                      */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: Maintains the interpreter system variables.            */
/*                                                                  */
/********************************************************************/

#define SYS_EMPTY_OBJECT              prog.sys_var[ 0]
#define SYS_MEM_EXCEPTION             prog.sys_var[ 1]
#define SYS_NUM_EXCEPTION             prog.sys_var[ 2]
#define SYS_RNG_EXCEPTION             prog.sys_var[ 3]
#define SYS_IO_EXCEPTION              prog.sys_var[ 4]
#define SYS_ACT_ILLEGAL_EXCEPTION     prog.sys_var[ 5]
#define SYS_FALSE_OBJECT              prog.sys_var[ 6]
#define SYS_TRUE_OBJECT               prog.sys_var[ 7]
#define SYS_TYPE_TYPE                 prog.sys_var[ 8]
#define SYS_EXPR_TYPE                 prog.sys_var[ 9]
#define SYS_INT_TYPE                  prog.sys_var[10]
#define SYS_BIGINT_TYPE               prog.sys_var[11]
#define SYS_CHAR_TYPE                 prog.sys_var[12]
#define SYS_STRI_TYPE                 prog.sys_var[13]
#define SYS_PROC_TYPE                 prog.sys_var[14]
#define SYS_FLT_TYPE                  prog.sys_var[15]
#define SYS_ASSIGN_OBJECT             prog.sys_var[16]
#define SYS_CREA_OBJECT               prog.sys_var[17]
#define SYS_DESTR_OBJECT              prog.sys_var[18]
#define SYS_ORD_OBJECT                prog.sys_var[19]
#define SYS_IN_OBJECT                 prog.sys_var[20]
#define SYS_PROT_OUTFILE_OBJECT       prog.sys_var[21]
#define SYS_FLUSH_OBJECT              prog.sys_var[22]
#define SYS_WRITE_OBJECT              prog.sys_var[23]
#define SYS_WRLN_OBJECT               prog.sys_var[24]
#define SYS_MAIN_OBJECT               prog.sys_var[25]


#ifdef ANSI_C

int find_sysvar (stritype);
void init_sysvar (void);

#else

int find_sysvar ();
void init_sysvar ();

#endif

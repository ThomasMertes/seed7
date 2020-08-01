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
/*  Module: General                                                 */
/*  File: seed7/src/syvarutl.h                                      */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: Maintains the interpreter system variables.            */
/*                                                                  */
/********************************************************************/

#define EMPTY_OBJECT(prog)          (prog)->sys_var[ 0]
#define MEM_EXCEPTION(prog)         (prog)->sys_var[ 1]
#define NUM_EXCEPTION(prog)         (prog)->sys_var[ 2]
#define OVF_EXCEPTION(prog)         (prog)->sys_var[ 3]
#define RNG_EXCEPTION(prog)         (prog)->sys_var[ 4]
#define FIL_EXCEPTION(prog)         (prog)->sys_var[ 5]
#define ACT_ILLEGAL_EXCEPTION(prog) (prog)->sys_var[ 6]
#define FALSE_OBJECT(prog)          (prog)->sys_var[ 7]
#define TRUE_OBJECT(prog)           (prog)->sys_var[ 8]
#define TYPE_TYPE(prog)             (prog)->sys_var[ 9]
#define EXPR_TYPE(prog)             (prog)->sys_var[10]
#define INT_TYPE(prog)              (prog)->sys_var[11]
#define BIGINT_TYPE(prog)           (prog)->sys_var[12]
#define CHAR_TYPE(prog)             (prog)->sys_var[13]
#define STRI_TYPE(prog)             (prog)->sys_var[14]
#define PROC_TYPE(prog)             (prog)->sys_var[15]
#define FLT_TYPE(prog)              (prog)->sys_var[16]
#define ASSIGN_OBJECT(prog)         (prog)->sys_var[17]
#define CREA_OBJECT(prog)           (prog)->sys_var[18]
#define DESTR_OBJECT(prog)          (prog)->sys_var[19]
#define ORD_OBJECT(prog)            (prog)->sys_var[20]
#define IN_OBJECT(prog)             (prog)->sys_var[21]
#define PROT_OUTFILE_OBJECT(prog)   (prog)->sys_var[22]
#define FLUSH_OBJECT(prog)          (prog)->sys_var[23]
#define WRITE_OBJECT(prog)          (prog)->sys_var[24]
#define WRLN_OBJECT(prog)           (prog)->sys_var[25]
#define MAIN_OBJECT(prog)           (prog)->sys_var[26]

#define SYS_EMPTY_OBJECT          EMPTY_OBJECT(&prog)
#define SYS_MEM_EXCEPTION         MEM_EXCEPTION(&prog)
#define SYS_NUM_EXCEPTION         NUM_EXCEPTION(&prog)
#define SYS_OVF_EXCEPTION         OVF_EXCEPTION(&prog)
#define SYS_RNG_EXCEPTION         RNG_EXCEPTION(&prog)
#define SYS_FIL_EXCEPTION         FIL_EXCEPTION(&prog)
#define SYS_ACT_ILLEGAL_EXCEPTION ACT_ILLEGAL_EXCEPTION(&prog)
#define SYS_FALSE_OBJECT          FALSE_OBJECT(&prog)
#define SYS_TRUE_OBJECT           TRUE_OBJECT(&prog)
#define SYS_TYPE_TYPE             TYPE_TYPE(&prog)
#define SYS_EXPR_TYPE             EXPR_TYPE(&prog)
#define SYS_INT_TYPE              INT_TYPE(&prog)
#define SYS_BIGINT_TYPE           BIGINT_TYPE(&prog)
#define SYS_CHAR_TYPE             CHAR_TYPE(&prog)
#define SYS_STRI_TYPE             STRI_TYPE(&prog)
#define SYS_PROC_TYPE             PROC_TYPE(&prog)
#define SYS_FLT_TYPE              FLT_TYPE(&prog)
#define SYS_ASSIGN_OBJECT         ASSIGN_OBJECT(&prog)
#define SYS_CREA_OBJECT           CREA_OBJECT(&prog)
#define SYS_DESTR_OBJECT          DESTR_OBJECT(&prog)
#define SYS_ORD_OBJECT            ORD_OBJECT(&prog)
#define SYS_IN_OBJECT             IN_OBJECT(&prog)
#define SYS_PROT_OUTFILE_OBJECT   PROT_OUTFILE_OBJECT(&prog)
#define SYS_FLUSH_OBJECT          FLUSH_OBJECT(&prog)
#define SYS_WRITE_OBJECT          WRITE_OBJECT(&prog)
#define SYS_WRLN_OBJECT           WRLN_OBJECT(&prog)
#define SYS_MAIN_OBJECT           MAIN_OBJECT(&prog)


int find_sysvar (const_striType stri);
void init_sysvar (void);

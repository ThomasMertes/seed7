/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2021  Thomas Mertes                        */
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
/*  Module: Main                                                    */
/*  File: seed7/src/primitiv.c                                      */
/*  Changes: 1992, 1993, 1994, 2004 - 2021  Thomas Mertes           */
/*  Content: Table definitions for all primitive actions.           */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdio.h"

#include "common.h"
#include "data.h"
#include "actutl.h"
#include "actlib.h"
#include "arrlib.h"
#include "biglib.h"
#include "binlib.h"
#include "blnlib.h"
#include "bstlib.h"
#include "chrlib.h"
#include "cmdlib.h"
#include "conlib.h"
#include "dcllib.h"
#include "drwlib.h"
#include "enulib.h"
#include "fillib.h"
#include "fltlib.h"
#include "hshlib.h"
#include "intlib.h"
#include "itflib.h"
#include "kbdlib.h"
#include "pcslib.h"
#include "pollib.h"
#include "prclib.h"
#include "prglib.h"
#include "reflib.h"
#include "rfllib.h"
#include "sctlib.h"
#include "sellib.h"
#include "setlib.h"
#include "soclib.h"
#include "sqllib.h"
#include "strlib.h"
#include "timlib.h"
#include "typlib.h"
#include "ut8lib.h"

#undef EXTERN
#define EXTERN
#include "primitiv.h"


static const objectCategory p_no_args[]              = {SYMBOLOBJECT};
static const objectCategory p_ace[]                  = {ACTENTRYOBJECT};
static const objectCategory p_ace_ace[]              = {ACTENTRYOBJECT, ACTENTRYOBJECT};
static const objectCategory p_ace_op_ace[]           = {ACTENTRYOBJECT, SYMBOLOBJECT, ACTENTRYOBJECT};
static const objectCategory p_act_op_ace[]           = {ACTOBJECT, SYMBOLOBJECT, ACTENTRYOBJECT};
static const objectCategory p_any[]                  = {ILLEGALOBJECT};
static const objectCategory p_op_any[]               = {SYMBOLOBJECT, ILLEGALOBJECT};
static const objectCategory p_op_any_op_any[]        = {SYMBOLOBJECT, ILLEGALOBJECT, SYMBOLOBJECT, ILLEGALOBJECT};
static const objectCategory p_arr[]                  = {ARRAYOBJECT};
static const objectCategory p_arr_int[]              = {ARRAYOBJECT, INTOBJECT};
static const objectCategory p_arr_int_arr[]          = {ARRAYOBJECT, INTOBJECT, ARRAYOBJECT};
static const objectCategory p_arr_int_int[]          = {ARRAYOBJECT, INTOBJECT, INTOBJECT};
static const objectCategory p_arr_int_any[]          = {ARRAYOBJECT, INTOBJECT, ILLEGALOBJECT};
static const objectCategory p_arr_op_arr[]           = {ARRAYOBJECT, SYMBOLOBJECT, ARRAYOBJECT};
static const objectCategory p_arr_op_int[]           = {ARRAYOBJECT, SYMBOLOBJECT, INTOBJECT};
static const objectCategory p_arr_op_int_op_int[]    = {ARRAYOBJECT, SYMBOLOBJECT, INTOBJECT, SYMBOLOBJECT, INTOBJECT};
static const objectCategory p_arr_op_any[]           = {ARRAYOBJECT, SYMBOLOBJECT, ILLEGALOBJECT};
static const objectCategory p_arr_op_op_int[]        = {ARRAYOBJECT, SYMBOLOBJECT, SYMBOLOBJECT, INTOBJECT};
static const objectCategory p_op_arr[]               = {SYMBOLOBJECT, ARRAYOBJECT};
static const objectCategory p_op_op_arr[]            = {SYMBOLOBJECT, SYMBOLOBJECT, ARRAYOBJECT};
static const objectCategory p_op_int_op_arr[]        = {SYMBOLOBJECT, INTOBJECT, SYMBOLOBJECT, ARRAYOBJECT};
static const objectCategory p_any_op_any[]           = {ILLEGALOBJECT, SYMBOLOBJECT, ILLEGALOBJECT};
static const objectCategory p_op_op_any[]            = {SYMBOLOBJECT, SYMBOLOBJECT, ILLEGALOBJECT};
static const objectCategory p_op_int_op_any[]        = {SYMBOLOBJECT, INTOBJECT, SYMBOLOBJECT, ILLEGALOBJECT};
static const objectCategory p_big[]                  = {BIGINTOBJECT};
static const objectCategory p_big_big[]              = {BIGINTOBJECT, BIGINTOBJECT};
static const objectCategory p_big_bln[]              = {BIGINTOBJECT, BOOLOBJECT};
static const objectCategory p_big_op_big[]           = {BIGINTOBJECT, SYMBOLOBJECT, BIGINTOBJECT};
static const objectCategory p_big_op_int[]           = {BIGINTOBJECT, SYMBOLOBJECT, INTOBJECT};
static const objectCategory p_op_big[]               = {SYMBOLOBJECT, BIGINTOBJECT};
static const objectCategory p_op_op_big[]            = {SYMBOLOBJECT, SYMBOLOBJECT, BIGINTOBJECT};
static const objectCategory p_bln[]                  = {BOOLOBJECT};
static const objectCategory p_bln_typ_int[]          = {BOOLOBJECT, TYPEOBJECT, INTOBJECT};
static const objectCategory p_bln_typ_str[]          = {BOOLOBJECT, TYPEOBJECT, STRIOBJECT};
static const objectCategory p_bln_op_bln[]           = {BOOLOBJECT, SYMBOLOBJECT, BOOLOBJECT};
static const objectCategory p_bln_op_any_op_any[]    = {BOOLOBJECT, SYMBOLOBJECT, ILLEGALOBJECT, SYMBOLOBJECT, ILLEGALOBJECT};
static const objectCategory p_op_bln[]               = {SYMBOLOBJECT, BOOLOBJECT};
static const objectCategory p_op_bln_op_prc[]        = {SYMBOLOBJECT, BOOLOBJECT, SYMBOLOBJECT, BLOCKOBJECT};
static const objectCategory p_op_bln_op_prc_prc[]    = {SYMBOLOBJECT, BOOLOBJECT, SYMBOLOBJECT, BLOCKOBJECT, BLOCKOBJECT};
static const objectCategory p_op_op_bln[]            = {SYMBOLOBJECT, SYMBOLOBJECT, BOOLOBJECT};
static const objectCategory p_blo[]                  = {BLOCKOBJECT};
static const objectCategory p_bst[]                  = {BSTRIOBJECT};
static const objectCategory p_bst_bst[]              = {BSTRIOBJECT, BSTRIOBJECT};
static const objectCategory p_bst_bln[]              = {BSTRIOBJECT, BOOLOBJECT};
static const objectCategory p_bst_4_int[]            = {BSTRIOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT};
static const objectCategory p_bst_set_arr_fil[]      = {BSTRIOBJECT, SETOBJECT, ARRAYOBJECT, FILEOBJECT};
static const objectCategory p_bst_op_bst[]           = {BSTRIOBJECT, SYMBOLOBJECT, BSTRIOBJECT};
static const objectCategory p_bst_op_int[]           = {BSTRIOBJECT, SYMBOLOBJECT, INTOBJECT};
static const objectCategory p_chr[]                  = {CHAROBJECT};
static const objectCategory p_chr_chr[]              = {CHAROBJECT, CHAROBJECT};
static const objectCategory p_chr_op_chr[]           = {CHAROBJECT, SYMBOLOBJECT, CHAROBJECT};
static const objectCategory p_op_chr[]               = {SYMBOLOBJECT, CHAROBJECT};
static const objectCategory p_dbs[]                  = {DATABASEOBJECT};
static const objectCategory p_dbs_dbs[]              = {DATABASEOBJECT, DATABASEOBJECT};
static const objectCategory p_dbs_bln[]              = {DATABASEOBJECT, BOOLOBJECT};
static const objectCategory p_dbs_str[]              = {DATABASEOBJECT, STRIOBJECT};
static const objectCategory p_dbs_op_dbs[]           = {DATABASEOBJECT, SYMBOLOBJECT, DATABASEOBJECT};
static const objectCategory p_enu[]                  = {ENUMOBJECT};
static const objectCategory p_enu_rfl[]              = {ENUMOBJECT, REFLISTOBJECT};
static const objectCategory p_enu_op_enu[]           = {ENUMOBJECT, SYMBOLOBJECT, ENUMOBJECT};
static const objectCategory p_op_enu[]               = {SYMBOLOBJECT, ENUMOBJECT};
static const objectCategory p_op_op_enu[]            = {SYMBOLOBJECT, SYMBOLOBJECT, ENUMOBJECT};
static const objectCategory p_exp[]                  = {EXPROBJECT};
static const objectCategory p_fil[]                  = {FILEOBJECT};
static const objectCategory p_fil_big[]              = {FILEOBJECT, BIGINTOBJECT};
static const objectCategory p_fil_chr[]              = {FILEOBJECT, CHAROBJECT};
static const objectCategory p_fil_fil[]              = {FILEOBJECT, FILEOBJECT};
static const objectCategory p_fil_int[]              = {FILEOBJECT, INTOBJECT};
static const objectCategory p_fil_str[]              = {FILEOBJECT, STRIOBJECT};
static const objectCategory p_fil_op_fil[]           = {FILEOBJECT, SYMBOLOBJECT, FILEOBJECT};
static const objectCategory p_flt[]                  = {FLOATOBJECT};
static const objectCategory p_flt_flt[]              = {FLOATOBJECT, FLOATOBJECT};
static const objectCategory p_flt_op_flt[]           = {FLOATOBJECT, SYMBOLOBJECT, FLOATOBJECT};
static const objectCategory p_flt_op_int[]           = {FLOATOBJECT, SYMBOLOBJECT, INTOBJECT};
static const objectCategory p_flt_flt_int[]          = {FLOATOBJECT, FLOATOBJECT, INTOBJECT};
static const objectCategory p_op_flt[]               = {SYMBOLOBJECT, FLOATOBJECT};
static const objectCategory p_hel_4_ref[]            = {HASHELEMOBJECT,  REFOBJECT,  REFOBJECT,  REFOBJECT,  REFOBJECT};
static const objectCategory p_hel_op_hel[]           = {HASHELEMOBJECT, SYMBOLOBJECT, HASHELEMOBJECT};
static const objectCategory p_hsh[]                  = {HASHOBJECT};
static const objectCategory p_hsh_hsh_4_ref[]        = {HASHOBJECT, HASHOBJECT, REFOBJECT, REFOBJECT, REFOBJECT, REFOBJECT};
static const objectCategory p_hsh_ref_ref[]          = {HASHOBJECT, REFOBJECT, REFOBJECT};
static const objectCategory p_hsh_any_int_ref[]      = {HASHOBJECT, ILLEGALOBJECT, INTOBJECT, REFOBJECT};
static const objectCategory p_hsh_op_int_ref[]       = {HASHOBJECT, SYMBOLOBJECT, INTOBJECT, REFOBJECT};
static const objectCategory p_any_hsh_prc_ref[]      = {ILLEGALOBJECT, HASHOBJECT, BLOCKOBJECT, REFOBJECT};
static const objectCategory p_int[]                  = {INTOBJECT};
static const objectCategory p_int_int[]              = {INTOBJECT, INTOBJECT};
static const objectCategory p_int_int_int[]          = {INTOBJECT, INTOBJECT, INTOBJECT};
static const objectCategory p_int_int_int_int[]      = {INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT};
static const objectCategory p_4_int[]                = {INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT};
static const objectCategory p_4_int_str[]            = {INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, STRIOBJECT};
static const objectCategory p_7_int_bln[]            = {INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, BOOLOBJECT};
static const objectCategory p_8_int[]                = {INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT};
static const objectCategory p_8_int_bln[]            = {INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, BOOLOBJECT};
static const objectCategory p_9_int_bln[]            = {INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, BOOLOBJECT};
static const objectCategory p_int_ref[]              = {INTOBJECT, REFOBJECT};
static const objectCategory p_int_rfl[]              = {INTOBJECT, REFLISTOBJECT};
static const objectCategory p_int_str_int[]          = {INTOBJECT, STRIOBJECT, INTOBJECT};
static const objectCategory p_int_op_int[]           = {INTOBJECT, SYMBOLOBJECT, INTOBJECT};
static const objectCategory p_int_op_set[]           = {INTOBJECT, SYMBOLOBJECT, SETOBJECT};
static const objectCategory p_int_op_any[]           = {INTOBJECT, SYMBOLOBJECT, ILLEGALOBJECT};
static const objectCategory p_int_op_op_int[]        = {INTOBJECT, SYMBOLOBJECT, SYMBOLOBJECT, INTOBJECT};
static const objectCategory p_int_op_op_set[]        = {INTOBJECT, SYMBOLOBJECT, SYMBOLOBJECT, SETOBJECT};
static const objectCategory p_op_int[]               = {SYMBOLOBJECT, INTOBJECT};
static const objectCategory p_op_int_int[]           = {SYMBOLOBJECT, INTOBJECT, INTOBJECT};
static const objectCategory p_op_4_int[]             = {SYMBOLOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT};
static const objectCategory p_op_5_int[]             = {SYMBOLOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT};
static const objectCategory p_op_int_op_int[]        = {SYMBOLOBJECT, INTOBJECT, SYMBOLOBJECT, INTOBJECT};
static const objectCategory p_op_op_int[]            = {SYMBOLOBJECT, SYMBOLOBJECT, INTOBJECT};
static const objectCategory p_itf[]                  = {INTERFACEOBJECT};
static const objectCategory p_itf_itf[]              = {INTERFACEOBJECT, INTERFACEOBJECT};
static const objectCategory p_itf_op_itf[]           = {INTERFACEOBJECT, SYMBOLOBJECT, INTERFACEOBJECT};
static const objectCategory p_itf_op_sct[]           = {INTERFACEOBJECT, SYMBOLOBJECT, STRUCTOBJECT};
static const objectCategory p_pcs[]                  = {PROCESSOBJECT};
static const objectCategory p_pcs_pcs[]              = {PROCESSOBJECT, PROCESSOBJECT};
static const objectCategory p_pcs_op_pcs[]           = {PROCESSOBJECT, SYMBOLOBJECT, PROCESSOBJECT};
static const objectCategory p_plt[]                  = {POINTLISTOBJECT};
static const objectCategory p_plt_plt[]              = {POINTLISTOBJECT, POINTLISTOBJECT};
static const objectCategory p_plt_op_plt[]           = {POINTLISTOBJECT, SYMBOLOBJECT, POINTLISTOBJECT};
static const objectCategory p_pol[]                  = {POLLOBJECT};
static const objectCategory p_pol_int[]              = {POLLOBJECT, INTOBJECT};
static const objectCategory p_pol_itf[]              = {POLLOBJECT, INTERFACEOBJECT};
static const objectCategory p_pol_soc[]              = {POLLOBJECT, SOCKETOBJECT};
static const objectCategory p_pol_soc_int[]          = {POLLOBJECT, SOCKETOBJECT, INTOBJECT};
static const objectCategory p_pol_soc_int_itf[]      = {POLLOBJECT, SOCKETOBJECT, INTOBJECT, INTERFACEOBJECT};
static const objectCategory p_pol_op_pol[]           = {POLLOBJECT, SYMBOLOBJECT, POLLOBJECT};
static const objectCategory p_prc[]                  = {BLOCKOBJECT};
static const objectCategory p_prc_op_prc[]           = {BLOCKOBJECT, SYMBOLOBJECT, BLOCKOBJECT};
static const objectCategory p_op_prc[]               = {SYMBOLOBJECT, BLOCKOBJECT};
static const objectCategory p_op_prc_op_prc[]        = {SYMBOLOBJECT, BLOCKOBJECT, SYMBOLOBJECT, BLOCKOBJECT};
static const objectCategory p_op_prc_op_bln[]        = {SYMBOLOBJECT, BLOCKOBJECT, SYMBOLOBJECT, BOOLOBJECT};
static const objectCategory p_op_prc_op_op_op_prc[]  = {SYMBOLOBJECT, BLOCKOBJECT, SYMBOLOBJECT, SYMBOLOBJECT, SYMBOLOBJECT, BLOCKOBJECT};
static const objectCategory p_op_any_op_prc[]        = {SYMBOLOBJECT, ILLEGALOBJECT, SYMBOLOBJECT, BLOCKOBJECT};
static const objectCategory p_op_op_exp[]            = {SYMBOLOBJECT, SYMBOLOBJECT, EXPROBJECT};
static const objectCategory p_op_op_prc_op_exp[]     = {SYMBOLOBJECT, SYMBOLOBJECT, BLOCKOBJECT, SYMBOLOBJECT, EXPROBJECT};
static const objectCategory p_prg[]                  = {PROGOBJECT};
static const objectCategory p_prg_prg[]              = {PROGOBJECT, PROGOBJECT};
static const objectCategory p_prg_arr_set_str[]      = {PROGOBJECT, ARRAYOBJECT, SETOBJECT, STRIOBJECT};
static const objectCategory p_prg_ref[]              = {PROGOBJECT, REFOBJECT};
static const objectCategory p_prg_ref_rfl[]          = {PROGOBJECT, REFOBJECT, REFLISTOBJECT};
static const objectCategory p_prg_rfl[]              = {PROGOBJECT, REFLISTOBJECT};
static const objectCategory p_prg_str[]              = {PROGOBJECT, STRIOBJECT};
static const objectCategory p_prg_op_prg[]           = {PROGOBJECT, SYMBOLOBJECT, PROGOBJECT};
static const objectCategory p_op_prm[]               = {SYMBOLOBJECT, FORMPARAMOBJECT};
static const objectCategory p_ref[]                  = {REFOBJECT};
static const objectCategory p_ref_ref[]              = {REFOBJECT, REFOBJECT};
static const objectCategory p_ref_bln[]              = {REFOBJECT, BOOLOBJECT};
static const objectCategory p_ref_int[]              = {REFOBJECT, INTOBJECT};
static const objectCategory p_ref_rfl[]              = {REFOBJECT, REFLISTOBJECT};
static const objectCategory p_ref_typ[]              = {REFOBJECT, TYPEOBJECT};
static const objectCategory p_ref_op_ref[]           = {REFOBJECT, SYMBOLOBJECT, REFOBJECT};
static const objectCategory p_ref_op_rfl[]           = {REFOBJECT, SYMBOLOBJECT, REFLISTOBJECT};
static const objectCategory p_ref_op_op_rfl[]        = {REFOBJECT, SYMBOLOBJECT, SYMBOLOBJECT, REFLISTOBJECT};
static const objectCategory p_op_ref[]               = {SYMBOLOBJECT, REFOBJECT};
static const objectCategory p_op_ref_op_rfl_op_prc[] = {SYMBOLOBJECT, REFOBJECT, SYMBOLOBJECT, REFLISTOBJECT, SYMBOLOBJECT, BLOCKOBJECT};
static const objectCategory p_rfl[]                  = {REFLISTOBJECT};
static const objectCategory p_rfl_ref[]              = {REFLISTOBJECT, REFOBJECT};
static const objectCategory p_rfl_op_rfl[]           = {REFLISTOBJECT, SYMBOLOBJECT, REFLISTOBJECT};
static const objectCategory p_rfl_op_int[]           = {REFLISTOBJECT, SYMBOLOBJECT, INTOBJECT};
static const objectCategory p_rfl_op_int_op_int[]    = {REFLISTOBJECT, SYMBOLOBJECT, INTOBJECT, SYMBOLOBJECT, INTOBJECT};
static const objectCategory p_rfl_op_op_int[]        = {REFLISTOBJECT, SYMBOLOBJECT, SYMBOLOBJECT, INTOBJECT};
static const objectCategory p_rfl_op_op_int_op_ref[] = {REFLISTOBJECT, SYMBOLOBJECT, SYMBOLOBJECT, INTOBJECT, SYMBOLOBJECT, REFOBJECT};
static const objectCategory p_sct[]                  = {STRUCTOBJECT};
static const objectCategory p_sct_ref[]              = {STRUCTOBJECT, REFOBJECT};
static const objectCategory p_sct_op_sct[]           = {STRUCTOBJECT, SYMBOLOBJECT, STRUCTOBJECT};
static const objectCategory p_sct_op_op[]            = {STRUCTOBJECT, SYMBOLOBJECT, SYMBOLOBJECT};
static const objectCategory p_op_op_sct[]            = {SYMBOLOBJECT, SYMBOLOBJECT, STRUCTOBJECT};
static const objectCategory p_sel[]                  = {STRUCTELEMOBJECT};
static const objectCategory p_sel_sel[]              = {STRUCTELEMOBJECT, STRUCTELEMOBJECT};
static const objectCategory p_sel_op_sel[]           = {STRUCTELEMOBJECT, SYMBOLOBJECT, STRUCTELEMOBJECT};
static const objectCategory p_op_sel[]               = {SYMBOLOBJECT, STRUCTELEMOBJECT};
static const objectCategory p_set[]                  = {SETOBJECT};
static const objectCategory p_set_set[]              = {SETOBJECT, SETOBJECT};
static const objectCategory p_set_int[]              = {SETOBJECT, INTOBJECT};
static const objectCategory p_set_op_set[]           = {SETOBJECT, SYMBOLOBJECT, SETOBJECT};
static const objectCategory p_op_op_set[]            = {SYMBOLOBJECT, SYMBOLOBJECT, SETOBJECT};
static const objectCategory p_soc[]                  = {SOCKETOBJECT};
static const objectCategory p_soc_bst[]              = {SOCKETOBJECT, BSTRIOBJECT};
static const objectCategory p_soc_chr[]              = {SOCKETOBJECT, CHAROBJECT};
static const objectCategory p_soc_int[]              = {SOCKETOBJECT, INTOBJECT};
static const objectCategory p_soc_int_bln[]          = {SOCKETOBJECT, INTOBJECT, BOOLOBJECT};
static const objectCategory p_soc_int_chr[]          = {SOCKETOBJECT, INTOBJECT, CHAROBJECT};
static const objectCategory p_soc_int_int[]          = {SOCKETOBJECT, INTOBJECT, INTOBJECT};
static const objectCategory p_soc_str[]              = {SOCKETOBJECT, STRIOBJECT};
static const objectCategory p_soc_str_int[]          = {SOCKETOBJECT, STRIOBJECT, INTOBJECT};
static const objectCategory p_soc_str_int_bst[]      = {SOCKETOBJECT, STRIOBJECT, INTOBJECT, BSTRIOBJECT};
static const objectCategory p_soc_str_int_int[]      = {SOCKETOBJECT, STRIOBJECT, INTOBJECT, INTOBJECT};
static const objectCategory p_soc_str_int_int_bst[]  = {SOCKETOBJECT, STRIOBJECT, INTOBJECT, INTOBJECT, BSTRIOBJECT};
static const objectCategory p_soc_op_soc[]           = {SOCKETOBJECT, SYMBOLOBJECT, SOCKETOBJECT};
static const objectCategory p_sqs[]                  = {SQLSTMTOBJECT};
static const objectCategory p_sqs_sqs[]              = {SQLSTMTOBJECT, SQLSTMTOBJECT};
static const objectCategory p_sqs_int[]              = {SQLSTMTOBJECT, INTOBJECT};
static const objectCategory p_sqs_int_big[]          = {SQLSTMTOBJECT, INTOBJECT, BIGINTOBJECT};
static const objectCategory p_sqs_int_big_big[]      = {SQLSTMTOBJECT, INTOBJECT, BIGINTOBJECT, BIGINTOBJECT};
static const objectCategory p_sqs_int_bln[]          = {SQLSTMTOBJECT, INTOBJECT, BOOLOBJECT};
static const objectCategory p_sqs_int_bst[]          = {SQLSTMTOBJECT, INTOBJECT, BSTRIOBJECT};
static const objectCategory p_sqs_8_int[]            = {SQLSTMTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT};
static const objectCategory p_sqs_9_int[]            = {SQLSTMTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT};
static const objectCategory p_sqs_9_int_bln[]        = {SQLSTMTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, BOOLOBJECT};
static const objectCategory p_sqs_int_flt[]          = {SQLSTMTOBJECT, INTOBJECT, FLOATOBJECT};
static const objectCategory p_sqs_int_int[]          = {SQLSTMTOBJECT, INTOBJECT, INTOBJECT};
static const objectCategory p_sqs_int_str[]          = {SQLSTMTOBJECT, INTOBJECT, STRIOBJECT};
static const objectCategory p_sqs_op_sqs[]           = {SQLSTMTOBJECT, SYMBOLOBJECT, SQLSTMTOBJECT};
static const objectCategory p_str[]                  = {STRIOBJECT};
static const objectCategory p_str_str[]              = {STRIOBJECT, STRIOBJECT};
static const objectCategory p_str_arr[]              = {STRIOBJECT, ARRAYOBJECT};
static const objectCategory p_str_arr_fil_fil[]      = {STRIOBJECT, ARRAYOBJECT, FILEOBJECT, FILEOBJECT};
static const objectCategory p_str_arr_fil_fil_fil[]  = {STRIOBJECT, ARRAYOBJECT, FILEOBJECT, FILEOBJECT, FILEOBJECT};
static const objectCategory p_str_arr_str[]          = {STRIOBJECT, ARRAYOBJECT, STRIOBJECT};
static const objectCategory p_str_arr_str_str_str[]  = {STRIOBJECT, ARRAYOBJECT, STRIOBJECT, STRIOBJECT, STRIOBJECT};
static const objectCategory p_str_chr[]              = {STRIOBJECT, CHAROBJECT};
static const objectCategory p_str_chr_int[]          = {STRIOBJECT, CHAROBJECT, INTOBJECT};
static const objectCategory p_str_int[]              = {STRIOBJECT, INTOBJECT};
static const objectCategory p_str_8_int[]            = {STRIOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT};
static const objectCategory p_str_8_int_bln[]        = {STRIOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, BOOLOBJECT};
static const objectCategory p_str_set[]              = {STRIOBJECT, SETOBJECT};
static const objectCategory p_str_set_arr_fil[]      = {STRIOBJECT, SETOBJECT, ARRAYOBJECT, FILEOBJECT};
static const objectCategory p_str_str_str[]          = {STRIOBJECT, STRIOBJECT, STRIOBJECT};
static const objectCategory p_str_str_int[]          = {STRIOBJECT, STRIOBJECT, INTOBJECT};
static const objectCategory p_str_op_str[]           = {STRIOBJECT, SYMBOLOBJECT, STRIOBJECT};
static const objectCategory p_str_op_chr[]           = {STRIOBJECT, SYMBOLOBJECT, CHAROBJECT};
static const objectCategory p_str_op_int[]           = {STRIOBJECT, SYMBOLOBJECT, INTOBJECT};
static const objectCategory p_str_op_int_op_int[]    = {STRIOBJECT, SYMBOLOBJECT, INTOBJECT, SYMBOLOBJECT, INTOBJECT};
static const objectCategory p_str_op_op_int[]        = {STRIOBJECT, SYMBOLOBJECT, SYMBOLOBJECT, INTOBJECT};
static const objectCategory p_typ[]                  = {TYPEOBJECT};
static const objectCategory p_typ_typ[]              = {TYPEOBJECT, TYPEOBJECT};
static const objectCategory p_typ_op_any[]           = {TYPEOBJECT, SYMBOLOBJECT, ILLEGALOBJECT};
static const objectCategory p_typ_op_typ[]           = {TYPEOBJECT, SYMBOLOBJECT, TYPEOBJECT};
static const objectCategory p_typ_op_op_any[]        = {TYPEOBJECT, SYMBOLOBJECT, SYMBOLOBJECT, ILLEGALOBJECT};
static const objectCategory p_str_op_op_int_op_str[] = {STRIOBJECT, SYMBOLOBJECT, SYMBOLOBJECT, INTOBJECT, SYMBOLOBJECT, STRIOBJECT};
static const objectCategory p_str_op_op_int_op_chr[] = {STRIOBJECT, SYMBOLOBJECT, SYMBOLOBJECT, INTOBJECT, SYMBOLOBJECT, CHAROBJECT};
static const objectCategory p_op_str[]               = {SYMBOLOBJECT, STRIOBJECT};
static const objectCategory p_op_str_int_3_str[]     = {SYMBOLOBJECT, STRIOBJECT, INTOBJECT, STRIOBJECT, STRIOBJECT, STRIOBJECT};
static const objectCategory p_op_str_int_4_str[]     = {SYMBOLOBJECT, STRIOBJECT, INTOBJECT, STRIOBJECT, STRIOBJECT, STRIOBJECT, STRIOBJECT};
static const objectCategory p_op_5_str[]             = {SYMBOLOBJECT, STRIOBJECT, STRIOBJECT, STRIOBJECT, STRIOBJECT, STRIOBJECT};
static const objectCategory p_op_chr_op_str_op_prc[] = {SYMBOLOBJECT, CHAROBJECT, SYMBOLOBJECT, STRIOBJECT, SYMBOLOBJECT, BLOCKOBJECT};
static const objectCategory p_op_typ[]               = {SYMBOLOBJECT, TYPEOBJECT};
static const objectCategory p_op_typ_op[]            = {SYMBOLOBJECT, TYPEOBJECT, SYMBOLOBJECT};
static const objectCategory p_op_typ_op_exp[]        = {SYMBOLOBJECT, TYPEOBJECT, SYMBOLOBJECT, EXPROBJECT};
static const objectCategory p_op_typ_op_exp_op_exp[] = {SYMBOLOBJECT, TYPEOBJECT, SYMBOLOBJECT, EXPROBJECT, SYMBOLOBJECT, EXPROBJECT};
static const objectCategory p_op_typ_op_exp_op_op[]  = {SYMBOLOBJECT, TYPEOBJECT, SYMBOLOBJECT, EXPROBJECT, SYMBOLOBJECT, SYMBOLOBJECT};
static const objectCategory p_op_op_typ_op[]         = {SYMBOLOBJECT, SYMBOLOBJECT, TYPEOBJECT, SYMBOLOBJECT};
static const objectCategory p_op_op_typ_op_exp[]     = {SYMBOLOBJECT, SYMBOLOBJECT, TYPEOBJECT, SYMBOLOBJECT, EXPROBJECT};
static const objectCategory p_voi_op_voi[]           = {VOIDOBJECT, SYMBOLOBJECT, VOIDOBJECT};
static const objectCategory p_win[]                  = {WINOBJECT};
static const objectCategory p_win_win[]              = {WINOBJECT, WINOBJECT};
static const objectCategory p_win_win_6_int[]        = {WINOBJECT, WINOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT};
static const objectCategory p_win_bln[]              = {WINOBJECT, BOOLOBJECT};
static const objectCategory p_win_chr_bln[]          = {WINOBJECT, CHAROBJECT, BOOLOBJECT};
static const objectCategory p_win_int[]              = {WINOBJECT, INTOBJECT};
static const objectCategory p_win_int_int[]          = {WINOBJECT, INTOBJECT, INTOBJECT};
static const objectCategory p_win_int_int_int[]      = {WINOBJECT, INTOBJECT, INTOBJECT, INTOBJECT};
static const objectCategory p_win_int_int_win[]      = {WINOBJECT, INTOBJECT, INTOBJECT, WINOBJECT};
static const objectCategory p_win_int_int_plt_int[]  = {WINOBJECT, INTOBJECT, INTOBJECT, POINTLISTOBJECT, INTOBJECT};
static const objectCategory p_win_4_int[]            = {WINOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT};
static const objectCategory p_win_4_int_win[]        = {WINOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, WINOBJECT};
static const objectCategory p_win_5_int[]            = {WINOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, INTOBJECT};
static const objectCategory p_win_str[]              = {WINOBJECT, STRIOBJECT};
static const objectCategory p_win_op_win[]           = {WINOBJECT, SYMBOLOBJECT, WINOBJECT};

static const objectCategory p_win_int_int_str_int_int[] = {WINOBJECT, INTOBJECT, INTOBJECT, STRIOBJECT, INTOBJECT, INTOBJECT};
static const objectCategory p_hsh_any_int_ref_ref_ref[] = {HASHOBJECT, ILLEGALOBJECT, INTOBJECT, REFOBJECT, REFOBJECT, REFOBJECT};
static const objectCategory p_hsh_any_int_any_ref_ref[] = {HASHOBJECT, ILLEGALOBJECT, INTOBJECT, ILLEGALOBJECT, REFOBJECT, REFOBJECT};
static const objectCategory p_any_any_hsh_prc_ref_ref[] = {ILLEGALOBJECT, ILLEGALOBJECT, HASHOBJECT, BLOCKOBJECT, REFOBJECT, REFOBJECT};
static const objectCategory p_hsh_any_any_int_3_ref[] = {HASHOBJECT, ILLEGALOBJECT, ILLEGALOBJECT, INTOBJECT, REFOBJECT, REFOBJECT, REFOBJECT};
static const objectCategory p_op_prc_op_prc_op_op_prc[] = {SYMBOLOBJECT, BLOCKOBJECT, SYMBOLOBJECT, BLOCKOBJECT, SYMBOLOBJECT, SYMBOLOBJECT, BLOCKOBJECT};
static const objectCategory p_op_any_op_prc_op_op_prc[] = {SYMBOLOBJECT, ILLEGALOBJECT, SYMBOLOBJECT, BLOCKOBJECT, SYMBOLOBJECT, SYMBOLOBJECT, BLOCKOBJECT};
static const objectCategory p_op_op_int_op_str_op_prc[] = {SYMBOLOBJECT, SYMBOLOBJECT, INTOBJECT, SYMBOLOBJECT, STRIOBJECT, SYMBOLOBJECT, BLOCKOBJECT};
static const objectCategory p_win_3_int_flt_flt_int[] = {WINOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, FLOATOBJECT, FLOATOBJECT, INTOBJECT};
static const objectCategory p_hsh_any_any_int_4_ref[] = {HASHOBJECT, ILLEGALOBJECT, ILLEGALOBJECT, INTOBJECT, REFOBJECT, REFOBJECT, REFOBJECT, REFOBJECT};
static const objectCategory p_op_chr_op_int_op_str_op_prc[] = {SYMBOLOBJECT, CHAROBJECT, SYMBOLOBJECT, INTOBJECT, SYMBOLOBJECT, STRIOBJECT, SYMBOLOBJECT, BLOCKOBJECT};
static const objectCategory p_op_ref_op_int_op_rfl_op_prc[] = {SYMBOLOBJECT, REFOBJECT, SYMBOLOBJECT, INTOBJECT, SYMBOLOBJECT, REFLISTOBJECT, SYMBOLOBJECT, BLOCKOBJECT};
static const objectCategory p_op_ref_op_rfl_op_bln_op_prc[] = {SYMBOLOBJECT, REFOBJECT, SYMBOLOBJECT, REFLISTOBJECT, SYMBOLOBJECT, BOOLOBJECT, SYMBOLOBJECT, BLOCKOBJECT};
static const objectCategory p_prg_int_int_str_int_int_str_str[] = {PROGOBJECT, INTOBJECT, INTOBJECT, STRIOBJECT, INTOBJECT, INTOBJECT, STRIOBJECT, STRIOBJECT};
static const objectCategory p_op_int_op_int_op_int_op_prc[] = {SYMBOLOBJECT, INTOBJECT, SYMBOLOBJECT, INTOBJECT, SYMBOLOBJECT, INTOBJECT, SYMBOLOBJECT, BLOCKOBJECT};
static const objectCategory p_win_3_int_flt_flt_int_int[] = {WINOBJECT, INTOBJECT, INTOBJECT, INTOBJECT, FLOATOBJECT, FLOATOBJECT, INTOBJECT, INTOBJECT};
static const objectCategory p_op_int_op_int_op_int_op_int_op_prc[] = {SYMBOLOBJECT, INTOBJECT, SYMBOLOBJECT, INTOBJECT, SYMBOLOBJECT, INTOBJECT, SYMBOLOBJECT, INTOBJECT, SYMBOLOBJECT, BLOCKOBJECT};
static const objectCategory p_op_op_op_typ_op_exp_op_any_op_exp[] = {SYMBOLOBJECT, SYMBOLOBJECT, SYMBOLOBJECT, TYPEOBJECT, SYMBOLOBJECT, EXPROBJECT, SYMBOLOBJECT, ILLEGALOBJECT, SYMBOLOBJECT, EXPROBJECT};
static const objectCategory p_op_op_op_typ_op_exp_op_any_op_prc_op_exp[] = {SYMBOLOBJECT, SYMBOLOBJECT, SYMBOLOBJECT, TYPEOBJECT, SYMBOLOBJECT, EXPROBJECT, SYMBOLOBJECT, ILLEGALOBJECT, SYMBOLOBJECT, BLOCKOBJECT, SYMBOLOBJECT, EXPROBJECT};


#define argCountAndArgs(categoryList) sizeof(categoryList) / sizeof(objectCategory), categoryList

#define par_no_args              0, NULL
#define par_no_check             argCountAndArgs(p_no_args)
#define par_ace                  argCountAndArgs(p_ace)
#define par_ace_ace              argCountAndArgs(p_ace_ace)
#define par_ace_op_ace           argCountAndArgs(p_ace_op_ace)
#define par_act_op_ace           argCountAndArgs(p_act_op_ace)
#define par_any                  argCountAndArgs(p_any)
#define par_op_any               argCountAndArgs(p_op_any)
#define par_op_any_op_any        argCountAndArgs(p_op_any_op_any)
#define par_arr                  argCountAndArgs(p_arr)
#define par_arr_int              argCountAndArgs(p_arr_int)
#define par_arr_int_arr          argCountAndArgs(p_arr_int_arr)
#define par_arr_int_int          argCountAndArgs(p_arr_int_int)
#define par_arr_int_any          argCountAndArgs(p_arr_int_any)
#define par_arr_op_arr           argCountAndArgs(p_arr_op_arr)
#define par_arr_op_int           argCountAndArgs(p_arr_op_int)
#define par_arr_op_int_op_int    argCountAndArgs(p_arr_op_int_op_int)
#define par_arr_op_any           argCountAndArgs(p_arr_op_any)
#define par_arr_op_op_int        argCountAndArgs(p_arr_op_op_int)
#define par_op_arr               argCountAndArgs(p_op_arr)
#define par_op_op_arr            argCountAndArgs(p_op_op_arr)
#define par_op_int_op_arr        argCountAndArgs(p_op_int_op_arr)
#define par_any_op_any           argCountAndArgs(p_any_op_any)
#define par_op_op_any            argCountAndArgs(p_op_op_any)
#define par_op_int_op_any        argCountAndArgs(p_op_int_op_any)
#define par_big                  argCountAndArgs(p_big)
#define par_big_big              argCountAndArgs(p_big_big)
#define par_big_bln              argCountAndArgs(p_big_bln)
#define par_big_op_big           argCountAndArgs(p_big_op_big)
#define par_big_op_int           argCountAndArgs(p_big_op_int)
#define par_op_big               argCountAndArgs(p_op_big)
#define par_op_op_big            argCountAndArgs(p_op_op_big)
#define par_bln                  argCountAndArgs(p_bln)
#define par_bln_typ_int          argCountAndArgs(p_bln_typ_int)
#define par_bln_typ_str          argCountAndArgs(p_bln_typ_str)
#define par_bln_op_bln           argCountAndArgs(p_bln_op_bln)
#define par_bln_op_any_op_any    argCountAndArgs(p_bln_op_any_op_any)
#define par_op_bln               argCountAndArgs(p_op_bln)
#define par_op_bln_op_prc        argCountAndArgs(p_op_bln_op_prc)
#define par_op_bln_op_prc_prc    argCountAndArgs(p_op_bln_op_prc_prc)
#define par_op_op_bln            argCountAndArgs(p_op_op_bln)
#define par_blo                  argCountAndArgs(p_blo)
#define par_bst                  argCountAndArgs(p_bst)
#define par_bst_bst              argCountAndArgs(p_bst_bst)
#define par_bst_bln              argCountAndArgs(p_bst_bln)
#define par_bst_4_int            argCountAndArgs(p_bst_4_int)
#define par_bst_set_arr_fil      argCountAndArgs(p_bst_set_arr_fil)
#define par_bst_op_bst           argCountAndArgs(p_bst_op_bst)
#define par_bst_op_int           argCountAndArgs(p_bst_op_int)
#define par_chr                  argCountAndArgs(p_chr)
#define par_chr_chr              argCountAndArgs(p_chr_chr)
#define par_chr_op_chr           argCountAndArgs(p_chr_op_chr)
#define par_op_chr               argCountAndArgs(p_op_chr)
#define par_dbs                  argCountAndArgs(p_dbs)
#define par_dbs_dbs              argCountAndArgs(p_dbs_dbs)
#define par_dbs_bln              argCountAndArgs(p_dbs_bln)
#define par_dbs_str              argCountAndArgs(p_dbs_str)
#define par_dbs_op_dbs           argCountAndArgs(p_dbs_op_dbs)
#define par_enu                  argCountAndArgs(p_enu)
#define par_enu_rfl              argCountAndArgs(p_enu_rfl)
#define par_enu_op_enu           argCountAndArgs(p_enu_op_enu)
#define par_op_enu               argCountAndArgs(p_op_enu)
#define par_op_op_enu            argCountAndArgs(p_op_op_enu)
#define par_exp                  argCountAndArgs(p_exp)
#define par_fil                  argCountAndArgs(p_fil)
#define par_fil_big              argCountAndArgs(p_fil_big)
#define par_fil_chr              argCountAndArgs(p_fil_chr)
#define par_fil_fil              argCountAndArgs(p_fil_fil)
#define par_fil_int              argCountAndArgs(p_fil_int)
#define par_fil_str              argCountAndArgs(p_fil_str)
#define par_fil_op_fil           argCountAndArgs(p_fil_op_fil)
#define par_flt                  argCountAndArgs(p_flt)
#define par_flt_flt              argCountAndArgs(p_flt_flt)
#define par_flt_op_flt           argCountAndArgs(p_flt_op_flt)
#define par_flt_op_int           argCountAndArgs(p_flt_op_int)
#define par_flt_flt_int          argCountAndArgs(p_flt_flt_int)
#define par_op_flt               argCountAndArgs(p_op_flt)
#define par_hel_4_ref            argCountAndArgs(p_hel_4_ref)
#define par_hel_op_hel           argCountAndArgs(p_hel_op_hel)
#define par_hsh                  argCountAndArgs(p_hsh)
#define par_hsh_hsh_4_ref        argCountAndArgs(p_hsh_hsh_4_ref)
#define par_hsh_ref_ref          argCountAndArgs(p_hsh_ref_ref)
#define par_hsh_any_int_ref      argCountAndArgs(p_hsh_any_int_ref)
#define par_hsh_op_int_ref       argCountAndArgs(p_hsh_op_int_ref)
#define par_any_hsh_prc_ref      argCountAndArgs(p_any_hsh_prc_ref)
#define par_int                  argCountAndArgs(p_int)
#define par_int_int              argCountAndArgs(p_int_int)
#define par_int_int_int          argCountAndArgs(p_int_int_int)
#define par_int_int_int_int      argCountAndArgs(p_int_int_int_int)
#define par_4_int                argCountAndArgs(p_4_int)
#define par_4_int_str            argCountAndArgs(p_4_int_str)
#define par_7_int_bln            argCountAndArgs(p_7_int_bln)
#define par_8_int                argCountAndArgs(p_8_int)
#define par_8_int_bln            argCountAndArgs(p_8_int_bln)
#define par_9_int_bln            argCountAndArgs(p_9_int_bln)
#define par_int_ref              argCountAndArgs(p_int_ref)
#define par_int_rfl              argCountAndArgs(p_int_rfl)
#define par_int_str_int          argCountAndArgs(p_int_str_int)
#define par_int_op_int           argCountAndArgs(p_int_op_int)
#define par_int_op_set           argCountAndArgs(p_int_op_set)
#define par_int_op_any           argCountAndArgs(p_int_op_any)
#define par_int_op_op_int        argCountAndArgs(p_int_op_op_int)
#define par_int_op_op_set        argCountAndArgs(p_int_op_op_set)
#define par_op_int               argCountAndArgs(p_op_int)
#define par_op_int_int           argCountAndArgs(p_op_int_int)
#define par_op_4_int             argCountAndArgs(p_op_4_int)
#define par_op_5_int             argCountAndArgs(p_op_5_int)
#define par_op_int_op_int        argCountAndArgs(p_op_int_op_int)
#define par_op_op_int            argCountAndArgs(p_op_op_int)
#define par_itf                  argCountAndArgs(p_itf)
#define par_itf_itf              argCountAndArgs(p_itf_itf)
#define par_itf_op_itf           argCountAndArgs(p_itf_op_itf)
#define par_itf_op_sct           argCountAndArgs(p_itf_op_sct)
#define par_pcs                  argCountAndArgs(p_pcs)
#define par_pcs_pcs              argCountAndArgs(p_pcs_pcs)
#define par_pcs_op_pcs           argCountAndArgs(p_pcs_op_pcs)
#define par_plt                  argCountAndArgs(p_plt)
#define par_plt_plt              argCountAndArgs(p_plt_plt)
#define par_plt_op_plt           argCountAndArgs(p_plt_op_plt)
#define par_pol                  argCountAndArgs(p_pol)
#define par_pol_int              argCountAndArgs(p_pol_int)
#define par_pol_itf              argCountAndArgs(p_pol_itf)
#define par_pol_soc              argCountAndArgs(p_pol_soc)
#define par_pol_soc_int          argCountAndArgs(p_pol_soc_int)
#define par_pol_soc_int_itf      argCountAndArgs(p_pol_soc_int_itf)
#define par_pol_op_pol           argCountAndArgs(p_pol_op_pol)
#define par_prc                  argCountAndArgs(p_prc)
#define par_prc_op_prc           argCountAndArgs(p_prc_op_prc)
#define par_op_prc               argCountAndArgs(p_op_prc)
#define par_op_prc_op_prc        argCountAndArgs(p_op_prc_op_prc)
#define par_op_prc_op_bln        argCountAndArgs(p_op_prc_op_bln)
#define par_op_prc_op_op_op_prc  argCountAndArgs(p_op_prc_op_op_op_prc)
#define par_op_any_op_prc        argCountAndArgs(p_op_any_op_prc)
#define par_op_op_exp            argCountAndArgs(p_op_op_exp)
#define par_op_op_prc_op_exp     argCountAndArgs(p_op_op_prc_op_exp)
#define par_prg                  argCountAndArgs(p_prg)
#define par_prg_prg              argCountAndArgs(p_prg_prg)
#define par_prg_arr_set_str      argCountAndArgs(p_prg_arr_set_str)
#define par_prg_ref              argCountAndArgs(p_prg_ref)
#define par_prg_ref_rfl          argCountAndArgs(p_prg_ref_rfl)
#define par_prg_rfl              argCountAndArgs(p_prg_rfl)
#define par_prg_str              argCountAndArgs(p_prg_str)
#define par_prg_op_prg           argCountAndArgs(p_prg_op_prg)
#define par_op_prm               argCountAndArgs(p_op_prm)
#define par_ref                  argCountAndArgs(p_ref)
#define par_ref_ref              argCountAndArgs(p_ref_ref)
#define par_ref_bln              argCountAndArgs(p_ref_bln)
#define par_ref_int              argCountAndArgs(p_ref_int)
#define par_ref_rfl              argCountAndArgs(p_ref_rfl)
#define par_ref_typ              argCountAndArgs(p_ref_typ)
#define par_ref_op_ref           argCountAndArgs(p_ref_op_ref)
#define par_ref_op_rfl           argCountAndArgs(p_ref_op_rfl)
#define par_ref_op_op_rfl        argCountAndArgs(p_ref_op_op_rfl)
#define par_op_ref               argCountAndArgs(p_op_ref)
#define par_op_ref_op_rfl_op_prc argCountAndArgs(p_op_ref_op_rfl_op_prc)
#define par_rfl                  argCountAndArgs(p_rfl)
#define par_rfl_ref              argCountAndArgs(p_rfl_ref)
#define par_rfl_op_rfl           argCountAndArgs(p_rfl_op_rfl)
#define par_rfl_op_int           argCountAndArgs(p_rfl_op_int)
#define par_rfl_op_int_op_int    argCountAndArgs(p_rfl_op_int_op_int)
#define par_rfl_op_op_int        argCountAndArgs(p_rfl_op_op_int)
#define par_rfl_op_op_int_op_ref argCountAndArgs(p_rfl_op_op_int_op_ref)
#define par_sct                  argCountAndArgs(p_sct)
#define par_sct_ref              argCountAndArgs(p_sct_ref)
#define par_sct_op_sct           argCountAndArgs(p_sct_op_sct)
#define par_sct_op_op            argCountAndArgs(p_sct_op_op)
#define par_op_op_sct            argCountAndArgs(p_op_op_sct)
#define par_sel                  argCountAndArgs(p_sel)
#define par_sel_sel              argCountAndArgs(p_sel_sel)
#define par_sel_op_sel           argCountAndArgs(p_sel_op_sel)
#define par_op_sel               argCountAndArgs(p_op_sel)
#define par_set                  argCountAndArgs(p_set)
#define par_set_set              argCountAndArgs(p_set_set)
#define par_set_int              argCountAndArgs(p_set_int)
#define par_set_op_set           argCountAndArgs(p_set_op_set)
#define par_op_op_set            argCountAndArgs(p_op_op_set)
#define par_soc                  argCountAndArgs(p_soc)
#define par_soc_bst              argCountAndArgs(p_soc_bst)
#define par_soc_chr              argCountAndArgs(p_soc_chr)
#define par_soc_int              argCountAndArgs(p_soc_int)
#define par_soc_int_bln          argCountAndArgs(p_soc_int_bln)
#define par_soc_int_chr          argCountAndArgs(p_soc_int_chr)
#define par_soc_int_int          argCountAndArgs(p_soc_int_int)
#define par_soc_str              argCountAndArgs(p_soc_str)
#define par_soc_str_int          argCountAndArgs(p_soc_str_int)
#define par_soc_str_int_bst      argCountAndArgs(p_soc_str_int_bst)
#define par_soc_str_int_int      argCountAndArgs(p_soc_str_int_int)
#define par_soc_str_int_int_bst  argCountAndArgs(p_soc_str_int_int_bst)
#define par_soc_op_soc           argCountAndArgs(p_soc_op_soc)
#define par_sqs                  argCountAndArgs(p_sqs)
#define par_sqs_sqs              argCountAndArgs(p_sqs_sqs)
#define par_sqs_int              argCountAndArgs(p_sqs_int)
#define par_sqs_int_big          argCountAndArgs(p_sqs_int_big)
#define par_sqs_int_big_big      argCountAndArgs(p_sqs_int_big_big)
#define par_sqs_int_bln          argCountAndArgs(p_sqs_int_bln)
#define par_sqs_int_bst          argCountAndArgs(p_sqs_int_bst)
#define par_sqs_8_int            argCountAndArgs(p_sqs_8_int)
#define par_sqs_9_int            argCountAndArgs(p_sqs_9_int)
#define par_sqs_9_int_bln        argCountAndArgs(p_sqs_9_int_bln)
#define par_sqs_int_flt          argCountAndArgs(p_sqs_int_flt)
#define par_sqs_int_int          argCountAndArgs(p_sqs_int_int)
#define par_sqs_int_str          argCountAndArgs(p_sqs_int_str)
#define par_sqs_op_sqs           argCountAndArgs(p_sqs_op_sqs)
#define par_str                  argCountAndArgs(p_str)
#define par_str_str              argCountAndArgs(p_str_str)
#define par_str_arr              argCountAndArgs(p_str_arr)
#define par_str_arr_fil_fil      argCountAndArgs(p_str_arr_fil_fil)
#define par_str_arr_fil_fil_fil  argCountAndArgs(p_str_arr_fil_fil_fil)
#define par_str_arr_str          argCountAndArgs(p_str_arr_str)
#define par_str_arr_str_str_str  argCountAndArgs(p_str_arr_str_str_str)
#define par_str_chr              argCountAndArgs(p_str_chr)
#define par_str_chr_int          argCountAndArgs(p_str_chr_int)
#define par_str_int              argCountAndArgs(p_str_int)
#define par_str_8_int            argCountAndArgs(p_str_8_int)
#define par_str_8_int_bln        argCountAndArgs(p_str_8_int_bln)
#define par_str_set              argCountAndArgs(p_str_set)
#define par_str_set_arr_fil      argCountAndArgs(p_str_set_arr_fil)
#define par_str_str_str          argCountAndArgs(p_str_str_str)
#define par_str_str_int          argCountAndArgs(p_str_str_int)
#define par_str_op_str           argCountAndArgs(p_str_op_str)
#define par_str_op_chr           argCountAndArgs(p_str_op_chr)
#define par_str_op_int           argCountAndArgs(p_str_op_int)
#define par_str_op_int_op_int    argCountAndArgs(p_str_op_int_op_int)
#define par_str_op_op_int        argCountAndArgs(p_str_op_op_int)
#define par_str_op_op_int_op_str argCountAndArgs(p_str_op_op_int_op_str)
#define par_str_op_op_int_op_chr argCountAndArgs(p_str_op_op_int_op_chr)
#define par_op_str               argCountAndArgs(p_op_str)
#define par_op_str_int_3_str     argCountAndArgs(p_op_str_int_3_str)
#define par_op_str_int_4_str     argCountAndArgs(p_op_str_int_4_str)
#define par_op_5_str             argCountAndArgs(p_op_5_str)
#define par_op_chr_op_str_op_prc argCountAndArgs(p_op_chr_op_str_op_prc)
#define par_typ                  argCountAndArgs(p_typ)
#define par_typ_typ              argCountAndArgs(p_typ_typ)
#define par_typ_op_any           argCountAndArgs(p_typ_op_any)
#define par_typ_op_typ           argCountAndArgs(p_typ_op_typ)
#define par_typ_op_op_any        argCountAndArgs(p_typ_op_op_any)
#define par_op_typ               argCountAndArgs(p_op_typ)
#define par_op_typ_op            argCountAndArgs(p_op_typ_op)
#define par_op_typ_op_exp        argCountAndArgs(p_op_typ_op_exp)
#define par_op_typ_op_exp_op_exp argCountAndArgs(p_op_typ_op_exp_op_exp)
#define par_op_typ_op_exp_op_op  argCountAndArgs(p_op_typ_op_exp_op_op)
#define par_op_op_typ_op         argCountAndArgs(p_op_op_typ_op)
#define par_op_op_typ_op_exp     argCountAndArgs(p_op_op_typ_op_exp)
#define par_voi_op_voi           argCountAndArgs(p_voi_op_voi)
#define par_win                  argCountAndArgs(p_win)
#define par_win_win              argCountAndArgs(p_win_win)
#define par_win_win_6_int        argCountAndArgs(p_win_win_6_int)
#define par_win_bln              argCountAndArgs(p_win_bln)
#define par_win_chr_bln          argCountAndArgs(p_win_chr_bln)
#define par_win_int              argCountAndArgs(p_win_int)
#define par_win_int_int          argCountAndArgs(p_win_int_int)
#define par_win_int_int_int      argCountAndArgs(p_win_int_int_int)
#define par_win_int_int_win      argCountAndArgs(p_win_int_int_win)
#define par_win_int_int_plt_int  argCountAndArgs(p_win_int_int_plt_int)
#define par_win_4_int            argCountAndArgs(p_win_4_int)
#define par_win_4_int_win        argCountAndArgs(p_win_4_int_win)
#define par_win_5_int            argCountAndArgs(p_win_5_int)
#define par_win_str              argCountAndArgs(p_win_str)
#define par_win_op_win           argCountAndArgs(p_win_op_win)

#define par_win_int_int_str_int_int                   argCountAndArgs(p_win_int_int_str_int_int)
#define par_hsh_any_int_ref_ref_ref                   argCountAndArgs(p_hsh_any_int_ref_ref_ref)
#define par_hsh_any_int_any_ref_ref                   argCountAndArgs(p_hsh_any_int_any_ref_ref)
#define par_any_any_hsh_prc_ref_ref                   argCountAndArgs(p_any_any_hsh_prc_ref_ref)
#define par_hsh_any_any_int_3_ref                     argCountAndArgs(p_hsh_any_any_int_3_ref)
#define par_op_prc_op_prc_op_op_prc                   argCountAndArgs(p_op_prc_op_prc_op_op_prc)
#define par_op_any_op_prc_op_op_prc                   argCountAndArgs(p_op_any_op_prc_op_op_prc)
#define par_op_op_int_op_str_op_prc                   argCountAndArgs(p_op_op_int_op_str_op_prc)
#define par_win_3_int_flt_flt_int                     argCountAndArgs(p_win_3_int_flt_flt_int)
#define par_hsh_any_any_int_4_ref                     argCountAndArgs(p_hsh_any_any_int_4_ref)
#define par_op_chr_op_int_op_str_op_prc               argCountAndArgs(p_op_chr_op_int_op_str_op_prc)
#define par_op_ref_op_int_op_rfl_op_prc               argCountAndArgs(p_op_ref_op_int_op_rfl_op_prc)
#define par_op_ref_op_rfl_op_bln_op_prc               argCountAndArgs(p_op_ref_op_rfl_op_bln_op_prc)
#define par_prg_int_int_str_int_int_str_str           argCountAndArgs(p_prg_int_int_str_int_int_str_str)
#define par_op_int_op_int_op_int_op_prc               argCountAndArgs(p_op_int_op_int_op_int_op_prc)
#define par_win_3_int_flt_flt_int_int                 argCountAndArgs(p_win_3_int_flt_flt_int_int)
#define par_op_int_op_int_op_int_op_int_op_prc        argCountAndArgs(p_op_int_op_int_op_int_op_int_op_prc)
#define par_op_op_op_typ_op_exp_op_any_op_exp         argCountAndArgs(p_op_op_op_typ_op_exp_op_any_op_exp)
#define par_op_op_op_typ_op_exp_op_any_op_prc_op_exp  argCountAndArgs(p_op_op_op_typ_op_exp_op_any_op_prc_op_exp)


static const actEntryRecord actEntryTable[] = {
    { "ACT_ILLEGAL",                  act_illegal,                  ILLEGALOBJECT,     par_no_args},

    { "ACE_CMP",                      ace_cmp,                      INTOBJECT,         par_ace_ace},
    { "ACE_CPY",                      ace_cpy,                      VOIDOBJECT,        par_ace_op_ace},
    { "ACE_CREATE",                   ace_create,                   VOIDOBJECT,        par_ace_op_ace},
    { "ACE_EQ",                       ace_eq,                       BOOLOBJECT,        par_ace_op_ace},
    { "ACE_GEN",                      ace_gen,                      ACTENTRYOBJECT,    par_op_str},
    { "ACE_HASHCODE",                 ace_hashcode,                 INTOBJECT,         par_ace},
    { "ACE_ICONV1",                   ace_iconv1,                   ACTENTRYOBJECT,    par_int},
    { "ACE_ICONV3",                   ace_iconv3,                   ACTENTRYOBJECT,    par_op_op_int},
    { "ACE_NE",                       ace_ne,                       BOOLOBJECT,        par_ace_op_ace},
    { "ACE_ORD",                      ace_ord,                      INTOBJECT,         par_ace},
    { "ACE_STR",                      ace_str,                      STRIOBJECT,        par_ace},
    { "ACE_VALUE",                    ace_value,                    ACTENTRYOBJECT,    par_ref},

    { "ACT_CREATE",                   act_create,                   VOIDOBJECT,        par_act_op_ace},

    { "ARR_APPEND",                   arr_append,                   VOIDOBJECT,        par_arr_op_arr},
    { "ARR_ARRLIT",                   arr_arrlit,                   ARRAYOBJECT,       par_op_op_arr},
    { "ARR_ARRLIT2",                  arr_arrlit2,                  ARRAYOBJECT,       par_op_int_op_arr},
    { "ARR_BASELIT",                  arr_baselit,                  ARRAYOBJECT,       par_op_op_any},
    { "ARR_BASELIT2",                 arr_baselit2,                 ARRAYOBJECT,       par_op_int_op_any},
    { "ARR_CAT",                      arr_cat,                      ARRAYOBJECT,       par_arr_op_arr},
    { "ARR_CONV",                     arr_conv,                     ARRAYOBJECT,       par_op_op_arr},
    { "ARR_CPY",                      arr_cpy,                      VOIDOBJECT,        par_arr_op_arr},
    { "ARR_CREATE",                   arr_create,                   VOIDOBJECT,        par_arr_op_arr},
    { "ARR_DESTR",                    arr_destr,                    VOIDOBJECT,        par_arr},
    { "ARR_EMPTY",                    arr_empty,                    ARRAYOBJECT,       par_no_args},
    { "ARR_EXTEND",                   arr_extend,                   ARRAYOBJECT,       par_arr_op_any},
    { "ARR_GEN",                      arr_gen,                      ARRAYOBJECT,       par_any_op_any},
    { "ARR_HEAD",                     arr_head,                     ARRAYOBJECT,       par_arr_op_op_int},
    { "ARR_IDX",                      arr_idx,                      ILLEGALOBJECT,     par_arr_op_int},
    { "ARR_INSERT",                   arr_insert,                   VOIDOBJECT,        par_arr_int_any},
    { "ARR_INSERT_ARRAY",             arr_insert_array,             VOIDOBJECT,        par_arr_int_arr},
    { "ARR_LNG",                      arr_lng,                      INTOBJECT,         par_arr},
    { "ARR_MAXIDX",                   arr_maxidx,                   INTOBJECT,         par_arr},
    { "ARR_MINIDX",                   arr_minidx,                   INTOBJECT,         par_arr},
    { "ARR_PUSH",                     arr_push,                     VOIDOBJECT,        par_arr_op_any},
    { "ARR_RANGE",                    arr_range,                    ARRAYOBJECT,       par_arr_op_int_op_int},
    { "ARR_REMOVE",                   arr_remove,                   ILLEGALOBJECT,     par_arr_int},
    { "ARR_REMOVE_ARRAY",             arr_remove_array,             ARRAYOBJECT,       par_arr_int_int},
    { "ARR_SORT",                     arr_sort,                     ARRAYOBJECT,       par_arr},
    { "ARR_SORT_REVERSE",             arr_sort_reverse,             ARRAYOBJECT,       par_arr},
    { "ARR_SUBARR",                   arr_subarr,                   ARRAYOBJECT,       par_arr_op_int_op_int},
    { "ARR_TAIL",                     arr_tail,                     ARRAYOBJECT,       par_arr_op_int},
    { "ARR_TIMES",                    arr_times,                    ARRAYOBJECT,       par_int_op_any},

    { "BIG_ABS",                      big_abs,                      BIGINTOBJECT,      par_big},
    { "BIG_ADD",                      big_add,                      BIGINTOBJECT,      par_big_op_big},
    { "BIG_ADD_ASSIGN",               big_add_assign,               VOIDOBJECT,        par_big_op_big},
    { "BIG_BIT_LENGTH",               big_bit_length,               INTOBJECT,         par_big},
    { "BIG_CMP",                      big_cmp,                      INTOBJECT,         par_big_big},
    { "BIG_CONV",                     big_conv,                     BIGINTOBJECT,      par_op_op_big},
    { "BIG_CPY",                      big_cpy,                      VOIDOBJECT,        par_big_op_big},
    { "BIG_CREATE",                   big_create,                   VOIDOBJECT,        par_big_op_big},
    { "BIG_DECR",                     big_decr,                     VOIDOBJECT,        par_big},
    { "BIG_DESTR",                    big_destr,                    VOIDOBJECT,        par_big},
    { "BIG_DIV",                      big_div,                      BIGINTOBJECT,      par_big_op_big},
    { "BIG_DIV_REM",                  big_div_rem,                  STRUCTOBJECT,      par_big_op_big},
    { "BIG_EQ",                       big_eq,                       BOOLOBJECT,        par_big_op_big},
    { "BIG_FROM_BSTRI_BE",            big_from_bstri_be,            BIGINTOBJECT,      par_bst_bln},
    { "BIG_FROM_BSTRI_LE",            big_from_bstri_le,            BIGINTOBJECT,      par_bst_bln},
    { "BIG_GCD",                      big_gcd,                      BIGINTOBJECT,      par_big_big},
    { "BIG_GE",                       big_ge,                       BOOLOBJECT,        par_big_op_big},
    { "BIG_GT",                       big_gt,                       BOOLOBJECT,        par_big_op_big},
    { "BIG_HASHCODE",                 big_hashcode,                 INTOBJECT,         par_big},
    { "BIG_ICONV1",                   big_iconv1,                   BIGINTOBJECT,      par_int},
    { "BIG_ICONV3",                   big_iconv3,                   BIGINTOBJECT,      par_op_op_int},
    { "BIG_INCR",                     big_incr,                     VOIDOBJECT,        par_big},
    { "BIG_IPOW",                     big_ipow,                     BIGINTOBJECT,      par_big_op_int},
    { "BIG_LE",                       big_le,                       BOOLOBJECT,        par_big_op_big},
    { "BIG_LOG10",                    big_log10,                    BIGINTOBJECT,      par_big},
    { "BIG_LOG2",                     big_log2,                     BIGINTOBJECT,      par_big},
    { "BIG_LOWEST_SET_BIT",           big_lowest_set_bit,           INTOBJECT,         par_big},
    { "BIG_LSHIFT",                   big_lshift,                   BIGINTOBJECT,      par_big_op_int},
    { "BIG_LSHIFT_ASSIGN",            big_lshift_assign,            VOIDOBJECT,        par_big_op_int},
    { "BIG_LT",                       big_lt,                       BOOLOBJECT,        par_big_op_big},
    { "BIG_MDIV",                     big_mdiv,                     BIGINTOBJECT,      par_big_op_big},
    { "BIG_MOD",                      big_mod,                      BIGINTOBJECT,      par_big_op_big},
    { "BIG_MULT",                     big_mult,                     BIGINTOBJECT,      par_big_op_big},
    { "BIG_MULT_ASSIGN",              big_mult_assign,              VOIDOBJECT,        par_big_op_big},
    { "BIG_NE",                       big_ne,                       BOOLOBJECT,        par_big_op_big},
    { "BIG_NEGATE",                   big_negate,                   BIGINTOBJECT,      par_op_big},
    { "BIG_ODD",                      big_odd,                      BOOLOBJECT,        par_big},
    { "BIG_ORD",                      big_ord,                      INTOBJECT,         par_big},
    { "BIG_PARSE1",                   big_parse1,                   BIGINTOBJECT,      par_str},
    { "BIG_PARSE_BASED",              big_parse_based,              BIGINTOBJECT,      par_str_int},
    { "BIG_PLUS",                     big_plus,                     BIGINTOBJECT,      par_op_big},
    { "BIG_PRED",                     big_pred,                     BIGINTOBJECT,      par_big},
    { "BIG_RADIX",                    big_RADIX,                    STRIOBJECT,        par_big_op_int},
    { "BIG_RAND",                     big_rand,                     BIGINTOBJECT,      par_big_big},
    { "BIG_REM",                      big_rem,                      BIGINTOBJECT,      par_big_op_big},
    { "BIG_RSHIFT",                   big_rshift,                   BIGINTOBJECT,      par_big_op_int},
    { "BIG_RSHIFT_ASSIGN",            big_rshift_assign,            VOIDOBJECT,        par_big_op_int},
    { "BIG_SBTR",                     big_sbtr,                     BIGINTOBJECT,      par_big_op_big},
    { "BIG_SBTR_ASSIGN",              big_sbtr_assign,              VOIDOBJECT,        par_big_op_big},
    { "BIG_STR",                      big_str,                      STRIOBJECT,        par_big},
    { "BIG_SUCC",                     big_succ,                     BIGINTOBJECT,      par_big},
    { "BIG_TO_BSTRI_BE",              big_to_bstri_be,              BSTRIOBJECT,       par_big_bln},
    { "BIG_TO_BSTRI_LE",              big_to_bstri_le,              BSTRIOBJECT,       par_big_bln},
    { "BIG_VALUE",                    big_value,                    BIGINTOBJECT,      par_ref},
    { "BIG_radix",                    big_radix,                    STRIOBJECT,        par_big_op_int},

    { "BIN_AND",                      bin_and,                      INTOBJECT,         par_int_op_int},
    { "BIN_AND_ASSIGN",               bin_and_assign,               VOIDOBJECT,        par_int_op_int},
    { "BIN_BIG",                      bin_big,                      BIGINTOBJECT,      par_int},
    { "BIN_BINARY",                   bin_binary,                   INTOBJECT,         par_big},
    { "BIN_BIT_LENGTH",               bin_bit_length,               INTOBJECT,         par_int},
    { "BIN_BYTES_BE_2_UINT",          bin_bytes_be_2_uint,          INTOBJECT,         par_str},
    { "BIN_BYTES_LE_2_UINT",          bin_bytes_le_2_uint,          INTOBJECT,         par_str},
    { "BIN_CARD",                     bin_card,                     INTOBJECT,         par_int},
    { "BIN_CMP",                      bin_cmp,                      INTOBJECT,         par_int_int},
    { "BIN_GET_BINARY_FROM_SET",      bin_get_binary_from_set,      INTOBJECT,         par_set_int},
    { "BIN_LOWEST_SET_BIT",           bin_lowest_set_bit,           INTOBJECT,         par_int},
    { "BIN_LSHIFT",                   bin_lshift,                   INTOBJECT,         par_int_op_int},
    { "BIN_LSHIFT_ASSIGN",            bin_lshift_assign,            VOIDOBJECT,        par_int_op_int},
    { "BIN_N_BYTES_BE",               bin_n_bytes_be,               STRIOBJECT,        par_int_op_int},
    { "BIN_N_BYTES_LE",               bin_n_bytes_le,               STRIOBJECT,        par_int_op_int},
    { "BIN_OR",                       bin_or,                       INTOBJECT,         par_int_op_int},
    { "BIN_OR_ASSIGN",                bin_or_assign,                VOIDOBJECT,        par_int_op_int},
    { "BIN_RADIX",                    bin_RADIX,                    STRIOBJECT,        par_int_op_int},
    { "BIN_RSHIFT",                   bin_rshift,                   INTOBJECT,         par_int_op_int},
    { "BIN_RSHIFT_ASSIGN",            bin_rshift_assign,            VOIDOBJECT,        par_int_op_int},
    { "BIN_STR",                      bin_str,                      STRIOBJECT,        par_int},
    { "BIN_XOR",                      bin_xor,                      INTOBJECT,         par_int_op_int},
    { "BIN_XOR_ASSIGN",               bin_xor_assign,               VOIDOBJECT,        par_int_op_int},
    { "BIN_radix",                    bin_radix,                    STRIOBJECT,        par_int_op_int},

    { "BLN_AND",                      bln_and,                      BOOLOBJECT,        par_bln_op_bln},
    { "BLN_AND_ASSIGN",               bln_and_assign,               VOIDOBJECT,        par_bln_op_bln},
    { "BLN_CPY",                      bln_cpy,                      VOIDOBJECT,        par_bln_op_bln},
    { "BLN_CREATE",                   bln_create,                   VOIDOBJECT,        par_bln_op_bln},
    { "BLN_EQ",                       bln_eq,                       BOOLOBJECT,        par_bln_op_bln},
    { "BLN_GE",                       bln_ge,                       BOOLOBJECT,        par_bln_op_bln},
    { "BLN_GT",                       bln_gt,                       BOOLOBJECT,        par_bln_op_bln},
    { "BLN_ICONV1",                   bln_iconv1,                   BOOLOBJECT,        par_int},
    { "BLN_ICONV3",                   bln_iconv3,                   BOOLOBJECT,        par_op_op_int},
    { "BLN_LE",                       bln_le,                       BOOLOBJECT,        par_bln_op_bln},
    { "BLN_LT",                       bln_lt,                       BOOLOBJECT,        par_bln_op_bln},
    { "BLN_NE",                       bln_ne,                       BOOLOBJECT,        par_bln_op_bln},
    { "BLN_NOT",                      bln_not,                      BOOLOBJECT,        par_op_bln},
    { "BLN_OR",                       bln_or,                       BOOLOBJECT,        par_bln_op_bln},
    { "BLN_ORD",                      bln_ord,                      INTOBJECT,         par_bln},
    { "BLN_OR_ASSIGN",                bln_or_assign,                VOIDOBJECT,        par_bln_op_bln},
    { "BLN_PRED",                     bln_pred,                     BOOLOBJECT,        par_bln},
    { "BLN_SUCC",                     bln_succ,                     BOOLOBJECT,        par_bln},
    { "BLN_TERNARY",                  bln_ternary,                  ILLEGALOBJECT,     par_bln_op_any_op_any},
    { "BLN_VALUE",                    bln_value,                    BOOLOBJECT,        par_ref},
    { "BLN_XOR_ASSIGN",               bln_xor_assign,               VOIDOBJECT,        par_bln_op_bln},

    { "BST_APPEND",                   bst_append,                   VOIDOBJECT,        par_bst_op_bst},
    { "BST_CAT",                      bst_cat,                      BSTRIOBJECT,       par_bst_op_bst},
    { "BST_CMP",                      bst_cmp,                      INTOBJECT,         par_bst_bst},
    { "BST_CPY",                      bst_cpy,                      VOIDOBJECT,        par_bst_op_bst},
    { "BST_CREATE",                   bst_create,                   VOIDOBJECT,        par_bst_op_bst},
    { "BST_DESTR",                    bst_destr,                    VOIDOBJECT,        par_bst},
    { "BST_EMPTY",                    bst_empty,                    BSTRIOBJECT,       par_no_args},
    { "BST_EQ",                       bst_eq,                       BOOLOBJECT,        par_bst_op_bst},
    { "BST_HASHCODE",                 bst_hashcode,                 INTOBJECT,         par_bst},
    { "BST_IDX",                      bst_idx,                      CHAROBJECT,        par_bst_op_int},
    { "BST_LNG",                      bst_lng,                      INTOBJECT,         par_bst},
    { "BST_NE",                       bst_ne,                       BOOLOBJECT,        par_bst_op_bst},
    { "BST_PARSE1",                   bst_parse1,                   BSTRIOBJECT,       par_str},
    { "BST_STR",                      bst_str,                      STRIOBJECT,        par_bst},
    { "BST_VALUE",                    bst_value,                    BSTRIOBJECT,       par_ref},

    { "CHR_CMP",                      chr_cmp,                      INTOBJECT,         par_chr_chr},
    { "CHR_CPY",                      chr_cpy,                      VOIDOBJECT,        par_chr_op_chr},
    { "CHR_CREATE",                   chr_create,                   VOIDOBJECT,        par_chr_op_chr},
    { "CHR_C_LITERAL",                chr_c_literal,                STRIOBJECT,        par_chr},
    { "CHR_DECR",                     chr_decr,                     VOIDOBJECT,        par_chr},
    { "CHR_EQ",                       chr_eq,                       BOOLOBJECT,        par_chr_op_chr},
    { "CHR_GE",                       chr_ge,                       BOOLOBJECT,        par_chr_op_chr},
    { "CHR_GT",                       chr_gt,                       BOOLOBJECT,        par_chr_op_chr},
    { "CHR_HASHCODE",                 chr_hashcode,                 INTOBJECT,         par_chr},
    { "CHR_ICONV1",                   chr_iconv1,                   CHAROBJECT,        par_int},
    { "CHR_ICONV3",                   chr_iconv3,                   CHAROBJECT,        par_op_op_int},
    { "CHR_INCR",                     chr_incr,                     VOIDOBJECT,        par_chr},
    { "CHR_IS_LETTER",                chr_is_letter,                BOOLOBJECT,        par_chr},
    { "CHR_LE",                       chr_le,                       BOOLOBJECT,        par_chr_op_chr},
    { "CHR_LOW",                      chr_low,                      CHAROBJECT,        par_chr},
    { "CHR_LT",                       chr_lt,                       BOOLOBJECT,        par_chr_op_chr},
    { "CHR_NE",                       chr_ne,                       BOOLOBJECT,        par_chr_op_chr},
    { "CHR_ORD",                      chr_ord,                      INTOBJECT,         par_chr},
    { "CHR_PRED",                     chr_pred,                     CHAROBJECT,        par_chr},
    { "CHR_STR",                      chr_str,                      STRIOBJECT,        par_chr},
    { "CHR_SUCC",                     chr_succ,                     CHAROBJECT,        par_chr},
    { "CHR_UP",                       chr_up,                       CHAROBJECT,        par_chr},
    { "CHR_VALUE",                    chr_value,                    CHAROBJECT,        par_ref},
    { "CHR_WIDTH",                    chr_width,                    INTOBJECT,         par_chr},

    { "CMD_BIG_FILESIZE",             cmd_big_filesize,             BIGINTOBJECT,      par_str},
    { "CMD_CHDIR",                    cmd_chdir,                    VOIDOBJECT,        par_str},
    { "CMD_CLONE_FILE",               cmd_clone_file,               VOIDOBJECT,        par_str_str},
    { "CMD_CONFIG_VALUE",             cmd_config_value,             STRIOBJECT,        par_str},
    { "CMD_COPY_FILE",                cmd_copy_file,                VOIDOBJECT,        par_str_str},
    { "CMD_ENVIRONMENT",              cmd_environment,              ARRAYOBJECT,       par_no_args},
    { "CMD_FILESIZE",                 cmd_filesize,                 INTOBJECT,         par_str},
    { "CMD_FILETYPE",                 cmd_filetype,                 INTOBJECT,         par_str},
    { "CMD_FILETYPE_SL",              cmd_filetype_sl,              INTOBJECT,         par_str},
    { "CMD_FINAL_PATH",               cmd_final_path,               STRIOBJECT,        par_str},
    { "CMD_GETCWD",                   cmd_getcwd,                   STRIOBJECT,        par_no_args},
    { "CMD_GETENV",                   cmd_getenv,                   STRIOBJECT,        par_str},
    { "CMD_GET_ATIME",                cmd_get_atime,                VOIDOBJECT,        par_str_8_int_bln},
    { "CMD_GET_ATIME_OF_SYMLINK",     cmd_get_atime_of_symlink,     VOIDOBJECT,        par_str_8_int_bln},
    { "CMD_GET_CTIME",                cmd_get_ctime,                VOIDOBJECT,        par_str_8_int_bln},
    { "CMD_GET_FILE_MODE",            cmd_get_file_mode,            SETOBJECT,         par_str},
    { "CMD_GET_FILE_MODE_OF_SYMLINK", cmd_get_file_mode_of_symlink, SETOBJECT,         par_str},
    { "CMD_GET_GROUP",                cmd_get_group,                STRIOBJECT,        par_str},
    { "CMD_GET_GROUP_OF_SYMLINK",     cmd_get_group_of_symlink,     STRIOBJECT,        par_str},
    { "CMD_GET_MTIME",                cmd_get_mtime,                VOIDOBJECT,        par_str_8_int_bln},
    { "CMD_GET_MTIME_OF_SYMLINK",     cmd_get_mtime_of_symlink,     VOIDOBJECT,        par_str_8_int_bln},
    { "CMD_GET_OWNER",                cmd_get_owner,                STRIOBJECT,        par_str},
    { "CMD_GET_OWNER_OF_SYMLINK",     cmd_get_owner_of_symlink,     STRIOBJECT,        par_str},
    { "CMD_GET_SEARCH_PATH",          cmd_get_search_path,          ARRAYOBJECT,       par_no_args},
    { "CMD_HOME_DIR",                 cmd_home_dir,                 STRIOBJECT,        par_no_args},
    { "CMD_MAKE_DIR",                 cmd_make_dir,                 VOIDOBJECT,        par_str},
    { "CMD_MAKE_LINK",                cmd_make_link,                VOIDOBJECT,        par_str_str},
    { "CMD_MOVE",                     cmd_move,                     VOIDOBJECT,        par_str_str},
    { "CMD_READ_DIR",                 cmd_read_dir,                 ARRAYOBJECT,       par_str},
    { "CMD_READ_LINK",                cmd_read_link,                STRIOBJECT,        par_str},
    { "CMD_READ_LINK_ABSOLUTE",       cmd_read_link_absolute,       STRIOBJECT,        par_str},
    { "CMD_REMOVE_FILE",              cmd_remove_file,              VOIDOBJECT,        par_str},
    { "CMD_REMOVE_TREE",              cmd_remove_tree,              VOIDOBJECT,        par_str},
    { "CMD_SETENV",                   cmd_setenv,                   VOIDOBJECT,        par_str_str},
    { "CMD_SET_ATIME",                cmd_set_atime,                VOIDOBJECT,        par_str_8_int},
    { "CMD_SET_FILE_MODE",            cmd_set_file_mode,            VOIDOBJECT,        par_str_set},
    { "CMD_SET_GROUP",                cmd_set_group,                VOIDOBJECT,        par_str_str},
    { "CMD_SET_GROUP_OF_SYMLINK",     cmd_set_group_of_symlink,     VOIDOBJECT,        par_str_str},
    { "CMD_SET_MTIME",                cmd_set_mtime,                VOIDOBJECT,        par_str_8_int},
    { "CMD_SET_MTIME_OF_SYMLINK",     cmd_set_mtime_of_symlink,     VOIDOBJECT,        par_str_8_int},
    { "CMD_SET_OWNER",                cmd_set_owner,                VOIDOBJECT,        par_str_str},
    { "CMD_SET_OWNER_OF_SYMLINK",     cmd_set_owner_of_symlink,     VOIDOBJECT,        par_str_str},
    { "CMD_SET_SEARCH_PATH",          cmd_set_search_path,          VOIDOBJECT,        par_arr},
    { "CMD_SHELL_COMMAND_LINE",       cmd_shell_command_line,       STRIOBJECT,        par_str_arr_str_str_str},
    { "CMD_SHELL_ESCAPE",             cmd_shell_escape,             STRIOBJECT,        par_str},
    { "CMD_SHELL_EXECUTE",            cmd_shell_execute,            INTOBJECT,         par_str_arr_str_str_str},
    { "CMD_TO_OS_PATH",               cmd_to_os_path,               STRIOBJECT,        par_str},
    { "CMD_UNSETENV",                 cmd_unsetenv,                 VOIDOBJECT,        par_str},

    { "CON_CLEAR",                    con_clear,                    VOIDOBJECT,        par_op_4_int},
    { "CON_COLUMN",                   con_column,                   INTOBJECT,         par_no_args},
    { "CON_CURSOR",                   con_cursor,                   VOIDOBJECT,        par_op_bln},
    { "CON_FLUSH",                    con_flush,                    VOIDOBJECT,        par_no_args},
    { "CON_HEIGHT",                   con_height,                   INTOBJECT,         par_no_args},
    { "CON_H_SCROLL",                 con_h_scroll,                 VOIDOBJECT,        par_op_5_int},
    { "CON_LINE",                     con_line,                     INTOBJECT,         par_no_args},
    { "CON_OPEN",                     con_open,                     VOIDOBJECT,        par_no_args},
    { "CON_SETPOS",                   con_setpos,                   VOIDOBJECT,        par_op_int_int},
    { "CON_V_SCROLL",                 con_v_scroll,                 VOIDOBJECT,        par_op_5_int},
    { "CON_WIDTH",                    con_width,                    INTOBJECT,         par_no_args},
    { "CON_WRITE",                    con_write,                    VOIDOBJECT,        par_op_str},

    { "DCL_ATTR",                     dcl_attr,                     FORMPARAMOBJECT,   par_op_typ},
    { "DCL_CONST",                    dcl_const,                    VOIDOBJECT,        par_op_typ_op_exp_op_exp},
    { "DCL_ELEMENTS",                 dcl_elements,                 REFLISTOBJECT,     par_blo},
    { "DCL_FWD",                      dcl_fwd,                      VOIDOBJECT,        par_op_typ_op_exp_op_op},
    { "DCL_FWDVAR",                   dcl_fwdvar,                   VOIDOBJECT,        par_op_typ_op_exp_op_op},
    { "DCL_GETFUNC",                  dcl_getfunc,                  REFOBJECT,         par_op_any},
    { "DCL_GETOBJ",                   dcl_getobj,                   REFOBJECT,         par_op_any},
    { "DCL_GLOBAL",                   dcl_global,                   VOIDOBJECT,        par_op_prc},
    { "DCL_IN1",                      dcl_in1,                      FORMPARAMOBJECT,   par_op_typ_op},
    { "DCL_IN1VAR",                   dcl_in1var,                   FORMPARAMOBJECT,   par_op_op_typ_op},
    { "DCL_IN2",                      dcl_in2,                      FORMPARAMOBJECT,   par_op_typ_op_exp},
    { "DCL_IN2VAR",                   dcl_in2var,                   FORMPARAMOBJECT,   par_op_op_typ_op_exp},
    { "DCL_INOUT1",                   dcl_inout1,                   FORMPARAMOBJECT,   par_op_typ_op},
    { "DCL_INOUT2",                   dcl_inout2,                   FORMPARAMOBJECT,   par_op_typ_op_exp},
    { "DCL_PARAM_ATTR",               dcl_param_attr,               FORMPARAMOBJECT,   par_op_prm},
    { "DCL_REF1",                     dcl_ref1,                     FORMPARAMOBJECT,   par_op_typ_op},
    { "DCL_REF2",                     dcl_ref2,                     FORMPARAMOBJECT,   par_op_typ_op_exp},
    { "DCL_SYMB",                     dcl_symb,                     FORMPARAMOBJECT,   par_op_any},
    { "DCL_SYNTAX",                   dcl_syntax,                   VOIDOBJECT,        par_op_typ_op_exp_op_exp},
    { "DCL_VAL1",                     dcl_val1,                     FORMPARAMOBJECT,   par_op_typ_op},
    { "DCL_VAL2",                     dcl_val2,                     FORMPARAMOBJECT,   par_op_typ_op_exp},
    { "DCL_VAR",                      dcl_var,                      VOIDOBJECT,        par_op_typ_op_exp_op_exp},

#if WITH_DRAW
    { "DRW_BORDER",                   drw_border,                   ARRAYOBJECT,       par_win},
    { "DRW_CAPTURE",                  drw_capture,                  WINOBJECT,         par_int_int_int_int},
    { "DRW_CLEAR",                    drw_clear,                    VOIDOBJECT,        par_win_int},
    { "DRW_CMP",                      drw_cmp,                      INTOBJECT,         par_win_win},
    { "DRW_CONV_POINT_LIST",          drw_conv_point_list,          ARRAYOBJECT,       par_plt},
    { "DRW_COPYAREA",                 drw_copyarea,                 VOIDOBJECT,        par_win_win_6_int},
    { "DRW_CPY",                      drw_cpy,                      VOIDOBJECT,        par_win_op_win},
    { "DRW_CREATE",                   drw_create,                   VOIDOBJECT,        par_win_op_win},
    { "DRW_DESTR",                    drw_destr,                    VOIDOBJECT,        par_win},
    { "DRW_EMPTY",                    drw_empty,                    WINOBJECT,         par_no_args},
    { "DRW_EQ",                       drw_eq,                       BOOLOBJECT,        par_win_op_win},
    { "DRW_FLUSH",                    drw_flush,                    VOIDOBJECT,        par_no_args},
    { "DRW_FPOLY_LINE",               drw_fpoly_line,               VOIDOBJECT,        par_win_int_int_plt_int},
    { "DRW_GEN_POINT_LIST",           drw_gen_point_list,           POINTLISTOBJECT,   par_arr},
    { "DRW_GET_IMAGE_PIXEL",          drw_get_image_pixel,          INTOBJECT,         par_bst_4_int},
    { "DRW_GET_PIXEL",                drw_get_pixel,                INTOBJECT,         par_win_int_int},
    { "DRW_GET_PIXEL_ARRAY",          drw_get_pixel_array,          ARRAYOBJECT,       par_win},
    { "DRW_GET_PIXEL_DATA",           drw_get_pixel_data,           BSTRIOBJECT,       par_win},
    { "DRW_GET_PIXEL_DATA_FROM_ARRAY",drw_get_pixel_data_from_array,BSTRIOBJECT,       par_arr},
    { "DRW_GET_PIXMAP",               drw_get_pixmap,               WINOBJECT,         par_win_4_int},
    { "DRW_GET_PIXMAP_FROM_PIXELS",   drw_get_pixmap_from_pixels,   WINOBJECT,         par_arr},
    { "DRW_HASHCODE",                 drw_hashcode,                 INTOBJECT,         par_win},
    { "DRW_HEIGHT",                   drw_height,                   INTOBJECT,         par_win},
    { "DRW_NE",                       drw_ne,                       BOOLOBJECT,        par_win_op_win},
    { "DRW_NEW_PIXMAP",               drw_new_pixmap,               WINOBJECT,         par_int_int},
    { "DRW_OPEN",                     drw_open,                     WINOBJECT,         par_4_int_str},
    { "DRW_OPEN_SUB_WINDOW",          drw_open_sub_window,          WINOBJECT,         par_win_4_int},
    { "DRW_PARC",                     drw_parc,                     VOIDOBJECT,        par_win_3_int_flt_flt_int},
    { "DRW_PCIRCLE",                  drw_pcircle,                  VOIDOBJECT,        par_win_4_int},
    { "DRW_PFARC",                    drw_pfarc,                    VOIDOBJECT,        par_win_3_int_flt_flt_int_int},
    { "DRW_PFARCCHORD",               drw_pfarcchord,               VOIDOBJECT,        par_win_3_int_flt_flt_int},
    { "DRW_PFARCPIESLICE",            drw_pfarcpieslice,            VOIDOBJECT,        par_win_3_int_flt_flt_int},
    { "DRW_PFCIRCLE",                 drw_pfcircle,                 VOIDOBJECT,        par_win_4_int},
    { "DRW_PFELLIPSE",                drw_pfellipse,                VOIDOBJECT,        par_win_5_int},
    { "DRW_PIXEL_TO_RGB",             drw_pixel_to_rgb,             VOIDOBJECT,        par_4_int},
    { "DRW_PLINE",                    drw_pline,                    VOIDOBJECT,        par_win_5_int},
    { "DRW_POINTER_XPOS",             drw_pointer_xpos,             INTOBJECT,         par_win},
    { "DRW_POINTER_YPOS",             drw_pointer_ypos,             INTOBJECT,         par_win},
    { "DRW_POLY_LINE",                drw_poly_line,                VOIDOBJECT,        par_win_int_int_plt_int},
    { "DRW_PPOINT",                   drw_ppoint,                   VOIDOBJECT,        par_win_int_int_int},
    { "DRW_PRECT",                    drw_prect,                    VOIDOBJECT,        par_win_5_int},
    { "DRW_PUT",                      drw_put,                      VOIDOBJECT,        par_win_int_int_win},
    { "DRW_PUT_SCALED",               drw_put_scaled,               VOIDOBJECT,        par_win_4_int_win},
    { "DRW_RGB_COLOR",                drw_rgb_color,                INTOBJECT,         par_int_int_int},
    { "DRW_SCREEN_HEIGHT",            drw_screen_height,            INTOBJECT,         par_no_args},
    { "DRW_SCREEN_WIDTH",             drw_screen_width,             INTOBJECT,         par_no_args},
    { "DRW_SET_CONTENT",              drw_set_content,              VOIDOBJECT,        par_win_win},
    { "DRW_SET_CURSOR_VISIBLE",       drw_set_cursor_visible,       VOIDOBJECT,        par_win_bln},
    { "DRW_SET_POINTER_POS",          drw_set_pointer_pos,          VOIDOBJECT,        par_win_int_int},
    { "DRW_SET_POS",                  drw_set_pos,                  VOIDOBJECT,        par_win_int_int},
    { "DRW_SET_SIZE",                 drw_set_size,                 VOIDOBJECT,        par_win_int_int},
    { "DRW_SET_TRANSPARENT_COLOR",    drw_set_transparent_color,    VOIDOBJECT,        par_win_int},
    { "DRW_SET_WINDOW_NAME",          drw_set_window_name,          VOIDOBJECT,        par_win_str},
    { "DRW_TEXT",                     drw_text,                     VOIDOBJECT,        par_win_int_int_str_int_int},
    { "DRW_TO_BOTTOM",                drw_to_bottom,                VOIDOBJECT,        par_win},
    { "DRW_TO_TOP",                   drw_to_top,                   VOIDOBJECT,        par_win},
    { "DRW_VALUE",                    drw_value,                    WINOBJECT,         par_ref},
    { "DRW_WIDTH",                    drw_width,                    INTOBJECT,         par_win},
    { "DRW_XPOS",                     drw_xpos,                     INTOBJECT,         par_win},
    { "DRW_YPOS",                     drw_ypos,                     INTOBJECT,         par_win},
#endif

    { "ENU_CONV",                     enu_conv,                     ENUMOBJECT,        par_op_op_enu},
    { "ENU_CPY",                      enu_cpy,                      VOIDOBJECT,        par_enu_op_enu},
    { "ENU_CREATE",                   enu_create,                   VOIDOBJECT,        par_enu_op_enu},
    { "ENU_EQ",                       enu_eq,                       BOOLOBJECT,        par_enu_op_enu},
    { "ENU_GENLIT",                   enu_genlit,                   VOIDOBJECT,        par_any},
    { "ENU_ICONV2",                   enu_iconv2,                   ILLEGALOBJECT,     par_int_rfl},
    { "ENU_ICONV3_EXCEPT",            enu_iconv3_except,            ILLEGALOBJECT,     par_op_op_int},
    { "ENU_LITERAL",                  enu_literal,                  STRIOBJECT,        par_enu},
    { "ENU_NE",                       enu_ne,                       BOOLOBJECT,        par_enu_op_enu},
    { "ENU_ORD2",                     enu_ord2,                     INTOBJECT,         par_enu_rfl},
    { "ENU_ORD_EXCEPT",               enu_ord_except,               INTOBJECT,         par_enu},
    { "ENU_VALUE",                    enu_value,                    ILLEGALOBJECT,     par_ref},

    { "FIL_BIG_LNG",                  fil_big_lng,                  BIGINTOBJECT,      par_fil},
    { "FIL_BIG_SEEK",                 fil_big_seek,                 VOIDOBJECT,        par_fil_big},
    { "FIL_BIG_TELL",                 fil_big_tell,                 BIGINTOBJECT,      par_fil},
    { "FIL_CLOSE",                    fil_close,                    VOIDOBJECT,        par_fil},
    { "FIL_CPY",                      fil_cpy,                      VOIDOBJECT,        par_fil_op_fil},
    { "FIL_CREATE",                   fil_create,                   VOIDOBJECT,        par_fil_op_fil},
    { "FIL_DESTR",                    fil_destr,                    VOIDOBJECT,        par_fil},
    { "FIL_EMPTY",                    fil_empty,                    FILEOBJECT,        par_no_args},
    { "FIL_EOF",                      fil_eof,                      BOOLOBJECT,        par_fil},
    { "FIL_EQ",                       fil_eq,                       BOOLOBJECT,        par_fil_op_fil},
    { "FIL_ERR",                      fil_err,                      FILEOBJECT,        par_no_args},
    { "FIL_FLUSH",                    fil_flush,                    VOIDOBJECT,        par_fil},
    { "FIL_GETC",                     fil_getc,                     CHAROBJECT,        par_fil},
    { "FIL_GETS",                     fil_gets,                     STRIOBJECT,        par_fil_int},
    { "FIL_HAS_NEXT",                 fil_has_next,                 BOOLOBJECT,        par_fil},
    { "FIL_IN",                       fil_in,                       FILEOBJECT,        par_no_args},
    { "FIL_INPUT_READY",              fil_input_ready,              BOOLOBJECT,        par_fil},
    { "FIL_LINE_READ",                fil_line_read,                STRIOBJECT,        par_fil_chr},
    { "FIL_LITERAL",                  fil_literal,                  STRIOBJECT,        par_fil},
    { "FIL_LNG",                      fil_lng,                      INTOBJECT,         par_fil},
    { "FIL_NE",                       fil_ne,                       BOOLOBJECT,        par_fil_op_fil},
    { "FIL_OPEN",                     fil_open,                     FILEOBJECT,        par_str_str},
    { "FIL_OPEN_NULL_DEVICE",         fil_open_null_device,         FILEOBJECT,        par_no_args},
    { "FIL_OUT",                      fil_out,                      FILEOBJECT,        par_no_args},
    { "FIL_PIPE",                     fil_pipe,                     VOIDOBJECT,        par_fil_fil},
    { "FIL_POPEN",                    fil_popen,                    FILEOBJECT,        par_str_arr_str},
    { "FIL_PRINT",                    fil_print,                    VOIDOBJECT,        par_str},
    { "FIL_SEEK",                     fil_seek,                     VOIDOBJECT,        par_fil_int},
    { "FIL_SEEKABLE",                 fil_seekable,                 BOOLOBJECT,        par_fil},
    { "FIL_TELL",                     fil_tell,                     INTOBJECT,         par_fil},
    { "FIL_TERMINATED_READ",          fil_terminated_read,          STRIOBJECT,        par_fil_chr},
    { "FIL_TRUNCATE",                 fil_truncate,                 VOIDOBJECT,        par_fil_int},
    { "FIL_VALUE",                    fil_value,                    FILEOBJECT,        par_ref},
    { "FIL_WORD_READ",                fil_word_read,                STRIOBJECT,        par_fil_chr},
    { "FIL_WRITE",                    fil_write,                    VOIDOBJECT,        par_fil_str},

#if WITH_FLOAT
    { "FLT_ABS",                      flt_abs,                      FLOATOBJECT,       par_flt},
    { "FLT_ACOS",                     flt_acos,                     FLOATOBJECT,       par_flt},
    { "FLT_ADD",                      flt_add,                      FLOATOBJECT,       par_flt_op_flt},
    { "FLT_ADD_ASSIGN",               flt_add_assign,               VOIDOBJECT,        par_flt_op_flt},
    { "FLT_ASIN",                     flt_asin,                     FLOATOBJECT,       par_flt},
    { "FLT_ATAN",                     flt_atan,                     FLOATOBJECT,       par_flt},
    { "FLT_ATAN2",                    flt_atan2,                    FLOATOBJECT,       par_flt_flt},
    { "FLT_BITS2DOUBLE",              flt_bits2double,              FLOATOBJECT,       par_int},
    { "FLT_BITS2SINGLE",              flt_bits2single,              FLOATOBJECT,       par_int},
    { "FLT_CEIL",                     flt_ceil,                     FLOATOBJECT,       par_flt},
    { "FLT_CMP",                      flt_cmp,                      INTOBJECT,         par_flt_flt},
    { "FLT_COS",                      flt_cos,                      FLOATOBJECT,       par_flt},
    { "FLT_COSH",                     flt_cosh,                     FLOATOBJECT,       par_flt},
    { "FLT_CPY",                      flt_cpy,                      VOIDOBJECT,        par_flt_op_flt},
    { "FLT_CREATE",                   flt_create,                   VOIDOBJECT,        par_flt_op_flt},
    { "FLT_DECOMPOSE",                flt_decompose,                VOIDOBJECT,        par_flt_flt_int},
    { "FLT_DGTS",                     flt_dgts,                     STRIOBJECT,        par_flt_op_int},
    { "FLT_DIV",                      flt_div,                      FLOATOBJECT,       par_flt_op_flt},
    { "FLT_DIV_ASSIGN",               flt_div_assign,               VOIDOBJECT,        par_flt_op_flt},
    { "FLT_DOUBLE2BITS",              flt_double2bits,              INTOBJECT,         par_flt},
    { "FLT_EQ",                       flt_eq,                       BOOLOBJECT,        par_flt_op_flt},
    { "FLT_EXP",                      flt_exp,                      FLOATOBJECT,       par_flt},
    { "FLT_EXPM1",                    flt_expm1,                    FLOATOBJECT,       par_flt},
    { "FLT_FLOOR",                    flt_floor,                    FLOATOBJECT,       par_flt},
    { "FLT_GE",                       flt_ge,                       BOOLOBJECT,        par_flt_op_flt},
    { "FLT_GT",                       flt_gt,                       BOOLOBJECT,        par_flt_op_flt},
    { "FLT_HASHCODE",                 flt_hashcode,                 INTOBJECT,         par_flt},
    { "FLT_ICONV1",                   flt_iconv1,                   FLOATOBJECT,       par_int},
    { "FLT_ICONV3",                   flt_iconv3,                   FLOATOBJECT,       par_op_op_int},
    { "FLT_IPOW",                     flt_ipow,                     FLOATOBJECT,       par_flt_op_int},
    { "FLT_ISNAN",                    flt_isnan,                    BOOLOBJECT,        par_flt},
    { "FLT_ISNEGATIVEZERO",           flt_isnegativezero,           BOOLOBJECT,        par_flt},
    { "FLT_LE",                       flt_le,                       BOOLOBJECT,        par_flt_op_flt},
    { "FLT_LOG",                      flt_log,                      FLOATOBJECT,       par_flt},
    { "FLT_LOG10",                    flt_log10,                    FLOATOBJECT,       par_flt},
    { "FLT_LOG1P",                    flt_log1p,                    FLOATOBJECT,       par_flt},
    { "FLT_LOG2",                     flt_log2,                     FLOATOBJECT,       par_flt},
    { "FLT_LSHIFT",                   flt_lshift,                   FLOATOBJECT,       par_flt_op_int},
    { "FLT_LT",                       flt_lt,                       BOOLOBJECT,        par_flt_op_flt},
    { "FLT_MOD",                      flt_mod,                      FLOATOBJECT,       par_flt_op_flt},
    { "FLT_MULT",                     flt_mult,                     FLOATOBJECT,       par_flt_op_flt},
    { "FLT_MULT_ASSIGN",              flt_mult_assign,              VOIDOBJECT,        par_flt_op_flt},
    { "FLT_NE",                       flt_ne,                       BOOLOBJECT,        par_flt_op_flt},
    { "FLT_NEGATE",                   flt_negate,                   FLOATOBJECT,       par_op_flt},
    { "FLT_PARSE1",                   flt_parse1,                   FLOATOBJECT,       par_str},
    { "FLT_PLUS",                     flt_plus,                     FLOATOBJECT,       par_op_flt},
    { "FLT_POW",                      flt_pow,                      FLOATOBJECT,       par_flt_op_flt},
    { "FLT_RAND",                     flt_rand,                     FLOATOBJECT,       par_flt_flt},
    { "FLT_REM",                      flt_rem,                      FLOATOBJECT,       par_flt_op_flt},
    { "FLT_ROUND",                    flt_round,                    INTOBJECT,         par_flt},
    { "FLT_RSHIFT",                   flt_rshift,                   FLOATOBJECT,       par_flt_op_int},
    { "FLT_SBTR",                     flt_sbtr,                     FLOATOBJECT,       par_flt_op_flt},
    { "FLT_SBTR_ASSIGN",              flt_sbtr_assign,              VOIDOBJECT,        par_flt_op_flt},
    { "FLT_SCI",                      flt_sci,                      STRIOBJECT,        par_flt_op_int},
    { "FLT_SIN",                      flt_sin,                      FLOATOBJECT,       par_flt},
    { "FLT_SINGLE2BITS",              flt_single2bits,              INTOBJECT,         par_flt},
    { "FLT_SINH",                     flt_sinh,                     FLOATOBJECT,       par_flt},
    { "FLT_SQRT",                     flt_sqrt,                     FLOATOBJECT,       par_flt},
    { "FLT_STR",                      flt_str,                      STRIOBJECT,        par_flt},
    { "FLT_STR_SCIENTIFIC",           flt_str_scientific,           STRIOBJECT,        par_flt},
    { "FLT_TAN",                      flt_tan,                      FLOATOBJECT,       par_flt},
    { "FLT_TANH",                     flt_tanh,                     FLOATOBJECT,       par_flt},
    { "FLT_TRUNC",                    flt_trunc,                    INTOBJECT,         par_flt},
    { "FLT_VALUE",                    flt_value,                    FLOATOBJECT,       par_ref},
#endif

    { "GEN_DESTR",                    gen_destr,                    VOIDOBJECT,        par_any},

#if WITH_DRAW
    { "GKB_BUTTON_PRESSED",           gkb_button_pressed,           BOOLOBJECT,        par_op_chr},
    { "GKB_CLICKED_XPOS",             gkb_clicked_xpos,             INTOBJECT,         par_no_args},
    { "GKB_CLICKED_YPOS",             gkb_clicked_ypos,             INTOBJECT,         par_no_args},
    { "GKB_GETC",                     gkb_getc,                     CHAROBJECT,        par_no_args},
    { "GKB_GETS",                     gkb_gets,                     STRIOBJECT,        par_op_int},
    { "GKB_INPUT_READY",              gkb_input_ready,              BOOLOBJECT,        par_no_args},
    { "GKB_LINE_READ",                gkb_line_read,                STRIOBJECT,        par_op_chr},
    { "GKB_RAW_GETC",                 gkb_raw_getc,                 CHAROBJECT,        par_no_args},
    { "GKB_SELECT_INPUT",             gkb_select_input,             VOIDOBJECT,        par_win_chr_bln},
    { "GKB_WINDOW",                   gkb_window,                   WINOBJECT,         par_no_args},
    { "GKB_WORD_READ",                gkb_word_read,                STRIOBJECT,        par_op_chr},
#endif

    { "HSH_CONCAT_KEY_VALUE",         hsh_concat_key_value,         HASHELEMOBJECT,    par_hel_op_hel},
    { "HSH_CONTAINS",                 hsh_contains,                 BOOLOBJECT,        par_hsh_op_int_ref},
    { "HSH_CPY",                      hsh_cpy,                      VOIDOBJECT,        par_hsh_hsh_4_ref},
    { "HSH_CREATE",                   hsh_create,                   VOIDOBJECT,        par_hsh_hsh_4_ref},
    { "HSH_DESTR",                    hsh_destr,                    VOIDOBJECT,        par_hsh_ref_ref},
    { "HSH_EMPTY",                    hsh_empty,                    HASHOBJECT,        par_no_args},
    { "HSH_EXCL",                     hsh_excl,                     VOIDOBJECT,        par_hsh_any_int_ref_ref_ref},
    { "HSH_FOR",                      hsh_for,                      VOIDOBJECT,        par_any_hsh_prc_ref},
    { "HSH_FOR_DATA_KEY",             hsh_for_data_key,             VOIDOBJECT,        par_any_any_hsh_prc_ref_ref},
    { "HSH_FOR_KEY",                  hsh_for_key,                  VOIDOBJECT,        par_any_hsh_prc_ref},
    { "HSH_GEN_HASH",                 hsh_gen_hash,                 HASHOBJECT,        par_hel_4_ref},
    { "HSH_GEN_KEY_VALUE",            hsh_gen_key_value,            HASHELEMOBJECT,    par_op_any_op_any},
    { "HSH_IDX",                      hsh_idx,                      ILLEGALOBJECT,     par_hsh_any_int_ref},
    { "HSH_IDX2",                     hsh_idx2,                     ILLEGALOBJECT,     par_hsh_any_int_any_ref_ref},
    { "HSH_INCL",                     hsh_incl,                     VOIDOBJECT,        par_hsh_any_any_int_4_ref},
    { "HSH_KEYS",                     hsh_keys,                     ARRAYOBJECT,       par_hsh_ref_ref},
    { "HSH_LNG",                      hsh_lng,                      INTOBJECT,         par_hsh},
    { "HSH_RAND_KEY",                 hsh_rand_key,                 ILLEGALOBJECT,     par_hsh},
    { "HSH_UPDATE",                   hsh_update,                   ILLEGALOBJECT,     par_hsh_any_any_int_3_ref},
    { "HSH_VALUES",                   hsh_values,                   ARRAYOBJECT,       par_hsh_ref_ref},

    { "INT_ABS",                      int_abs,                      INTOBJECT,         par_int},
    { "INT_ADD",                      int_add,                      INTOBJECT,         par_int_op_int},
    { "INT_ADD_ASSIGN",               int_add_assign,               VOIDOBJECT,        par_int_op_int},
    { "INT_BINOM",                    int_binom,                    INTOBJECT,         par_int_op_int},
    { "INT_BIT_LENGTH",               int_bit_length,               INTOBJECT,         par_int},
    { "INT_BYTES_BE_2_INT",           int_bytes_be_2_int,           INTOBJECT,         par_str},
    { "INT_BYTES_BE_2_UINT",          int_bytes_be_2_uint,          INTOBJECT,         par_str},
    { "INT_BYTES_BE_SIGNED",          int_bytes_be_signed,          STRIOBJECT,        par_int},
    { "INT_BYTES_BE_UNSIGNED",        int_bytes_be_unsigned,        STRIOBJECT,        par_int},
    { "INT_BYTES_LE_2_INT",           int_bytes_le_2_int,           INTOBJECT,         par_str},
    { "INT_BYTES_LE_2_UINT",          int_bytes_le_2_uint,          INTOBJECT,         par_str},
    { "INT_BYTES_LE_SIGNED",          int_bytes_le_signed,          STRIOBJECT,        par_int},
    { "INT_BYTES_LE_UNSIGNED",        int_bytes_le_unsigned,        STRIOBJECT,        par_int},
    { "INT_CMP",                      int_cmp,                      INTOBJECT,         par_int_int},
    { "INT_CPY",                      int_cpy,                      VOIDOBJECT,        par_int_op_int},
    { "INT_CREATE",                   int_create,                   VOIDOBJECT,        par_int_op_int},
    { "INT_DECR",                     int_decr,                     VOIDOBJECT,        par_int},
    { "INT_DIV",                      int_div,                      INTOBJECT,         par_int_op_int},
    { "INT_EQ",                       int_eq,                       BOOLOBJECT,        par_int_op_int},
    { "INT_FACT",                     int_fact,                     INTOBJECT,         par_op_int},
    { "INT_GE",                       int_ge,                       BOOLOBJECT,        par_int_op_int},
    { "INT_GT",                       int_gt,                       BOOLOBJECT,        par_int_op_int},
    { "INT_HASHCODE",                 int_hashcode,                 INTOBJECT,         par_int},
    { "INT_ICONV1",                   int_iconv1,                   INTOBJECT,         par_int},
    { "INT_ICONV3",                   int_iconv3,                   INTOBJECT,         par_op_op_int},
    { "INT_INCR",                     int_incr,                     VOIDOBJECT,        par_int},
    { "INT_LE",                       int_le,                       BOOLOBJECT,        par_int_op_int},
    { "INT_LOG10",                    int_log10,                    INTOBJECT,         par_int},
    { "INT_LOG2",                     int_log2,                     INTOBJECT,         par_int},
    { "INT_LOWEST_SET_BIT",           int_lowest_set_bit,           INTOBJECT,         par_int},
    { "INT_LPAD0",                    int_lpad0,                    STRIOBJECT,        par_int_op_int},
    { "INT_LSHIFT",                   int_lshift,                   INTOBJECT,         par_int_op_int},
    { "INT_LSHIFT_ASSIGN",            int_lshift_assign,            VOIDOBJECT,        par_int_op_int},
    { "INT_LT",                       int_lt,                       BOOLOBJECT,        par_int_op_int},
    { "INT_MDIV",                     int_mdiv,                     INTOBJECT,         par_int_op_int},
    { "INT_MOD",                      int_mod,                      INTOBJECT,         par_int_op_int},
    { "INT_MULT",                     int_mult,                     INTOBJECT,         par_int_op_int},
    { "INT_MULT_ASSIGN",              int_mult_assign,              VOIDOBJECT,        par_int_op_int},
    { "INT_NE",                       int_ne,                       BOOLOBJECT,        par_int_op_int},
    { "INT_NEGATE",                   int_negate,                   INTOBJECT,         par_op_int},
    { "INT_N_BYTES_BE_SIGNED",        int_n_bytes_be_signed,        STRIOBJECT,        par_int_op_op_int},
    { "INT_N_BYTES_BE_UNSIGNED",      int_n_bytes_be_unsigned,      STRIOBJECT,        par_int_op_op_int},
    { "INT_N_BYTES_LE_SIGNED",        int_n_bytes_le_signed,        STRIOBJECT,        par_int_op_op_int},
    { "INT_N_BYTES_LE_UNSIGNED",      int_n_bytes_le_unsigned,      STRIOBJECT,        par_int_op_op_int},
    { "INT_ODD",                      int_odd,                      BOOLOBJECT,        par_int},
    { "INT_PARSE1",                   int_parse1,                   INTOBJECT,         par_str},
    { "INT_PLUS",                     int_plus,                     INTOBJECT,         par_op_int},
    { "INT_POW",                      int_pow,                      INTOBJECT,         par_int_op_int},
    { "INT_PRED",                     int_pred,                     INTOBJECT,         par_int},
    { "INT_RADIX",                    int_RADIX,                    STRIOBJECT,        par_int_op_int},
    { "INT_RAND",                     int_rand,                     INTOBJECT,         par_int_int},
    { "INT_REM",                      int_rem,                      INTOBJECT,         par_int_op_int},
    { "INT_RSHIFT",                   int_rshift,                   INTOBJECT,         par_int_op_int},
    { "INT_RSHIFT_ASSIGN",            int_rshift_assign,            VOIDOBJECT,        par_int_op_int},
    { "INT_SBTR",                     int_sbtr,                     INTOBJECT,         par_int_op_int},
    { "INT_SBTR_ASSIGN",              int_sbtr_assign,              VOIDOBJECT,        par_int_op_int},
    { "INT_SQRT",                     int_sqrt,                     INTOBJECT,         par_int},
    { "INT_STR",                      int_str,                      STRIOBJECT,        par_int},
    { "INT_SUCC",                     int_succ,                     INTOBJECT,         par_int},
    { "INT_VALUE",                    int_value,                    INTOBJECT,         par_ref},
    { "INT_radix",                    int_radix,                    STRIOBJECT,        par_int_op_int},

    { "ITF_CMP",                      itf_cmp,                      INTOBJECT,         par_itf_itf},
    { "ITF_CONV2",                    itf_conv2,                    INTERFACEOBJECT,   par_op_op_sct},
    { "ITF_CPY",                      itf_cpy,                      VOIDOBJECT,        par_itf_op_itf},
    { "ITF_CPY2",                     itf_cpy2,                     VOIDOBJECT,        par_itf_op_sct},
    { "ITF_CREATE",                   itf_create,                   VOIDOBJECT,        par_itf_op_itf},
    { "ITF_CREATE2",                  itf_create2,                  VOIDOBJECT,        par_itf_op_sct},
    { "ITF_DESTR",                    itf_destr,                    VOIDOBJECT,        par_itf},
    { "ITF_EQ",                       itf_eq,                       BOOLOBJECT,        par_itf_op_itf},
    { "ITF_HASHCODE",                 itf_hashcode,                 INTOBJECT,         par_itf},
    { "ITF_NE",                       itf_ne,                       BOOLOBJECT,        par_itf_op_itf},
    { "ITF_SELECT",                   itf_select,                   ILLEGALOBJECT,     par_sct_op_op},
    { "ITF_TO_INTERFACE",             itf_to_interface,             INTERFACEOBJECT,   par_sct},

    { "KBD_GETC",                     kbd_getc,                     CHAROBJECT,        par_no_args},
    { "KBD_GETS",                     kbd_gets,                     STRIOBJECT,        par_op_int},
    { "KBD_INPUT_READY",              kbd_input_ready,              BOOLOBJECT,        par_no_args},
    { "KBD_LINE_READ",                kbd_line_read,                STRIOBJECT,        par_op_chr},
    { "KBD_RAW_GETC",                 kbd_raw_getc,                 CHAROBJECT,        par_no_args},
    { "KBD_WORD_READ",                kbd_word_read,                STRIOBJECT,        par_op_chr},

    { "PCS_CHILD_STDERR",             pcs_child_stderr,             FILEOBJECT,        par_pcs},
    { "PCS_CHILD_STDIN",              pcs_child_stdin,              FILEOBJECT,        par_pcs},
    { "PCS_CHILD_STDOUT",             pcs_child_stdout,             FILEOBJECT,        par_pcs},
    { "PCS_CMP",                      pcs_cmp,                      INTOBJECT,         par_pcs_pcs},
    { "PCS_CPY",                      pcs_cpy,                      VOIDOBJECT,        par_pcs_op_pcs},
    { "PCS_CREATE",                   pcs_create,                   VOIDOBJECT,        par_pcs_op_pcs},
    { "PCS_DESTR",                    pcs_destr,                    VOIDOBJECT,        par_pcs},
    { "PCS_EMPTY",                    pcs_empty,                    PROCESSOBJECT,     par_no_args},
    { "PCS_EQ",                       pcs_eq,                       BOOLOBJECT,        par_pcs_op_pcs},
    { "PCS_EXIT_VALUE",               pcs_exit_value,               INTOBJECT,         par_pcs},
    { "PCS_HASHCODE",                 pcs_hashcode,                 INTOBJECT,         par_pcs},
    { "PCS_IS_ALIVE",                 pcs_is_alive,                 BOOLOBJECT,        par_pcs},
    { "PCS_KILL",                     pcs_kill,                     VOIDOBJECT,        par_pcs},
    { "PCS_NE",                       pcs_ne,                       BOOLOBJECT,        par_pcs_op_pcs},
    { "PCS_PIPE2",                    pcs_pipe2,                    VOIDOBJECT,        par_str_arr_fil_fil},
    { "PCS_PTY",                      pcs_pty,                      VOIDOBJECT,        par_str_arr_fil_fil},
    { "PCS_START",                    pcs_start,                    PROCESSOBJECT,     par_str_arr_fil_fil_fil},
    { "PCS_START_PIPE",               pcs_start_pipe,               PROCESSOBJECT,     par_str_arr},
    { "PCS_STR",                      pcs_str,                      STRIOBJECT,        par_pcs},
    { "PCS_VALUE",                    pcs_value,                    PROCESSOBJECT,     par_ref},
    { "PCS_WAIT_FOR",                 pcs_wait_for,                 VOIDOBJECT,        par_pcs},

#if WITH_DRAW
    { "PLT_BSTRING",                  plt_bstring,                  BSTRIOBJECT,       par_plt},
    { "PLT_CMP",                      plt_cmp,                      INTOBJECT,         par_plt_plt},
    { "PLT_CPY",                      plt_cpy,                      VOIDOBJECT,        par_plt_op_plt},
    { "PLT_CREATE",                   plt_create,                   VOIDOBJECT,        par_plt_op_plt},
    { "PLT_DESTR",                    plt_destr,                    VOIDOBJECT,        par_plt},
    { "PLT_EMPTY",                    plt_empty,                    POINTLISTOBJECT,   par_no_args},
    { "PLT_EQ",                       plt_eq,                       BOOLOBJECT,        par_plt_op_plt},
    { "PLT_HASHCODE",                 plt_hashcode,                 INTOBJECT,         par_plt},
    { "PLT_NE",                       plt_ne,                       BOOLOBJECT,        par_plt_op_plt},
    { "PLT_POINT_LIST",               plt_point_list,               POINTLISTOBJECT,   par_bst},
    { "PLT_VALUE",                    plt_value,                    POINTLISTOBJECT,   par_ref},
#endif

    { "POL_ADD_CHECK",                pol_add_check,                VOIDOBJECT,        par_pol_soc_int_itf},
    { "POL_CLEAR",                    pol_clear,                    VOIDOBJECT,        par_pol},
    { "POL_CPY",                      pol_cpy,                      VOIDOBJECT,        par_pol_op_pol},
    { "POL_CREATE",                   pol_create,                   VOIDOBJECT,        par_pol_op_pol},
    { "POL_DESTR",                    pol_destr,                    VOIDOBJECT,        par_pol},
    { "POL_EMPTY",                    pol_empty,                    POLLOBJECT,        par_no_args},
    { "POL_GET_CHECK",                pol_get_check,                INTOBJECT,         par_pol_soc},
    { "POL_GET_FINDING",              pol_get_finding,              INTOBJECT,         par_pol_soc},
    { "POL_HAS_NEXT",                 pol_has_next,                 BOOLOBJECT,        par_pol},
    { "POL_ITER_CHECKS",              pol_iter_checks,              VOIDOBJECT,        par_pol_int},
    { "POL_ITER_FINDINGS",            pol_iter_findings,            VOIDOBJECT,        par_pol_int},
    { "POL_NEXT_FILE",                pol_next_file,                INTERFACEOBJECT,   par_pol_itf},
    { "POL_POLL",                     pol_poll,                     VOIDOBJECT,        par_pol},
    { "POL_REMOVE_CHECK",             pol_remove_check,             VOIDOBJECT,        par_pol_soc_int},
    { "POL_VALUE",                    pol_value,                    POLLOBJECT,        par_ref},

    { "PRC_ARGS",                     prc_args,                     ARRAYOBJECT,       par_no_args},
    { "PRC_ASSERT",                   prc_assert,                   VOIDOBJECT,        par_op_bln},
    { "PRC_BEGIN",                    prc_begin,                    BLOCKOBJECT,       par_op_op_exp},
    { "PRC_BEGIN_NOOP",               prc_begin_noop,               BLOCKOBJECT,       par_no_args},
    { "PRC_BLOCK",                    prc_block,                    VOIDOBJECT,        par_op_prc_op_prc},
    { "PRC_BLOCK_CATCH_ALL",          prc_block_catch_all,          VOIDOBJECT,        par_op_prc_op_op_op_prc},
    { "PRC_BLOCK_OTHERWISE",          prc_block_otherwise,          VOIDOBJECT,        par_op_prc_op_prc_op_op_prc},
    { "PRC_CASE",                     prc_case,                     VOIDOBJECT,        par_op_any_op_prc},
    { "PRC_CASE_DEF",                 prc_case_def,                 VOIDOBJECT,        par_op_any_op_prc_op_op_prc},
    { "PRC_CASE_HASHSET",             prc_case_hashset,             VOIDOBJECT,        par_op_any_op_prc},
    { "PRC_CASE_HASHSET_DEF",         prc_case_hashset_def,         VOIDOBJECT,        par_op_any_op_prc_op_op_prc},
    { "PRC_CPY",                      prc_cpy,                      VOIDOBJECT,        par_prc_op_prc},
    { "PRC_CREATE",                   prc_create,                   VOIDOBJECT,        par_prc_op_prc},
    { "PRC_DESTR",                    prc_destr,                    VOIDOBJECT,        par_prc},
    { "PRC_DYNAMIC",                  prc_dynamic,                  ILLEGALOBJECT,     par_no_check},
    { "PRC_EXIT",                     prc_exit,                     VOIDOBJECT,        par_int},
    { "PRC_FOR_DOWNTO",               prc_for_downto,               VOIDOBJECT,        par_op_int_op_int_op_int_op_prc},
    { "PRC_FOR_DOWNTO_STEP",          prc_for_downto_step,          VOIDOBJECT,        par_op_int_op_int_op_int_op_int_op_prc},
    { "PRC_FOR_TO",                   prc_for_to,                   VOIDOBJECT,        par_op_int_op_int_op_int_op_prc},
    { "PRC_FOR_TO_STEP",              prc_for_to_step,              VOIDOBJECT,        par_op_int_op_int_op_int_op_int_op_prc},
    { "PRC_GET_RUN_ERROR",            prc_get_run_error,            VOIDOBJECT,        par_int_str_int},
    { "PRC_HEAPSTAT",                 prc_heapstat,                 VOIDOBJECT,        par_no_args},
    { "PRC_HSIZE",                    prc_hsize,                    INTOBJECT,         par_no_args},
    { "PRC_IF",                       prc_if,                       VOIDOBJECT,        par_op_bln_op_prc},
    { "PRC_IF_ELSIF",                 prc_if_elsif,                 VOIDOBJECT,        par_op_bln_op_prc_prc},
    { "PRC_IF_NOOP",                  prc_if_noop,                  VOIDOBJECT,        par_op_bln_op_prc},
    { "PRC_INCLUDE",                  prc_include,                  VOIDOBJECT,        par_op_str},
    { "PRC_LINE",                     prc_line,                     INTOBJECT,         par_no_args},
    { "PRC_LOCAL",                    prc_local,                    BLOCKOBJECT,       par_op_op_prc_op_exp},
    { "PRC_NOOP",                     prc_noop,                     VOIDOBJECT,        par_no_args},
    { "PRC_RAISE",                    prc_raise,                    VOIDOBJECT,        par_op_enu},
    { "PRC_REPEAT",                   prc_repeat,                   VOIDOBJECT,        par_op_prc_op_bln},
    { "PRC_REPEAT_NOOP",              prc_repeat_noop,              VOIDOBJECT,        par_op_op_bln},
    { "PRC_RES_BEGIN",                prc_res_begin,                BLOCKOBJECT,       par_op_op_op_typ_op_exp_op_any_op_exp},
    { "PRC_RES_LOCAL",                prc_res_local,                BLOCKOBJECT,       par_op_op_op_typ_op_exp_op_any_op_prc_op_exp},
    { "PRC_RETURN",                   prc_return,                   BLOCKOBJECT,       par_op_any},
    { "PRC_RETURN2",                  prc_return2,                  BLOCKOBJECT,       par_typ_op_any},
    { "PRC_RETURN_VAR",               prc_return_var,               BLOCKOBJECT,       par_op_op_any},
    { "PRC_RETURN_VAR2",              prc_return_var2,              BLOCKOBJECT,       par_typ_op_op_any},
    { "PRC_SEMICOLON",                prc_semicolon,                VOIDOBJECT,        par_voi_op_voi},
    { "PRC_SETTRACE",                 prc_settrace,                 VOIDOBJECT,        par_str},
    { "PRC_TRACE",                    prc_trace,                    VOIDOBJECT,        par_no_check},
    { "PRC_WHILE",                    prc_while,                    VOIDOBJECT,        par_op_bln_op_prc},
    { "PRC_WHILE_NOOP",               prc_while_noop,               VOIDOBJECT,        par_op_bln},

    { "PRG_BSTRI_PARSE",              prg_bstri_parse,              PROGOBJECT,        par_bst_set_arr_fil},
    { "PRG_CMP",                      prg_cmp,                      INTOBJECT,         par_prg_prg},
    { "PRG_CPY",                      prg_cpy,                      VOIDOBJECT,        par_prg_op_prg},
    { "PRG_CREATE",                   prg_create,                   VOIDOBJECT,        par_prg_op_prg},
    { "PRG_DESTR",                    prg_destr,                    VOIDOBJECT,        par_prg},
    { "PRG_EMPTY",                    prg_empty,                    PROGOBJECT,        par_no_args},
    { "PRG_EQ",                       prg_eq,                       BOOLOBJECT,        par_prg_op_prg},
    { "PRG_ERROR_COUNT",              prg_error_count,              INTOBJECT,         par_prg},
    { "PRG_EVAL",                     prg_eval,                     REFOBJECT,         par_prg_ref},
    { "PRG_EVAL_WITH_ARGS",           prg_eval_with_args,           REFOBJECT,         par_prg_ref_rfl},
    { "PRG_EXEC",                     prg_exec,                     VOIDOBJECT,        par_prg_arr_set_str},
    { "PRG_FIL_PARSE",                prg_fil_parse,                PROGOBJECT,        par_str_set_arr_fil},
    { "PRG_GET_ERROR",                prg_get_error,                VOIDOBJECT,        par_prg_int_int_str_int_int_str_str},
    { "PRG_GLOBAL_OBJECTS",           prg_global_objects,           REFLISTOBJECT,     par_prg},
    { "PRG_HASHCODE",                 prg_hashcode,                 INTOBJECT,         par_prg},
    { "PRG_MATCH",                    prg_match,                    REFOBJECT,         par_prg_rfl},
    { "PRG_MATCH_EXPR",               prg_match_expr,               REFOBJECT,         par_prg_rfl},
    { "PRG_NAME",                     prg_name,                     STRIOBJECT,        par_prg},
    { "PRG_NE",                       prg_ne,                       BOOLOBJECT,        par_prg_op_prg},
    { "PRG_OWN_NAME",                 prg_own_name,                 STRIOBJECT,        par_no_args},
    { "PRG_OWN_PATH",                 prg_own_path,                 STRIOBJECT,        par_no_args},
    { "PRG_PATH",                     prg_path,                     STRIOBJECT,        par_prg},
    { "PRG_STRUCT_SYMBOLS",           prg_struct_symbols,           REFLISTOBJECT,     par_prg},
    { "PRG_STR_PARSE",                prg_str_parse,                PROGOBJECT,        par_str_set_arr_fil},
    { "PRG_SYOBJECT",                 prg_syobject,                 REFOBJECT,         par_prg_str},
    { "PRG_SYSVAR",                   prg_sysvar,                   REFOBJECT,         par_prg_str},
    { "PRG_VALUE",                    prg_value,                    PROGOBJECT,        par_ref},

#if WITH_REFERENCE
    { "REF_ALLOC_INT",                ref_alloc_int,                REFOBJECT,         par_bln_typ_int},
    { "REF_ALLOC_LIST",               ref_alloc_list,               REFOBJECT,         par_int_ref},
    { "REF_ALLOC_REF",                ref_alloc_ref,                REFOBJECT,         par_int_ref},
    { "REF_ALLOC_STRI",               ref_alloc_stri,               REFOBJECT,         par_bln_typ_str},
    { "REF_APPEND_PARAMS",            ref_append_params,            VOIDOBJECT,        par_ref_rfl},
    { "REF_ARRMAXIDX",                ref_arrmaxidx,                INTOBJECT,         par_ref},
    { "REF_ARRMINIDX",                ref_arrminidx,                INTOBJECT,         par_ref},
    { "REF_ARRTOLIST",                ref_arrtolist,                REFLISTOBJECT,     par_ref},
    { "REF_BODY",                     ref_body,                     REFOBJECT,         par_ref},
    { "REF_CATEGORY",                 ref_category,                 INTOBJECT,         par_ref},
    { "REF_CAT_PARSE",                ref_cat_parse,                INTOBJECT,         par_str},
    { "REF_CAT_STR",                  ref_cat_str,                  STRIOBJECT,        par_int},
    { "REF_CMP",                      ref_cmp,                      INTOBJECT,         par_ref_ref},
    { "REF_CPY",                      ref_cpy,                      VOIDOBJECT,        par_ref_op_ref},
    { "REF_CREATE",                   ref_create,                   VOIDOBJECT,        par_ref_op_ref},
    { "REF_EQ",                       ref_eq,                       BOOLOBJECT,        par_ref_op_ref},
    { "REF_FILE",                     ref_file,                     STRIOBJECT,        par_ref},
    { "REF_GETREF",                   ref_getref,                   REFOBJECT,         par_any},
    { "REF_HASHCODE",                 ref_hashcode,                 INTOBJECT,         par_ref},
    { "REF_HSHDATATOLIST",            ref_hshdatatolist,            REFLISTOBJECT,     par_ref},
    { "REF_HSHKEYSTOLIST",            ref_hshkeystolist,            REFLISTOBJECT,     par_ref},
    { "REF_HSHLENGTH",                ref_hshlength,                INTOBJECT,         par_ref},
    { "REF_ISSYMB",                   ref_issymb,                   BOOLOBJECT,        par_ref},
    { "REF_ISTEMP",                   ref_istemp,                   BOOLOBJECT,        par_ref},
    { "REF_ISVAR",                    ref_isvar,                    BOOLOBJECT,        par_ref},
    { "REF_ITFTOSCT",                 ref_itftosct,                 REFOBJECT,         par_ref},
    { "REF_LINE",                     ref_line,                     INTOBJECT,         par_ref},
    { "REF_LOCAL_CONSTS",             ref_local_consts,             REFLISTOBJECT,     par_ref},
    { "REF_LOCAL_VARS",               ref_local_vars,               REFLISTOBJECT,     par_ref},
    { "REF_NE",                       ref_ne,                       BOOLOBJECT,        par_ref_op_ref},
    { "REF_NIL",                      ref_nil,                      REFOBJECT,         par_no_args},
    { "REF_NUM",                      ref_num,                      INTOBJECT,         par_ref},
    { "REF_PARAMS",                   ref_params,                   REFLISTOBJECT,     par_ref},
    { "REF_PATH",                     ref_path,                     STRIOBJECT,        par_ref},
    { "REF_RESINI",                   ref_resini,                   REFOBJECT,         par_ref},
    { "REF_RESULT",                   ref_result,                   REFOBJECT,         par_ref},
    { "REF_SCTTOLIST",                ref_scttolist,                REFLISTOBJECT,     par_ref},
    { "REF_SETCATEGORY",              ref_setcategory,              VOIDOBJECT,        par_ref_int},
    { "REF_SETPARAMS",                ref_setparams,                VOIDOBJECT,        par_ref_rfl},
    { "REF_SETTYPE",                  ref_settype,                  VOIDOBJECT,        par_ref_typ},
    { "REF_SETVAR",                   ref_setvar,                   VOIDOBJECT,        par_ref_bln},
    { "REF_STR",                      ref_str,                      STRIOBJECT,        par_ref},
    { "REF_SYMB",                     ref_symb,                     FORMPARAMOBJECT,   par_op_ref},
    { "REF_TRACE",                    ref_trace,                    VOIDOBJECT,        par_any},
    { "REF_TYPE",                     ref_type,                     TYPEOBJECT,        par_ref},
    { "REF_VALUE",                    ref_value,                    REFOBJECT,         par_ref},

    { "RFL_APPEND",                   rfl_append,                   VOIDOBJECT,        par_rfl_op_rfl},
    { "RFL_CAT",                      rfl_cat,                      REFLISTOBJECT,     par_rfl_op_rfl},
    { "RFL_CPY",                      rfl_cpy,                      VOIDOBJECT,        par_rfl_op_rfl},
    { "RFL_CREATE",                   rfl_create,                   VOIDOBJECT,        par_rfl_op_rfl},
    { "RFL_DESTR",                    rfl_destr,                    VOIDOBJECT,        par_rfl},
    { "RFL_ELEM",                     rfl_elem,                     BOOLOBJECT,        par_ref_op_rfl},
    { "RFL_ELEMCPY",                  rfl_elemcpy,                  VOIDOBJECT,        par_rfl_op_op_int_op_ref},
    { "RFL_EMPTY",                    rfl_empty,                    REFLISTOBJECT,     par_no_args},
    { "RFL_EQ",                       rfl_eq,                       BOOLOBJECT,        par_rfl_op_rfl},
    { "RFL_EXCL",                     rfl_excl,                     VOIDOBJECT,        par_rfl_ref},
    { "RFL_EXPR",                     rfl_expr,                     REFLISTOBJECT,     par_exp},
    { "RFL_FOR",                      rfl_for,                      VOIDOBJECT,        par_op_ref_op_rfl_op_prc},
    { "RFL_FOR_UNTIL",                rfl_for_until,                VOIDOBJECT,        par_op_ref_op_rfl_op_bln_op_prc},
    { "RFL_FOR_VAR_KEY",              rfl_for_var_key,              VOIDOBJECT,        par_op_ref_op_int_op_rfl_op_prc},
    { "RFL_HEAD",                     rfl_head,                     REFLISTOBJECT,     par_rfl_op_op_int},
    { "RFL_IDX",                      rfl_idx,                      REFOBJECT,         par_rfl_op_int},
    { "RFL_INCL",                     rfl_incl,                     VOIDOBJECT,        par_rfl_ref},
    { "RFL_IPOS",                     rfl_ipos,                     INTOBJECT,         par_rfl_ref},
    { "RFL_LNG",                      rfl_lng,                      INTOBJECT,         par_rfl},
    { "RFL_MKLIST",                   rfl_mklist,                   REFLISTOBJECT,     par_ref},
    { "RFL_NE",                       rfl_ne,                       BOOLOBJECT,        par_rfl_op_rfl},
    { "RFL_NOT_ELEM",                 rfl_not_elem,                 BOOLOBJECT,        par_ref_op_op_rfl},
    { "RFL_POS",                      rfl_pos,                      INTOBJECT,         par_rfl_ref},
    { "RFL_RANGE",                    rfl_range,                    REFLISTOBJECT,     par_rfl_op_int_op_int},
    { "RFL_SET_VALUE",                rfl_set_value,                VOIDOBJECT,        par_ref_rfl},
    { "RFL_TAIL",                     rfl_tail,                     REFLISTOBJECT,     par_rfl_op_int},
    { "RFL_TRACE",                    rfl_trace,                    VOIDOBJECT,        par_rfl},
    { "RFL_VALUE",                    rfl_value,                    REFLISTOBJECT,     par_ref},
#endif

    { "SCT_CAT",                      sct_cat,                      STRUCTOBJECT,      par_sct_op_sct},
    { "SCT_CONV",                     sct_conv,                     STRUCTOBJECT,      par_op_op_sct},
    { "SCT_CPY",                      sct_cpy,                      VOIDOBJECT,        par_sct_op_sct},
    { "SCT_CREATE",                   sct_create,                   VOIDOBJECT,        par_sct_op_sct},
    { "SCT_DESTR",                    sct_destr,                    VOIDOBJECT,        par_sct},
    { "SCT_EMPTY",                    sct_empty,                    STRUCTOBJECT,      par_no_args},
    { "SCT_INCL",                     sct_incl,                     VOIDOBJECT,        par_sct_ref},
    { "SCT_LNG",                      sct_lng,                      INTOBJECT,         par_sct},
    { "SCT_SELECT",                   sct_select,                   ILLEGALOBJECT,     par_sct_op_op},

    { "SEL_CMP",                      sel_cmp,                      INTOBJECT,         par_sel_sel},
    { "SEL_CONV1",                    sel_conv1,                    STRUCTELEMOBJECT,  par_ref},
    { "SEL_CPY",                      sel_cpy,                      VOIDOBJECT,        par_sel_op_sel},
    { "SEL_CREATE",                   sel_create,                   VOIDOBJECT,        par_sel_op_sel},
    { "SEL_EQ",                       sel_eq,                       BOOLOBJECT,        par_sel_op_sel},
    { "SEL_HASHCODE",                 sel_hashcode,                 INTOBJECT,         par_sel},
    { "SEL_NAME",                     sel_name,                     STRIOBJECT,        par_sel},
    { "SEL_NE",                       sel_ne,                       BOOLOBJECT,        par_sel_op_sel},
    { "SEL_SYMB",                     sel_symb,                     FORMPARAMOBJECT,   par_op_sel},
    { "SEL_TYPE",                     sel_type,                     TYPEOBJECT,        par_sel},

    { "SET_ARRLIT",                   set_arrlit,                   SETOBJECT,         par_op_arr},
    { "SET_BASELIT",                  set_baselit,                  SETOBJECT,         par_op_int},
    { "SET_CARD",                     set_card,                     INTOBJECT,         par_set},
    { "SET_CMP",                      set_cmp,                      INTOBJECT,         par_set_set},
    { "SET_CONV1",                    set_conv1,                    SETOBJECT,         par_set},
    { "SET_CONV3",                    set_conv3,                    SETOBJECT,         par_op_op_set},
    { "SET_CPY",                      set_cpy,                      VOIDOBJECT,        par_set_op_set},
    { "SET_CREATE",                   set_create,                   VOIDOBJECT,        par_set_op_set},
    { "SET_DESTR",                    set_destr,                    VOIDOBJECT,        par_set},
    { "SET_DIFF",                     set_diff,                     SETOBJECT,         par_set_op_set},
    { "SET_DIFF_ASSIGN",              set_diff_assign,              VOIDOBJECT,        par_set_op_set},
    { "SET_ELEM",                     set_elem,                     BOOLOBJECT,        par_int_op_set},
    { "SET_EMPTY",                    set_empty,                    SETOBJECT,         par_no_args},
    { "SET_EQ",                       set_eq,                       BOOLOBJECT,        par_set_op_set},
    { "SET_EXCL",                     set_excl,                     VOIDOBJECT,        par_set_int},
    { "SET_GE",                       set_ge,                       BOOLOBJECT,        par_set_op_set},
    { "SET_GT",                       set_gt,                       BOOLOBJECT,        par_set_op_set},
    { "SET_HASHCODE",                 set_hashcode,                 INTOBJECT,         par_set},
    { "SET_ICONV1",                   set_iconv1,                   SETOBJECT,         par_int},
    { "SET_ICONV3",                   set_iconv3,                   SETOBJECT,         par_op_op_int},
    { "SET_INCL",                     set_incl,                     VOIDOBJECT,        par_set_int},
    { "SET_INTERSECT",                set_intersect,                SETOBJECT,         par_set_op_set},
    { "SET_INTERSECT_ASSIGN",         set_intersect_assign,         VOIDOBJECT,        par_set_op_set},
    { "SET_LE",                       set_le,                       BOOLOBJECT,        par_set_op_set},
    { "SET_LT",                       set_lt,                       BOOLOBJECT,        par_set_op_set},
    { "SET_MAX",                      set_max,                      INTOBJECT,         par_set},
    { "SET_MIN",                      set_min,                      INTOBJECT,         par_set},
    { "SET_NE",                       set_ne,                       BOOLOBJECT,        par_set_op_set},
    { "SET_NEXT",                     set_next,                     INTOBJECT,         par_set_int},
    { "SET_NOT_ELEM",                 set_not_elem,                 BOOLOBJECT,        par_int_op_op_set},
    { "SET_RAND",                     set_rand,                     INTOBJECT,         par_set},
    { "SET_RANGELIT",                 set_rangelit,                 SETOBJECT,         par_op_int_op_int},
    { "SET_SCONV1",                   set_sconv1,                   INTOBJECT,         par_set},
    { "SET_SCONV3",                   set_sconv3,                   INTOBJECT,         par_op_op_set},
    { "SET_SYMDIFF",                  set_symdiff,                  SETOBJECT,         par_set_op_set},
    { "SET_UNION",                    set_union,                    SETOBJECT,         par_set_op_set},
    { "SET_UNION_ASSIGN",             set_union_assign,             VOIDOBJECT,        par_set_op_set},
    { "SET_VALUE",                    set_value,                    SETOBJECT,         par_ref},

    { "SOC_ACCEPT",                   soc_accept,                   SOCKETOBJECT,      par_soc_bst},
    { "SOC_ADDR_FAMILY",              soc_addr_family,              INTOBJECT,         par_bst},
    { "SOC_ADDR_NUMERIC",             soc_addr_numeric,             STRIOBJECT,        par_bst},
    { "SOC_ADDR_SERVICE",             soc_addr_service,             STRIOBJECT,        par_bst},
    { "SOC_BIND",                     soc_bind,                     VOIDOBJECT,        par_soc_bst},
    { "SOC_CLOSE",                    soc_close,                    VOIDOBJECT,        par_soc},
    { "SOC_CONNECT",                  soc_connect,                  VOIDOBJECT,        par_soc_bst},
    { "SOC_CPY",                      soc_cpy,                      VOIDOBJECT,        par_soc_op_soc},
    { "SOC_CREATE",                   soc_create,                   VOIDOBJECT,        par_soc_op_soc},
    { "SOC_DESTR",                    soc_destr,                    VOIDOBJECT,        par_soc},
    { "SOC_EMPTY",                    soc_empty,                    SOCKETOBJECT,      par_no_args},
    { "SOC_EQ",                       soc_eq,                       BOOLOBJECT,        par_soc_op_soc},
    { "SOC_GETC",                     soc_getc,                     CHAROBJECT,        par_soc_chr},
    { "SOC_GETS",                     soc_gets,                     STRIOBJECT,        par_soc_int_chr},
    { "SOC_GET_HOSTNAME",             soc_get_hostname,             STRIOBJECT,        par_no_args},
    { "SOC_GET_LOCAL_ADDR",           soc_get_local_addr,           BSTRIOBJECT,       par_soc},
    { "SOC_GET_PEER_ADDR",            soc_get_peer_addr,            BSTRIOBJECT,       par_soc},
    { "SOC_HAS_NEXT",                 soc_has_next,                 BOOLOBJECT,        par_soc},
    { "SOC_INET_ADDR",                soc_inet_addr,                BSTRIOBJECT,       par_str_int},
    { "SOC_INET_LOCAL_ADDR",          soc_inet_local_addr,          BSTRIOBJECT,       par_int},
    { "SOC_INET_SERV_ADDR",           soc_inet_serv_addr,           BSTRIOBJECT,       par_int},
    { "SOC_INPUT_READY",              soc_input_ready,              BOOLOBJECT,        par_soc_int_int},
    { "SOC_LINE_READ",                soc_line_read,                STRIOBJECT,        par_soc_chr},
    { "SOC_LISTEN",                   soc_listen,                   VOIDOBJECT,        par_soc_int},
    { "SOC_NE",                       soc_ne,                       BOOLOBJECT,        par_soc_op_soc},
    { "SOC_ORD",                      soc_ord,                      INTOBJECT,         par_soc},
    { "SOC_RECV",                     soc_recv,                     INTOBJECT,         par_soc_str_int_int},
    { "SOC_RECVFROM",                 soc_recvfrom,                 INTOBJECT,         par_soc_str_int_int_bst},
    { "SOC_SEND",                     soc_send,                     INTOBJECT,         par_soc_str_int},
    { "SOC_SENDTO",                   soc_sendto,                   INTOBJECT,         par_soc_str_int_bst},
    { "SOC_SET_OPT_BOOL",             soc_set_opt_bool,             VOIDOBJECT,        par_soc_int_bln},
    { "SOC_SOCKET",                   soc_socket,                   SOCKETOBJECT,      par_int_int_int},
    { "SOC_VALUE",                    soc_value,                    SOCKETOBJECT,      par_ref},
    { "SOC_WORD_READ",                soc_word_read,                STRIOBJECT,        par_soc_chr},
    { "SOC_WRITE",                    soc_write,                    VOIDOBJECT,        par_soc_str},

#if WITH_SQL
    { "SQL_BIND_BIGINT",              sql_bind_bigint,              VOIDOBJECT,        par_sqs_int_big},
    { "SQL_BIND_BIGRAT",              sql_bind_bigrat,              VOIDOBJECT,        par_sqs_int_big_big},
    { "SQL_BIND_BOOL",                sql_bind_bool,                VOIDOBJECT,        par_sqs_int_bln},
    { "SQL_BIND_BSTRI",               sql_bind_bstri,               VOIDOBJECT,        par_sqs_int_bst},
    { "SQL_BIND_DURATION",            sql_bind_duration,            VOIDOBJECT,        par_sqs_8_int},
    { "SQL_BIND_FLOAT",               sql_bind_float,               VOIDOBJECT,        par_sqs_int_flt},
    { "SQL_BIND_INT",                 sql_bind_int,                 VOIDOBJECT,        par_sqs_int_int},
    { "SQL_BIND_NULL",                sql_bind_null,                VOIDOBJECT,        par_sqs_int},
    { "SQL_BIND_STRI",                sql_bind_stri,                VOIDOBJECT,        par_sqs_int_str},
    { "SQL_BIND_TIME",                sql_bind_time,                VOIDOBJECT,        par_sqs_9_int},
    { "SQL_CLOSE",                    sql_close,                    VOIDOBJECT,        par_dbs},
    { "SQL_CMP_DB",                   sql_cmp_db,                   INTOBJECT,         par_dbs_dbs},
    { "SQL_CMP_STMT",                 sql_cmp_stmt,                 INTOBJECT,         par_sqs_sqs},
    { "SQL_COLUMN_BIGINT",            sql_column_bigint,            BIGINTOBJECT,      par_sqs_int},
    { "SQL_COLUMN_BIGRAT",            sql_column_bigrat,            VOIDOBJECT,        par_sqs_int_big_big},
    { "SQL_COLUMN_BOOL",              sql_column_bool,              BOOLOBJECT,        par_sqs_int},
    { "SQL_COLUMN_BSTRI",             sql_column_bstri,             BSTRIOBJECT,       par_sqs_int},
    { "SQL_COLUMN_DURATION",          sql_column_duration,          VOIDOBJECT,        par_sqs_8_int},
    { "SQL_COLUMN_FLOAT",             sql_column_float,             FLOATOBJECT,       par_sqs_int},
    { "SQL_COLUMN_INT",               sql_column_int,               INTOBJECT,         par_sqs_int},
    { "SQL_COLUMN_STRI",              sql_column_stri,              STRIOBJECT,        par_sqs_int},
    { "SQL_COLUMN_TIME",              sql_column_time,              VOIDOBJECT,        par_sqs_9_int_bln},
    { "SQL_COMMIT",                   sql_commit,                   VOIDOBJECT,        par_dbs},
    { "SQL_CPY_DB",                   sql_cpy_db,                   VOIDOBJECT,        par_dbs_op_dbs},
    { "SQL_CPY_STMT",                 sql_cpy_stmt,                 VOIDOBJECT,        par_sqs_op_sqs},
    { "SQL_CREATE_DB",                sql_create_db,                VOIDOBJECT,        par_dbs_op_dbs},
    { "SQL_CREATE_STMT",              sql_create_stmt,              VOIDOBJECT,        par_sqs_op_sqs},
    { "SQL_DB_CATEGORY",              sql_db_category,              INTOBJECT,         par_dbs},
    { "SQL_DESTR_DB",                 sql_destr_db,                 VOIDOBJECT,        par_dbs},
    { "SQL_DESTR_STMT",               sql_destr_stmt,               VOIDOBJECT,        par_sqs},
    { "SQL_DRIVER",                   sql_driver,                   INTOBJECT,         par_dbs},
    { "SQL_EMPTY_DB",                 sql_empty_db,                 DATABASEOBJECT,    par_no_args},
    { "SQL_EMPTY_STMT",               sql_empty_stmt,               SQLSTMTOBJECT,     par_no_args},
    { "SQL_EQ_DB",                    sql_eq_db,                    BOOLOBJECT,        par_dbs_op_dbs},
    { "SQL_EQ_STMT",                  sql_eq_stmt,                  BOOLOBJECT,        par_sqs_op_sqs},
    { "SQL_ERR_CODE",                 sql_err_code,                 INTOBJECT,         par_no_args},
    { "SQL_ERR_DB_FUNC",              sql_err_db_func,              STRIOBJECT,        par_no_args},
    { "SQL_ERR_LIB_FUNC",             sql_err_lib_func,             STRIOBJECT,        par_no_args},
    { "SQL_ERR_MESSAGE",              sql_err_message,              STRIOBJECT,        par_no_args},
    { "SQL_EXECUTE",                  sql_execute,                  VOIDOBJECT,        par_sqs},
    { "SQL_FETCH",                    sql_fetch,                    BOOLOBJECT,        par_sqs},
    { "SQL_GET_AUTO_COMMIT",          sql_get_auto_commit,          BOOLOBJECT,        par_dbs},
    { "SQL_IS_NULL",                  sql_is_null,                  BOOLOBJECT,        par_sqs_int},
    { "SQL_NE_DB",                    sql_ne_db,                    BOOLOBJECT,        par_dbs_op_dbs},
    { "SQL_NE_STMT",                  sql_ne_stmt,                  BOOLOBJECT,        par_sqs_op_sqs},
    { "SQL_OPEN_DB2",                 sql_open_db2,                 DATABASEOBJECT,    par_op_str_int_3_str},
    { "SQL_OPEN_FIRE",                sql_open_fire,                DATABASEOBJECT,    par_op_str_int_3_str},
    { "SQL_OPEN_INFORMIX",            sql_open_informix,            DATABASEOBJECT,    par_op_str_int_4_str},
    { "SQL_OPEN_LITE",                sql_open_lite,                DATABASEOBJECT,    par_op_str_int_3_str},
    { "SQL_OPEN_MY",                  sql_open_my,                  DATABASEOBJECT,    par_op_str_int_3_str},
    { "SQL_OPEN_OCI",                 sql_open_oci,                 DATABASEOBJECT,    par_op_str_int_3_str},
    { "SQL_OPEN_ODBC",                sql_open_odbc,                DATABASEOBJECT,    par_op_5_str},
    { "SQL_OPEN_POST",                sql_open_post,                DATABASEOBJECT,    par_op_str_int_3_str},
    { "SQL_OPEN_SQLSRV",              sql_open_sqlsrv,              DATABASEOBJECT,    par_op_str_int_3_str},
    { "SQL_OPEN_TDS",                 sql_open_tds,                 DATABASEOBJECT,    par_op_str_int_3_str},
    { "SQL_PREPARE",                  sql_prepare,                  SQLSTMTOBJECT,     par_dbs_str},
    { "SQL_ROLLBACK",                 sql_rollback,                 VOIDOBJECT,        par_dbs},
    { "SQL_SET_AUTO_COMMIT",          sql_set_auto_commit,          VOIDOBJECT,        par_dbs_bln},
    { "SQL_STMT_COLUMN_COUNT",        sql_stmt_column_count,        INTOBJECT,         par_sqs},
    { "SQL_STMT_COLUMN_NAME",         sql_stmt_column_name,         STRIOBJECT,        par_sqs_int},
#endif

    { "STR_APPEND",                   str_append,                   VOIDOBJECT,        par_str_op_str},
    { "STR_CAT",                      str_cat,                      STRIOBJECT,        par_str_op_str},
    { "STR_CHIPOS",                   str_chipos,                   INTOBJECT,         par_str_chr_int},
    { "STR_CHPOS",                    str_chpos,                    INTOBJECT,         par_str_chr},
    { "STR_CHSPLIT",                  str_chsplit,                  ARRAYOBJECT,       par_str_chr},
    { "STR_CMP",                      str_cmp,                      INTOBJECT,         par_str_str},
    { "STR_CPY",                      str_cpy,                      VOIDOBJECT,        par_str_op_str},
    { "STR_CREATE",                   str_create,                   VOIDOBJECT,        par_str_op_str},
    { "STR_C_LITERAL",                str_c_literal,                STRIOBJECT,        par_str},
    { "STR_DESTR",                    str_destr,                    VOIDOBJECT,        par_str},
    { "STR_ELEMCPY",                  str_elemcpy,                  VOIDOBJECT,        par_str_op_op_int_op_chr},
    { "STR_EQ",                       str_eq,                       BOOLOBJECT,        par_str_op_str},
    { "STR_FOR",                      str_for,                      VOIDOBJECT,        par_op_chr_op_str_op_prc},
    { "STR_FOR_KEY",                  str_for_key,                  VOIDOBJECT,        par_op_op_int_op_str_op_prc},
    { "STR_FOR_VAR_KEY",              str_for_var_key,              VOIDOBJECT,        par_op_chr_op_int_op_str_op_prc},
    { "STR_FROM_UTF8",                str_from_utf8,                STRIOBJECT,        par_str},
    { "STR_GE",                       str_ge,                       BOOLOBJECT,        par_str_op_str},
    { "STR_GT",                       str_gt,                       BOOLOBJECT,        par_str_op_str},
    { "STR_HASHCODE",                 str_hashcode,                 INTOBJECT,         par_str},
    { "STR_HEAD",                     str_head,                     STRIOBJECT,        par_str_op_op_int},
    { "STR_IDX",                      str_idx,                      CHAROBJECT,        par_str_op_int},
    { "STR_IPOS",                     str_ipos,                     INTOBJECT,         par_str_str_int},
    { "STR_LE",                       str_le,                       BOOLOBJECT,        par_str_op_str},
    { "STR_LITERAL",                  str_literal,                  STRIOBJECT,        par_str},
    { "STR_LNG",                      str_lng,                      INTOBJECT,         par_str},
    { "STR_LOW",                      str_low,                      STRIOBJECT,        par_str},
    { "STR_LPAD",                     str_lpad,                     STRIOBJECT,        par_str_op_int},
    { "STR_LPAD0",                    str_lpad0,                    STRIOBJECT,        par_str_op_int},
    { "STR_LT",                       str_lt,                       BOOLOBJECT,        par_str_op_str},
    { "STR_LTRIM",                    str_ltrim,                    STRIOBJECT,        par_str},
    { "STR_MULT",                     str_mult,                     STRIOBJECT,        par_str_op_int},
    { "STR_NE",                       str_ne,                       BOOLOBJECT,        par_str_op_str},
    { "STR_POS",                      str_pos,                      INTOBJECT,         par_str_str},
    { "STR_POSCPY",                   str_poscpy,                   VOIDOBJECT,        par_str_op_op_int_op_str},
    { "STR_PUSH",                     str_push,                     VOIDOBJECT,        par_str_op_chr},
    { "STR_RANGE",                    str_range,                    STRIOBJECT,        par_str_op_int_op_int},
    { "STR_RCHIPOS",                  str_rchipos,                  INTOBJECT,         par_str_chr_int},
    { "STR_RCHPOS",                   str_rchpos,                   INTOBJECT,         par_str_chr},
    { "STR_REPL",                     str_repl,                     STRIOBJECT,        par_str_str_str},
    { "STR_RIPOS",                    str_ripos,                    INTOBJECT,         par_str_str_int},
    { "STR_RPAD",                     str_rpad,                     STRIOBJECT,        par_str_op_int},
    { "STR_RPOS",                     str_rpos,                     INTOBJECT,         par_str_str},
    { "STR_RTRIM",                    str_rtrim,                    STRIOBJECT,        par_str},
    { "STR_SPLIT",                    str_split,                    ARRAYOBJECT,       par_str_str},
    { "STR_STR",                      str_str,                      STRIOBJECT,        par_str},
    { "STR_SUBSTR",                   str_substr,                   STRIOBJECT,        par_str_op_int_op_int},
    { "STR_SUBSTR_FIXLEN",            str_substr_fixlen,            STRIOBJECT,        par_str_op_int_op_int},
    { "STR_TAIL",                     str_tail,                     STRIOBJECT,        par_str_op_int},
    { "STR_TO_UTF8",                  str_to_utf8,                  STRIOBJECT,        par_str},
    { "STR_TRIM",                     str_trim,                     STRIOBJECT,        par_str},
    { "STR_UP",                       str_up,                       STRIOBJECT,        par_str},
    { "STR_VALUE",                    str_value,                    STRIOBJECT,        par_ref},

    { "TIM_AWAIT",                    tim_await,                    VOIDOBJECT,        par_8_int},
    { "TIM_FROM_TIMESTAMP",           tim_from_timestamp,           VOIDOBJECT,        par_9_int_bln},
    { "TIM_NOW",                      tim_now,                      VOIDOBJECT,        par_8_int_bln},
    { "TIM_SET_LOCAL_TZ",             tim_set_local_tz,             VOIDOBJECT,        par_7_int_bln},

    { "TYP_ADDINTERFACE",             typ_addinterface,             VOIDOBJECT,        par_typ_typ},
    { "TYP_CMP",                      typ_cmp,                      INTOBJECT,         par_typ_typ},
    { "TYP_CPY",                      typ_cpy,                      VOIDOBJECT,        par_typ_op_typ},
    { "TYP_CREATE",                   typ_create,                   VOIDOBJECT,        par_typ_op_typ},
    { "TYP_DESTR",                    typ_destr,                    VOIDOBJECT,        par_typ},
    { "TYP_EQ",                       typ_eq,                       BOOLOBJECT,        par_typ_op_typ},
    { "TYP_FUNC",                     typ_func,                     TYPEOBJECT,        par_op_typ},
    { "TYP_GENSUB",                   typ_gensub,                   TYPEOBJECT,        par_typ},
    { "TYP_GENTYPE",                  typ_gentype,                  TYPEOBJECT,        par_no_args},
    { "TYP_HASHCODE",                 typ_hashcode,                 INTOBJECT,         par_typ},
    { "TYP_INTERFACES",               typ_interfaces,               ARRAYOBJECT,       par_typ},
    { "TYP_ISDERIVED",                typ_isderived,                BOOLOBJECT,        par_typ},
    { "TYP_ISFUNC",                   typ_isfunc,                   BOOLOBJECT,        par_typ},
    { "TYP_ISVARFUNC",                typ_isvarfunc,                BOOLOBJECT,        par_typ},
    { "TYP_MATCHOBJ",                 typ_matchobj,                 REFOBJECT,         par_typ},
    { "TYP_META",                     typ_meta,                     TYPEOBJECT,        par_typ},
    { "TYP_NE",                       typ_ne,                       BOOLOBJECT,        par_typ_op_typ},
    { "TYP_NUM",                      typ_num,                      INTOBJECT,         par_typ},
    { "TYP_RESULT",                   typ_result,                   TYPEOBJECT,        par_typ},
    { "TYP_SET_IN_PARAM_REF",         typ_set_in_param_ref,         VOIDOBJECT,        par_typ},
    { "TYP_SET_IN_PARAM_VALUE",       typ_set_in_param_value,       VOIDOBJECT,        par_typ},
    { "TYP_STR",                      typ_str,                      STRIOBJECT,        par_typ},
    { "TYP_TYPEOF",                   typ_typeof,                   TYPEOBJECT,        par_any},
    { "TYP_VALUE",                    typ_value,                    TYPEOBJECT,        par_ref},
    { "TYP_VARCONV",                  typ_varconv,                  ILLEGALOBJECT,     par_op_op_any},
    { "TYP_VARFUNC",                  typ_varfunc,                  TYPEOBJECT,        par_op_typ},

    { "UT8_GETC",                     ut8_getc,                     CHAROBJECT,        par_fil},
    { "UT8_GETS",                     ut8_gets,                     STRIOBJECT,        par_fil_int},
    { "UT8_LINE_READ",                ut8_line_read,                STRIOBJECT,        par_fil_chr},
    { "UT8_SEEK",                     ut8_seek,                     VOIDOBJECT,        par_fil_int},
    { "UT8_WORD_READ",                ut8_word_read,                STRIOBJECT,        par_fil_chr},
    { "UT8_WRITE",                    ut8_write,                    VOIDOBJECT,        par_fil_str},
  };



void init_primitiv (void)

  { /* init_primitiv */
    actTable.size = sizeof(actEntryTable) / sizeof(actEntryRecord);
    actTable.table = actEntryTable;
  } /* init_primitiv */

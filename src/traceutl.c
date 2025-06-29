/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2008, 2010 - 2015, 2018  Thomas Mertes     */
/*                2019, 2021, 2023 - 2025  Thomas Mertes            */
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
/*  File: seed7/src/traceutl.c                                      */
/*  Changes: 1990 - 1994, 2008, 2010 - 2015, 2018  Thomas Mertes    */
/*           2019, 2021, 2023 - 2025  Thomas Mertes                 */
/*  Content: Tracing and protocol functions.                        */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS -1
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "limits.h"
#include "ctype.h"

#include "common.h"
#include "data.h"
#include "data_rtl.h"
#include "os_decls.h"
#include "heaputl.h"
#include "flistutl.h"
#include "datautl.h"
#include "striutl.h"
#include "chclsutl.h"
#include "entutl.h"
#include "identutl.h"
#include "syvarutl.h"
#include "actutl.h"
#include "infile.h"
#include "findid.h"
#include "doany.h"
#include "option.h"
#include "set_rtl.h"
#include "str_rtl.h"
#include "ut8_rtl.h"
#include "big_drv.h"
#include "con_rtl.h"
#include "con_drv.h"
#include "pcs_drv.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "traceutl.h"


static fileRecord protFileRecord = {NULL, 0, FALSE, TRUE};
static fileType protfile = &protFileRecord;
static boolType internal_protocol = FALSE;

/* #define prot_ptr(ptr) */
#define prot_ptr(ptr) prot_int((intType) (memSizeType) (ptr))



void prot_flush (void)

  {
    traceRecord trace_backup;

  /* prot_flush */
    if (internal_protocol) {
      if (SYS_PROT_OUTFILE_OBJECT != NULL) {
        memcpy(&trace_backup, &trace, sizeof(traceRecord));
        memset(&trace, 0, sizeof(traceRecord));
        do_flush(SYS_PROT_OUTFILE_OBJECT);
        memcpy(&trace, &trace_backup, sizeof(traceRecord));
      } /* if */
    } else {
      if (protfile->cFile == NULL) {
        protfile->cFile = stdout;
      } /* if */
      fflush(protfile->cFile);
    } /* if */
  } /* prot_flush */



void prot_nl (void)

  {
    traceRecord trace_backup;

  /* prot_nl */
    if (internal_protocol) {
      if (SYS_PROT_OUTFILE_OBJECT != NULL) {
        memcpy(&trace_backup, &trace, sizeof(traceRecord));
        memset(&trace, 0, sizeof(traceRecord));
        do_wrnl(SYS_PROT_OUTFILE_OBJECT);
        memcpy(&trace, &trace_backup, sizeof(traceRecord));
      } /* if */
    } else {
      if (protfile->cFile == NULL) {
        protfile->cFile = stdout;
      } /* if */
      fputs("\n", protfile->cFile);
    } /* if */
  } /* prot_nl */



void prot_cstri (const_cstriType cstri)

  {
    traceRecord trace_backup;

  /* prot_cstri */
    if (internal_protocol) {
      if (SYS_PROT_OUTFILE_OBJECT != NULL) {
        memcpy(&trace_backup, &trace, sizeof(traceRecord));
        memset(&trace, 0, sizeof(traceRecord));
        do_wrcstri(SYS_PROT_OUTFILE_OBJECT, cstri);
        memcpy(&trace, &trace_backup, sizeof(traceRecord));
      } /* if */
    } else {
      if (protfile->cFile == NULL) {
        protfile->cFile = stdout;
      } /* if */
      if (cstri == NULL) {
        cstri = "*NULL*";
      } /* if */
#ifdef USE_CONSOLE_FOR_PROT_CSTRI
      if (protfile->cFile == stdout) {
        striType stri;

        stri = cstri_to_stri(cstri);
        if (stri != NULL) {
          conWrite(stri);
          FREE_STRI(stri);
        } else {
          fputs(cstri, protfile->cFile);
        } /* if */
      } else {
        fputs(cstri, protfile->cFile);
      } /* if */
#else
      fputs(cstri, protfile->cFile);
#endif
    } /* if */
  } /* prot_cstri */



void prot_cstri8 (const const_cstriType cstri8)

  {
    traceRecord trace_backup;
    striType stri;
    errInfoType err_info = OKAY_NO_ERROR;

  /* prot_cstri8 */
    if (internal_protocol) {
      if (SYS_PROT_OUTFILE_OBJECT != NULL) {
        memcpy(&trace_backup, &trace, sizeof(traceRecord));
        memset(&trace, 0, sizeof(traceRecord));
        stri = cstri8_to_stri(cstri8, &err_info);
        if (stri != NULL) {
          do_wrstri(SYS_PROT_OUTFILE_OBJECT, stri);
          FREE_STRI(stri);
        } else {
          do_wrcstri(SYS_PROT_OUTFILE_OBJECT, cstri8);
        } /* if */
        memcpy(&trace, &trace_backup, sizeof(traceRecord));
      } /* if */
    } else {
      if (protfile->cFile == NULL) {
        protfile->cFile = stdout;
      } /* if */
      if (protfile->cFile == stdout) {
        stri = cstri8_to_stri(cstri8, &err_info);
        if (stri != NULL) {
          conWrite(stri);
          FREE_STRI(stri);
        } else {
          fputs(cstri8, protfile->cFile);
        } /* if */
      } else {
        fputs(cstri8, protfile->cFile);
      } /* if */
    } /* if */
  } /* prot_cstri8 */



void prot_cchar (char ch)

  {
    char buffer[2];

  /* prot_cchar */
    buffer[0] = ch;
    buffer[1] = '\0';
    prot_cstri(buffer);
  } /* prot_cchar */



void prot_writeln (const_cstriType stri)

  { /* prot_writeln */
    prot_cstri(stri);
    prot_nl();
  } /* prot_writeln */



void prot_int (intType ivalue)

  {
    char buffer[51];

  /* prot_int */
    sprintf(buffer, FMT_D, ivalue);
    prot_cstri(buffer);
  } /* prot_int */



void prot_bigint (const const_bigIntType bigIntValue)

  {
    striType stri;

  /* prot_bigint */
    if (bigIntValue == NULL) {
      prot_cstri("NULL");
    } else {
      stri = bigStr(bigIntValue);
      prot_cstri(striAsUnquotedCStri(stri));
      strDestr(stri);
    } /* if */
  } /* prot_bigint */



static void prot_bigint_hex (const const_bigIntType bigIntValue)

  {
    cstriType cstri;

  /* prot_bigint_hex */
    cstri = bigHexCStri(bigIntValue);
    prot_cstri(cstri);
    UNALLOC_CSTRI(cstri, strlen(cstri));
  } /* prot_bigint_hex */



#if WITH_FLOAT
void prot_float (floatType floatValue)

  {
    char buffer[51];

  /* prot_float */
    sprintf(buffer, FMT_E, floatValue);
    prot_cstri(buffer);
  } /* prot_float */
#endif



void prot_char (charType cvalue)

  {
    char buffer[51];

  /* prot_char */
    if (cvalue < 127) {
      if (cvalue < ' ') {
        sprintf(buffer, "\'%s\'", stri_escape_sequence[cvalue]);
      } else {
        sprintf(buffer, "\'%c\'", (int) cvalue);
      } /* if */
    } else {
      sprintf(buffer, "\'\\%lu;\'", (unsigned long) cvalue);
    } /* if */
    prot_cstri(buffer);
  } /* prot_char */



void prot_string (const striType stri)

  {
    traceRecord trace_backup;

  /* prot_string */
    if (internal_protocol) {
      if (SYS_PROT_OUTFILE_OBJECT != NULL) {
        memcpy(&trace_backup, &trace, sizeof(traceRecord));
        memset(&trace, 0, sizeof(traceRecord));
        do_wrstri(SYS_PROT_OUTFILE_OBJECT, stri);
        memcpy(&trace, &trace_backup, sizeof(traceRecord));
      } /* if */
    } else {
      if (protfile->cFile == NULL) {
        protfile->cFile = stdout;
      } /* if */
      if (protfile->cFile == stdout) {
        conWrite(stri);
      } else {
        ut8Write(protfile, stri);
      } /* if */
    } /* if */
  } /* prot_string */



void prot_stri_unquoted (const const_striType stri)

  { /* prot_stri_unquoted */
    prot_cstri(striAsUnquotedCStri(stri));
  } /* prot_stri_unquoted */



void prot_stri (const const_striType stri)

  { /* prot_stri */
    if (stri != NULL) {
      prot_cstri("\"");
      prot_stri_unquoted(stri);
      prot_cstri("\"");
    } else {
      prot_cstri(" *NULL_STRING* ");
    } /* if */
  } /* prot_stri */



void prot_bstri (bstriType bstri)

  { /* prot_bstri */
    if (bstri != NULL) {
      prot_cstri("\"");
      prot_cstri(bstriAsUnquotedCStri(bstri));
      prot_cstri("\"");
    } else {
      prot_cstri(" *NULL_BYTE_STRING* ");
    } /* if */
  } /* prot_bstri */



void prot_set (const_setType setValue)

  {
    intType position;
    bitSetType bitset_elem;
    unsigned int bit_index;
    boolType first_elem;

  /* prot_set */
    if (setValue != NULL) {
      prot_cstri("set[");
      prot_int(setValue->min_position);
      prot_cstri("/");
      prot_int(setValue->max_position);
      prot_cstri("]{");
      first_elem = TRUE;
      for (position = setValue->min_position; position <= setValue->max_position; position++) {
        bitset_elem = setValue->bitset[position - setValue->min_position];
        if (bitset_elem != 0) {
          for (bit_index = 0; bit_index < CHAR_BIT * sizeof(bitSetType); bit_index++) {
            if (bitset_elem & ((bitSetType) 1) << bit_index) {
              if (first_elem) {
                first_elem = FALSE;
              } else {
                prot_cstri(", ");
              } /* if */
              prot_int(position << bitset_shift | (intType) bit_index);
            } /* if */
          } /* for */
        } /* if */
      } /* for */
      prot_cstri("}");
    } else {
      prot_cstri(" *NULL_SET* ");
    } /* if */
  } /* prot_set */



void prot_block (const_blockType blockValue)

  { /* prot_block */
    if (blockValue == NULL) {
      prot_cstri(" *NULL_BLOCK* ");
    } else {
      prot_cstri("func result ");
      trace1(blockValue->result.object);
    } /* if */
  } /* prot_block */



void prot_heapsize (void)

  {
    char buffer[51];

  /* prot_heapsize */
    sprintf(buffer, "%6lu", (unsigned long) heapsize());
    prot_cstri(buffer);
  } /* prot_heapsize */



void printcategory (objectCategory category)

  { /* printcategory */
    logFunction(printf("printcategory\n"););
    if (category >= SYMBOLOBJECT && category <= ILLEGALOBJECT) {
      prot_cstri(category_cstri(category));
    } else {
      prot_int((intType) category);
    } /* if */
    logFunction(printf("printcategory -->\n"););
  } /* printcategory */



static char *obj_ptr (objectType anyobject)

  {
    static char out_buf[50];

  /* obj_ptr */
    logFunction(printf("obj_ptr\n"););
    sprintf(out_buf, FMT_X_MEM, (memSizeType) anyobject);
    logFunction(printf("obj_ptr -->\n"););
    return out_buf;
  } /* obj_ptr */



void printtype (const_typeType anytype)

  { /* printtype */
    logFunction(printf("printtype\n"););
    if (anytype != NULL) {
      if (anytype->name != NULL) {
        prot_cstri8(id_string(anytype->name));
      } else if (anytype->result_type != NULL) {
        if (anytype->is_varfunc_type) {
          prot_cstri("varfunc ");
        } else {
          prot_cstri("func ");
        } /* if */
        printtype(anytype->result_type);
      } else {
        prot_cstri(" *ANONYM_TYPE* ");
      } /* if */
      /* prot_cstri(" <");
      prot_cstri(obj_ptr(anytype->match_obj));
      prot_cstri(">"); */
    } else {
      prot_cstri(" *NULL_TYPE* ");
    } /* if */
    logFunction(printf("printtype -->\n"););
  } /* printtype */



static void print_real_value (const_objectType anyobject)

  {
    structType structValue;
    striType stri;

  /* print_real_value */
    logFunction(printf("print_real_value\n"););
    switch (CATEGORY_OF_OBJ(anyobject)) {
      case INTOBJECT:
        prot_int(anyobject->value.intValue);
        break;
      case BIGINTOBJECT:
        prot_bigint_hex(anyobject->value.bigIntValue);
        break;
      case CHAROBJECT:
        prot_char(anyobject->value.charValue);
        break;
      case STRIOBJECT:
        prot_stri(anyobject->value.striValue);
        break;
      case BSTRIOBJECT:
      case POINTLISTOBJECT:
        prot_bstri(anyobject->value.bstriValue);
        break;
      case FILEOBJECT:
        if (anyobject->value.fileValue == NULL) {
          prot_cstri(" *NULL_FILE* ");
        } else {
          if (anyobject->value.fileValue->cFile == NULL) {
            prot_cstri(" *CLIB_NULL_FILE* ");
          } else if (anyobject->value.fileValue->cFile == stdin) {
            prot_cstri("stdin");
          } else if (anyobject->value.fileValue->cFile == stdout) {
            prot_cstri("stdout");
          } else if (anyobject->value.fileValue->cFile == stderr) {
            prot_cstri("stderr");
          } else {
            prot_cstri("file ");
            prot_int((intType) safe_fileno(anyobject->value.fileValue->cFile));
          } /* if */
          if (anyobject->value.fileValue->usage_count != 0) {
            prot_cstri("<");
            prot_int((intType) anyobject->value.fileValue->usage_count);
            prot_cstri(">");
          } /* if */
        } /* if */
        break;
      case SOCKETOBJECT:
        prot_cstri("socket ");
        prot_int((intType) anyobject->value.socketValue);
        break;
#if WITH_FLOAT
      case FLOATOBJECT:
        prot_float(anyobject->value.floatValue);
        break;
#endif
      case ARRAYOBJECT:
        if (anyobject->value.arrayValue != NULL) {
          prot_cstri("array[");
          prot_int(anyobject->value.arrayValue->min_position);
          prot_cstri("..");
          prot_int(anyobject->value.arrayValue->max_position);
          prot_cstri("]");
        } else {
          prot_cstri(" *NULL_ARRAY* ");
        } /* if */
        break;
      case HASHOBJECT:
        if (anyobject->value.hashValue != NULL) {
          prot_cstri("hash[");
          prot_int((intType) anyobject->value.hashValue->size);
          prot_cstri("]");
        } else {
          prot_cstri(" *NULL_HASH* ");
        } /* if */
        break;
      case STRUCTOBJECT:
        structValue = anyobject->value.structValue;
        if (structValue != NULL) {
          prot_cstri("struct[");
          prot_int((intType) structValue->size);
          prot_cstri("]");
          if (structValue->usage_count != 0) {
            prot_cstri("<");
            prot_int((intType) structValue->usage_count);
            prot_cstri(">");
          } else {
            prot_cstri(" ");
            prot_int((intType) (memSizeType) structValue);
          } /* if */
          /* prot_cstri(" ");
             prot_ptr(structValue); */
        } else {
          prot_cstri(" *NULL_STRUCT* ");
        } /* if */
        break;
      case INTERFACEOBJECT:
        if (anyobject->value.objValue != NULL) {
          if (CATEGORY_OF_OBJ(anyobject->value.objValue) == STRUCTOBJECT) {
            structValue = anyobject->value.objValue->value.structValue;
            if (structValue != NULL) {
              prot_cstri("interface[");
              prot_int((intType) structValue->size);
              prot_cstri("]");
              if (structValue->usage_count != 0) {
                prot_cstri("<");
                prot_int((intType) structValue->usage_count);
                prot_cstri(">");
              } /* if */
              prot_cstri(" ");
              prot_ptr(anyobject);
              prot_cstri(" ");
              prot_ptr(anyobject->value.objValue);
              prot_cstri(" ");
              prot_ptr(structValue);
            } else {
              prot_cstri(" *INTERFACE_NULL_STRUCT* ");
            } /* if */
          } else {
            prot_cstri(" *INTERFACE_TO_");
            printcategory(CATEGORY_OF_OBJ(anyobject->value.objValue));
            prot_cstri("* ");
            prot_ptr(anyobject->value.objValue);
            prot_cstri(" ");
            prot_ptr(anyobject);
          } /* if */
          prot_nl();
          prot_cstri("  ");
          trace1(anyobject->value.objValue);
        } else {
          prot_cstri(" *NULL_INTERFACE* ");
        } /* if */
        break;
      case SETOBJECT:
        prot_set(anyobject->value.setValue);
        break;
      case ACTOBJECT:
        prot_cstri("action \"");
        prot_cstri(getActEntry(anyobject->value.actValue)->name);
        prot_cstri("\"");
        break;
      case BLOCKOBJECT:
        prot_block(anyobject->value.blockValue);
        break;
      case WINOBJECT:
        if (anyobject->value.winValue == NULL) {
          prot_cstri(" *NULL_WINDOW* ");
        } else {
          prot_cstri("window [");
          prot_int((intType) anyobject->value.winValue->usage_count);
          prot_cstri("] ");
          prot_ptr(anyobject->value.winValue);
        } /* if */
        break;
      case PROCESSOBJECT:
        if (anyobject->value.winValue == NULL) {
          prot_cstri(" *NULL_PROCESS* ");
        } else {
          prot_cstri("process [");
          prot_int((intType) anyobject->value.processValue->usage_count);
          prot_cstri("] ");
          stri = pcsStr(anyobject->value.processValue);
          prot_stri(stri);
          strDestr(stri);
        } /* if */
        break;
      case PROGOBJECT:
        if (anyobject->value.progValue == NULL) {
          prot_cstri(" *NULL_PROG* ");
        } else {
          prot_cstri("prog ");
          prot_ptr(anyobject->value.progValue);
        } /* if */
        break;
      default:
        break;
    } /* switch */
    logFunction(printf("print_real_value -->\n"););
  } /* print_real_value */



void printvalue (const_objectType anyobject)

  { /* printvalue */
    logFunction(printf("printvalue\n"););
    if (HAS_ENTITY(anyobject) &&
        IS_NORMAL_IDENT(GET_ENTITY(anyobject)->ident)) {
      prot_cstri8(id_string(GET_ENTITY(anyobject)->ident));
    } else {
      print_real_value(anyobject);
    } /* if */
    logFunction(printf("printvalue -->\n"););
  } /* printvalue */



void printobject (const_objectType anyobject)

  { /* printobject */
    logFunction(printf("printobject\n"););
    if (anyobject == NULL) {
      prot_cstri("(NULL)");
    } else {
      if (TEMP_OBJECT(anyobject)) {
        prot_cstri("[TEMP] ");
      } /* if */
      if (TEMP2_OBJECT(anyobject)) {
        prot_cstri("[TEMP2] ");
      } /* if */
      if (IS_UNUSED(anyobject)) {
        prot_cstri("[UNUSED] ");
      } /* if */
      switch (CATEGORY_OF_OBJ(anyobject)) {
        case VARENUMOBJECT:
          if (HAS_ENTITY(anyobject)) {
            prot_cstri8(id_string(GET_ENTITY(anyobject)->ident));
          } else {
            prot_cstri("*NULL_ENTITY_OBJECT*");
          } /* if */
          break;
        case TYPEOBJECT:
          printtype(anyobject->value.typeValue);
          break;
        case FORMPARAMOBJECT:
          prot_cstri("param ");
          if (anyobject->value.objValue != NULL &&
              CATEGORY_OF_OBJ(anyobject->value.objValue) == TYPEOBJECT) {
            prot_cstri("attr ");
          } /* if */
          printobject(anyobject->value.objValue);
          break;
        case VALUEPARAMOBJECT:
          prot_cstri("value param ");
          printobject(anyobject->value.objValue);
          break;
        case REFPARAMOBJECT:
          prot_cstri("ref param ");
          printobject(anyobject->value.objValue);
          break;
        case REFOBJECT:
          prot_cstri("refobject ");
          printobject(anyobject->value.objValue);
          break;
        case RESULTOBJECT:
        case LOCALVOBJECT:
          printobject(anyobject->value.objValue);
          break;
        case INTOBJECT:
        case BIGINTOBJECT:
        case CHAROBJECT:
        case STRIOBJECT:
        case BSTRIOBJECT:
        case FILEOBJECT:
        case SOCKETOBJECT:
        case FLOATOBJECT:
        case ARRAYOBJECT:
        case HASHOBJECT:
        case STRUCTOBJECT:
        case INTERFACEOBJECT:
        case SETOBJECT:
        case BLOCKOBJECT:
        case WINOBJECT:
        case POINTLISTOBJECT:
        case PROCESSOBJECT:
        case PROGOBJECT:
          printvalue(anyobject);
          break;
        case ACTOBJECT:
          print_real_value(anyobject);
          break;
        case LISTOBJECT:
        case EXPROBJECT:
          if (HAS_ENTITY(anyobject) &&
              GET_ENTITY(anyobject)->ident != NULL) {
            if (GET_ENTITY(anyobject)->ident->name != NULL) {
              if (GET_ENTITY(anyobject)->ident->name[0] != '\0') {
                prot_cstri8(id_string(GET_ENTITY(anyobject)->ident));
              } else {
                prot_cstri("(");
                prot_list(anyobject->value.listValue);
                prot_cstri(")");
              } /* if */
            } else {
              prot_cstri("(");
              prot_list(anyobject->value.listValue);
              prot_cstri(")");
            } /* if */
          } else {
            prot_cstri("(");
            prot_list(anyobject->value.listValue);
            prot_cstri(")");
          } /* if */
          break;
        default:
          if (HAS_ENTITY(anyobject)) {
            prot_cstri8(id_string(GET_ENTITY(anyobject)->ident));
          } else {
            printcategory(CATEGORY_OF_OBJ(anyobject));
            prot_cstri(" *NULL_ENTITY_OBJECT*");
          } /* if */
          break;
      } /* switch */
/*    prot_cstri(" <");
      printcategory(CATEGORY_OF_OBJ(anyobject));
      prot_cstri(">"); */
    } /* if */
    logFunction(printf("printobject -->\n"););
  } /* printobject */



static void printformparam (const_objectType aParam)

  { /* printformparam */
    logFunction(printf("printformparam\n"););
    if (aParam != NULL) {
      switch (CATEGORY_OF_OBJ(aParam)) {
        case VALUEPARAMOBJECT:
          if (VAR_OBJECT(aParam)) {
            prot_cstri("in var");
          } else {
            prot_cstri("val ");
          } /* if */
          printtype(aParam->type_of);
          if (HAS_ENTITY(aParam)) {
            prot_cstri(": ");
            prot_cstri8(id_string(GET_ENTITY(aParam)->ident));
          } else {
            prot_cstri(" param");
          } /* if */
          break;
        case REFPARAMOBJECT:
          if (VAR_OBJECT(aParam)) {
            prot_cstri("inout ");
          } else {
            prot_cstri("ref ");
          } /* if */
          printtype(aParam->type_of);
          if (HAS_ENTITY(aParam)) {
            prot_cstri(": ");
            prot_cstri8(id_string(GET_ENTITY(aParam)->ident));
          } else {
            prot_cstri(" param");
          } /* if */
          break;
        case TYPEOBJECT:
          prot_cstri("attr ");
          printtype(aParam->type_of);
          break;
        case SYMBOLOBJECT:
          prot_cstri("symb ");
          printobject(aParam);
          break;
        default:
          prot_cstri("unknown ");
          printobject(aParam);
          break;
      } /* switch */
    } else {
      prot_cstri(" *NULL_PARAMETER* ");
    } /* if */
    logFunction(printf("printformparam -->\n"););
  } /* printformparam */



static void printparam (const_objectType aParam)

  { /* printparam */
    logFunction(printf("printparam\n"););
    prot_cstri("(");
    printformparam(aParam->value.objValue);
    prot_cstri(")");
    logFunction(printf("printparam -->\n"););
  } /* printparam */



static void printObjectName (const_objectType traceobject)

  { /* printObjectName */
    if (HAS_POSINFO(traceobject)) {
      prot_string(objectFileName(traceobject));
      prot_cstri("(");
      prot_int((intType) GET_LINE_NUM(traceobject));
      prot_cstri(")");
    } else if (HAS_PROPERTY(traceobject)) {
      if (HAS_ENTITY(traceobject)) {
        if (GET_ENTITY(traceobject)->ident != NULL) {
          prot_cstri8(id_string(GET_ENTITY(traceobject)->ident));
        } else if (traceobject->descriptor.property->params != NULL) {
          prot_params(traceobject->descriptor.property->params);
        } else if (GET_ENTITY(traceobject)->fparam_list != NULL) {
          prot_name(GET_ENTITY(traceobject)->fparam_list);
        } else {
          prot_cstri8(id_string(NULL));
        } /* if */
      } else {
        prot_string(objectFileName(traceobject));
        prot_cstri("(");
        prot_int((intType) traceobject->descriptor.property->line);
        prot_cstri(")");
      } /* if */
    } else {
      prot_cstri("*NULL_PROPERTY_OBJECT*");
    } /* if */
  } /* printObjectName */



void printLocObj (const_locObjType locObj)

  { /* printLocObj */
    if (locObj == NULL) {
      prot_cstri("***NULL_LOCOBJ***");
    } else {
      prot_cstri("locObj:");
      prot_nl();
      prot_cstri("object: ");
      printobject(locObj->object);
      prot_nl();
      prot_cstri("init_value: ");
      printobject(locObj->init_value);
      prot_nl();
      prot_cstri("create_call_obj: ");
      printobject(locObj->create_call_obj);
      prot_nl();
      prot_cstri("destroy_call_obj: ");
      printobject(locObj->destroy_call_obj);
      prot_nl();
    } /* if */
  } /* printLocObj */



void printLocList (locListType loclist)

  { /* printLocList */
    while (loclist != NULL) {
      printf(" ");
      printObjectName(loclist->local.object);
      loclist = loclist->next;
    } /* while */
  } /* printLocList */



void prot_list_limited (const_listType list, int depthLimit)

  {
    int number;

  /* prot_list_limited */
    logFunction(printf("prot_list_limited\n"););
    prot_cstri("{");
    number = 0;
    while (list != NULL && number <= 50) {
      if (list->obj == NULL) {
        prot_cstri("*NULL_OBJECT*");
      } else if (!LEGAL_CATEGORY_FIELD(list->obj)) {
        prot_cstri("*CORRUPT_CATEGORY_FIELD*");
      } else {
        /* printcategory(CATEGORY_OF_OBJ(list->obj)); fflush(stdout); */
        switch (CATEGORY_OF_OBJ(list->obj)) {
          case LISTOBJECT:
          case EXPROBJECT:
            if (depthLimit != 0) {
              prot_list_limited(list->obj->value.listValue, depthLimit - 1);
            } else {
              prot_cstri(" *** details suppressed *** ");
            } /* if */
            break;
          case CALLOBJECT:
          case MATCHOBJECT:
            if (CATEGORY_OF_OBJ(list->obj->value.listValue->obj) == ACTOBJECT) {
              prot_cstri(getActEntry(list->obj->value.listValue->obj->value.actValue)->name);
            } else if (HAS_ENTITY(list->obj->value.listValue->obj) &&
                GET_ENTITY(list->obj->value.listValue->obj)->ident != NULL) {
              prot_cstri8(id_string(GET_ENTITY(list->obj->value.listValue->obj)->ident));
            } else {
              printtype(list->obj->value.listValue->obj->type_of);
              prot_cstri(": <");
              printcategory(CATEGORY_OF_OBJ(list->obj->value.listValue->obj));
              prot_cstri("> ");
            } /* if */
            prot_cstri("(");
            if (depthLimit != 0) {
              prot_list_limited(list->obj->value.listValue->next, depthLimit - 1);
            } else {
              prot_cstri(" *** details suppressed *** ");
            } /* if */
            prot_cstri(")");
            break;
#ifdef OUT_OF_ORDER
          case VALUEPARAMOBJECT:
          case REFPARAMOBJECT:
          case RESULTOBJECT:
          case LOCALVOBJECT:
            printobject(list->obj->value.objValue);
            break;
#endif
          case FORMPARAMOBJECT:
            printparam(list->obj);
            break;
          case INTOBJECT:
          case BIGINTOBJECT:
          case CHAROBJECT:
          case STRIOBJECT:
          case BSTRIOBJECT:
          case FILEOBJECT:
          case SOCKETOBJECT:
          case FLOATOBJECT:
          case ARRAYOBJECT:
          case HASHOBJECT:
          case STRUCTOBJECT:
          case INTERFACEOBJECT:
          case SETOBJECT:
          case ACTOBJECT:
          case BLOCKOBJECT:
          case WINOBJECT:
          case POINTLISTOBJECT:
          case PROCESSOBJECT:
            printvalue(list->obj);
            break;
          case VARENUMOBJECT:
            if (list->obj->value.objValue != NULL) {
              if (HAS_ENTITY(list->obj->value.objValue) &&
                  IS_NORMAL_IDENT(GET_ENTITY(list->obj->value.objValue)->ident)) {
                prot_cstri8(id_string(GET_ENTITY(list->obj->value.objValue)->ident));
              } else {
                prot_cstri("<");
                printcategory(CATEGORY_OF_OBJ(list->obj->value.objValue));
                prot_cstri(">");
              } /* if */
            } else {
              prot_cstri(" *NULL_REF* ");
            } /* if */
            break;
          case TYPEOBJECT:
            printtype(list->obj->value.typeValue);
            break;
#ifdef OUT_OF_ORDER
          case SYMBOLOBJECT:
            printcategory(CATEGORY_OF_OBJ(list->obj));
            prot_cstri(" ");
            prot_int((intType) list->obj);
            prot_cstri(" ");
            fflush(stdout);
            if (HAS_ENTITY(list->obj)) {
              prot_int((intType) GET_ENTITY(list->obj));
              prot_cstri(" ");
              fflush(stdout);
              if (GET_ENTITY(list->obj)->ident != NULL) {
                prot_cstri8(id_string(GET_ENTITY(list->obj)->ident));
              } /* if */
            } /* if */
            break;
#endif
          default:
            /*
            printf("VAR_OBJECT=%s, ",    VAR_OBJECT(list->obj)    ? "TRUE" : "FALSE");
            printf("TEMP_OBJECT=%s, ",   TEMP_OBJECT(list->obj)   ? "TRUE" : "FALSE");
            printf("TEMP2_OBJECT=%s, ",  TEMP2_OBJECT(list->obj)  ? "TRUE" : "FALSE");
            printf("HAS_POSINFO=%s, ",   HAS_POSINFO(list->obj)   ? "TRUE" : "FALSE");
            printf("HAS_MATCH_ERR=%s, ", HAS_MATCH_ERR(list->obj) ? "TRUE" : "FALSE");
            printf("HAS_PROPERTY=%s, ",  HAS_PROPERTY(list->obj)  ? "TRUE" : "FALSE");
            printf("HAS_ENTITY=%s, ",    HAS_ENTITY(list->obj)    ? "TRUE" : "FALSE");
            */
            if (HAS_ENTITY(list->obj) &&
                GET_ENTITY(list->obj)->ident != NULL) {
              prot_cstri8(id_string(GET_ENTITY(list->obj)->ident));
            } else {
              printtype(list->obj->type_of);
              prot_cstri(": <");
              printcategory(CATEGORY_OF_OBJ(list->obj));
              prot_cstri("> ");
              if (HAS_POSINFO(list->obj)) {
                prot_string(objectFileName(list->obj));
                prot_cstri("(");
                prot_int((intType) GET_LINE_NUM(list->obj));
                prot_cstri(")");
              } else {
                prot_cstri("*NULL_ENTITY_OBJECT*");
              } /* if */
            } /* if */
            break;
        } /* switch */
      } /* if */
      prot_cstri(" ");
      list = list->next;
      number++;
    } /* while */
    if (list != NULL) {
      prot_cstri("*AND_SO_ON*");
    } /* if */
    prot_cstri("}");
    logFunction(printf("prot_list_limited -->\n"););
  } /* prot_list_limited */



void prot_list (const_listType list)

  { /* prot_list */
    prot_list_limited(list, -1);
  } /* prot_list */



void prot_dot_expr (const_listType list)

  {
    int number;
    const_cstriType idString;

  /* prot_dot_expr */
    logFunction(printf("prot_dot_expr\n"););
    number = 0;
    while (list != NULL && number <= 50) {
      if (list->obj == NULL) {
        prot_cstri("*NULL_OBJECT*");
      } else if (!LEGAL_CATEGORY_FIELD(list->obj)) {
        prot_cstri("*CORRUPT_CATEGORY_FIELD*");
      } else {
        prot_cstri(".");
        /* printcategory(CATEGORY_OF_OBJ(list->obj)); fflush(stdout); */
        switch (CATEGORY_OF_OBJ(list->obj)) {
          case EXPROBJECT:
            prot_cstri("(");
            prot_dot_expr(list->obj->value.listValue);
            prot_cstri(")");
            break;
          case SYMBOLOBJECT:
            if (HAS_ENTITY(list->obj) &&
                GET_ENTITY(list->obj)->ident != NULL) {
              idString = id_string(GET_ENTITY(list->obj)->ident);
              if (isalpha(idString[0])) {
                prot_cstri8(idString);
              } else {
                prot_cstri(" ");
                prot_cstri8(idString);
                prot_cstri(" ");
              } /* if */
            } else {
              prot_cstri(" ? ");
            } /* if */
            break;
          default:
            prot_cstri(" ? ");
            break;
        } /* switch */
      } /* if */
      list = list->next;
      number++;
    } /* while */
    if (list != NULL) {
      prot_cstri("*AND_SO_ON*");
    } /* if */
    logFunction(printf("prot_dot_expr -->\n"););
  } /* prot_dot_expr */



void prot_params (const_listType list)

  {
    const_listType list_end;
    const_listType list_elem;
    boolType first_elem = TRUE;
    boolType previous_elem_was_symbol = FALSE;

  /* prot_params */
    if (list != NULL) {
      list_end = list;
      while (list_end->next != NULL) {
        list_end = list_end->next;
      } /* while */
      if (CATEGORY_OF_OBJ(list->obj) != SYMBOLOBJECT &&
          CATEGORY_OF_OBJ(list_end->obj) == SYMBOLOBJECT &&
          HAS_ENTITY(list_end->obj) &&
          GET_ENTITY(list_end->obj)->ident != NULL &&
          GET_ENTITY(list_end->obj)->ident->infix_priority == 0) {
        prot_cstri8(id_string(GET_ENTITY(list_end->obj)->ident));
        first_elem = FALSE;
        previous_elem_was_symbol = TRUE;
      } else {
        list_end = NULL;
      } /* if */
      list_elem = list;
      while (list_elem != list_end) {
        if (list_elem->obj == NULL) {
          prot_cstri("*NULL_OBJECT*");
        } else {
          switch (CATEGORY_OF_OBJ(list_elem->obj)) {
            case VALUEPARAMOBJECT:
            case REFPARAMOBJECT:
            case TYPEOBJECT:
              if (first_elem) {
                prot_cstri("(");
              } else if (previous_elem_was_symbol) {
                prot_cstri(" (");
              } else {
                prot_cstri(", ");
              } /* if */
              printformparam(list_elem->obj);
              previous_elem_was_symbol = FALSE;
              break;
            default:
              if (previous_elem_was_symbol) {
                prot_cstri(" ");
              } else if (!first_elem) {
                prot_cstri(") ");
              } /* if */
              printobject(list_elem->obj);
              previous_elem_was_symbol = TRUE;
              break;
          } /* switch */
          first_elem = FALSE;
        } /* if */
        list_elem = list_elem->next;
      } /* while */
      if (!first_elem && !previous_elem_was_symbol) {
        prot_cstri(")");
      } /* if */
    } else {
      prot_cstri("{}");
    } /* if */
  } /* prot_params */



void prot_name (const_listType list)

  {
    const_listType list_end;

  /* prot_name */
    if (list != NULL) {
      list_end = list;
      while (list_end->next != NULL) {
        list_end = list_end->next;
      } /* while */
      if (CATEGORY_OF_OBJ(list->obj) != SYMBOLOBJECT &&
          CATEGORY_OF_OBJ(list_end->obj) == SYMBOLOBJECT &&
          HAS_ENTITY(list_end->obj) &&
          GET_ENTITY(list_end->obj)->ident != NULL &&
          GET_ENTITY(list_end->obj)->ident->infix_priority == 0) {
        prot_cstri8(id_string(GET_ENTITY(list_end->obj)->ident));
        prot_cstri(" (");
        while (list->next != NULL) {
          if (list->obj == NULL) {
            prot_cstri("*NULL_OBJECT*");
          } else {
            switch (CATEGORY_OF_OBJ(list->obj)) {
              case FORMPARAMOBJECT:
                printformparam(list->obj->value.objValue);
                break;
              default:
                printobject(list->obj);
                break;
            } /* switch */
          } /* if */
          list = list->next;
          if (list->next != NULL) {
            prot_cstri(", ");
          } /* if */
        } /* while */
        prot_cstri(")");
      } else {
        prot_list(list);
      } /* if */
    } else {
      prot_cstri("{}");
    } /* if */
  } /* prot_name */



static void prot_owner (const_ownerType owner)

  { /* prot_owner */
    logFunction(printf("prot_owner\n"););
    while (owner != NULL) {
      printobject(owner->obj);
      owner = owner->next;
      if (owner != NULL) {
        prot_cstri(".");
      } /* if */
    } /* while */
    logFunction(printf("prot_owner -->\n"););
  } /* prot_owner */



static void list_ident_names (const_identType anyident)

  { /* list_ident_names */
    logFunction(printf("list_ident_names\n"););
    if (anyident != NULL) {
      list_ident_names(anyident->next1);
      if (anyident->entity != NULL) {
        if (anyident->entity->data.owner != NULL) {
          prot_cstri8(id_string(anyident));
          prot_cstri(" is ");
          printobject(anyident->entity->data.owner->obj);
          prot_nl();
        } /* if */
      } /* if */
      list_ident_names(anyident->next2);
    } /* if */
    logFunction(printf("list_ident_names -->\n"););
  } /* list_ident_names */



void trace_node (const_nodeType anynode)

  { /* trace_node */
    logFunction(printf("trace_node\n"););
    if (anynode == NULL) {
      prot_cstri(" *NULL_NODE* ");
    } else {
      if (anynode->match_obj != NULL) {
        if (HAS_ENTITY(anynode->match_obj)) {
          prot_cstri8(id_string(GET_ENTITY(anynode->match_obj)->ident));
        } else {
          prot_cstri(" *NULL_MATCH_OBJ_ENTITY* ");
        } /* if */
      } else {
        prot_cstri(" *NULL_MATCH_OBJ* ");
      } /* if */
      prot_cstri(" is ");
      if (anynode->entity != NULL) {
        if (anynode->entity->data.owner != NULL) {
          printobject(anynode->entity->data.owner->obj);
        } else {
          prot_cstri(" *NULL_ENTITY_OBJECTS* ");
        } /* if */
      } else {
        prot_cstri(" *NULL_ENTITY* ");
      } /* if */
    } /* if */
    logFunction(printf("trace_node -->\n"););
  } /* trace_node */



static void list_match_object (const_objectType anyobject, char *buffer)

  { /* list_match_object */
    logFunction(printf("list_match_object\n"););
    if (anyobject != NULL) {
      if (HAS_ENTITY(anyobject)) {
        strcat(buffer, id_string(GET_ENTITY(anyobject)->ident));
      } else {
        strcat(buffer, " *NULL_MATCH_OBJ_ENTITY* ");
      } /* if */
    } else {
      strcat(buffer, " *NULL_MATCH_OBJ* ");
    } /* if */
    logFunction(printf("list_match_object -->\n"););
  } /* list_match_object */



static void list_node_names (const_nodeType anynode, char *buffer)

  {
    size_t buf_len;
    size_t buf_len2;

  /* list_node_names */
    logFunction(printf("list_node_names\n"););
    if (anynode != NULL) {
      if (anynode->usage_count > 0) {
        buf_len = strlen(buffer);
        if (anynode->match_obj != NULL) {
          if (CATEGORY_OF_OBJ(anynode->match_obj) == TYPEOBJECT) {
            if (anynode->match_obj->value.typeValue != NULL) {
              if (anynode->match_obj->value.typeValue->name != NULL) {
                strcat(buffer, id_string(anynode->match_obj->value.typeValue->name));
              } else if (anynode->match_obj->value.typeValue->result_type != NULL &&
                  anynode->match_obj->value.typeValue->result_type->name != NULL) {
                if (anynode->match_obj->value.typeValue->is_varfunc_type) {
                  strcat(buffer, "varfunc ");
                } else {
                  strcat(buffer, "func ");
                } /* if */
                strcat(buffer, id_string(anynode->match_obj->value.typeValue->result_type->name));
              } else {
                strcat(buffer, " *ANONYM_TYPE* ");
              } /* if */
            } else {
              strcat(buffer, " *NULL_TYPE* ");
            } /* if */
          } else {
            list_match_object(anynode->match_obj, buffer);
          } /* if */
        } else {
          strcat(buffer, " *NULL_MATCH_OBJ* ");
        } /* if */
        strcat(buffer, " <");
        strcat(buffer, obj_ptr(anynode->match_obj));
        strcat(buffer, ">");
        if (anynode->entity != NULL) {
          if (anynode->entity->data.owner != NULL) {
            prot_cstri8(buffer);
            prot_cstri(" is ");
            printobject(anynode->entity->data.owner->obj);
            prot_nl();
          } /* if */
        } /* if */
        buf_len2 = strlen(buffer);
        if (anynode->symbol != NULL) {
          strcat(buffer, " ");
          list_node_names(anynode->symbol, buffer);
          buffer[buf_len2] = '\0';
        } /* if */
        if (anynode->inout_param != NULL) {
          strcat(buffer, " inout_param ");
          list_node_names(anynode->inout_param, buffer);
          buffer[buf_len2] = '\0';
        } /* if */
        if (anynode->other_param != NULL) {
          strcat(buffer, " other_param ");
          list_node_names(anynode->other_param, buffer);
          buffer[buf_len2] = '\0';
        } /* if */
        if (anynode->attr != NULL) {
          strcat(buffer, " attr ");
          list_node_names(anynode->attr, buffer);
          buffer[buf_len2] = '\0';
        } /* if */
        buffer[buf_len] = '\0';
        list_node_names(anynode->next1, buffer);
        list_node_names(anynode->next2, buffer);
        buffer[buf_len] = '\0';
      } else {
        list_node_names(anynode->next1, buffer);
        list_node_names(anynode->next2, buffer);
      } /* if */
    } /* if */
    logFunction(printf("list_node_names -->\n"););
  } /* list_node_names */



void trace_nodes (void)

  {
    int position;
    int character;
    char buffer[4096];

  /* trace_nodes */
    logFunction(printf("trace_nodes\n"););
    prot_cstri("Names declared:");
    prot_nl();
    for (position = 0; position < ID_TABLE_SIZE; position++) {
      list_ident_names(prog->ident.table[position]);
    } /* for */
    for (character = (int) '!'; character <= (int) '~'; character++) {
      if (op_character(character) ||
          char_class(character) == LEFTPARENCHAR ||
          char_class(character) == PARENCHAR) {
        if (prog->ident.table1[character]->entity != NULL) {
          if (prog->ident.table1[character]->entity->data.owner != NULL) {
            prot_cstri8(id_string(prog->ident.table1[character]));
            prot_cstri(" is ");
            printobject(prog->ident.table1[character]->entity->data.owner->obj);
            prot_nl();
          } /* if */
        } /* if */
      } /* if */
    } /* for */
    if (prog->declaration_root->symbol != NULL) {
      buffer[0] = '\0';
      list_node_names(prog->declaration_root->symbol, buffer);
    } /* if */
    if (prog->declaration_root->inout_param != NULL) {
      strcpy(buffer, "inout_param ");
      list_node_names(prog->declaration_root->inout_param, buffer);
    } /* if */
    if (prog->declaration_root->other_param != NULL) {
      strcpy(buffer, "other_param ");
      list_node_names(prog->declaration_root->other_param, buffer);
    } /* if */
    if (prog->declaration_root->attr != NULL) {
      strcpy(buffer, "attr ");
      list_node_names(prog->declaration_root->attr, buffer);
    } /* if */
    prot_cstri("----------");
    prot_nl();
    logFunction(printf("trace_nodes -->\n"););
  } /* trace_nodes */



void printnodes (const_nodeType anynode)

  { /* printnodes */
    logFunction(printf("printnodes\n"););
    if (anynode != NULL) {
      printnodes(anynode->next1);
      if (anynode->usage_count == 0) {
        prot_cstri(" USAGE=0: ");
      } /* if */
      if (anynode->match_obj != NULL) {
        if (CATEGORY_OF_OBJ(anynode->match_obj) == TYPEOBJECT) {
          printtype(anynode->match_obj->value.typeValue);
        } else {
          if (HAS_ENTITY(anynode->match_obj)) {
            prot_cstri8(id_string(GET_ENTITY(anynode->match_obj)->ident));
          } else {
            prot_cstri(" *NULL_MATCH_OBJ_ENTITY* ");
          } /* if */
        } /* if */
        prot_cstri("=");
        prot_ptr(anynode->match_obj);
      } else {
        prot_cstri(" *NULL_MATCH_OBJ* ");
      } /* if */
      prot_cstri(" ");
      printnodes(anynode->next2);
    } /* if */
    logFunction(printf("printnodes -->\n"););
  } /* printnodes */



void trace1 (const_objectType traceobject)

  { /* trace1 */
    logFunction(printf("trace1\n"););
    if (traceobject == NULL) {
      prot_cstri("*NULL_OBJECT*");
    } else if (!LEGAL_CATEGORY_FIELD(traceobject)) {
      prot_cstri("*CORRUPT_CATEGORY_FIELD*");
    } else if (CATEGORY_OF_OBJ(traceobject) > ILLEGALOBJECT) {
      prot_cstri("*ILLEGAL_CATEGORY_");
      printcategory(CATEGORY_OF_OBJ(traceobject));
      prot_cstri("*");
    } else {
      if (VAR_OBJECT(traceobject)) {
        prot_cstri("var");
      } else {
        prot_cstri("const");
      } /* if */
      if (TEMP_OBJECT(traceobject)) {
        prot_cstri(" [TEMP]");
      } /* if */
      if (TEMP2_OBJECT(traceobject)) {
        prot_cstri(" [TEMP2]");
      } /* if */
      if (IS_UNUSED(traceobject)) {
        prot_cstri(" [UNUSED]");
      } /* if */
      if (traceobject->type_of != NULL) {
        prot_cstri(" ");
        printtype(traceobject->type_of);
      } /* if */
      prot_cstri(": ");
      printObjectName(traceobject);
      prot_cstri(" is <");
      printcategory(CATEGORY_OF_OBJ(traceobject));
      prot_cstri("> ");
      if (IS_UNUSED(traceobject)) {
        prot_cstri("UNUSED");
      } else {
        switch (CATEGORY_OF_OBJ(traceobject)) {
          case REFOBJECT:
          case ENUMLITERALOBJECT:
          case CONSTENUMOBJECT:
          case VARENUMOBJECT:
          case VALUEPARAMOBJECT:
          case REFPARAMOBJECT:
          case RESULTOBJECT:
          case LOCALVOBJECT:
            /* prot_ptr(traceobject);
               prot_cstri(" "); */
            /* prot_ptr(traceobject->value.objValue);
               prot_nl(); */
            prot_cstri("  ");
            trace1(traceobject->value.objValue);
            break;
          case FORMPARAMOBJECT:
            printparam(traceobject);
            break;
          case TYPEOBJECT:
            printobject(traceobject);
            prot_cstri(" ");
            if (traceobject->value.typeValue != NULL) {
              if (traceobject->value.typeValue->meta != NULL) {
                prot_cstri("^");
                printtype(traceobject->value.typeValue->meta);
                prot_cstri("^");
              } /* if */
              if (traceobject->value.typeValue->result_type != NULL) {
                prot_cstri("[");
                printtype(traceobject->value.typeValue->result_type);
                prot_cstri("]");
              } /* if */
            } else {
              prot_cstri(" *NULL_TYPE* ");
            } /* if */
            break;
          case INTOBJECT:
          case BIGINTOBJECT:
          case CHAROBJECT:
          case STRIOBJECT:
          case BSTRIOBJECT:
          case FILEOBJECT:
          case SOCKETOBJECT:
          case FLOATOBJECT:
          case ARRAYOBJECT:
          case HASHOBJECT:
          case STRUCTOBJECT:
          case INTERFACEOBJECT:
          case SETOBJECT:
          case ACTOBJECT:
          case BLOCKOBJECT:
          case WINOBJECT:
          case POINTLISTOBJECT:
          case PROCESSOBJECT:
            print_real_value(traceobject);
            break;
#ifndef OUT_OF_ORDER
          case LISTOBJECT:
          case EXPROBJECT:
            prot_list(traceobject->value.listValue);
            break;
#endif
          case CALLOBJECT:
          case MATCHOBJECT:
            if (traceobject->value.listValue == NULL) {
              prot_cstri(" *EMPTY_LIST* ");
            } else if (traceobject->value.listValue->obj == NULL) {
              prot_cstri(" *NULL_CALLOBJECT* ");
            } else {
              prot_cstri("<");
              printcategory(CATEGORY_OF_OBJ(traceobject->value.listValue->obj));
              prot_cstri("> ");
              if (CATEGORY_OF_OBJ(traceobject->value.listValue->obj) == ACTOBJECT) {
                prot_cstri(getActEntry(traceobject->value.listValue->obj->value.actValue)->name);
              } else if (HAS_ENTITY(traceobject->value.listValue->obj) &&
                  GET_ENTITY(traceobject->value.listValue->obj)->ident != NULL) {
                prot_cstri8(id_string(GET_ENTITY(traceobject->value.listValue->obj)->ident));
              } else {
                printtype(traceobject->value.listValue->obj->type_of);
                prot_cstri(": <");
                printcategory(CATEGORY_OF_OBJ(traceobject->value.listValue->obj));
                prot_cstri("> ");
              } /* if */
              prot_cstri("(");
              prot_list(traceobject->value.listValue->next);
              prot_cstri(")");
            } /* if */
            break;
          case PROGOBJECT:
            if (traceobject->value.progValue == NULL) {
              prot_cstri("NULL");
            } else {
              prot_stri(traceobject->value.progValue->program_name);
            } /* if */
            break;
          case SYMBOLOBJECT:
            prot_cstri("file ");
            prot_int((intType) GET_POS_FILE_NUM(traceobject));
            prot_cstri("(");
            prot_int((intType) GET_POS_LINE_NUM(traceobject));
            prot_cstri(")");
            break;
          default:
            prot_int(traceobject->value.intValue);
            break;
        } /* switch */
      } /* if */
    } /* if */
    logFunction(printf("trace1 -->\n"););
  } /* trace1 */



void trace_entity (const_entityType anyentity)

  { /* trace_entity */
    if (anyentity != NULL) {
      prot_cstri("anyentity->ident ");
      prot_cstri8(id_string(anyentity->ident));
      prot_cstri("\n");
      prot_cstri("anyentity->syobject ");
      trace1(anyentity->syobject);
      prot_cstri("\n");
      prot_cstri("anyentity->fparam_list ");
      prot_list(anyentity->fparam_list);
      prot_cstri("\n");
      prot_cstri("anyentity->data.owner ");
      prot_owner(anyentity->data.owner);
      prot_cstri("\n");
    } else {
      prot_cstri("anyentity is NULL\n");
    } /* if */
  } /* trace_entity */



void trace_list (const_listType list)

  { /* trace_list */
    while (list != NULL) {
      trace1(list->obj),
      prot_nl();
      list = list->next;
    } /* while */
  } /* trace_list */



void set_protfile_name (const const_striType protfile_name)

  {
    os_striType os_protfile_name;
    static const os_charType os_mode[] = {'w', 0};
    int path_info = PATH_IS_NORMAL;
    errInfoType err_info = OKAY_NO_ERROR;

  /* set_protfile_name */
    logFunction(printf("set_protfile_name\n"););
    if (protfile_name != NULL && protfile_name->size != 0) {
      os_protfile_name = cp_to_os_path(protfile_name, &path_info, &err_info);
      if (unlikely(os_protfile_name != NULL)) {
        if (protfile->cFile != NULL && protfile->cFile != stdout) {
          fclose(protfile->cFile);
        } /* if */
        protfile->cFile = os_fopen(os_protfile_name, os_mode);
        os_stri_free(os_protfile_name);
        if (protfile->cFile == NULL) {
          protfile->cFile = stdout;
        } /* if */
      } /* if */
    } else if (protfile->cFile == NULL) {
      protfile->cFile = stdout;
    } /* if */
    logFunction(printf("set_protfile_name -->\n"););
  } /* set_protfile_name */



#if LOG_FUNCTIONS || LOG_FUNCTIONS_EVERYWHERE
void printTraceOptions (uintType options)

  { /* printTraceOptions */
    if ((options & TRACE_ACTIONS        ) != 0) { printf("ACTIONS\n"); }
    if ((options & TRACE_DO_ACTION_CHECK) != 0) { printf("DO_ACTION_CHECK\n"); }
    if ((options & TRACE_DYNAMIC_CALLS  ) != 0) { printf("DYNAMIC_CALLS\n"); }
    if ((options & TRACE_EXCEPTIONS     ) != 0) { printf("EXCEPTIONS\n"); }
    if ((options & TRACE_HEAP_SIZE      ) != 0) { printf("HEAP_SIZE\n"); }
    if ((options & TRACE_MATCH          ) != 0) { printf("MATCH\n"); }
    if ((options & TRACE_EXECUTIL       ) != 0) { printf("EXECUTIL\n"); }
    if ((options & TRACE_SIGNALS        ) != 0) { printf("SIGNALS\n"); }
  } /* printTraceOptions */
#endif



void set_trace (uintType options)

  { /* set_trace */
    logFunction(printf("set_trace:\n");
                printTraceOptions(options););
    trace.actions       = (options & TRACE_ACTIONS        ) != 0;
    trace.check_actions = (options & TRACE_DO_ACTION_CHECK) != 0;
    trace.dynamic       = (options & TRACE_DYNAMIC_CALLS  ) != 0;
    trace.exceptions    = (options & TRACE_EXCEPTIONS     ) != 0;
    trace.heapsize      = (options & TRACE_HEAP_SIZE      ) != 0;
    trace.match         = (options & TRACE_MATCH          ) != 0;
    trace.executil      = (options & TRACE_EXECUTIL       ) != 0;
    trace.signals       = (options & TRACE_SIGNALS        ) != 0;
    logFunction(printf("set_trace -->\n"););
  } /* set_trace */



#define DO_FLAG(bits) *options = (*options & ~(uintType) (bits)) | (flag & (bits));



void mapTraceFlags (const_striType trace_level, uintType *options)

  {
    memSizeType position;
    uintType flag = (uintType) -1;

  /* mapTraceFlags */
    logFunction(printf("mapTraceFlags\n"););
    if (trace_level != NULL) {
      for (position = 0; position < trace_level->size; position++) {
        /* printf("option: %c\n", trace_level->mem[position]);
           printf("options:     %lo\n", *options);
           printf("flag:        %lo\n", flag); */
        switch (trace_level->mem[position]) {
          case '+': flag = (uintType) -1;  break;
          case '-': flag =  0;             break;
          case 'a': DO_FLAG(TRACE_ACTIONS);          break;
          case 'c': DO_FLAG(TRACE_DO_ACTION_CHECK);  break;
          case 'd': DO_FLAG(TRACE_DYNAMIC_CALLS);    break;
          case 'e': DO_FLAG(TRACE_EXCEPTIONS);       break;
          case 'h': DO_FLAG(TRACE_HEAP_SIZE);        break;
          case 'm': DO_FLAG(TRACE_MATCH);            break;
          case 'u': DO_FLAG(TRACE_EXECUTIL);         break;
          case 's': DO_FLAG(TRACE_SIGNALS);          break;
          case '*': DO_FLAG(TRACE_ACTIONS       | TRACE_DO_ACTION_CHECK |
                            TRACE_DYNAMIC_CALLS | TRACE_EXCEPTIONS      |
                            TRACE_HEAP_SIZE     | TRACE_MATCH           |
                            TRACE_EXECUTIL      | TRACE_SIGNALS);
            break;
        } /* switch */
      } /* for */
    } /* if */
    logFunction(printf("mapTraceFlags --> :\n");
                printTraceOptions(*options););
  } /* mapTraceFlags */



void mapTraceFlags2 (const_cstriType ctrace_level, uintType *options)

  {
    striType trace_level;

  /* mapTraceFlags2 */
    logFunction(printf("mapTraceFlags2\n"););
    if (ctrace_level != NULL) {
      trace_level = cstri8_or_cstri_to_stri(ctrace_level);
      if (trace_level != NULL) {
        mapTraceFlags(trace_level, options);
        FREE_STRI(trace_level);
      } /* if */
    } /* if */
    logFunction(printf("mapTraceFlags2 -->\n"););
  } /* mapTraceFlags2 */

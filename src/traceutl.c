/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2012  Thomas Mertes                        */
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
/*  Changes: 1990 - 1994, 2012  Thomas Mertes                       */
/*  Content: Tracing and protocol procedures.                       */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "heaputl.h"
#include "flistutl.h"
#include "datautl.h"
#include "striutl.h"
#include "chclsutl.h"
#include "entutl.h"
#include "syvarutl.h"
#include "actutl.h"
#include "infile.h"
#include "findid.h"
#include "doany.h"
#include "set_rtl.h"
#include "big_drv.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "traceutl.h"


FILE *protfile = NULL; /* was: stdout; */
booltype internal_protocol = FALSE;



#ifdef ANSI_C

void prot_flush (void)
#else

void prot_flush ()
#endif

  {
    tracerecord trace_backup;

  /* prot_flush */
    if (internal_protocol) {
      if (SYS_PROT_OUTFILE_OBJECT != NULL) {
        memcpy(&trace_backup, &trace, sizeof(tracerecord));
        set_trace("", 0, NULL);
        do_flush(SYS_PROT_OUTFILE_OBJECT);
        memcpy(&trace, &trace_backup, sizeof(tracerecord));
      } /* if */
    } else {
      fflush(protfile);
    } /* if */
  } /* prot_flush */



#ifdef ANSI_C

void prot_nl (void)
#else

void prot_nl ()
#endif

  {
    tracerecord trace_backup;

  /* prot_nl */
    if (internal_protocol) {
      if (SYS_PROT_OUTFILE_OBJECT != NULL) {
        memcpy(&trace_backup, &trace, sizeof(tracerecord));
        set_trace("", 0, NULL);
        do_wrnl(SYS_PROT_OUTFILE_OBJECT);
        memcpy(&trace, &trace_backup, sizeof(tracerecord));
      } /* if */
    } else {
      fputs("\n", protfile);
    } /* if */
  } /* prot_nl */



#ifdef ANSI_C

void prot_cstri (const_cstritype stri)
#else

void prot_cstri (stri)
cstritype stri;
#endif

  {
    tracerecord trace_backup;

  /* prot_cstri */
    if (internal_protocol) {
      if (SYS_PROT_OUTFILE_OBJECT != NULL) {
        memcpy(&trace_backup, &trace, sizeof(tracerecord));
        set_trace("", 0, NULL);
        do_wrcstri(SYS_PROT_OUTFILE_OBJECT, stri);
        memcpy(&trace, &trace_backup, sizeof(tracerecord));
      } /* if */
    } else {
      fputs(stri, protfile);
    } /* if */
  } /* prot_cstri */



#ifdef ANSI_C

void prot_writeln (const_cstritype stri)
#else

void prot_writeln (stri)
cstritype stri;
#endif

  { /* prot_writeln */
    prot_cstri(stri);
    prot_nl();
  } /* prot_writeln */



#ifdef ANSI_C

void prot_int (inttype ivalue)
#else

void prot_int (ivalue)
inttype ivalue;
#endif

  {
    char buffer[51];

  /* prot_int */
    sprintf(buffer, INTTYPE_FORMAT, ivalue);
    prot_cstri(buffer);
  } /* prot_int */



#ifdef ANSI_C

void prot_bigint (const const_biginttype bintvalue)
#else

void prot_bigint (bintvalue)
biginttype bintvalue;
#endif

  {
    cstritype cstri;

  /* prot_bigint */
    cstri = bigHexCStri(bintvalue);
    prot_cstri(cstri);
    UNALLOC_CSTRI(cstri, strlen(cstri));
  } /* prot_bigint */



#ifdef ANSI_C

static void prot_char (chartype cvalue)
#else

static void prot_char (cvalue)
chartype cvalue;
#endif

  {
    char buffer[51];

  /* prot_char */
    if (cvalue <= (chartype) 26) {
      sprintf(buffer, "\'\\%c\'", ((int) cvalue) + '@');
    } else if (cvalue <= (chartype) 31) {
      sprintf(buffer, "\'\\%lu\\\'", (unsigned long) cvalue);
    } else if (cvalue <= (chartype) 127) {
      sprintf(buffer, "\'%c\'", (int) cvalue);
    } else {
      sprintf(buffer, "\'\\%lu\\\'", (unsigned long) cvalue);
    } /* if */
    prot_cstri(buffer);
  } /* prot_char */



#ifdef ANSI_C

void prot_os_stri (const const_os_stritype os_stri)
#else

void prot_os_stri (os_stri)
const const_os_stritype os_stri;
#endif

  {
    const_os_stritype stri;

  /* prot_os_stri */
    if (os_stri != NULL) {
      stri = os_stri;
      prot_cstri("\"");
      {
        char buffer[51];

        for (; *stri != 0 && stri - os_stri <= 128; stri++) {
          if (*stri <= (os_chartype) 31) {
            sprintf(buffer, "\\%03lo", (unsigned long) (os_uchartype) *stri);
          } else if (*stri == (os_chartype) '\\') {
            sprintf(buffer, "\\\\");
          } else if (*stri == (os_chartype) '\"') {
            sprintf(buffer, "\\\"");
          } else if (*stri <= (os_chartype) 127) {
            sprintf(buffer, "%c", (int) *stri);
          } else if (*stri == (os_chartype) -1) {
            sprintf(buffer, "\\EOF\\");
          } else if (*stri <= (os_chartype) 255) {
            sprintf(buffer, "\\%3lo", (unsigned long) (os_uchartype) *stri);
          } else {
            sprintf(buffer, "\\u%4lx\\", (unsigned long) (os_uchartype) *stri);
          } /* if */
          prot_cstri(buffer);
          /* putc((int) *stri, protfile); */
        } /* for */
      }
      if (stri - os_stri > 128) {
        prot_cstri("\\ *AND_SO_ON* SIZE=");
        for (; *stri != 0; stri++) {
        } /* for */
        prot_int((inttype) (stri - os_stri));
      } /* if */
      prot_cstri("\"");
    } else {
      prot_cstri(" *NULL_OS_STRING* ");
    } /* if */
  } /* prot_os_stri */



#ifdef ANSI_C

void prot_stri (const const_stritype stri)
#else

void prot_stri (stri)
stritype stri;
#endif

  {
    memsizetype size;

  /* prot_stri */
    if (stri != NULL) {
      size = stri->size;
      if (size > 128) {
        size = 128;
      } /* if */
      prot_cstri("\"");
      {
        const strelemtype *str;
        memsizetype len;
        char buffer[51];

        for (str = stri->mem, len = size;
            len > 0; str++, len--) {
          if (*str <= (chartype) 26) {
            sprintf(buffer, "\\%c", ((int) *str) + '@');
          } else if (*str <= (chartype) 31) {
            sprintf(buffer, "\\%lu\\", (unsigned long) *str);
          } else if (*str == (chartype) '\\') {
            sprintf(buffer, "\\\\");
          } else if (*str == (chartype) '\"') {
            sprintf(buffer, "\\\"");
          } else if (*str <= (chartype) 127) {
            sprintf(buffer, "%c", (int) *str);
          } else if (*str == (chartype) -1) {
            sprintf(buffer, "\\-1\\");
          } else {
            sprintf(buffer, "\\%lu\\", (unsigned long) *str);
          } /* if */
          prot_cstri(buffer);
          /* putc((int) *str, protfile); */
        } /* for */
      }
      if (stri->size > 128) {
        prot_cstri("\\ *AND_SO_ON* SIZE=");
        prot_int((inttype) stri->size);
      } /* if */
      prot_cstri("\"");
    } else {
      prot_cstri(" *NULL_STRING* ");
    } /* if */
  } /* prot_stri */



#ifdef ANSI_C

void prot_bstri (bstritype bstri)
#else

void prot_bstri (bstri)
bstritype bstri;
#endif

  {
    memsizetype size;

  /* prot_bstri */
    if (bstri != NULL) {
      size = bstri->size;
      if (size > 128) {
        size = 128;
      } /* if */
      prot_cstri("\"");
      {
        uchartype *str;
        memsizetype len;
        char buffer[51];

        for (str = bstri->mem, len = size;
            len > 0; str++, len--) {
          if (*str <= (uchartype) 26) {
            sprintf(buffer, "\\%c", ((int) *str) + '@');
          } else if (*str <= (uchartype) 31) {
            sprintf(buffer, "\\%u\\", *str);
          } else if (*str <= (uchartype) 127) {
            sprintf(buffer, "%c", (int) *str);
          } else if (*str == (uchartype) -1) {
            sprintf(buffer, "\\-1\\");
          } else {
            sprintf(buffer, "\\%u\\", *str);
          } /* if */
          prot_cstri(buffer);
        } /* while */
      }
      if (bstri->size > 128) {
        prot_cstri("\\ *AND_SO_ON* SIZE=");
        prot_int((inttype) bstri->size);
      } /* if */
      prot_cstri("\"");
    } else {
      prot_cstri(" *NULL_BYTE_STRING* ");
    } /* if */
  } /* prot_bstri */



#ifdef ANSI_C

void prot_set (const_settype setValue)
#else

void prot_set (setValue)
settype setValue;
#endif

  {
    inttype position;
    bitsettype bitset_elem;
    unsigned int bit_index;
    booltype first_elem;

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
          for (bit_index = 0; bit_index < 8 * sizeof(bitsettype); bit_index++) {
            if (bitset_elem & ((bitsettype) 1) << bit_index) {
              if (first_elem) {
                first_elem = FALSE;
              } else {
                prot_cstri(", ");
              } /* if */
              prot_int(position << bitset_shift | (inttype) bit_index);
            } /* if */
          } /* for */
        } /* if */
      } /* for */
      prot_cstri("}");
    } else {
      prot_cstri(" *NULL_SET* ");
    } /* if */
  } /* prot_set */



#ifdef ANSI_C

void prot_heapsize (void)
#else

void prot_heapsize ()
#endif

  {
    char buffer[51];

  /* prot_heapsize */
    sprintf(buffer, "%6lu", (unsigned long) heapsize());
    prot_cstri(buffer);
  } /* prot_heapsize */



#ifdef ANSI_C

void printcategory (objectcategory category)
#else

void printcategory (category)
objectcategory category;
#endif

  { /* printcategory */
#ifdef TRACE_TRACE
    printf("BEGIN printcategory\n");
#endif
    if (category >= SYMBOLOBJECT && category <= PROGOBJECT) {
      prot_cstri(category_cstri(category));
    } else {
      prot_int((inttype) category);
    } /* if */
#ifdef TRACE_TRACE
    printf("END printcategory\n");
#endif
  } /* printcategory */



#ifdef ANSI_C

static char *obj_ptr (objecttype anyobject)
#else

static char *obj_ptr (anyobject)
objecttype anyobject;
#endif

  {
    static char out_buf[50];

  /* obj_ptr */
#ifdef TRACE_TRACE
    printf("BEGIN obj_ptr\n");
#endif
    sprintf(out_buf, "%lx", (unsigned long) anyobject);
#ifdef TRACE_TRACE
    printf("END obj_ptr\n");
#endif
  return out_buf;
  } /* obj_ptr */



#ifdef ANSI_C

void printtype (const_typetype anytype)
#else

void printtype (anytype)
typetype anytype;
#endif

  { /* printtype */
#ifdef TRACE_TRACE
    printf("BEGIN printtype\n");
#endif
    if (anytype != NULL) {
      if (anytype->name != NULL) {
        prot_cstri(id_string(anytype->name));
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
#ifdef TRACE_TRACE
    printf("END printtype\n");
#endif
  } /* printtype */



#ifdef ANSI_C

static void print_real_value (const_objecttype anyobject)
#else

static void print_real_value (anyobject)
objecttype anyobject;
#endif

  { /* print_real_value */
#ifdef TRACE_TRACE
    printf("BEGIN print_real_value\n");
#endif
    switch (CATEGORY_OF_OBJ(anyobject)) {
      case INTOBJECT:
        prot_int(anyobject->value.intvalue);
        break;
      case BIGINTOBJECT:
        prot_bigint(anyobject->value.bigintvalue);
        break;
      case CHAROBJECT:
        prot_char(anyobject->value.charvalue);
        break;
      case STRIOBJECT:
        prot_stri(anyobject->value.strivalue);
        break;
      case BSTRIOBJECT:
        prot_bstri(anyobject->value.bstrivalue);
        break;
      case FILEOBJECT:
        if (anyobject->value.filevalue == NULL) {
          prot_cstri(" *NULL_FILE* ");
        } else if (anyobject->value.filevalue == stdin) {
          prot_cstri("stdin");
        } else if (anyobject->value.filevalue == stdout) {
          prot_cstri("stdout");
        } else if (anyobject->value.filevalue == stderr) {
          prot_cstri("stderr");
        } else {
          prot_cstri("file ");
          prot_int((inttype) fileno(anyobject->value.filevalue));
        } /* if */
        break;
      case SOCKETOBJECT:
        prot_cstri("socket ");
        prot_int((inttype) anyobject->value.socketvalue);
        break;
#ifdef WITH_FLOAT
      case FLOATOBJECT:
        printf("%f", anyobject->value.floatvalue);
        break;
#endif
      case ARRAYOBJECT:
        if (anyobject->value.arrayvalue != NULL) {
          prot_cstri("array[");
          prot_int(anyobject->value.arrayvalue->min_position);
          prot_cstri("..");
          prot_int(anyobject->value.arrayvalue->max_position);
          prot_cstri("]");
        } else {
          prot_cstri(" *NULL_ARRAY* ");
        } /* if */
        break;
      case HASHOBJECT:
        if (anyobject->value.hashvalue != NULL) {
          prot_cstri("hash[");
          prot_int((inttype) anyobject->value.hashvalue->size);
          prot_cstri("]");
        } else {
          prot_cstri(" *NULL_HASH* ");
        } /* if */
        break;
      case STRUCTOBJECT:
        if (anyobject->value.structvalue != NULL) {
          prot_cstri("struct[");
          prot_int((inttype) anyobject->value.structvalue->size);
          prot_cstri("]");
        } else {
          prot_cstri(" *NULL_STRUCT* ");
        } /* if */
        break;
      case SETOBJECT:
        prot_set(anyobject->value.setvalue);
        break;
      case ACTOBJECT:
        prot_cstri("action \"");
        prot_cstri(get_primact(anyobject->value.actvalue)->name);
        prot_cstri("\"");
        break;
      case BLOCKOBJECT:
        if (anyobject->value.blockvalue == NULL) {
          prot_cstri(" *NULL_BLOCK* ");
        } else {
          prot_cstri("func ");
          prot_int((inttype) anyobject->value.blockvalue);
        } /* if */
        break;
      case WINOBJECT:
        if (anyobject->value.winvalue == NULL) {
          prot_cstri(" *NULL_WINDOW* ");
        } else {
          prot_cstri("window [");
          prot_int((inttype) anyobject->value.winvalue->usage_count);
          prot_cstri("] ");
          prot_int((inttype) anyobject->value.winvalue);
        } /* if */
        break;
      case PROGOBJECT:
        if (anyobject->value.progvalue == NULL) {
          prot_cstri(" *NULL_PROG* ");
        } else {
          prot_cstri("prog ");
          prot_int((inttype) anyobject->value.progvalue);
        } /* if */
        break;
      default:
        break;
    } /* switch */
#ifdef TRACE_TRACE
    printf("END print_real_value\n");
#endif
  } /* print_real_value */



#ifdef ANSI_C

void printvalue (const_objecttype anyobject)
#else

void printvalue (anyobject)
objecttype anyobject;
#endif

  { /* printvalue */
#ifdef TRACE_TRACE
    printf("BEGIN printvalue\n");
#endif
    if (HAS_ENTITY(anyobject) &&
        GET_ENTITY(anyobject) != entity.literal &&
        GET_ENTITY(anyobject)->ident != NULL) {
      prot_cstri(id_string(GET_ENTITY(anyobject)->ident));
    } else {
      print_real_value(anyobject);
    } /* if */
#ifdef TRACE_TRACE
    printf("END printvalue\n");
#endif
  } /* printvalue */



#ifdef ANSI_C

void printobject (const_objecttype anyobject)
#else

void printobject (anyobject)
objecttype anyobject;
#endif

  { /* printobject */
#ifdef TRACE_TRACE
    printf("BEGIN printobject\n");
#endif
    if (anyobject == NULL) {
      prot_cstri("(NULL)");
    } else {
      if (TEMP_OBJECT(anyobject)) {
        prot_cstri("[TEMP] ");
      } /* if */
      if (TEMP2_OBJECT(anyobject)) {
        prot_cstri("[TEMP2] ");
      } /* if */
      switch (CATEGORY_OF_OBJ(anyobject)) {
        case VARENUMOBJECT:
          if (HAS_ENTITY(anyobject)) {
            prot_cstri(id_string(GET_ENTITY(anyobject)->ident));
          } else {
            prot_cstri("*NULL_ENTITY_OBJECT*");
          } /* if */
          break;
        case TYPEOBJECT:
          printtype(anyobject->value.typevalue);
          break;
        case FORMPARAMOBJECT:
          prot_cstri("param ");
          if (anyobject->value.objvalue != NULL &&
              CATEGORY_OF_OBJ(anyobject->value.objvalue) == TYPEOBJECT) {
            prot_cstri("attr ");
          } /* if */
          printobject(anyobject->value.objvalue);
          break;
        case VALUEPARAMOBJECT:
          prot_cstri("value param ");
          printobject(anyobject->value.objvalue);
          break;
        case REFPARAMOBJECT:
          prot_cstri("ref param ");
          printobject(anyobject->value.objvalue);
          break;
        case REFOBJECT:
          prot_cstri("refobject ");
          printobject(anyobject->value.objvalue);
          break;
        case RESULTOBJECT:
        case LOCALVOBJECT:
          printobject(anyobject->value.objvalue);
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
        case SETOBJECT:
        case BLOCKOBJECT:
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
                prot_cstri(id_string(GET_ENTITY(anyobject)->ident));
              } else {
                prot_cstri("(");
                prot_list(anyobject->value.listvalue);
                prot_cstri(")");
              } /* if */
            } else {
              prot_cstri("(");
              prot_list(anyobject->value.listvalue);
              prot_cstri(")");
            } /* if */
          } else {
            prot_cstri("(");
            prot_list(anyobject->value.listvalue);
            prot_cstri(")");
          } /* if */
          break;
        default:
          if (HAS_ENTITY(anyobject)) {
            prot_cstri(id_string(GET_ENTITY(anyobject)->ident));
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
#ifdef TRACE_TRACE
    printf("END printobject\n");
#endif
  } /* printobject */



#ifdef ANSI_C

static void printformparam (const_objecttype aParam)
#else

static void printformparam (aParam)
objecttype aParam;
#endif

  { /* printformparam */
#ifdef TRACE_TRACE
    printf("BEGIN printformparam\n");
#endif
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
            prot_cstri(id_string(GET_ENTITY(aParam)->ident));
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
            prot_cstri(id_string(GET_ENTITY(aParam)->ident));
          } else {
            prot_cstri(" param");
          } /* if */
          break;
        case TYPEOBJECT:
          prot_cstri("attr ");
          printtype(aParam->type_of);
          break;
        default:
          prot_cstri("unknown ");
          printobject(aParam);
          break;
      } /* switch */
    } else {
      prot_cstri(" *NULL_PARAMETER* ");
    } /* if */
#ifdef TRACE_TRACE
    printf("END printformparam\n");
#endif
  } /* printformparam */



#ifdef ANSI_C

static void printparam (const_objecttype aParam)
#else

static void printparam (aParam)
objecttype aParam;
#endif

  { /* printparam */
#ifdef TRACE_TRACE
    printf("BEGIN printparam\n");
#endif
    prot_cstri("(");
    printformparam(aParam->value.objvalue);
    prot_cstri(")");
#ifdef TRACE_TRACE
    printf("END printparam\n");
#endif
  } /* printparam */



#ifdef ANSI_C

void prot_list (const_listtype list)
#else

void prot_list (list)
listtype list;
#endif

  {
    int number;

  /* prot_list */
#ifdef TRACE_TRACE
    printf("BEGIN prot_list\n");
#endif
    prot_cstri("{");
    number = 0;
    while (list != NULL && number <= 50) {
      if (list->obj == NULL) {
        prot_cstri("*NULL_OBJECT*");
      } else {
        /* printcategory(CATEGORY_OF_OBJ(list->obj)); fflush(stdout); */
        switch (CATEGORY_OF_OBJ(list->obj)) {
          case LISTOBJECT:
          case EXPROBJECT:
            prot_list(list->obj->value.listvalue);
            break;
          case CALLOBJECT:
          case MATCHOBJECT:
            if (CATEGORY_OF_OBJ(list->obj->value.listvalue->obj) == ACTOBJECT) {
              prot_cstri(get_primact(list->obj->value.listvalue->obj->value.actvalue)->name);
            } else if (HAS_ENTITY(list->obj->value.listvalue->obj) &&
                GET_ENTITY(list->obj->value.listvalue->obj)->ident != NULL) {
              prot_cstri(id_string(GET_ENTITY(list->obj->value.listvalue->obj)->ident));
            } else {
              printtype(list->obj->value.listvalue->obj->type_of);
              prot_cstri(": <");
              printcategory(CATEGORY_OF_OBJ(list->obj->value.listvalue->obj));
              prot_cstri("> ");
            } /* if */
            prot_cstri("(");
            prot_list(list->obj->value.listvalue->next);
            prot_cstri(")");
            break;
#ifdef OUT_OF_ORDER
          case VALUEPARAMOBJECT:
          case REFPARAMOBJECT:
          case RESULTOBJECT:
          case LOCALVOBJECT:
            printobject(list->obj->value.objvalue);
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
          case SETOBJECT:
          case ACTOBJECT:
          case BLOCKOBJECT:
          case WINOBJECT:
            printvalue(list->obj);
            break;
          case VARENUMOBJECT:
            if (list->obj->value.objvalue != NULL) {
              if (HAS_ENTITY(list->obj->value.objvalue) &&
                  GET_ENTITY(list->obj->value.objvalue)->ident != NULL &&
                  GET_ENTITY(list->obj->value.objvalue)->ident != prog.ident.literal) {
                prot_cstri(id_string(GET_ENTITY(list->obj->value.objvalue)->ident));
              } else {
                prot_cstri("<");
                printcategory(CATEGORY_OF_OBJ(list->obj->value.objvalue));
                prot_cstri(">");
              } /* if */
            } else {
              prot_cstri(" *NULL_REF* ");
            } /* if */
            break;
          case TYPEOBJECT:
            printtype(list->obj->value.typevalue);
            break;
#ifdef OUT_OF_ORDER
          case SYMBOLOBJECT:
            printcategory(CATEGORY_OF_OBJ(list->obj));
            prot_cstri(" ");
            prot_int((inttype) list->obj);
            prot_cstri(" ");
            fflush(stdout);
            if (HAS_ENTITY(list->obj)) {
              prot_int((inttype) GET_ENTITY(list->obj));
              prot_cstri(" ");
              fflush(stdout);
              if (GET_ENTITY(list->obj)->ident != NULL) {
                prot_cstri(id_string(GET_ENTITY(list->obj)->ident));
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
              prot_cstri(id_string(GET_ENTITY(list->obj)->ident));
            } else {
              printtype(list->obj->type_of);
              prot_cstri(": <");
              printcategory(CATEGORY_OF_OBJ(list->obj));
              prot_cstri("> ");
              if (HAS_POSINFO(list->obj)) {
                prot_cstri((const_cstritype) get_file_name_ustri(GET_FILE_NUM(list->obj)));
                prot_cstri("(");
                prot_int((inttype) GET_LINE_NUM(list->obj));
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
#ifdef TRACE_TRACE
    printf("END prot_list\n");
#endif
  } /* prot_list */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

void prot_list (listtype list)
#else

void prot_list (list)
listtype list;
#endif

  { /* prot_list */
#ifdef TRACE_TRACE
    printf("BEGIN prot_list\n");
#endif
    while (list != (listtype) NULL) {
      printobject(list->obj);
      list = list->next;
      if (list != (listtype) NULL) {
        prot_cstri(".");
      } /* if */
    } /* while */
#ifdef TRACE_TRACE
    printf("END prot_list\n");
#endif
  } /* prot_list */
#endif



#ifdef ANSI_C

void prot_name (const_listtype list)
#else

void prot_name (list)
listtype list;
#endif

  {
    const_listtype list_end;

  /* prot_name */
    if (list != NULL) {
      list_end = list;
      while (list_end->next != NULL) {
        list_end = list_end->next;
      } /* while */
      if (CATEGORY_OF_OBJ(list_end->obj) == SYMBOLOBJECT &&
          HAS_ENTITY(list_end->obj) &&
          GET_ENTITY(list_end->obj)->ident != NULL &&
          GET_ENTITY(list_end->obj)->ident->infix_priority == 0) {
        prot_cstri(id_string(GET_ENTITY(list_end->obj)->ident));
        prot_cstri(" (");
        while (list->next != NULL) {
          if (list->obj == NULL) {
            prot_cstri("*NULL_OBJECT*");
          } else {
            switch (CATEGORY_OF_OBJ(list->obj)) {
              case FORMPARAMOBJECT:
                printformparam(list->obj->value.objvalue);
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



#ifdef ANSI_C

static void prot_owner (const_ownertype owner)
#else

static void prot_owner (owner)
ownertype owner;
#endif

  { /* prot_owner */
#ifdef TRACE_TRACE
    printf("BEGIN prot_owner\n");
#endif
    while (owner != NULL) {
      printobject(owner->obj);
      owner = owner->next;
      if (owner != NULL) {
        prot_cstri(".");
      } /* if */
    } /* while */
#ifdef TRACE_TRACE
    printf("END prot_owner\n");
#endif
  } /* prot_owner */



#ifdef ANSI_C

static void list_ident_names (const_identtype anyident)
#else

static void list_ident_names (anyident)
identtype anyident;
#endif

  { /* list_ident_names */
#ifdef TRACE_TRACE
    printf("BEGIN list_ident_names\n");
#endif
    if (anyident != NULL) {
      list_ident_names(anyident->next1);
      if (anyident->entity != NULL) {
        if (anyident->entity->owner != NULL) {
          prot_cstri(id_string(anyident));
          prot_cstri(" is ");
          printobject(anyident->entity->owner->obj);
          prot_nl();
        } /* if */
      } /* if */
      list_ident_names(anyident->next2);
    } /* if */
#ifdef TRACE_TRACE
    printf("END list_ident_names\n");
#endif
  } /* list_ident_names */



#ifdef ANSI_C

void trace_node (const_nodetype anynode)
#else

void trace_node (anynode)
nodetype anynode;
#endif

  { /* trace_node */
#ifdef TRACE_TRACE
    printf("BEGIN trace_node\n");
#endif
    if (anynode == NULL) {
      prot_cstri(" *NULL_NODE* ");
    } else {
      if (anynode->match_obj != NULL) {
        if (HAS_ENTITY(anynode->match_obj)) {
          prot_cstri(id_string(GET_ENTITY(anynode->match_obj)->ident));
        } else {
          prot_cstri(" *NULL_MATCH_OBJ_ENTITY* ");
        } /* if */
      } else {
        prot_cstri(" *NULL_MATCH_OBJ* ");
      } /* if */
      prot_cstri(" is ");
      if (anynode->entity != NULL) {
        if (anynode->entity->owner != NULL) {
          printobject(anynode->entity->owner->obj);
        } else {
          prot_cstri(" *NULL_ENTITY_OBJECTS* ");
        } /* if */
      } else {
        prot_cstri(" *NULL_ENTITY* ");
      } /* if */
    } /* if */
    prot_nl();
#ifdef TRACE_TRACE
    printf("END trace_node\n");
#endif
  } /* trace_node */



#ifdef ANSI_C

static void list_match_object (const_objecttype anyobject, char *buffer)
#else

static void list_match_object (anyobject, buffer)
objecttype anyobject;
char *buffer;
#endif

  { /* list_match_object */
#ifdef TRACE_TRACE
    printf("BEGIN list_node_names\n");
#endif
    if (anyobject != NULL) {
      if (HAS_ENTITY(anyobject)) {
        strcat(buffer, id_string(GET_ENTITY(anyobject)->ident));
      } else {
        strcat(buffer, " *NULL_MATCH_OBJ_ENTITY* ");
      } /* if */
    } else {
      strcat(buffer, " *NULL_MATCH_OBJ* ");
    } /* if */
#ifdef TRACE_TRACE
    printf("END list_match_object\n");
#endif
  } /* list_match_object */



#ifdef ANSI_C

static void list_node_names (const_nodetype anynode, char *buffer)
#else

static void list_node_names (anynode, buffer)
nodetype anynode;
char *buffer;
#endif

  {
    size_t buf_len;
    size_t buf_len2;

  /* list_node_names */
#ifdef TRACE_TRACE
    printf("BEGIN list_node_names\n");
#endif
    if (anynode != NULL) {
      if (anynode->usage_count > 0) {
        buf_len = strlen(buffer);
        if (anynode->match_obj != NULL) {
          if (CATEGORY_OF_OBJ(anynode->match_obj) == TYPEOBJECT) {
            if (anynode->match_obj->value.typevalue != NULL) {
              if (anynode->match_obj->value.typevalue->name != NULL) {
                strcat(buffer, id_string(anynode->match_obj->value.typevalue->name));
              } else if (anynode->match_obj->value.typevalue->result_type != NULL &&
                  anynode->match_obj->value.typevalue->result_type->name != NULL) {
                if (anynode->match_obj->value.typevalue->is_varfunc_type) {
                  strcat(buffer, "varfunc ");
                } else {
                  strcat(buffer, "func ");
                } /* if */
                strcat(buffer, id_string(anynode->match_obj->value.typevalue->result_type->name));
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
          if (anynode->entity->owner != NULL) {
            prot_cstri(buffer);
            prot_cstri(" is ");
            printobject(anynode->entity->owner->obj);
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
#ifdef TRACE_TRACE
    printf("END list_node_names\n");
#endif
  } /* list_node_names */



#ifdef ANSI_C

void trace_nodes (void)
#else

void trace_nodes ()
#endif

  {
    int position;
    int character;
    char buffer[4096];

  /* trace_nodes */
#ifdef TRACE_TRACE
    printf("BEGIN trace_nodes\n");
#endif
    prot_cstri("Names declared:");
    prot_nl();
    for (position = 0; position < ID_TABLE_SIZE; position++) {
      list_ident_names(prog.ident.table[position]);
    } /* for */
    for (character = (int) '!'; character <= (int) '~'; character++) {
      if (op_character(character) ||
          char_class(character) == LEFTPARENCHAR ||
          char_class(character) == PARENCHAR) {
        if (prog.ident.table1[character]->entity != NULL) {
          if (prog.ident.table1[character]->entity->owner != NULL) {
            prot_cstri(id_string(prog.ident.table1[character]));
            prot_cstri(" is ");
            printobject(prog.ident.table1[character]->entity->owner->obj);
            prot_nl();
          } /* if */
        } /* if */
      } /* if */
    } /* for */
    if (prog.declaration_root->symbol != NULL) {
      strcpy(buffer, "");
      list_node_names(prog.declaration_root->symbol, buffer);
    } /* if */
    if (prog.declaration_root->inout_param != NULL) {
      strcpy(buffer, "inout_param ");
      list_node_names(prog.declaration_root->inout_param, buffer);
    } /* if */
    if (prog.declaration_root->other_param != NULL) {
      strcpy(buffer, "other_param ");
      list_node_names(prog.declaration_root->other_param, buffer);
    } /* if */
    if (prog.declaration_root->attr != NULL) {
      strcpy(buffer, "attr ");
      list_node_names(prog.declaration_root->attr, buffer);
    } /* if */
    prot_cstri("----------");
    prot_nl();
#ifdef TRACE_TRACE
    printf("END trace_nodes\n");
#endif
  } /* trace_nodes */



#ifdef ANSI_C

void printnodes (const_nodetype anynode)
#else

void printnodes (anynode)
nodetype anynode;
#endif

  { /* printnodes */
#ifdef TRACE_TRACE
    printf("BEGIN printnodes\n");
#endif
    if (anynode != NULL) {
      printnodes(anynode->next1);
      if (anynode->usage_count == 0) {
        prot_cstri(" USAGE=0: ");
      } /* if */
      if (anynode->match_obj != NULL) {
        if (CATEGORY_OF_OBJ(anynode->match_obj) == TYPEOBJECT) {
          printtype(anynode->match_obj->value.typevalue);
        } else {
          if (HAS_ENTITY(anynode->match_obj)) {
            prot_cstri(id_string(GET_ENTITY(anynode->match_obj)->ident));
          } else {
            prot_cstri(" *NULL_MATCH_OBJ_ENTITY* ");
          } /* if */
        } /* if */
        prot_cstri("=");
        prot_int((inttype) anynode->match_obj);
      } else {
        prot_cstri(" *NULL_MATCH_OBJ* ");
      } /* if */
      prot_cstri(" ");
      printnodes(anynode->next2);
    } /* if */
#ifdef TRACE_TRACE
    printf("END printnodes\n");
#endif
  } /* printnodes */



#ifdef ANSI_C

void trace1 (const_objecttype traceobject)
#else

void trace1 (traceobject)
objecttype traceobject;
#endif

  { /* trace1 */
#ifdef TRACE_TRACE
    printf("BEGIN trace1\n");
#endif
    if (traceobject == NULL) {
      prot_cstri("*NULL_OBJECT*");
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
      if (traceobject->type_of != NULL) {
        prot_cstri(" ");
        printtype(traceobject->type_of);
      } /* if */
      prot_cstri(": ");
      if (HAS_POSINFO(traceobject)) {
        prot_cstri((const_cstritype) get_file_name_ustri(GET_FILE_NUM(traceobject)));
        prot_cstri("(");
        prot_int((inttype) GET_LINE_NUM(traceobject));
        prot_cstri(")");
      } else {
        if (HAS_ENTITY(traceobject)) {
          prot_cstri(id_string(GET_ENTITY(traceobject)->ident));
        } else {
          prot_cstri("*NULL_ENTITY_OBJECT*");
        } /* if */
      } /* if */
      prot_cstri(" is <");
      printcategory(CATEGORY_OF_OBJ(traceobject));
      prot_cstri("> ");
      switch (CATEGORY_OF_OBJ(traceobject)) {
        case REFOBJECT:
        case INTERFACEOBJECT:
        case ENUMLITERALOBJECT:
        case CONSTENUMOBJECT:
        case VARENUMOBJECT:
        case VALUEPARAMOBJECT:
        case REFPARAMOBJECT:
        case RESULTOBJECT:
        case LOCALVOBJECT:
          prot_nl();
          prot_cstri("  ");
          trace1(traceobject->value.objvalue);
          break;
        case FORMPARAMOBJECT:
          printparam(traceobject);
          break;
#ifdef OUT_OF_ORDER
        case MODULEOBJECT:
          prot_cstri("{ ");
          printnodes(traceobject->value.nodevalue);
          prot_cstri("}");
          break;
#endif
        case TYPEOBJECT:
          printobject(traceobject);
          prot_cstri(" ");
          if (traceobject->value.typevalue != NULL) {
            if (traceobject->value.typevalue->meta != NULL) {
              prot_cstri("^");
              printtype(traceobject->value.typevalue->meta);
              prot_cstri("^");
            } /* if */
            if (traceobject->value.typevalue->result_type != NULL) {
              prot_cstri("[");
              printtype(traceobject->value.typevalue->result_type);
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
        case SETOBJECT:
        case ACTOBJECT:
        case BLOCKOBJECT:
        case WINOBJECT:
          print_real_value(traceobject);
          break;
#ifndef OUT_OF_ORDER
        case LISTOBJECT:
        case EXPROBJECT:
          prot_list(traceobject->value.listvalue);
          break;
#endif
        case CALLOBJECT:
        case MATCHOBJECT:
          if (traceobject->value.listvalue == NULL) {
            prot_cstri(" *EMPTY_LIST* ");
          } else if (traceobject->value.listvalue->obj == NULL) {
            prot_cstri(" *NULL_CALLOBJECT* ");
          } else {
            prot_cstri("<");
            printcategory(CATEGORY_OF_OBJ(traceobject->value.listvalue->obj));
            prot_cstri("> ");
            if (CATEGORY_OF_OBJ(traceobject->value.listvalue->obj) == ACTOBJECT) {
              prot_cstri(get_primact(traceobject->value.listvalue->obj->value.actvalue)->name);
            } else if (HAS_ENTITY(traceobject->value.listvalue->obj) &&
                GET_ENTITY(traceobject->value.listvalue->obj)->ident != NULL) {
              prot_cstri(id_string(GET_ENTITY(traceobject->value.listvalue->obj)->ident));
            } else {
              printtype(traceobject->value.listvalue->obj->type_of);
              prot_cstri(": <");
              printcategory(CATEGORY_OF_OBJ(traceobject->value.listvalue->obj));
              prot_cstri("> ");
            } /* if */
            prot_cstri("(");
            prot_list(traceobject->value.listvalue->next);
            prot_cstri(")");
          } /* if */
          break;
        case PROGOBJECT:
          if (traceobject->value.progvalue == NULL) {
            prot_cstri("NULL");
          } else {
            prot_stri(traceobject->value.progvalue->source_file_name);
          } /* if */
          break;
        case SYMBOLOBJECT:
          prot_int((inttype) traceobject);
          break;
        default:
          break;
      } /* switch */
    } /* if */
#ifdef TRACE_TRACE
    printf("END trace1\n");
#endif
  } /* trace1 */



#ifdef ANSI_C

void trace_entity (const_entitytype anyentity)
#else

void trace_entity (anyentity)
entitytype anyentity;
#endif

  { /* trace_entity */
    if (anyentity != NULL) {
      prot_cstri("anyentity->ident ");
      prot_cstri(id_string(anyentity->ident));
      prot_cstri("\n");
      prot_cstri("anyentity->syobject ");
      trace1(anyentity->syobject);
      prot_cstri("\n");
      prot_cstri("anyentity->params ");
      prot_list(anyentity->name_list);
      prot_cstri("\n");
      prot_cstri("anyentity->owner ");
      prot_owner(anyentity->owner);
      prot_cstri("\n");
    } else {
      prot_cstri("anyentity is NULL\n");
    } /* if */
  } /* trace_entity */



#ifdef ANSI_C

void trace_list (const_listtype list)
#else

void trace_list (list)
listtype list;
#endif

  { /* trace_list */
    while (list != NULL) {
      trace1(list->obj),
      prot_nl();
      list = list->next;
    } /* while */
  } /* trace_list */



#ifdef ANSI_C

void set_trace (const_cstritype trace_level, int len, const_cstritype prot_file_name)
#else

void set_trace (trace_level, len, prot_file_name)
cstritype trace_level;
int len;
cstritype prot_file_name;
#endif

  {
    int position;
    booltype flag_value;

  /* set_trace */
#ifdef TRACE_TRACE
    printf("BEGIN set_trace\n");
#endif
    if (trace_level != NULL) {
      if (len == -1) {
        len = strlen(trace_level);
      } /* if */
      flag_value = TRUE;
      for (position = 0; position < len; position++) {
        switch (trace_level[position]) {
          case '+': flag_value = TRUE;                break;
          case '-': flag_value = FALSE;               break;
          case 'a': trace.actions = flag_value;       break;
          case 'c': trace.check_actions = flag_value; break;
          case 'd': trace.dynamic = flag_value;       break;
          case 'e': trace.exceptions = flag_value;    break;
          case 'h': trace.heapsize = flag_value;      break;
          case 'm': trace.match = flag_value;         break;
          case 'u': trace.executil = flag_value;      break;
          case '*':
            trace.actions = flag_value;
            trace.check_actions = flag_value;
            trace.dynamic = flag_value;
            trace.exceptions = flag_value;
            trace.heapsize = flag_value;
            trace.match = flag_value;
            trace.executil = flag_value;
            break;
        } /* switch */
      } /* for */
    } /* if */
    if (prot_file_name != NULL) {
      if (protfile != NULL) {
        fclose(protfile);
      } /* if */
      if ((protfile = fopen(prot_file_name, "w")) == NULL) {
        protfile = stdout;
      } /* if */
    } else if (protfile == NULL) {
      protfile = stdout;
    } /* if */
#ifdef TRACE_TRACE
    printf("END set_trace\n");
#endif
  } /* set_trace */



#ifdef ANSI_C

void set_trace2 (const_stritype trace_level)
#else

void set_trace2 (trace_level)
stritype trace_level;
#endif

  {
    cstritype ctrace_level;

  /* set_trace2 */
#ifdef TRACE_TRACE
    printf("BEGIN set_trace2\n");
#endif
    ctrace_level = cp_to_cstri(trace_level);
    if (ctrace_level != NULL) {
      set_trace(ctrace_level, (int) trace_level->size, NULL);
      free_cstri(ctrace_level, trace_level);
    } /* if */
#ifdef TRACE_TRACE
    printf("END set_trace2\n");
#endif
  } /* set_trace2 */

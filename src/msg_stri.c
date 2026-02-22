/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2025  Thomas Mertes                        */
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
/*  Module: Analyzer - Error                                        */
/*  File: seed7/src/msg_stri.c                                      */
/*  Changes: 2025  Thomas Mertes                                    */
/*  Content: Append various types to a string.                      */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "float.h"
#include "limits.h"
#include "ctype.h"

#include "common.h"
#include "data.h"
#include "data_rtl.h"
#include "os_decls.h"
#include "heaputl.h"
#include "striutl.h"
#include "datautl.h"
#include "identutl.h"
#include "actutl.h"
#include "flt_rtl.h"
#include "set_rtl.h"
#include "str_rtl.h"
#include "big_drv.h"
#include "pcs_drv.h"
#include "symbol.h"

#undef EXTERN
#define EXTERN
#include "msg_stri.h"



void appendString (striType *const msg, const const_striType stri)

  {
    memSizeType new_size;
    striType msg_dest;
    striType new_msg;
    memSizeType stri_size;

  /* appendString */
    logFunction(printf("appendString(\"%s\", ", striAsUnquotedCStri(*msg));
                printf("\"%s\")\n", striAsUnquotedCStri(stri)););
    msg_dest = *msg;
    if (likely(msg_dest != NULL)) {
      stri_size = stri->size;
      if (unlikely(msg_dest->size > MAX_STRI_LEN - stri_size)) {
        logError(printf("appendString(\"%s\", ",
                        striAsUnquotedCStri(msg_dest));
                 printf("\"%s\"): "
                        "Number of bytes does not fit into memSizeType.\n",
                        striAsUnquotedCStri(stri)););
        FREE_STRI(msg_dest);
        *msg = NULL;
      } else {
        new_size = msg_dest->size + stri_size;
#if WITH_STRI_CAPACITY
        if (new_size > msg_dest->capacity) {
          new_msg = growStri(msg_dest, new_size);
          if (unlikely(new_msg == NULL)) {
            logError(printf("appendString(\"%s\", ",
                            striAsUnquotedCStri(msg_dest));
                     printf("\"%s\"): growStri() failed.\n",
                            striAsUnquotedCStri(stri)););
            FREE_STRI(msg_dest);
            *msg = NULL;
            return;
          } else {
            msg_dest = new_msg;
            *msg = msg_dest;
          } /* if */
        } /* if */
        memcpy(&msg_dest->mem[msg_dest->size], stri->mem,
               stri_size * sizeof(strElemType));
        msg_dest->size = new_size;
#else
        GROW_STRI(new_msg, msg_dest, new_size);
        if (unlikely(new_msg == NULL)) {
          logError(printf("appendString(\"%s\", ",
                          striAsUnquotedCStri(msg_dest));
                   printf("\"%s\"): GROW_STRI() failed.\n",
                          striAsUnquotedCStri(stri)););
          FREE_STRI(msg_dest);
          *msg = NULL;
        } else {
          memcpy(&new_msg->mem[new_msg->size], stri->mem,
                 stri_size * sizeof(strElemType));
          new_msg->size = new_size;
          *msg = new_msg;
        } /* if */
#endif
      } /* if */
    } /* if */
    logFunction(printf("appendString(\"%s\", ", striAsUnquotedCStri(*msg));
                printf("\"%s\") -->\n", striAsUnquotedCStri(stri)););
  } /* appendString */



void appendChar (striType *const msg, const charType aChar)

  {
    memSizeType new_size;
    striType msg_dest;
    striType new_msg;

  /* appendChar */
    logFunction(printf("appendChar(\"%s\", '\\" FMT_U32 ";')\n",
                       striAsUnquotedCStri(*msg), aChar););
    msg_dest = *msg;
    if (likely(msg_dest != NULL)) {
      new_size = msg_dest->size + 1;
#if WITH_STRI_CAPACITY
      if (new_size > msg_dest->capacity) {
        new_msg = growStri(msg_dest, new_size);
        if (unlikely(new_msg == NULL)) {
          logError(printf("appendChar(\"%s\", '\\" FMT_U32 ";'): "
                          "growStri() failed.\n",
                          striAsUnquotedCStri(msg_dest), aChar););
          FREE_STRI(msg_dest);
          *msg = NULL;
          return;
        } else {
          msg_dest = new_msg;
          *msg = msg_dest;
        } /* if */
      } /* if */
      msg_dest->mem[msg_dest->size] = aChar;
      msg_dest->size = new_size;
#else
      GROW_STRI(new_msg, msg_dest, new_size);
      if (unlikely(new_msg == NULL)) {
        logError(printf("appendChar(\"%s\", '\\" FMT_U32 ";'): "
                        "GROW_STRI() failed.\n",
                        striAsUnquotedCStri(msg_dest), aChar););
        FREE_STRI(msg_dest);
        *msg = NULL;
      } else {
        new_msg->mem[new_msg->size] = aChar;
        new_msg->size = new_size;
        *msg = new_msg;
      } /* if */
#endif
    } /* if */
    logFunction(printf("appendChar(\"%s\", '\\" FMT_U32 ";') -->\n",
                       striAsUnquotedCStri(*msg), aChar););
  } /* appendChar */



void appendCStri (striType *const msg, const const_cstriType cstri)

  {
    striType stri;

  /* appendCStri */
    logFunction(printf("appendCStri(\"%s\", \"%s\")\n",
                       striAsUnquotedCStri(*msg), cstri););
    if (likely(*msg != NULL)) {
      stri = cstri_to_stri(cstri);
      if (unlikely(stri == NULL)) {
        logError(printf("appendCStri(\"%s\", \"%s\"): "
                        "cstri_to_stri() failed.\n",
                        striAsUnquotedCStri(*msg), cstri););
        FREE_STRI(*msg);
        *msg = NULL;
      } else {
        appendString(msg, stri);
        FREE_STRI(stri);
      } /* if */
    } /* if */
    logFunction(printf("appendCStri(\"%s\", \"%s\") -->\n",
                       striAsUnquotedCStri(*msg), cstri););
  } /* appendCStri */



void appendCStri8 (striType *const msg, const const_cstriType cstri8)

  {
    striType stri;
    errInfoType err_info = OKAY_NO_ERROR;

  /* appendCStri8 */
    logFunction(printf("appendCStri8(\"%s\", \"%s\")\n",
                       striAsUnquotedCStri(*msg), cstri8););
    if (likely(*msg != NULL)) {
      stri = cstri8_to_stri(cstri8, &err_info);
      if (stri != NULL) {
        appendString(msg, stri);
        FREE_STRI(stri);
      } else {
        appendCStri(msg, cstri8);
      } /* if */
    } /* if */
    logFunction(printf("appendCStri8(\"%s\", \"%s\") -->\n",
                       striAsUnquotedCStri(*msg), cstri8););
  } /* appendCStri8 */



void appendInt (striType *const msg, const intType number)

  {
    char buffer[INTTYPE_DECIMAL_SIZE + NULL_TERMINATION_LEN];

  /* appendInt */
    logFunction(printf("appendInt(\"%s\", " FMT_D ")\n",
                       striAsUnquotedCStri(*msg), number););
    sprintf(buffer, FMT_D, number);
    appendCStri(msg, buffer);
    logFunction(printf("appendInt(\"%s\", " FMT_D ") -->\n",
                       striAsUnquotedCStri(*msg), number););
  } /* appendInt */



#if WITH_FLOAT
void appendFloat (striType *const msg, const floatType number)

  {
    char bufferE[FMT_E_BUFFER_SIZE];
    char bufferF[DOUBLE_TO_CHAR_BUFFER_SIZE];
    memSizeType lenE;
    memSizeType lenF;

  /* appendFloat */
    logFunction(printf("appendFloat(\"%s\", " FMT_E ")\n",
                       striAsUnquotedCStri(*msg), number););
    if (os_isnan(number)) {
      appendCStri(msg, "NaN");
    } else if (number == POSITIVE_INFINITY) {
      appendCStri(msg, "Infinity");
    } else if (number == NEGATIVE_INFINITY) {
      appendCStri(msg, "-Infinity");
    } else {
      lenE = doubleToFormatE(number, bufferE);
      lenF = doubleToCharBuffer(number, FLOATTYPE_STR_LARGE_NUMBER,
                                FMT_E, bufferF);
      bufferF[lenF] = '\0';
      if (lenE < lenF) {
        appendCStri(msg, bufferE);
      } else {
        appendCStri(msg, bufferF);
      } /* if */
    } /* if */
    logFunction(printf("appendFloat(\"%s\", " FMT_E ") -->\n",
                       striAsUnquotedCStri(*msg), number););
  } /* appendFloat */
#endif



void appendBigInt (striType *const msg, const const_bigIntType number)

  {
    striType stri;

  /* appendBigInt */
    logFunction(printf("appendBigInt(\"%s\", %s)\n",
                       striAsUnquotedCStri(*msg), bigHexCStri(number)););
    if (likely(*msg != NULL)) {
      stri = bigStrDecimal(number);
      if (unlikely(stri == NULL)) {
        logError(printf("appendBigInt(\"%s\", %s): "
                        "bigStrDecimal() failed.\n",
                        striAsUnquotedCStri(*msg), bigHexCStri(number)););
        FREE_STRI(*msg);
        *msg = NULL;
      } else {
        appendString(msg, stri);
        FREE_STRI(stri);
      } /* if */
    } /* if */
    logFunction(printf("appendBigInt(\"%s\", %s) -->\n",
                       striAsUnquotedCStri(*msg), bigHexCStri(number)););
  } /* appendBigInt */



void appendCharLiteral (striType *const msg, const charType aChar)

  {
    char buffer[2 + MAXIMUM_UTF32_ESCAPE_WIDTH + NULL_TERMINATION_LEN];

  /* appendCharLiteral */
    logFunction(printf("appendCharLiteral(\"%s\", '\\" FMT_U32 ";')\n",
                       striAsUnquotedCStri(*msg), aChar););
    if (aChar < 127) {
      if (aChar < ' ') {
        sprintf(buffer, "'%s'", stri_escape_sequence[aChar]);
      } else {
        sprintf(buffer, "'%c'", (int) aChar);
      } /* if */
    } else {
      sprintf(buffer, "'\\%lu;'", (unsigned long) aChar);
    } /* if */
    appendCStri(msg, buffer);
    logFunction(printf("appendCharLiteral(\"%s\", '\\" FMT_U32 ";') -->\n",
                       striAsUnquotedCStri(*msg), aChar););
  } /* appendCharLiteral */



void appendStriLiteral (striType *const msg, const const_striType stri)

  {
    memSizeType size;
    strElemType ch;
    memSizeType idx;
    char escapeBuffer[ESC_SEQUENCE_MAX_LEN + NULL_TERMINATION_LEN];

  /* appendStriLiteral */
    logFunction(printf("appendStriLiteral(\"%s\", ", striAsUnquotedCStri(*msg));
                printf("\"%s\")\n", striAsUnquotedCStri(stri)););
    if (stri != NULL) {
      appendChar(msg, '"');
      size = stri->size;
      if (size > AND_SO_ON_LIMIT) {
        size = AND_SO_ON_LIMIT;
      } /* if */
      for (idx = 0; idx < size; idx++) {
        ch = stri->mem[idx];
        if (ch < 127) {
          if (ch < ' ') {
            appendCStri(msg, stri_escape_sequence[ch]);
          } else {
            if (ch == (charType) '\\' || ch == (charType) '"') {
              appendChar(msg, '\\');
            } /* if */
            appendChar(msg, ch);
          } /* if */
        } else if (ch == (charType) -1) {
          appendCStri(msg, "\\-1;");
        } else {
          sprintf(escapeBuffer, "\\" FMT_U32 ";", ch);
          appendCStri(msg, escapeBuffer);
        } /* if */
      } /* for */
      if (stri->size > AND_SO_ON_LIMIT) {
        appendCStri(msg, AND_SO_ON_TEXT);
        appendInt(msg, (intType) stri->size);
      } /* if */
      appendChar(msg, '"');
    } else {
      appendCStri(msg, " *NULL_STRING* ");
    } /* if */
    logFunction(printf("appendStriLiteral(\"%s\", ", striAsUnquotedCStri(*msg));
                printf("\"%s\") -->\n", striAsUnquotedCStri(stri)););
  } /* appendStriLiteral */



void appendBStriLiteral (striType *const msg, const const_bstriType bstri)

  { /* appendBStriLiteral */
    logFunction(printf("appendStriLiteral(\"%s\", ", striAsUnquotedCStri(*msg));
                printf("\"%s\")\n", bstriAsUnquotedCStri(bstri)););
    if (bstri != NULL) {
      appendChar(msg, '"');
      appendCStri(msg, bstriAsUnquotedCStri(bstri));
      appendChar(msg, '"');
    } else {
      appendCStri(msg, " *NULL_BYTE_STRING* ");
    } /* if */
    logFunction(printf("appendStriLiteral(\"%s\", ", striAsUnquotedCStri(*msg));
                printf("\"%s\") -->\n", bstriAsUnquotedCStri(bstri)););
  } /* appendBStriLiteral */



void appendSet (striType *const msg, const_setType setValue)

  {
    intType position;
    bitSetType bitset_elem;
    unsigned int bit_index;
    boolType first_elem;

  /* appendSet */
    logFunction(printf("appendSet(\"%s\", *)\n",
                       striAsUnquotedCStri(*msg)););
    if (setValue != NULL) {
      appendChar(msg, '{');
      first_elem = TRUE;
      for (position = setValue->min_position; position <= setValue->max_position; position++) {
        bitset_elem = setValue->bitset[position - setValue->min_position];
        if (bitset_elem != 0) {
          for (bit_index = 0; bit_index < CHAR_BIT * sizeof(bitSetType); bit_index++) {
            if (bitset_elem & ((bitSetType) 1) << bit_index) {
              if (first_elem) {
                first_elem = FALSE;
              } else {
                appendCStri(msg, ", ");
              } /* if */
              appendInt(msg, position << bitset_shift | (intType) bit_index);
            } /* if */
          } /* for */
        } /* if */
      } /* for */
      appendChar(msg, '}');
    } else {
      appendCStri(msg, " *NULL_SET* ");
    } /* if */
    logFunction(printf("appendSet(\"%s\", *) -->\n",
                       striAsUnquotedCStri(*msg)););
  } /* appendSet */



void appendType (striType *const msg, const const_typeType anytype)

  { /* appendType */
    logFunction(printf("appendType(\"%s\", *)\n",
                       striAsUnquotedCStri(*msg)););
    if (anytype != NULL) {
      if (anytype->name != NULL) {
        appendUStri(msg, anytype->name->entity->ident->name);
      } else if (anytype->result_type != NULL) {
        if (anytype->is_varfunc_type) {
          appendCStri(msg, "varfunc ");
        } else {
          appendCStri(msg, "func ");
        } /* if */
        appendType(msg, anytype->result_type);
      } else {
        appendCStri(msg, " *ANONYM_TYPE* ");
      } /* if */
    } else {
      appendCStri(msg, " *NULL_TYPE* ");
    } /* if */
    logFunction(printf("appendType(\"%s\", *) -->\n",
                       striAsUnquotedCStri(*msg)););
  } /* appendType */



void appendBlock (striType *const msg, const_blockType blockValue)

  { /* appendBlock */
    logFunction(printf("appendBlock(\"%s\", *)\n",
                       striAsUnquotedCStri(*msg)););
    if (blockValue == NULL) {
      appendCStri(msg, " *NULL_BLOCK* ");
    } else if (blockValue->result.object == NULL) {
      appendCStri(msg, " *NULL_OBJECT* ");
    } else {
      appendCStri(msg, "func result ");
      appendType(msg, blockValue->result.object->type_of);
    } /* if */
    logFunction(printf("appendBlock(\"%s\", *) -->\n",
                       striAsUnquotedCStri(*msg)););
  } /* appendBlock */



void appendCategory (striType *const msg, const objectCategory category)

  { /* appendCategory */
    logFunction(printf("appendSymbol(\"%s\", %u)\n",
                       striAsUnquotedCStri(*msg), category););
    if (category >= SYMBOLOBJECT && category <= ILLEGALOBJECT) {
      appendCStri(msg, category_cstri(category));
    } else {
      appendInt(msg, (intType) category);
    } /* if */
    logFunction(printf("appendSymbol(\"%s\", %u) -->\n",
                       striAsUnquotedCStri(*msg), category););
  } /* appendCategory */



void appendSymbol (striType *const msg, const symbolType *const aSymbol)

  { /* appendSymbol */
    logFunction(printf("appendSymbol(\"%s\", *)\n",
                       striAsUnquotedCStri(*msg)););
    if (aSymbol->sycategory == PARENSYMBOL) {
      appendChar(msg, '"');
      appendChar(msg, (charType) aSymbol->name[0]);
      appendChar(msg, '"');
    } else if (aSymbol->sycategory == INTLITERAL) {
      appendChar(msg, '"');
      appendInt(msg, aSymbol->intValue);
      appendChar(msg, '"');
    } else if (aSymbol->sycategory == BIGINTLITERAL) {
      appendChar(msg, '"');
      appendUStri(msg, aSymbol->name);
      appendCStri(msg, "_\"");
    } else if (aSymbol->sycategory == CHARLITERAL) {
      appendCharLiteral(msg, aSymbol->charValue);
    } else if (aSymbol->sycategory == STRILITERAL) {
      appendStriLiteral(msg, aSymbol->striValue);
#if WITH_FLOAT
    } else if (aSymbol->sycategory == FLOATLITERAL) {
      appendChar(msg, '"');
      appendUStri(msg, aSymbol->name);
      appendChar(msg, '"');
#endif
    } else {
      appendChar(msg, '"');
      appendUStri(msg, aSymbol->name);
      appendChar(msg, '"');
    } /* if */
    logFunction(printf("appendSymbol(\"%s\", *) -->\n",
                       striAsUnquotedCStri(*msg)););
  } /* appendSymbol */



void appendRealValue (striType *const msg, const const_objectType anyobject)

  {
    structType structValue;
    striType stri;

  /* appendRealValue */
    logFunction(printf("appendRealValue(\"%s\", *)\n",
                       striAsUnquotedCStri(*msg)););
    switch (CATEGORY_OF_OBJ(anyobject)) {
      case INTOBJECT:
        appendInt(msg, anyobject->value.intValue);
        break;
      case BIGINTOBJECT:
        appendBigInt(msg, anyobject->value.bigIntValue);
        break;
      case CHAROBJECT:
        appendCharLiteral(msg, anyobject->value.charValue);
        break;
      case STRIOBJECT:
        appendStriLiteral(msg, anyobject->value.striValue);
        break;
      case BSTRIOBJECT:
      case POINTLISTOBJECT:
        appendBStriLiteral(msg, anyobject->value.bstriValue);
        break;
      case FILEOBJECT:
        if (anyobject->value.fileValue == NULL) {
          appendCStri(msg, " *NULL_FILE* ");
        } else {
          if (anyobject->value.fileValue->cFile == NULL) {
            appendCStri(msg, " *CLIB_NULL_FILE* ");
          } else if (anyobject->value.fileValue->cFile == stdin) {
            appendCStri(msg, "stdin");
          } else if (anyobject->value.fileValue->cFile == stdout) {
            appendCStri(msg, "stdout");
          } else if (anyobject->value.fileValue->cFile == stderr) {
            appendCStri(msg, "stderr");
          } else {
            appendCStri(msg, "file ");
            appendInt(msg, (intType) safe_fileno(anyobject->value.fileValue->cFile));
          } /* if */
        } /* if */
        break;
      case SOCKETOBJECT:
        appendCStri(msg, "socket ");
        appendInt(msg, (intType) anyobject->value.socketValue);
        break;
#if WITH_FLOAT
      case FLOATOBJECT:
        appendFloat(msg, anyobject->value.floatValue);
        break;
#endif
      case ARRAYOBJECT:
        if (anyobject->value.arrayValue != NULL) {
          appendCStri(msg, "array[");
          appendInt(msg, anyobject->value.arrayValue->min_position);
          appendCStri(msg, "..");
          appendInt(msg, anyobject->value.arrayValue->max_position);
          appendChar(msg, ']');
        } else {
          appendCStri(msg, " *NULL_ARRAY* ");
        } /* if */
        break;
      case HASHOBJECT:
        if (anyobject->value.hashValue != NULL) {
          appendCStri(msg, "hash[");
          appendInt(msg, (intType) anyobject->value.hashValue->size);
          appendChar(msg, ']');
        } else {
          appendCStri(msg, " *NULL_HASH* ");
        } /* if */
        break;
      case STRUCTOBJECT:
        structValue = anyobject->value.structValue;
        if (structValue != NULL) {
          appendCStri(msg, "struct[");
          appendInt(msg, (intType) structValue->size);
          appendChar(msg, ']');
          if (structValue->usage_count != 0) {
            appendChar(msg, '<');
            appendInt(msg, (intType) structValue->usage_count);
            appendChar(msg, '>');
          } /* if */
          /* appendChar(msg, ' ');
             appendPtr(msg, structValue); */
        } else {
          appendCStri(msg, " *NULL_STRUCT* ");
        } /* if */
        break;
      case INTERFACEOBJECT:
        if (anyobject->value.objValue != NULL) {
          if (CATEGORY_OF_OBJ(anyobject->value.objValue) == STRUCTOBJECT) {
            structValue = anyobject->value.objValue->value.structValue;
            if (structValue != NULL) {
              appendCStri(msg, "interface[");
              appendInt(msg, (intType) structValue->size);
              appendChar(msg, ']');
              if (structValue->usage_count != 0) {
                appendChar(msg, '<');
                appendInt(msg, (intType) structValue->usage_count);
                appendChar(msg, '>');
              } /* if */
              appendChar(msg, ' ');
              appendPtr(msg, structValue);
              appendChar(msg, ' ');
              appendPtr(msg, anyobject);
            } else {
              appendCStri(msg, " *INTERFACE_NULL_STRUCT* ");
            } /* if */
          } else {
            appendCStri(msg, " *INTERFACE_TO_");
            appendCategory(msg, CATEGORY_OF_OBJ(anyobject->value.objValue));
            appendCStri(msg, "* ");
            appendPtr(msg, anyobject->value.objValue);
            appendChar(msg, ' ');
            appendPtr(msg, anyobject);
          } /* if */
        } else {
          appendCStri(msg, " *NULL_INTERFACE* ");
        } /* if */
        break;
      case SETOBJECT:
        appendSet(msg, anyobject->value.setValue);
        break;
      case ACTOBJECT:
        appendCStri(msg, "action \"");
        appendCStri(msg, getActEntry(anyobject->value.actValue)->name);
        appendChar(msg, '"');
        break;
      case BLOCKOBJECT:
        appendBlock(msg, anyobject->value.blockValue);
        break;
      case WINOBJECT:
        if (anyobject->value.winValue == NULL) {
          appendCStri(msg, " *NULL_WINDOW* ");
        } else {
          appendCStri(msg, "window [");
          appendInt(msg, (intType) anyobject->value.winValue->usage_count);
          appendCStri(msg, "] ");
          appendPtr(msg, anyobject->value.winValue);
        } /* if */
        break;
      case PROCESSOBJECT:
        if (anyobject->value.winValue == NULL) {
          appendCStri(msg, " *NULL_PROCESS* ");
        } else {
          appendCStri(msg, "process [");
          appendInt(msg, (intType) anyobject->value.processValue->usage_count);
          appendCStri(msg, "] ");
          stri = pcsStr(anyobject->value.processValue);
          appendStriLiteral(msg, stri);
          strDestr(stri);
        } /* if */
        break;
      case PROGOBJECT:
        if (anyobject->value.progValue == NULL) {
          appendCStri(msg, " *NULL_PROG* ");
        } else {
          appendCStri(msg, "prog ");
          appendPtr(msg, anyobject->value.progValue);
        } /* if */
        break;
      default:
        break;
    } /* switch */
    logFunction(printf("appendRealValue(\"%s\", *) -->\n",
                       striAsUnquotedCStri(*msg)););
  } /* appendRealValue */



void appendValue (striType *const msg, const_objectType anyobject)

  { /* appendValue */
    logFunction(printf("appendValue(\"%s\", *)\n",
                       striAsUnquotedCStri(*msg)););
    if (HAS_ENTITY(anyobject) &&
        IS_NORMAL_IDENT(GET_ENTITY(anyobject)->ident)) {
      appendCStri8(msg, id_string(GET_ENTITY(anyobject)->ident));
    } else {
      appendRealValue(msg, anyobject);
    } /* if */
    logFunction(printf("appendValue(\"%s\", *) -->\n",
                       striAsUnquotedCStri(*msg)););
  } /* appendValue */



void appendFormalParam (striType *const msg, const const_objectType formalParam)

  { /* appendFormalParam */
    logFunction(printf("appendFormalParam(\"%s\", *)\n",
                       striAsUnquotedCStri(*msg)););
    switch (CATEGORY_OF_OBJ(formalParam)) {
      case VALUEPARAMOBJECT:
        if (VAR_OBJECT(formalParam)) {
          appendCStri(msg, "in var ");
        } else {
          appendCStri(msg, "val ");
        } /* if */
        appendType(msg, formalParam->type_of);
        if (HAS_ENTITY(formalParam)) {
          appendCStri(msg, ": ");
          appendCStri8(msg, id_string(GET_ENTITY(formalParam)->ident));
        } else {
          appendCStri(msg, " param");
        } /* if */
        break;
      case REFPARAMOBJECT:
        if (VAR_OBJECT(formalParam)) {
          appendCStri(msg, "inout ");
        } else {
          appendCStri(msg, "ref ");
        } /* if */
        appendType(msg, formalParam->type_of);
        if (HAS_ENTITY(formalParam)) {
          appendCStri(msg, ": ");
          appendCStri8(msg, id_string(GET_ENTITY(formalParam)->ident));
        } else {
          appendCStri(msg, " param");
        } /* if */
        break;
      case TYPEOBJECT:
        appendCStri(msg, "attr ");
        if (HAS_ENTITY(formalParam)) {
          appendCStri8(msg, id_string(GET_ENTITY(formalParam)->ident));
        } else {
          appendType(msg, formalParam->value.typeValue);
        } /* if */
        break;
      default:
        appendCStri(msg, "unknown formal parameter");
        break;
    } /* switch */
    logFunction(printf("appendFormalParam(\"%s\", *) -->\n",
                       striAsUnquotedCStri(*msg)););
  } /* appendFormalParam */



static void appendListElement (striType *const msg, const const_objectType anyobject,
    int depthLimit)

  { /* appendListElement */
    if (anyobject == NULL) {
      appendCStri(msg, "*NULL_OBJECT*");
    } else if (!LEGAL_CATEGORY_FIELD(anyobject)) {
      appendCStri(msg, "*CORRUPT_CATEGORY_FIELD*");
    } else {
      switch (CATEGORY_OF_OBJ(anyobject)) {
        case LISTOBJECT:
        case EXPROBJECT:
          if (depthLimit != 0) {
            appendListLimited(msg, anyobject->value.listValue, depthLimit - 1);
          } else {
            appendCStri(msg, " *** details suppressed *** ");
          } /* if */
          break;
        case CALLOBJECT:
        case MATCHOBJECT:
          if (CATEGORY_OF_OBJ(anyobject->value.listValue->obj) == ACTOBJECT) {
            appendCStri(msg, getActEntry(anyobject->value.listValue->obj->value.actValue)->name);
          } else if (HAS_ENTITY(anyobject->value.listValue->obj) &&
              GET_ENTITY(anyobject->value.listValue->obj)->ident != NULL) {
            appendCStri8(msg, id_string(GET_ENTITY(anyobject->value.listValue->obj)->ident));
          } else {
            appendType(msg, anyobject->value.listValue->obj->type_of);
            appendCStri(msg, ": <");
            appendCategory(msg, CATEGORY_OF_OBJ(anyobject->value.listValue->obj));
            appendCStri(msg, "> ");
          } /* if */
          appendChar(msg, '(');
          if (depthLimit != 0) {
            appendListLimited(msg, anyobject->value.listValue->next, depthLimit - 1);
          } else {
            appendCStri(msg, " *** details suppressed *** ");
          } /* if */
          appendChar(msg, ')');
          break;
        case FORMPARAMOBJECT:
          appendChar(msg, '(');
          appendFormalParam(msg, anyobject->value.objValue);
          appendChar(msg, ')');
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
          appendValue(msg, anyobject);
          break;
        case VARENUMOBJECT:
          if (anyobject->value.objValue != NULL) {
            if (HAS_ENTITY(anyobject->value.objValue) &&
                IS_NORMAL_IDENT(GET_ENTITY(anyobject->value.objValue)->ident)) {
              appendCStri8(msg, id_string(GET_ENTITY(anyobject->value.objValue)->ident));
            } else {
              appendChar(msg, '<');
              appendCategory(msg, CATEGORY_OF_OBJ(anyobject->value.objValue));
              appendChar(msg, '>');
            } /* if */
          } else {
            appendCStri(msg, " *NULL_REF* ");
          } /* if */
          break;
        case TYPEOBJECT:
          appendType(msg, anyobject->value.typeValue);
          break;
        case REFOBJECT:
          if (anyobject->value.objValue == NULL) {
            appendCStri(msg, "NIL");
          } else {
            appendCStri(msg, "reference ");
            appendValue(msg, anyobject->value.objValue);
          } /* if */
          break;
        default:
          if (HAS_ENTITY(anyobject) &&
              GET_ENTITY(anyobject)->ident != NULL) {
            appendCStri8(msg, id_string(GET_ENTITY(anyobject)->ident));
          } else {
            appendType(msg, anyobject->type_of);
            appendCStri(msg, ": <");
            appendCategory(msg, CATEGORY_OF_OBJ(anyobject));
            appendChar(msg, '>');
          } /* if */
          break;
      } /* switch */
    } /* if */
  } /* appendListElement */



void appendListLimited (striType *const msg, const_listType list,
    int depthLimit)

  {
    const_listType list_end;
    int number = 0;

  /* appendListLimited */
    logFunction(printf("appendListLimited(\"%s\", *, %d)\n",
                       striAsUnquotedCStri(*msg), depthLimit););
    list_end = list;
    if (list_end != NULL) {
      while (list_end->next != NULL) {
        list_end = list_end->next;
      } /* while */
    } /* if */
    if (list != NULL &&
        list->obj != NULL &&
        LEGAL_CATEGORY_FIELD(list->obj) &&
	(CATEGORY_OF_OBJ(list->obj) != SYMBOLOBJECT ||
	 (HAS_ENTITY(list->obj) &&
          GET_ENTITY(list->obj)->ident != NULL &&
          GET_ENTITY(list->obj)->ident->prefix_priority == 0)) &&
        list_end != NULL &&
        list_end->obj != NULL &&
        LEGAL_CATEGORY_FIELD(list_end->obj) &&
	CATEGORY_OF_OBJ(list_end->obj) == SYMBOLOBJECT &&
        HAS_ENTITY(list_end->obj) &&
        GET_ENTITY(list_end->obj)->ident != NULL &&
        GET_ENTITY(list_end->obj)->ident->infix_priority == 0) {
      appendCStri8(msg, id_string(GET_ENTITY(list_end->obj)->ident));
      if (list != list_end) {
        appendChar(msg, '(');
        while (list != list_end && number <= 50) {
          appendListElement(msg, list->obj, depthLimit);
          list = list->next;
          if (list != list_end) {
            appendCStri(msg, ", ");
          } /* if */
          number++;
        } /* while */
        if (list != list_end) {
          appendCStri(msg, "*AND_SO_ON*");
        } /* if */
        appendChar(msg, ')');
      } /* if */
    } else {
      appendChar(msg, '{');
      while (list != NULL && number <= 50) {
        appendListElement(msg, list->obj, depthLimit);
        appendChar(msg, ' ');
        list = list->next;
        number++;
      } /* while */
      if (list != NULL) {
        appendCStri(msg, "*AND_SO_ON*");
      } /* if */
      appendChar(msg, '}');
    } /* if */
    logFunction(printf("appendListLimited(\"%s\", *, %d) -->\n",
                       striAsUnquotedCStri(*msg), depthLimit););
  } /* appendListLimited */



void appendList (striType *const msg, const const_listType list)

  { /* appendList */
    logFunction(printf("appendList(\"%s\", *)\n",
                       striAsUnquotedCStri(*msg)););
    appendListLimited(msg, list, -1);
    logFunction(printf("appendList(\"%s\", *) -->\n",
                       striAsUnquotedCStri(*msg)););
  } /* appendList */



void appendDotExpr (striType *const msg, const_listType list)

  {
    int number;
    const_cstriType idString;

  /* appendDotExpr */
    logFunction(printf("appendDotExpr(\"%s\", *)\n",
                       striAsUnquotedCStri(*msg)););
    number = 0;
    while (list != NULL && number <= 50) {
      if (list->obj == NULL) {
        appendCStri(msg, "*NULL_OBJECT*");
      } else if (!LEGAL_CATEGORY_FIELD(list->obj)) {
        appendCStri(msg, "*CORRUPT_CATEGORY_FIELD*");
      } else {
        appendChar(msg, '.');
        /* printcategory(CATEGORY_OF_OBJ(list->obj)); fflush(stdout); */
        switch (CATEGORY_OF_OBJ(list->obj)) {
          case EXPROBJECT:
            appendChar(msg, '(');
            appendDotExpr(msg, list->obj->value.listValue);
            appendChar(msg, ')');
            break;
          case SYMBOLOBJECT:
            if (HAS_ENTITY(list->obj) &&
                GET_ENTITY(list->obj)->ident != NULL) {
              idString = id_string(GET_ENTITY(list->obj)->ident);
              if (isalpha(idString[0])) {
                appendCStri8(msg, idString);
              } else {
                appendChar(msg, ' ');
                appendCStri8(msg, idString);
                appendChar(msg, ' ');
              } /* if */
            } else {
              appendCStri(msg, " ? ");
            } /* if */
            break;
          default:
            appendCStri(msg, " ? ");
            break;
        } /* switch */
      } /* if */
      list = list->next;
      number++;
    } /* while */
    if (list != NULL) {
      appendCStri(msg, "*AND_SO_ON*");
    } /* if */
    logFunction(printf("appendDotExpr(\"%s\", *) -->\n",
                       striAsUnquotedCStri(*msg)););
  } /* appendDotExpr */



void appendObject (striType *const msg, const const_objectType anyobject)

  { /* appendObject */
    logFunction(printf("appendObject(\"%s\", *)\n",
                       striAsUnquotedCStri(*msg)););
    if (anyobject == NULL) {
      appendCStri(msg, "(NULL)");
    } else {
      switch (CATEGORY_OF_OBJ(anyobject)) {
        case TYPEOBJECT:
          appendCStri(msg, "type ");
          appendType(msg, anyobject->value.typeValue);
          break;
        case VALUEPARAMOBJECT:
        case REFPARAMOBJECT:
          appendCStri(msg, "parameter (");
          appendFormalParam(msg, anyobject);
          appendChar(msg, ')');
          break;
        case INTOBJECT:
        case BIGINTOBJECT:
        case CHAROBJECT:
        case STRIOBJECT:
        case FILEOBJECT:
        case FLOATOBJECT:
        case ARRAYOBJECT:
        case STRUCTOBJECT:
        case BLOCKOBJECT:
          if (VAR_OBJECT(anyobject)) {
            appendCStri(msg, "variable ");
          } else {
            appendCStri(msg, "constant ");
          } /* if */
          appendType(msg, anyobject->type_of);
          appendCStri(msg, ": ");
          appendValue(msg, anyobject);
          break;
        case CALLOBJECT:
        case MATCHOBJECT:
          if (anyobject->value.listValue != NULL &&
              anyobject->value.listValue->obj != NULL) {
            appendCStri(msg, "expression (");
            appendList(msg, anyobject->value.listValue->next);
            appendCStri(msg, ") of type ");
            appendType(msg, anyobject->value.listValue->obj->type_of);
          } /* if */
          break;
        case LISTOBJECT:
        case EXPROBJECT:
          if (HAS_ENTITY(anyobject) &&
              GET_ENTITY(anyobject)->ident != NULL) {
            if (GET_ENTITY(anyobject)->ident->name != NULL) {
              if (GET_ENTITY(anyobject)->ident->name[0] != '\0') {
                appendCStri8(msg, id_string(GET_ENTITY(anyobject)->ident));
              } else {
                appendChar(msg, '(');
                appendList(msg, anyobject->value.listValue);
                appendChar(msg, ')');
              } /* if */
            } else {
              appendChar(msg, '(');
              appendList(msg, anyobject->value.listValue);
              appendChar(msg, ')');
            } /* if */
          } else {
            appendChar(msg, '(');
            appendList(msg, anyobject->value.listValue);
            appendChar(msg, ')');
          } /* if */
          break;
        default:
          if (HAS_ENTITY(anyobject)) {
            appendCStri8(msg, id_string(GET_ENTITY(anyobject)->ident));
          } else {
            appendCategory(msg, CATEGORY_OF_OBJ(anyobject));
            appendCStri(msg, " *NULL_ENTITY_OBJECT*");
          } /* if */
          break;
      } /* switch */
    } /* if */
    logFunction(printf("appendObject(\"%s\", *) -->\n",
                       striAsUnquotedCStri(*msg)););
  } /* appendObject */



void appendParams (striType *const msg, const const_listType params)

  {
    const_listType param_list_end;
    const_listType list_elem;
    boolType first_elem = TRUE;
    boolType previous_elem_was_symbol = FALSE;

  /* appendParams */
    logFunction(printf("appendParams(\"%s\", *)\n",
                       striAsUnquotedCStri(*msg)););
    if (params != NULL) {
      param_list_end = params;
      while (param_list_end->next != NULL) {
        param_list_end = param_list_end->next;
      } /* while */
      if (CATEGORY_OF_OBJ(params->obj) != SYMBOLOBJECT &&
          CATEGORY_OF_OBJ(param_list_end->obj) == SYMBOLOBJECT &&
          HAS_ENTITY(param_list_end->obj) &&
          GET_ENTITY(param_list_end->obj)->ident != NULL &&
          GET_ENTITY(param_list_end->obj)->ident->infix_priority == 0) {
        appendCStri8(msg, id_string(GET_ENTITY(param_list_end->obj)->ident));
        first_elem = FALSE;
        previous_elem_was_symbol = TRUE;
      } else {
        param_list_end = NULL;
      } /* if */
      list_elem = params;
      while (list_elem != param_list_end) {
        if (list_elem->obj == NULL) {
          appendCStri(msg, "*NULL_OBJECT*");
        } else {
          switch (CATEGORY_OF_OBJ(list_elem->obj)) {
            case VALUEPARAMOBJECT:
            case REFPARAMOBJECT:
            case TYPEOBJECT:
              if (first_elem) {
                appendChar(msg, '(');
              } else if (previous_elem_was_symbol) {
                appendCStri(msg, " (");
              } else {
                appendCStri(msg, ", ");
              } /* if */
              appendFormalParam(msg, list_elem->obj);
              previous_elem_was_symbol = FALSE;
              break;
            default:
              if (previous_elem_was_symbol) {
                appendChar(msg, ' ');
              } else if (!first_elem) {
                appendCStri(msg, ") ");
              } /* if */
              appendObject(msg, list_elem->obj);
              previous_elem_was_symbol = TRUE;
              break;
          } /* switch */
          first_elem = FALSE;
        } /* if */
        list_elem = list_elem->next;
      } /* while */
      if (!first_elem && !previous_elem_was_symbol) {
        appendChar(msg, ')');
      } /* if */
    } else {
      appendCStri(msg, "{}");
    } /* if */
    logFunction(printf("appendParams(\"%s\", *) -->\n",
                       striAsUnquotedCStri(*msg)););
  } /* appendParams */



void appendNameList (striType *const msg, const_listType params)

  {
    const_listType param_list_end;
    objectType formal_param;
    int in_formal_param_list = 0;

  /* appendNameList */
    logFunction(printf("appendNameList(\"%s\", *)\n",
                       striAsUnquotedCStri(*msg)););
    if (params != NULL) {
      param_list_end = params;
      while (param_list_end->next != NULL) {
        param_list_end = param_list_end->next;
      } /* while */
      if (CATEGORY_OF_OBJ(params->obj) != SYMBOLOBJECT &&
          CATEGORY_OF_OBJ(param_list_end->obj) == SYMBOLOBJECT &&
          HAS_ENTITY(param_list_end->obj) &&
          GET_ENTITY(param_list_end->obj)->ident != NULL &&
          GET_ENTITY(param_list_end->obj)->ident->infix_priority == 0) {
        appendCStri8(msg, id_string(GET_ENTITY(param_list_end->obj)->ident));
      } else {
        param_list_end = NULL;
      } /* if */
      while (params != param_list_end) {
        if (CATEGORY_OF_OBJ(params->obj) == FORMPARAMOBJECT) {
          if (in_formal_param_list) {
            appendCStri(msg, ", ");
          } else {
            appendCStri(msg, " (");
            in_formal_param_list = 1;
          } /* if */
          formal_param = params->obj->value.objValue;
          switch (CATEGORY_OF_OBJ(formal_param)) {
            case VALUEPARAMOBJECT:
            case REFPARAMOBJECT:
            case TYPEOBJECT:
              appendFormalParam(msg, formal_param);
              break;
            default:
              appendCStri(msg, "unknown formal parameter");
              break;
          } /* switch */
        } else {
          if (in_formal_param_list) {
            appendCStri(msg, ") ");
            in_formal_param_list = 0;
          } else {
            appendChar(msg, ' ');
          } /* if */
          switch (CATEGORY_OF_OBJ(params->obj)) {
            case SYMBOLOBJECT:
              if (HAS_ENTITY(params->obj)) {
                appendCStri8(msg, id_string(GET_ENTITY(params->obj)->ident));
              } else {
                appendCStri(msg, "*symbol*");
              } /* if */
              break;
            default:
              appendCStri(msg, "unknown formal parameter");
              break;
          } /* switch */
        } /* if */
        params = params->next;
      } /* while */
      if (in_formal_param_list) {
        appendChar(msg, ')');
      } /* if */
    } /* if */
    logFunction(printf("appendNameList(\"%s\", *) -->\n",
                       striAsUnquotedCStri(*msg)););
  } /* appendNameList */



void appendObjectWithParameters (striType *const msg, const_objectType obj_found)

  { /* appendObjectWithParameters */
    logFunction(printf("appendObjectWithParameters(\"%s\", *)\n",
                       striAsUnquotedCStri(*msg)););
    if (HAS_PROPERTY(obj_found) &&
        obj_found->descriptor.property->params != NULL) {
      appendParams(msg, obj_found->descriptor.property->params);
    } else if (HAS_ENTITY(obj_found)) {
      if (GET_ENTITY(obj_found)->fparam_list != NULL) {
        appendNameList(msg, GET_ENTITY(obj_found)->fparam_list);
      } else {
        appendCStri8(msg, id_string(GET_ENTITY(obj_found)->ident));
      } /* if */
    } /* if */
    logFunction(printf("appendObjectWithParameters(\"%s\", *) -->\n",
                       striAsUnquotedCStri(*msg)););
  } /* appendObjectWithParameters */

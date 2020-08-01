/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2008  Thomas Mertes                        */
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
/*  Module: Reflection                                              */
/*  File: seed7/src/datautl.c                                       */
/*  Changes: 1991, 1992, 1993, 1994  Thomas Mertes                  */
/*  Content: Procedures to maintain objects of type identType.      */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"

#undef EXTERN
#define EXTERN
#include "datautl.h"


static const_cstriType category_name[] = {
    "SYMBOLOBJECT",      /* pos (file, line) - Symbol object        */
                         /*                    created by read_atom */
                         /*                    and read_name        */
    "DECLAREDOBJECT",    /* NO VALUE -         Object declared and  */
                         /*                    not initialized      */
    "FORWARDOBJECT",     /* NO VALUE -         Object declared      */
                         /*                    forward              */
    "FWDREFOBJECT",      /* objValue -    Reference to Object which */
                         /*               was declared forward      */
    "BLOCKOBJECT",       /* blockValue - Procedure possibly with    */
                         /*              parameters, declared       */
                         /*              result or local variables  */
    "CALLOBJECT",        /* listValue - Subroutine call:            */
                         /*             First element is subroutine */
                         /*             Rest of list is parameters  */
    "MATCHOBJECT",       /* listValue - Don't exec subroutine call: */
                         /*             First element is subroutine */
                         /*             Rest of list is parameters  */
    "TYPEOBJECT",        /* typeValue -   type                      */
    "FORMPARAMOBJECT",   /* objValue -    Reference to formal param */
    "INTOBJECT",         /* intValue -    integer                   */
    "BIGINTOBJECT",      /* bigIntValue - bigInteger                */
    "CHAROBJECT",        /* charValue -   char                      */
    "STRIOBJECT",        /* striValue -   string                    */
    "BSTRIOBJECT",       /* bstriValue -  byte string               */
    "ARRAYOBJECT",       /* arrayValue -  array                     */
    "HASHOBJECT",        /* hashValue -   hash                      */
    "STRUCTOBJECT",      /* structValue - struct                    */
    "CLASSOBJECT",       /* structValue - struct                    */
    "INTERFACEOBJECT",   /* objValue -    Dynamic Object            */
    "SETOBJECT",         /* setValue -    set                       */
    "FILEOBJECT",        /* fileValue -   file                      */
    "FILEDESOBJECT",     /* fileDesValue - file descriptor          */
    "SOCKETOBJECT",      /* socketValue - socket                    */
    "POLLOBJECT",        /* pollValue -   poll list                 */
    "LISTOBJECT",        /* listValue -   list                      */
    "FLOATOBJECT",       /* floatValue -  float                     */
    "WINOBJECT",         /* winValue -    Window                    */
    "ENUMLITERALOBJECT", /* objValue -    Enumeration literal       */
    "CONSTENUMOBJECT",   /* objValue -    Constant enumeration obj  */
    "VARENUMOBJECT",     /* objValue -    Variable enumeration obj  */
    "REFOBJECT",         /* objValue -    reference                 */
    "REFLISTOBJECT",     /* listValue -   ref_list                  */
    "EXPROBJECT",        /* listValue -   expression                */
    "ACTOBJECT",         /* actValue -    Action                    */
    "VALUEPARAMOBJECT",  /* objValue -    Formal value parameter    */
    "REFPARAMOBJECT",    /* objValue -    Formal ref parameter      */
    "RESULTOBJECT",      /* objValue -    Result of procedure       */
    "LOCALVOBJECT",      /* objValue -    Local variable            */
    "DATABASEOBJECT",    /* databaseValue - Database                */
    "SQLSTMTOBJECT",     /* sqlStmtValue -  SQL statement           */
    "PROGOBJECT"         /* progValue -   Program                   */
  };



const_cstriType category_cstri (objectCategory category)

  {
    const_cstriType result;

  /* category_cstri */
    if (category >= SYMBOLOBJECT && category <= PROGOBJECT) {
      result = category_name[(int) category];
    } else {
      result = "*UNKNOWN*";
    } /* if */
    return result;
  } /* category_cstri */



intType category_value (const const_cstriType stri)

  {
    intType number;

  /* category_value */
    for (number = SYMBOLOBJECT; number <= PROGOBJECT; number++) {
      if (strcmp(stri, category_name[number]) == 0) {
        return number;
      } /* if */
    } /* for */
    return -1;
  } /* category_value */



const_cstriType id_string (const_identType actual_ident)

  {
    const_cstriType result;

  /* id_string */
    if (actual_ident == NULL) {
      result = " *NULL_IDENT* ";
    } else if (actual_ident->name == NULL) {
      result = " *NULL_NAME_IDENT* ";
    } else if (actual_ident->name[0] == '\0') {
      result = " *NULL_STRING_IDENT* ";
    } else if (memchr((cstriType) actual_ident->name, '\0', 51) == NULL) {
      result = " *GARBAGE_IDENT* ";
    } else {
      result = (const_cstriType) actual_ident->name;
    } /* if */
    return result;
  } /* id_string */



const_cstriType id_string2 (const_identType actual_ident)

  {
    const_cstriType result;

  /* id_string2 */
    if (actual_ident == NULL) {
      result = " *NULL_IDENT* ";
    } else if (actual_ident->name == NULL) {
      result = " *NULL_NAME_IDENT* ";
    } else {
      result = (const_cstriType) actual_ident->name;
    } /* if */
    return result;
  } /* id_string2 */

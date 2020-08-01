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
/*  Content: Procedures to maintain objects of type identtype.      */
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


static const_cstritype category_name[] = {
    "SYMBOLOBJECT",      /* pos (file, line) - Symbol object        */
                         /*                    created by read_atom */
                         /*                    and read_name        */
    "DECLAREDOBJECT",    /* NO VALUE -         Object declared and  */
                         /*                    not initialized      */
    "FORWARDOBJECT",     /* NO VALUE -         Object declared      */
                         /*                    forward              */
    "FWDREFOBJECT",      /* objvalue -    Reference to Object which */
                         /*               was declared forward      */
    "BLOCKOBJECT",       /* blockvalue - Procedure possibly with    */
                         /*              parameters, declared       */
                         /*              result or local variables  */
    "CALLOBJECT",        /* listvalue - Subroutine call:            */
                         /*             First element is subroutine */
                         /*             Rest of list is parameters  */
    "MATCHOBJECT",       /* listvalue - Don't exec subroutine call: */
                         /*             First element is subroutine */
                         /*             Rest of list is parameters  */
    "TYPEOBJECT",        /* typevalue -   type                      */
    "FORMPARAMOBJECT",   /* objvalue -    Reference to formal param */
    "INTOBJECT",         /* intvalue -    integer                   */
    "BIGINTOBJECT",      /* bigintvalue - bigInteger                */
    "CHAROBJECT",        /* charvalue -   char                      */
    "STRIOBJECT",        /* strivalue -   string                    */
    "BSTRIOBJECT",       /* bstrivalue -  byte string               */
    "ARRAYOBJECT",       /* arrayvalue -  array                     */
    "HASHOBJECT",        /* hashvalue -   hash                      */
    "STRUCTOBJECT",      /* structvalue - struct                    */
    "CLASSOBJECT",       /* structvalue - struct                    */
    "INTERFACEOBJECT",   /* objvalue -    Dynamic Object            */
    "SETOBJECT",         /* setvalue -    set                       */
    "FILEOBJECT",        /* filevalue -   file                      */
    "SOCKETOBJECT",      /* socketvalue - socket                    */
    "POLLOBJECT",        /* pollvalue -   poll list                 */
    "LISTOBJECT",        /* listvalue -   list                      */
    "FLOATOBJECT",       /* floatvalue -  float                     */
    "WINOBJECT",         /* winvalue -    Window                    */
    "ENUMLITERALOBJECT", /* objvalue -    Enumeration literal       */
    "CONSTENUMOBJECT",   /* objvalue -    Constant enumeration obj  */
    "VARENUMOBJECT",     /* objvalue -    Variable enumeration obj  */
    "REFOBJECT",         /* objvalue -    reference                 */
    "REFLISTOBJECT",     /* listvalue -   ref_list                  */
    "EXPROBJECT",        /* listvalue -   expression                */
    "ACTOBJECT",         /* actvalue -    Action                    */
    "VALUEPARAMOBJECT",  /* objvalue -    Formal value parameter    */
    "REFPARAMOBJECT",    /* objvalue -    Formal ref parameter      */
    "RESULTOBJECT",      /* objvalue -    Result of procedure       */
    "LOCALVOBJECT",      /* objvalue -    Local variable            */
    "PROGOBJECT"         /* progvalue -   Program                   */
  };



const_cstritype category_cstri (objectcategory category)

  {
    const_cstritype result;

  /* category_cstri */
    if (category >= SYMBOLOBJECT && category <= PROGOBJECT) {
      result = category_name[(int) category];
    } else {
      result = "*UNKNOWN*";
    } /* if */
    return result;
  } /* category_cstri */



inttype category_value (const const_cstritype stri)

  {
    inttype number;

  /* category_value */
    for (number = SYMBOLOBJECT; number <= PROGOBJECT; number++) {
      if (strcmp(stri, category_name[number]) == 0) {
        return number;
      } /* if */
    } /* for */
    return -1;
  } /* category_value */



const_cstritype id_string (const_identtype actual_ident)

  {
    const_cstritype result;

  /* id_string */
    if (actual_ident == NULL) {
      result = " *NULL_IDENT* ";
    } else if (actual_ident->name == NULL) {
      result = " *NULL_NAME_IDENT* ";
    } else if (actual_ident->name[0] == '\0') {
      result = " *NULL_STRING_IDENT* ";
    } else if (memchr((cstritype) actual_ident->name, '\0', 51) == NULL) {
      result = " *GARBAGE_IDENT* ";
    } else {
      result = (const_cstritype) actual_ident->name;
    } /* if */
    return result;
  } /* id_string */

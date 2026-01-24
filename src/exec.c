/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2004, 2011 - 2015, 2017  Thomas Mertes     */
/*                2019 - 2021  Thomas Mertes                        */
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
/*  Module: Interpreter                                             */
/*  File: seed7/src/exec.c                                          */
/*  Changes: 1999, 2000, 2004, 2011 - 2015, 2017  Thomas Mertes     */
/*           2019 - 2021  Thomas Mertes                             */
/*  Content: Main interpreter functions.                            */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "sigutl.h"
#include "heaputl.h"
#include "flistutl.h"
#include "datautl.h"
#include "entutl.h"
#include "syvarutl.h"
#include "listutl.h"
#include "traceutl.h"
#include "actutl.h"
#include "executl.h"
#include "objutl.h"
#include "runerr.h"
#include "match.h"
#include "prclib.h"

#undef EXTERN
#define EXTERN
#define DO_INIT
#include "exec.h"


extern boolType interpreter_exception;



void doSuspendInterpreter (int signalNum)

  { /* doSuspendInterpreter */
    logFunction(printf("doSuspendInterpreter(%d)\n", signalNum););
    interrupt_flag = TRUE;
    signal_number = signalNum;
  } /* doSuspendInterpreter */



objectType exec_object (register objectType object)

  {
    register objectType result;

  /* exec_object */
    logFunction(printf("exec_object ");
                trace1(object);
                printf("\n"););
    switch (CATEGORY_OF_OBJ(object)) {
      case CALLOBJECT:
        result = exec_call(object);
#ifdef OUT_OF_ORDER
        if (fail_flag) {
          printf("propagate exception ");
          trace1(object);
          printf("\n"); */
        } /* if */
#endif
        break;
      case VALUEPARAMOBJECT:
      case REFPARAMOBJECT:
      case RESULTOBJECT:
      case LOCALVOBJECT:
        if (object->value.objValue != NULL) {
          result = object->value.objValue;
        } else {
          result = object;
        } /* if */
        break;
      case MATCHOBJECT:
        /* This is NONSENSE:
        printf("exec_object1: MATCHOBJECT ");
        trace1(object);
        printf("\n");
        result = object->value.listValue->obj;
        printf("exec_object2: MATCHOBJECT ");
        trace1(result);
        printf("\n");
        break; */
      case DECLAREDOBJECT:
      case SYMBOLOBJECT:
      case BLOCKOBJECT:
      case TYPEOBJECT:
      case INTOBJECT:
      case BIGINTOBJECT:
      case CHAROBJECT:
      case STRIOBJECT:
      case BSTRIOBJECT:
      case ARRAYOBJECT:
      case HASHOBJECT:
      case STRUCTOBJECT:
      case FILEOBJECT:
      case LISTOBJECT:
#if WITH_FLOAT
      case FLOATOBJECT:
#endif
      case WINOBJECT:
      case POINTLISTOBJECT:
      case REFOBJECT:
      case REFLISTOBJECT:
      case EXPROBJECT:
      case ACTOBJECT:
      case ENUMLITERALOBJECT:
      case VARENUMOBJECT:
        result = object;
        break;
      default:
/*        printf("exec_object unknown ");
        trace1(object);
        printf("\n"); */
        result = object;
        break;
    } /* switch */
    logFunction(printf("exec_object --> ");
                trace1(result);
                printf("\n"););
    return result;
  } /* exec_object */



/**
 *  When a temporary value is entered into a reference parameter
 *  the TEMP flag must be cleared. This is necessary to avoid
 *  destroying the value inside the function before the end of
 *  the function is reached. Such temporary values are removed
 *  upon function exit by par_restore. When the TEMP flag
 *  is cleared for a temporary reference parameter the TEMP2
 *  flag is set instead. Note that there is no other place
 *  where the TEMP2 flag is set. This TEMP2 flag can be used
 *  by primitive actions like hsh_cpy, hsh_create, hsh_idx
 *  or arr_sort to avoid unnecessary copying of data values.
 *  This must be done with care, because the calling function
 *  cannot access the parameter after the primitive action was
 *  executed. For the actions mentioned above the surrounding
 *  functions are defined in seed7_05.s7i and take care of this.
 *  When a TEMP2 parameter is used for a deeper function call
 *  The TEMP2 flag is cleared to avoid unwanted effects.
 */
static inline void par_init (locListType form_param_list,
    listType *backup_form_params, listType act_param_list,
    listType *evaluated_act_params)

  {
    locListType form_param;
    listType *backup_insert_place;
    listType *evaluated_insert_place;
    listType param_list_elem;
    objectType param_value;
    errInfoType err_info = OKAY_NO_ERROR;

  /* par_init */
    logFunction(printf("par_init\n"););
    form_param = form_param_list;
    *backup_form_params = NULL;
    backup_insert_place = backup_form_params;
    *evaluated_act_params = NULL;
    evaluated_insert_place = evaluated_act_params;
    while (form_param != NULL && !fail_flag) {
      append_to_list(backup_insert_place,
          form_param->local.object->value.objValue, act_param_list);
      param_value = exec_object(act_param_list->obj);
      append_to_list(evaluated_insert_place, param_value, act_param_list);
      form_param = form_param->next;
      act_param_list = act_param_list->next;
    } /* while */
    if (fail_flag) {
      param_list_elem = *evaluated_act_params;
      while (param_list_elem != NULL) {
        if (param_list_elem->obj != NULL && TEMP_OBJECT(param_list_elem->obj)) {
          dump_any_temp(param_list_elem->obj);
        } /* if */
        param_list_elem = param_list_elem->next;
      } /* while */
    } else {
      form_param = form_param_list;
      param_list_elem = *evaluated_act_params;
      while (form_param != NULL && err_info == OKAY_NO_ERROR) {
        param_value = param_list_elem->obj;
        switch (CATEGORY_OF_OBJ(form_param->local.object)) {
          case VALUEPARAMOBJECT:
/*            printf("value param formal ");
            trace1(form_param->local.object);
            printf(" %lu\nparam value ", (unsigned long) form_param->local.object);
            trace1(param_value);
            printf(" %lu\n", (unsigned long) param_value); */
            if (TEMP_OBJECT(param_value)) {
              CLEAR_TEMP_FLAG(param_value);
              COPY_VAR_FLAG(param_value, form_param->local.object);
              form_param->local.object->value.objValue = param_value;
              param_list_elem->obj = NULL;
/*              printf("assign temp ");
              trace1(form_param->local.object);
              printf(" %lu\n", (unsigned long) form_param->local.object); */
            } else {
              CLEAR_TEMP2_FLAG(param_value);
              create_local_object(&form_param->local, param_value, &err_info);
/*              printf("assign obj ");
              trace1(form_param->local.object);
              printf(" %lu\n", (unsigned long) form_param->local.object); */
            } /* if */
            break;
          case REFPARAMOBJECT:
/*          printf("ref param formal " FMT_U_MEM ": ",
                   (memSizeType) form_param->local.object);
            trace1(form_param->local.object);
            printf("\nparam value " FMT_U_MEM ": ",
                   (memSizeType) param_value);
            trace1(param_value);
            printf("\n");  */
            form_param->local.object->value.objValue = param_value;
            if (TEMP_OBJECT(param_value)) {
              CLEAR_TEMP_FLAG(param_value);
              SET_TEMP2_FLAG(param_value);
              /* printf("ref to temp ");
              trace1(form_param->local.object);
              printf(" " FMT_U_MEM "\n",
                     (memSizeType) form_param->local.object); */
            } else {
              CLEAR_TEMP2_FLAG(param_value);
              param_list_elem->obj = NULL;
            } /* if */
            break;
          default:
            /* Do nothing for SYMBOLOBJECT and TYPEOBJECT. */
            break;
        } /* switch */
        form_param = form_param->next;
        param_list_elem = param_list_elem->next;
      } /* while */
    } /* if */
    logFunction(printf("par_init -->\n"););
  } /* par_init */



static inline void par_restore (const_locListType form_param,
    const_listType backup_form_params, const_listType evaluated_act_params)

  {
    failStateStruct savedFailState;

  /* par_restore */
    logFunction(printf("par_restore\n"););
    if (unlikely(fail_flag)) {
      saveFailState(&savedFailState);
      while (form_param != NULL) {
        switch (CATEGORY_OF_OBJ(form_param->local.object)) {
          case VALUEPARAMOBJECT:
            destroy_local_object(&form_param->local, TRUE);
            break;
          case REFPARAMOBJECT:
            if (evaluated_act_params->obj != NULL) {
              dump_any_temp(evaluated_act_params->obj);
            } /* if */
            break;
          default:
            /* Do nothing for SYMBOLOBJECT and TYPEOBJECT. */
            break;
        } /* switch */
        form_param->local.object->value.objValue = backup_form_params->obj;
        form_param = form_param->next;
        backup_form_params = backup_form_params->next;
        evaluated_act_params = evaluated_act_params->next;
      } /* while */
      restoreFailState(&savedFailState);
    } else {
      while (form_param != NULL) {
        if (!fail_flag) {
          switch (CATEGORY_OF_OBJ(form_param->local.object)) {
            case VALUEPARAMOBJECT:
              destroy_local_object(&form_param->local, FALSE);
              break;
            case REFPARAMOBJECT:
              if (evaluated_act_params->obj != NULL) {
                dump_any_temp(evaluated_act_params->obj);
              } /* if */
              break;
            default:
              /* Do nothing for SYMBOLOBJECT and TYPEOBJECT. */
              break;
          } /* switch */
        } /* if */
        form_param->local.object->value.objValue = backup_form_params->obj;
        form_param = form_param->next;
        backup_form_params = backup_form_params->next;
        evaluated_act_params = evaluated_act_params->next;
      } /* while */
    } /* if */
    logFunction(printf("par_restore -->\n"););
  } /* par_restore */



static void loc_init (const_locListType loc_var, listType *backup_loc_var,
    listType act_param_list)

  {
    listType *list_insert_place;
    errInfoType err_info = OKAY_NO_ERROR;

  /* loc_init */
    logFunction(printf("loc_init\n"););
    *backup_loc_var = NULL;
    list_insert_place = backup_loc_var;
    while (loc_var != NULL && !fail_flag) {
      append_to_list(list_insert_place,
          loc_var->local.object->value.objValue, act_param_list);
      create_local_object(&loc_var->local, loc_var->local.init_value, &err_info);
      loc_var = loc_var->next;
    } /* while */
    logFunction(printf("loc_init -->\n"););
  } /* loc_init */



static void loc_restore (const_locListType loc_var, const_listType backup_loc_var)

  {
    failStateStruct savedFailState;

  /* loc_restore */
    logFunction(printf("loc_restore\n"););
    if (unlikely(fail_flag)) {
      saveFailState(&savedFailState);
      while (loc_var != NULL) {
        destroy_local_object(&loc_var->local, TRUE);
        loc_var->local.object->value.objValue = backup_loc_var->obj;
        loc_var = loc_var->next;
        backup_loc_var = backup_loc_var->next;
      } /* while */
      restoreFailState(&savedFailState);
    } else {
      while (loc_var != NULL) {
        if (likely(!fail_flag)) {
          destroy_local_object(&loc_var->local, FALSE);
        } /* if */
        loc_var->local.object->value.objValue = backup_loc_var->obj;
        loc_var = loc_var->next;
        backup_loc_var = backup_loc_var->next;
      } /* while */
    } /* if */
    logFunction(printf("loc_restore -->\n"););
  } /* loc_restore */



static inline boolType res_init (const_locObjType block_result,
    objectType *backup_block_result)

  {
    errInfoType err_info = OKAY_NO_ERROR;

  /* res_init */
    logFunction(printf("res_init\n"););
    /* printf("block_result ");
    trace1(block_result->object);
    printf("\n");
    printf("result_init ");
    trace1(block_result->init_value);
    printf("\n"); */
    if (block_result->object != NULL) {
      /* Backup_block_result is initialized in res_init and used in */
      /* res_restore. Backup_block_result is initialized and used   */
      /* conditionally. In both cases (initialisation and use) the  */
      /* same condition is used. Possible compiler warnings that    */
      /* "it may be used uninitialized" can be ignored.             */
      *backup_block_result = block_result->object->value.objValue;
      create_local_object(block_result, block_result->init_value, &err_info);
    } /* if */
    logFunction(printf("res_init(" FMT_U_MEM ") -->\n",
                       block_result->object != NULL ?
                           ((memSizeType) block_result->object->value.objValue) : 0););
    return err_info == OKAY_NO_ERROR;
  } /* res_init */



static inline void res_restore (const_locObjType block_result,
    objectType backup_block_result, objectType *result)

  {
    errInfoType err_info = OKAY_NO_ERROR;

  /* res_restore */
    logFunction(printf("res_restore(" FMT_U_MEM ")\n",
                       block_result->object != NULL ?
                           ((memSizeType) block_result->object->value.objValue) : 0););
    if (block_result->object != NULL) {
      if (fail_flag) {
        dump_any_temp(block_result->object->value.objValue);
      } else {
        *result = block_result->object->value.objValue;
        /* CLEAR_VAR_FLAG(*result); */
        SET_TEMP_FLAG(*result);
      } /* if */
      /* Backup_block_result is initialized in res_init and used in */
      /* res_restore. Backup_block_result is initialized and used   */
      /* conditionally. In both cases (initialisation and use) the  */
      /* same condition is used. Possible compiler warnings that    */
      /* "it may be used uninitialized" can be ignored.             */
      block_result->object->value.objValue = backup_block_result;
    } else if (*result != NULL && !TEMP_OBJECT(*result) &&
        CATEGORY_OF_OBJ(*result) != ENUMLITERALOBJECT) {
#ifdef OUT_OF_ORDER
      printf("return non temp ");
      trace1(*result);
      printf("\n");
#endif
      if (block_result->create_call_obj != NULL) {
        *result = create_return_object(block_result, *result, &err_info);
        SET_TEMP_FLAG(*result);
      } /* if */
    } /* if */
    logFunction(printf("res_restore(" FMT_U_MEM ") -->\n",
                       (memSizeType) *result););
  } /* res_restore */



#ifdef OUT_OF_ORDER
static void show_arg_list (listType act_param_list)

  { /* show_arg_list */
    while (act_param_list != NULL) {
      if (act_param_list->obj != NULL) {
#ifdef WITH_PROTOCOL
        if (trace.actions) {
          prot_cstri("show_arg_list ");
          printcategory(CATEGORY_OF_OBJ(act_param_list->obj));
          prot_cstri(" ");
          prot_int((intType) act_param_list->obj);
          prot_cstri(" ");
          trace1(act_param_list->obj);
          prot_nl();
        } /* if */
#endif
      } else {
#ifdef WITH_PROTOCOL
        if (trace.actions) {
          prot_cstri("show_arg_list NULL");
          prot_nl();
        } /* if */
#endif
      } /* if */
      act_param_list = act_param_list->next;
    } /* while */
  } /* show_arg_list */
#endif



static objectType exec_lambda (const_blockType block,
    listType actual_parameters, objectType object)

  {
    objectType result;
    listType evaluated_act_params;
    listType backup_form_params;
    objectType backup_block_result;
    listType backup_loc_var;

  /* exec_lambda */
    logFunction(printf("exec_lambda\n"););
    par_init(block->params, &backup_form_params, actual_parameters,
        &evaluated_act_params);
    if (fail_flag) {
      free_list(backup_form_params);
      free_list(evaluated_act_params);
      result = fail_value;
    } else {
      loc_init(block->local_vars, &backup_loc_var, actual_parameters);
      if (fail_flag) {
        free_list(backup_loc_var);
        result = fail_value;
      } else {
        if (res_init(&block->result, &backup_block_result)) {
          result = exec_call(block->body);
          if (fail_flag) {
            errInfoType ignored_err_info;

            /* ignored_err_info is not checked since an exception was already raised */
            incl_list(&fail_stack, object, &ignored_err_info);
          } /* if */
          res_restore(&block->result, backup_block_result, &result);
        } else {
          result = raise_with_arguments(SYS_MEM_EXCEPTION, actual_parameters);
        } /* if */
        loc_restore(block->local_vars, backup_loc_var);
        free_list(backup_loc_var);
      } /* if */
      /* show_arg_list(evaluated_act_params); */
      par_restore(block->params, backup_form_params, evaluated_act_params);
      free_list(backup_form_params);
      free_list(evaluated_act_params);
    } /* if */
    logFunction(printf("exec_lambda --> ");
                trace1(result);
                printf("\n"););
    return result;
  } /* exec_lambda */



static listType eval_arg_list (register listType act_param_list, uint32Type *temp_bits_ptr)

  {
    listType evaluated_act_params = NULL;
    register objectType evaluated_object;
    register listType *evaluated_insert_place;
    uint32Type temp_bits = 0;
    int param_num = 0;

  /* eval_arg_list */
    evaluated_insert_place = &evaluated_act_params;
    while (act_param_list != NULL && !fail_flag) {
      evaluated_object = exec_object(act_param_list->obj);
      append_to_list(evaluated_insert_place, evaluated_object, act_param_list);
      if (evaluated_object != NULL && TEMP_OBJECT(evaluated_object)) {
        temp_bits |= (uint32Type) 1 << param_num;
      } /* if */
      act_param_list = act_param_list->next;
      param_num++;
    } /* while */
    *temp_bits_ptr = temp_bits;
    return evaluated_act_params;
  } /* eval_arg_list */



static void dump_arg_list (listType evaluated_act_params, uint32Type temp_bits)

  {
    register listType list_end;

  /* dump_arg_list */
    if (evaluated_act_params != NULL) {
      list_end = evaluated_act_params;
      while (list_end->next != NULL) {
        if (list_end->obj != NULL && temp_bits & 1 && TEMP_OBJECT(list_end->obj)) {
          dump_any_temp(list_end->obj);
        } /* if */
        list_end = list_end->next;
        temp_bits >>= 1;
      } /* while */
      if (list_end->obj != NULL && temp_bits & 1 && TEMP_OBJECT(list_end->obj)) {
        dump_any_temp(list_end->obj);
      } /* if */
      free_list2(evaluated_act_params, list_end);
    } /* if */
  } /* dump_arg_list */



static objectType exec_action (const_objectType act_object,
    listType act_param_list, objectType object)

  {
    listType evaluated_act_params;
    uint32Type temp_bits;
    objectType result;

  /* exec_action */
    logFunction(printf("exec_action(%s)\n",
                       getActEntry(act_object->value.actValue)->name););
#if CHECK_STACK
    if (checkStack(FALSE)) {
      return raise_with_arguments(SYS_MEM_EXCEPTION, act_param_list);
    } /* if */
#endif
    evaluated_act_params = eval_arg_list(act_param_list, &temp_bits);
    if (interrupt_flag) {
      if (!fail_flag) {
        curr_exec_object = object;
        curr_argument_list = evaluated_act_params;
        show_signal();
      } /* if */
      if (fail_flag) {
        dump_arg_list(evaluated_act_params, temp_bits);
        result = fail_value;
        logFunction(printf("exec_action fail_flag=%d -->\n", fail_flag););
        return result;
      } /* if */
    } /* if */
#if WITH_ACTION_CHECK
    if (trace.check_actions) {
      if (unlikely(act_object->value.actValue == actTable.table[0].action)) {
        logError(printf("evaluate: illegal action\n"););
        result = raise_with_arguments(SYS_ACT_ILLEGAL_EXCEPTION,
            evaluated_act_params);
      } else if (unlikely(getActEntry(act_object->value.actValue)->action ==
                          actTable.table[0].action)) {
        logError(printf("evaluate: illegal action " FMT_U_MEM "\n",
                        (memSizeType) act_object->value.actValue););
        result = raise_with_arguments(SYS_ACT_ILLEGAL_EXCEPTION,
            evaluated_act_params);
      } /* if */
    } /* if */
#endif
#ifdef WITH_PROTOCOL
    if (trace.actions) {
      /* heapStatistic(); */
      if (trace.heapsize) {
        prot_heapsize();
        prot_cstri(" ");
      } /* if */
      prot_cstri(getActEntry(act_object->value.actValue)->name);
      /* prot_cstri("(");
         prot_list(act_param_list);
         prot_cstri(") "); */
      prot_cstri("(");
      prot_list(evaluated_act_params);
      prot_cstri(") ");
      prot_flush();
      /* curr_action_object = act_object; */
      curr_exec_object = object;
      curr_argument_list = evaluated_act_params;
      result = (*(act_object->value.actValue))(evaluated_act_params);
      if (act_object->type_of != NULL) {
        if (act_object->type_of->result_type != NULL) {
          if (result != NULL) {
            if (result->type_of != act_object->type_of->result_type) {
              prot_cstri("** correct action result type from '");
              if (result->type_of == NULL) {
                prot_cstri("*NULL_TYPE*");
              } else {
                printobject(result->type_of->match_obj);
              } /* if */
              prot_cstri("' to '");
              printobject(act_object->type_of->result_type->match_obj);
              prot_cstri("' act_object type is ");
              printobject(act_object->type_of->match_obj);
            } /* if */
            if (result->type_of == NULL) {
              result->type_of = act_object->type_of->result_type;
            } /* if */
          } else {
            prot_cstri("** result == NULL for action ");
            prot_cstri(getActEntry(act_object->value.actValue)->name);
          } /* if */
        } else {
          prot_cstri("** act_object->type_of->result_type == NULL ");
        } /* if */
      } else {
        prot_cstri("** act_object->type_of == NULL ");
      } /* if */
      prot_cstri(" ==> ");
      printobject(result);
      if (trace.heapsize) {
        prot_cstri(" ");
        prot_heapsize();
      } /* if */
      prot_nl();
      prot_flush();
    } else {
#endif
      /* curr_action_object = act_object; */
      curr_exec_object = object;
      curr_argument_list = evaluated_act_params;
      result = (*(act_object->value.actValue))(evaluated_act_params);
      if (result != NULL && result->type_of == NULL) {
        result->type_of = act_object->type_of->result_type;
      } /* if */
#ifdef WITH_PROTOCOL
    } /* if */
#endif
    dump_arg_list(evaluated_act_params, temp_bits);
    logFunction(printf("exec_action fail_flag=%d -->\n", fail_flag););
    return result;
  } /* exec_action */



static void exec_all_parameters (const_listType act_param_list)

  { /* exec_all_parameters */
    logFunction(printf("exec_all_parameters\n"););
    while (act_param_list != NULL && !fail_flag) {
      exec_object(act_param_list->obj);
      act_param_list = act_param_list->next;
    } /* while */
    logFunction(printf("exec_all_parameters -->\n"););
  } /* exec_all_parameters */



objectType exec_call (objectType object)

  {
    objectType subroutine_object;
    listType actual_parameters;
    objectType result;

  /* exec_call */
    logFunction(printf("exec_call ");
                trace1(object);
                printf(" <-> ");
                trace1(object->value.listValue->obj);
                printf(" (");
                prot_list(object->value.listValue->next);
                printf(")\n"););
    subroutine_object = object->value.listValue->obj;
    actual_parameters = object->value.listValue->next;
/*  if (CATEGORY_OF_OBJ(subroutine_object) == REFPARAMOBJECT) {
      printf("refparamobject ");
      trace1(subroutine_object);
      printf(" value ");
      trace1(subroutine_object->value.objValue);
      printf(" params ");
      prot_list(actual_parameters);
      printf("\n");
      printf("\n");
      subroutine_object = subroutine_object->value.objValue;
    }  if */
    switch (CATEGORY_OF_OBJ(subroutine_object)) {
      case ACTOBJECT:
        result = exec_action(subroutine_object,
            actual_parameters, object);
        break;
      case BLOCKOBJECT:
/*        printf("blockobject ");
        trace1(subroutine_object);
        printf(" params ");
        prot_list(actual_parameters);
        printf("\n"); */
        result = exec_lambda(subroutine_object->value.blockValue,
            actual_parameters, object);
        break;
      case CONSTENUMOBJECT:
/*        printf("constenumobject ");
        trace1(subroutine_object);
        printf(" params ");
        prot_list(actual_parameters);
        printf("\n"); */
        exec_all_parameters(actual_parameters);
        result = subroutine_object->value.objValue;
        break;
      case INTOBJECT:
      case BIGINTOBJECT:
      case CHAROBJECT:
      case STRIOBJECT:
      case BSTRIOBJECT:
      case ARRAYOBJECT:
      case HASHOBJECT:
      case STRUCTOBJECT:
      case STRUCTELEMOBJECT:
      case SETOBJECT:
      case FILEOBJECT:
      case SOCKETOBJECT:
      case POLLOBJECT:
      case LISTOBJECT:
#if WITH_FLOAT
      case FLOATOBJECT:
#endif
      case WINOBJECT:
      case POINTLISTOBJECT:
      case PROCESSOBJECT:
      case VARENUMOBJECT:
      case ENUMLITERALOBJECT:
      case REFOBJECT:
      case REFLISTOBJECT:
      case TYPEOBJECT:
      case INTERFACEOBJECT:
      case PROGOBJECT:
      case DATABASEOBJECT:
      case SQLSTMTOBJECT:
      case DECLAREDOBJECT:
/*        printf("int/char/stri/array/file/type ");
        trace1(subroutine_object);
        printf(" params ");
        prot_list(actual_parameters);
        printf("\n"); */
        exec_all_parameters(actual_parameters);
        result = subroutine_object;
        break;
      case VALUEPARAMOBJECT:
      case REFPARAMOBJECT:
      case LOCALVOBJECT:
/*        printf("refparamobject ");
        trace1(subroutine_object);
        printf(" value ");
        trace1(subroutine_object->value.objValue);
        printf(" params ");
        prot_list(actual_parameters);
        printf("\n");
        printf("\n"); */
        result = evaluate(subroutine_object->value.objValue);
        /* result = exec_object(subroutine_object->value.objValue); */
        break;
      case MATCHOBJECT:
/*        printf("\nsubroutine_object: ");
        trace1(subroutine_object);
        printf(" params ");
        prot_list(actual_parameters);
        printf("\n");
        printf("\n"); */
        result = evaluate(subroutine_object);
        break;
      case FORWARDOBJECT:
        logError(printf("exec_call: forward object\n"););
        result = raise_with_arguments(SYS_ACT_ILLEGAL_EXCEPTION, actual_parameters);
        break;
      default:
        printf("category_of_obj: ");
        trace1(object);
        printf("\nsubroutine_object: ");
        trace1(subroutine_object);
        printf("\n");
        printf("\n");
        /* printf("%d\n", 1/0); */
/*        result = exec_dynamic(object->value.listValue); */
        result = NULL;
        break;
    } /* switch */
    logFunction(printf("exec_call --> ");
                trace1(result);
                printf("\n"););
    return result;
  } /* exec_call */



objectType do_exec_call (objectType object, errInfoType *err_info)

  {
    objectType result;

  /* do_exec_call */
    logFunction(printf("do_exec_call(");
                trace1(object);
                printf(", %d)\n", *err_info););
    result = exec_call(object);
    if (unlikely(fail_flag || result == NULL)) {
      *err_info = getErrInfoFromFailValue(fail_value);
      result = fail_value;
      leaveExceptionHandling();
    } /* if */
    logFunction(printf("do_exec_call(");
                trace1(object);
                printf(", %d) --> ", *err_info);
                trace1(result);
                printf("\n"););
    return result;
  } /* do_exec_call */



/**
 *  Evaluate a call-by-name parameter.
 *  An actual call-by-name parameter is not evaluated before a function
 *  is called. Call-by-name parameters are used for the conditions of loops,
 *  the statements in loop bodies, the right parameter of the ternary
 *  operator, etc.
 */
objectType evaluate (objectType object)

  {
    objectType result;

  /* evaluate */
    logFunction(printf("evaluate\n"););
#ifdef OUT_OF_ORDER
    if (fail_flag) {
      printf("evaluate fail_flag for ");
      trace1(object);
      printf("\n");
    } /* if */
#endif
    switch (CATEGORY_OF_OBJ(object)) {
      case MATCHOBJECT:
        result = exec_call(object);
        break;
      case VALUEPARAMOBJECT:
      case REFPARAMOBJECT:
      case RESULTOBJECT:
      case CONSTENUMOBJECT:
      case VARENUMOBJECT:
        result = object->value.objValue;
        break;
      case INTOBJECT:
      case BIGINTOBJECT:
      case CHAROBJECT:
      case STRIOBJECT:
      case BSTRIOBJECT:
      case ARRAYOBJECT:
      case HASHOBJECT:
      case STRUCTOBJECT:
      case SETOBJECT:
      case FILEOBJECT:
#if WITH_FLOAT
      case FLOATOBJECT:
#endif
      case WINOBJECT:
      case POINTLISTOBJECT:
      case REFOBJECT:
      case REFLISTOBJECT:
      case ENUMLITERALOBJECT:
        result = object;
        break;
      case BLOCKOBJECT:
        result = exec_lambda(object->value.blockValue, NULL, object);
        break;
      case ACTOBJECT:
        result = exec_action(object, NULL, NULL);
        break;
      default:
        logError(printf("evaluate: evaluate unknown\n");
                 trace1(object);
                 printf("\n"););
        result = raise_with_arguments(SYS_ACT_ILLEGAL_EXCEPTION, NULL);
        break;
    } /* switch */
    logFunction(printf("evaluate --> ");
                trace1(result);
                printf("\n"););
    return result;
  } /* evaluate */



objectType exec_dynamic (listType expr_list)

  {
    objectType dynamic_call_obj;
    objectType match_expr;
    listType actual_element;
    listType *list_insert_place;
    objectType element_value;
    listType temp_values = NULL;
    listType *temp_insert_place;
    listType temp_list_end;
    objectType match_result;
    errInfoType err_info = OKAY_NO_ERROR;
    objectType result = NULL;

  /* exec_dynamic */
    logFunction(printf("exec_dynamic(");
                prot_list(expr_list);
                printf(")\n"););
#ifdef WITH_PROTOCOL
    if (trace.dynamic) {
      if (trace.heapsize) {
        prot_heapsize();
        prot_cstri(" ");
      } /* if */
      prot_cstri("DYNAMIC ");
      prot_list(expr_list);
      prot_nl();
    } /* if */
#endif
    dynamic_call_obj = curr_exec_object;
    if (unlikely(!ALLOC_OBJECT(match_expr))) {
      err_info = MEMORY_ERROR;
    } else {
      match_expr->type_of = take_type(SYS_EXPR_TYPE);
      match_expr->descriptor.property = prog->property.literal;
      match_expr->value.listValue = NULL;
      list_insert_place = &match_expr->value.listValue;
      INIT_CATEGORY_OF_OBJ(match_expr, EXPROBJECT);
      temp_insert_place = &temp_values;
      actual_element = expr_list;
      while (actual_element != NULL && err_info == OKAY_NO_ERROR) {
        logMessage(printf("actual_element->obj ");
                   trace1(actual_element->obj);
                   printf("\n"););
        switch (CATEGORY_OF_OBJ(actual_element->obj)) {
          case VALUEPARAMOBJECT:
          case REFPARAMOBJECT:
          case RESULTOBJECT:
          case LOCALVOBJECT:
          case CONSTENUMOBJECT:
          case VARENUMOBJECT:
          case INTERFACEOBJECT:
          case STRUCTELEMOBJECT:
            element_value = actual_element->obj->value.objValue;
            break;
          default:
            element_value = actual_element->obj;
            break;
        } /* switch */
        logMessage(printf("element_value ");
                   trace1(element_value);
                   printf("\n"););
        if (unlikely(element_value == NULL)) {
          logError(printf("exec_dynamic(");
                   prot_list(expr_list);
                   printf("): element_value == NULL.\n");
                   trace1(actual_element->obj);
                   printf("\n"););
          err_info = ACTION_ERROR;
        } else {
          if (TEMP_OBJECT(element_value)) {
            /* The function exec_dynamic() is called indirectly from  */
            /* exec_action(). Further below the call of exec_call()   */
            /* could also lead to a call of exec_action(). At its end */
            /* exec_action() frees temporary objects. This could lead */
            /* to double frees of temporary values. To avoid that the */
            /* TEMP flag must be cleared here.                        */
            CLEAR_TEMP_FLAG(element_value);
            SET_TEMP_DYNAMIC_FLAG(element_value);
            temp_insert_place = append_element_to_list(temp_insert_place,
                element_value, &err_info);
          } /* if */
          if (likely(err_info == OKAY_NO_ERROR)) {
            list_insert_place = append_element_to_list(list_insert_place,
                element_value, &err_info);
            actual_element = actual_element->next;
          } /* if */
        } /* if */
      } /* while */
      if (likely(err_info == OKAY_NO_ERROR)) {
#ifdef WITH_PROTOCOL
        if (trace.dynamic) {
          if (trace.heapsize) {
            prot_heapsize();
            prot_cstri(" ");
          } /* if */
          prot_cstri("DYNAMIC2 ");
          prot_list(match_expr->value.listValue);
          prot_nl();
        } /* if */
#endif
        logMessage(printf("match_expr ");
                   trace1(match_expr);
                   printf("\n"););
        if (match_prog_expression(prog->declaration_root, match_expr) != NULL) {
          if ((match_result = match_object(match_expr)) != NULL) {
#ifdef WITH_PROTOCOL
            if (trace.dynamic) {
              prot_cstri("matched ==> ");
              trace1(match_result);
            } /* if */
#endif
            if (match_result->value.listValue->obj != dynamic_call_obj->value.listValue->obj) {
              result = exec_call(match_result);
              if (fail_flag) {
                errInfoType ignored_err_info;

                if (fail_stack->obj == match_result) {
                  pop_list(&fail_stack);
                } /* if */
                /* Since an exception has already been     */
                /* raised ignored_err_info is not checked. */
                incl_list(&fail_stack, dynamic_call_obj, &ignored_err_info);
              } /* if */

#ifdef WITH_PROTOCOL
              if (trace.dynamic) {
                if (trace.heapsize) {
                  prot_cstri(" ");
                  prot_heapsize();
                } /* if */
                prot_nl();
              } /* if */
#endif
            } else {
              logError(printf("exec_dynamic: Endless recursion.\n");
                       trace1(match_expr);
                       printf("\n"););
              err_info = ACTION_ERROR;
            } /* if */
          } else {
            logError(printf("exec_dynamic: match_object() failed.\n");
                     trace1(match_expr);
                     printf("\n"););
            err_info = ACTION_ERROR;
          } /* if */
        } else {
          logError(printf("exec_dynamic: match_prog_expression() failed.\n");
                   trace1(match_expr);
                   printf("\n"););
          err_info = ACTION_ERROR;
        } /* if */
      } /* if */
      free_list(match_expr->value.listValue);
      FREE_OBJECT(match_expr);
      if (temp_values != NULL) {
        /* Restore the TEMP flag such that           */
        /* exec_action() can free temporary objects. */
        actual_element = temp_values;
        do {
          CLEAR_TEMP_DYNAMIC_FLAG(actual_element->obj);
          SET_TEMP_FLAG(actual_element->obj);
          temp_list_end = temp_values;
          actual_element = actual_element->next;
        } while (actual_element != NULL);
        free_list2(temp_values, temp_list_end);
      } /* if */
    } /* if */
    if (unlikely(err_info != OKAY_NO_ERROR)) {
      return raise_with_obj_and_args(prog->sys_var[err_info],
                                     dynamic_call_obj, expr_list);
    } /* if */
    logFunction(printf("exec_dynamic(");
                prot_list(expr_list);
                printf(") --> ");
                trace1(result);
                printf("\n"););
    return result;
  } /* exec_dynamic */



objectType exec_expr (const progType currentProg, objectType object,
    errInfoType *err_info)

  {
    progType progBackup;
    boolType backup_interpreter_exception;
    objectType result;

  /* exec_expr */
    logFunction(printf("exec_expr\n"););
    if (currentProg != NULL) {
      set_fail_flag(FALSE);
      fail_value = NULL;
      fail_expression = NULL;
      fail_expr_stri = NULL;
      progBackup = prog;
      prog = currentProg;
      set_protfile_name(NULL);
      prog->option_flags = 0;
      set_trace(prog->option_flags);
      backup_interpreter_exception = interpreter_exception;
      interpreter_exception = TRUE;
      result = exec_object(object);
      if (fail_flag) {
        /*
        printf("\n*** Uncaught EXCEPTION ");
        printobject(fail_value);
        printf(" raised with\n");
        prot_list(fail_expression);
        printf("\n");
        */
        *err_info = getErrInfoFromFailValue(fail_value);
        leaveExceptionHandling();
      } else {
        if (TEMP_OBJECT(result)) {
          CLEAR_TEMP_FLAG(result);
          incl_list(&currentProg->exec_expr_temp_results, result, err_info);
        } /* if */
      } /* if */
      interpreter_exception = backup_interpreter_exception;
      prog = progBackup;
    } else {
      result = NULL;
    } /* if */
    logFunction(printf("exec_expr --> ");
                trace1(result);
                printf("\n"););
    return result;
  } /* exec_expr */

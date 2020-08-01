/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2004  Thomas Mertes                        */
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
/*  Changes: 1999, 2000, 2004  Thomas Mertes                        */
/*  Content: Main interpreter procedures.                           */
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


#if CHECK_STACK
extern char *stack_base;
extern memSizeType max_stack_size;
#endif

extern boolType interpreter_exception;



void suspendInterpreter (int signalNum)

  { /* suspendInterpreter */
    logFunction(printf("suspendInterpreter(%d)\n", signalNum););
    interrupt_flag = TRUE;
    signal_number = signalNum;
  } /* suspendInterpreter */



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
      case ARRAYOBJECT:
      case HASHOBJECT:
      case STRUCTOBJECT:
      case FILEOBJECT:
      case LISTOBJECT:
#ifdef WITH_FLOAT
      case FLOATOBJECT:
#endif
      case WINOBJECT:
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
/*            printf("ref param formal ");
            trace1(form_param->local.object);
            printf(" %lu\nparam value ", (unsigned long) form_param->local.object);
            trace1(param_value);
            printf(" %lu\n", (unsigned long) param_value); */
            form_param->local.object->value.objValue = param_value;
            if (TEMP_OBJECT(param_value)) {
              CLEAR_TEMP_FLAG(param_value);
              SET_TEMP2_FLAG(param_value);
              /* printf("ref to temp ");
              trace1(form_param->local.object);
              printf(" %lu\n", (unsigned long) form_param->local.object); */
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
    boolType save_interrupt_flag;
    boolType save_fail_flag;
    errInfoType err_info = OKAY_NO_ERROR;

  /* par_restore */
    logFunction(printf("par_restore\n"););
    save_interrupt_flag = interrupt_flag;
    save_fail_flag = fail_flag;
    set_fail_flag(FALSE);
    while (form_param != NULL && err_info == OKAY_NO_ERROR) {
#ifdef OUT_OF_ORDER
      if (trace.actions) {
        prot_cstri("par_restore ");
        if (evaluated_act_params->obj != NULL) {
          printcategory(CATEGORY_OF_OBJ(evaluated_act_params->obj));
          prot_cstri(" ");
          prot_int((intType) evaluated_act_params->obj);
          prot_cstri(" ");
          trace1(evaluated_act_params->obj);
        } else {
          prot_cstri("NULL");
        } /* if */
        prot_nl();
      } /* if */
#endif
      switch (CATEGORY_OF_OBJ(form_param->local.object)) {
        case VALUEPARAMOBJECT:
          destroy_local_object(&form_param->local, &err_info);
          FREE_OBJECT(form_param->local.object->value.objValue);
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
    interrupt_flag = save_interrupt_flag;
    fail_flag = save_fail_flag;
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
    boolType save_interrupt_flag;
    boolType save_fail_flag;
    errInfoType err_info = OKAY_NO_ERROR;

  /* loc_restore */
    logFunction(printf("loc_restore\n"););
    save_interrupt_flag = interrupt_flag;
    save_fail_flag = fail_flag;
    set_fail_flag(FALSE);
    while (loc_var != NULL) {
      destroy_local_object(&loc_var->local, &err_info);
      if (IS_UNUSED(loc_var->local.object->value.objValue)) {
        FREE_OBJECT(loc_var->local.object->value.objValue);
      } else if (CATEGORY_OF_OBJ(loc_var->local.object->value.objValue) != STRUCTOBJECT) {
        printf("loc not dumped: ");
        trace1(loc_var->local.object->value.objValue);
        printf("\n");
      } /* if */
      loc_var->local.object->value.objValue = backup_loc_var->obj;
      loc_var = loc_var->next;
      backup_loc_var = backup_loc_var->next;
    } /* while */
    interrupt_flag = save_interrupt_flag;
    fail_flag = save_fail_flag;
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
      /* Backup_block_result is initialized in res_init and used   */
      /* res_restore. Backup_block_result is initialized and used  */
      /* conditionally. In both cases (initialisation and use) the */
      /* same condition is used. Possible compiler warnings that   */
      /* "it may be used uninitialized" can be ignored.            */
      *backup_block_result = block_result->object->value.objValue;
      create_local_object(block_result, block_result->init_value, &err_info);
    } /* if */
    logFunction(printf("res_init(" FMT_U_MEM ") -->\n",
                       block_result->object ?
                           ((memSizeType) block_result->object->value.objValue) : 0););
    return err_info == OKAY_NO_ERROR;
  } /* res_init */



static inline void res_restore (const_locObjType block_result,
    objectType backup_block_result, objectType *result)

  {
    errInfoType err_info = OKAY_NO_ERROR;

  /* res_restore */
    logFunction(printf("res_restore\n"););
    if (block_result->object != NULL) {
      if (!fail_flag) {
        *result = block_result->object->value.objValue;
        /* CLEAR_VAR_FLAG(*result); */
        SET_TEMP_FLAG(*result);
      } /* if */
      /* Backup_block_result is initialized in res_init and used   */
      /* res_restore. Backup_block_result is initialized and used  */
      /* conditionally. In both cases (initialisation and use) the */
      /* same condition is used. Possible compiler warnings that   */
      /* "it may be used uninitialized" can be ignored.            */
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

#ifdef OUT_OF_ORDER
            if (!HAS_POSINFO(object)) {
              if (HAS_ENTITY(object)) {
                printf("HAS_ENTITY\n");
                if (GET_ENTITY(object)->ident != NULL) {
                  prot_cstri8(id_string(GET_ENTITY(object)->ident));
                } else if (GET_ENTITY(object)->name_list != NULL) {
                  prot_list(GET_ENTITY(object)->name_list);
                } /* if */
                printf("\n");
              } /* if */
              printf("!HAS_POSINFO\n");
              trace1(object);
              prot_nl();
            } /* if */
#endif

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
    logFunction(printf("exec_lambda -->\n"););
    return result;
  } /* exec_lambda */



static listType eval_arg_list (register listType act_param_list, uint32Type *temp_bits_ptr)

  {
    listType evaluated_act_params;
    register objectType evaluated_object;
    register listType *evaluated_insert_place;
    uint32Type temp_bits;
    int param_num = 0;

  /* eval_arg_list */
    evaluated_act_params = NULL;
    evaluated_insert_place = &evaluated_act_params;
    temp_bits = 0;
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
#if STACK_GROWS_UPWARD
    if ((char *) &evaluated_act_params - stack_base > max_stack_size) {
      max_stack_size = (char *) &evaluated_act_params - stack_base;
    } /* if */
    if (stack_base + CHECKED_STACK_SIZE_LIMIT < (char *) &evaluated_act_params) {
      printf("*** Stack size above limit\n");
      printf("size:  %8lu\n", (char *) &evaluated_act_params - stack_base);
      printf("limit: %8lu\n", CHECKED_STACK_SIZE_LIMIT);
      return raise_with_arguments(SYS_MEM_EXCEPTION, act_param_list);
    } /* if */
#else
    if (stack_base - (char *) &evaluated_act_params > max_stack_size) {
      max_stack_size = stack_base - (char *) &evaluated_act_params;
    } /* if */
    if (stack_base - CHECKED_STACK_SIZE_LIMIT > (char *) &evaluated_act_params) {
      printf("*** Stack size above limit\n");
      printf("size:  %8lu\n", stack_base - (char *) &evaluated_act_params);
      printf("limit: %8lu\n", CHECKED_STACK_SIZE_LIMIT);
      return raise_with_arguments(SYS_MEM_EXCEPTION, act_param_list);
    } /* if */
#endif
#endif
    evaluated_act_params = eval_arg_list(act_param_list, &temp_bits);
    if (interrupt_flag) {
      if (!fail_flag) {
        curr_exec_object = object;
        curr_argument_list = evaluated_act_params;
        show_signal();
      } /* if */
      if (fail_flag) {
        free_list(evaluated_act_params);
        result = fail_value;
        logFunction(printf("exec_action fail_flag=%d -->\n", fail_flag););
        return result;
      } /* if */
    } /* if */
#if WITH_ACTION_CHECK
    if (trace.check_actions) {
      if (unlikely(act_object->value.actValue == actTable.table[0].action)) {
        result = raise_with_arguments(SYS_ACT_ILLEGAL_EXCEPTION,
            evaluated_act_params);
      } /* if */
    } /* if */
#endif
#ifdef WITH_PROTOCOL
    if (trace.actions) {
      /* heap_statistic(); */
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
              prot_cstri("** correct action result type from \'");
              if (result->type_of == NULL) {
                prot_cstri("*NULL_TYPE*");
              } else {
                printobject(result->type_of->match_obj);
              } /* if */
              prot_cstri("\' to \'");
              printobject(act_object->type_of->result_type->match_obj);
              prot_cstri("\' act_object type is ");
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
      case SETOBJECT:
      case FILEOBJECT:
      case SOCKETOBJECT:
      case POLLOBJECT:
      case LISTOBJECT:
#ifdef WITH_FLOAT
      case FLOATOBJECT:
#endif
      case WINOBJECT:
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
    logFunction(printf("exec_call ");
                trace1(result);
                printf("\n"););
    return result;
  } /* exec_call */



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
      case STRUCTOBJECT:
#ifdef WITH_FLOAT
      case FLOATOBJECT:
#endif
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
        prot_cstri("evaluate unknown ");
        trace1(object);
        prot_nl();
        result = raise_with_arguments(SYS_ACT_ILLEGAL_EXCEPTION, NULL);
        break;
    } /* switch */
    logFunction(printf("evaluate --> ");
                trace1(result);
                printf("\n"););
    return result;
  } /* evaluate */



objectType eval_expression (objectType object)

  {
    objectType result;
    objectType matched_expression;
    objectType matched_object;

  /* eval_expression */
    logFunction(printf("eval_expression\n"););
    if ((matched_expression = match_expression(object)) != NULL) {
      if ((matched_object = match_object(matched_expression)) != NULL) {
/*
        printf("eval expression match succeeded ");
        trace1(matched_object);
        printf("\n"); */
        if (CATEGORY_OF_OBJ(matched_object) == CALLOBJECT) {
          result = exec_call(matched_object);
        } else {
          printf("eval_expression: match result not callobject ");
          trace1(matched_object);
          printf("\n");
          result = NULL;
        } /* if */
      } else {
        printf("eval_expression: match object failed ");
        trace1(matched_expression);
        printf("\n");
        result = NULL;
      } /* if */
    } else {
      printf("eval_expression: match expression failed ");
      trace1(object);
      printf("\n");
      result = NULL;
    } /* if */
    logFunction(printf("eval_expression -->\n"););
    return result;
  } /* eval_expression */



objectType exec_dynamic (listType expr_list)

  {
    objectType dynamic_call_obj;
    objectType match_expr;
    listType actual_element;
    listType *list_insert_place;
    objectType element_value;
    objectType match_result;
    objectType result = NULL;
    errInfoType err_info = OKAY_NO_ERROR;

  /* exec_dynamic */
    logFunction(printf("exec_dynamic\n"););
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
    if (ALLOC_OBJECT(match_expr)) {
      match_expr->type_of = take_type(SYS_EXPR_TYPE);
      match_expr->descriptor.property = prog->property.literal;
      match_expr->value.listValue = NULL;
      list_insert_place = &match_expr->value.listValue;
      INIT_CATEGORY_OF_OBJ(match_expr, EXPROBJECT);
      actual_element = expr_list;
      while (actual_element != NULL) {
/* printf("actual_element->obj ");
trace1(actual_element->obj);
printf("\n"); */
        switch (CATEGORY_OF_OBJ(actual_element->obj)) {
          case VALUEPARAMOBJECT:
          case REFPARAMOBJECT:
          case RESULTOBJECT:
          case LOCALVOBJECT:
          case CONSTENUMOBJECT:
          case VARENUMOBJECT:
          case INTERFACEOBJECT:
            element_value = actual_element->obj->value.objValue;
            break;
          default:
            element_value = actual_element->obj;
            break;
        } /* switch */
/* printf("element_value ");
trace1(element_value);
printf("\n"); */
#if !WITH_OBJECT_FREELIST
        /* When a freelist is used exec_action examines the     */
        /* object on the freelist and will not free it, because */
        /* the TEMP flag is not set for free list objects.      */
        if (TEMP_OBJECT(element_value)) {
          /* Exec_dynamic is called from the action PRC_DYNAMIC. */
          /* PRC_DYNAMIC is called from exec_action. Exec_action */
          /* frees temporary objects. To avoid double frees the  */
          /* TEMP flag must be cleared here.                     */
          CLEAR_TEMP_FLAG(element_value);
        } /* if */
#endif
        /* err_info is not checked after append! */
        list_insert_place = append_element_to_list(list_insert_place,
            element_value, &err_info);
        actual_element = actual_element->next;
      } /* while */
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
      /* printf("match_expr ");
      trace1(match_expr);
      printf("\n"); */
      if (match_prog_expression(prog->declaration_root, match_expr) != NULL &&
          (match_result = match_object(match_expr)) != NULL) {
#ifdef WITH_PROTOCOL
        if (trace.dynamic) {
          prot_cstri("matched ==> ");
          trace1(match_result);
        } /* if */
#endif
        result = exec_call(match_result);
        if (fail_flag) {
          errInfoType ignored_err_info;

          if (fail_stack->obj == match_result) {
            pop_list(&fail_stack);
          } /* if */

          /* ignored_err_info is not checked since an exception was already raised */
          incl_list(&fail_stack, dynamic_call_obj, &ignored_err_info);
        } /* if */

        if (match_result != match_expr) {
          FREE_OBJECT(match_result->value.listValue->obj);
          free_list(match_result->value.listValue);
          FREE_OBJECT(match_result);
        } else {
          free_list(match_expr->value.listValue);
          FREE_OBJECT(match_expr);
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
        return raise_with_arguments(SYS_ACT_ILLEGAL_EXCEPTION, expr_list);
      } /* if */
    } else {
      return raise_with_arguments(SYS_MEM_EXCEPTION, expr_list);
    } /* if */
    logFunction(printf("exec_dynamic -->\n"););
    return result;
  } /* exec_dynamic */



objectType exec_expr (const progType currentProg, objectType object,
    errInfoType *err_info)

  {
    progType progBackup;
    objectType result;

  /* exec_expr */
    logFunction(printf("exec_expr\n"););
    if (currentProg != NULL) {
      set_fail_flag(FALSE);
      fail_value = NULL;
      fail_expression = NULL;
      progBackup = prog;
      prog = currentProg;
      set_protfile_name(NULL);
      prog->option_flags = 0;
      set_trace(prog->option_flags);
      interpreter_exception = TRUE;
      result = exec_object(object);
      interpreter_exception = FALSE;
      if (fail_flag) {
        /*
        printf("\n*** Uncaught EXCEPTION ");
        printobject(fail_value);
        printf(" raised with\n");
        prot_list(fail_expression);
        printf("\n");
        */
        if (fail_value == SYS_MEM_EXCEPTION) {
          *err_info = MEMORY_ERROR;
        } else if (fail_value == SYS_NUM_EXCEPTION) {
          *err_info = NUMERIC_ERROR;
        } else if (fail_value == SYS_OVF_EXCEPTION) {
          *err_info = OVERFLOW_ERROR;
        } else if (fail_value == SYS_RNG_EXCEPTION) {
          *err_info = RANGE_ERROR;
        } else if (fail_value == SYS_FIL_EXCEPTION) {
          *err_info = FILE_ERROR;
        } else if (fail_value == SYS_DB_EXCEPTION) {
          *err_info = DATABASE_ERROR;
        } else if (fail_value == SYS_ACT_ILLEGAL_EXCEPTION) {
          *err_info = ACTION_ERROR;
        } /* if */
        set_fail_flag(FALSE);
        fail_value = NULL;
        fail_expression = NULL;
      } /* if */
      prog = progBackup;
    } else {
      result = NULL;
    } /* if */
    logFunction(printf("exec_expr --> ");
                trace1(result);
                printf("\n"););
    return result;
  } /* exec_expr */

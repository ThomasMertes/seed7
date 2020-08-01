/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: Interpreter                                             */
/*  File: seed7/src/exec.c                                          */
/*  Changes: 1999, 2000, 2004  Thomas Mertes                        */
/*  Content: Main interpreter procedures.                           */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data.h"
#include "sigutl.h"
#include "heaputl.h"
#include "flistutl.h"
#include "identutl.h"
#include "entutl.h"
#include "syvarutl.h"
#include "listutl.h"
#include "traceutl.h"
#include "actutl.h"
#include "executl.h"
#include "runerr.h"
#include "memory.h"
#include "match.h"
#include "option.h"

#undef EXTERN
#define EXTERN
#include "exec.h"

#undef TRACE_EXEC



#ifdef ANSI_C

objecttype exec_call (objecttype);
#else

objecttype exec_call ();
#endif



#ifdef ANSI_C

objecttype exec_object (register objecttype object)
#else

objecttype exec_object (object)
register objecttype object;
#endif

  {
    register objecttype result;

  /* exec_object */
#ifdef TRACE_EXEC
    printf("BEGIN exec_object ");
    trace1(object);
    printf("\n");
#endif
    switch (CLASS_OF_OBJ(object)) {
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
        result = object->value.objvalue;
        break;
      case MATCHOBJECT:
        /* This is NONSENSE:
        printf("exec_object1: MATCHOBJECT ");
        trace1(object);
        printf("\n");
        result = object->value.listvalue->obj;
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
#ifdef TRACE_EXEC
    printf("END exec_object ");
    trace1(result);
    printf("\n");
#endif
    return(result);
  } /* exec_object */



#ifdef ANSI_C

static INLINE void par_init (loclisttype form_param_list,
    listtype *backup_form_params, listtype act_param_list,
    listtype *evaluated_act_params)
#else

static INLINE void par_init (form_param_list, backup_form_params,
    act_param_list, evaluated_act_params)
loclisttype form_param_list;
listtype *backup_form_params;
listtype act_param_list;
listtype *evaluated_act_params;
#endif

  {
    loclisttype form_param;
    listtype *backup_insert_place;
    listtype *evaluated_insert_place;
    listtype param_list_elem;
    objecttype param_value;
    errinfotype err_info = OKAY_NO_ERROR;

  /* par_init */
#ifdef TRACE_EXEC
    printf("BEGIN par_init\n");
#endif
    form_param = form_param_list;
    *backup_form_params = NULL;
    backup_insert_place = backup_form_params;    
    *evaluated_act_params = NULL;
    evaluated_insert_place = evaluated_act_params;
    while (form_param != NULL && !fail_flag) {
      append_to_list(backup_insert_place,
          form_param->local.object->value.objvalue, act_param_list);
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
        switch (CLASS_OF_OBJ(form_param->local.object)) {
          case VALUEPARAMOBJECT:
/*            printf("value param formal ");
            trace1(form_param->local.object);
            printf(" %lu\nparam value ", (unsigned long) form_param->local.object);
            trace1(param_value);
            printf(" %lu\n", (unsigned long) param_value); */
            if (TEMP_OBJECT(param_value)) {
              CLEAR_TEMP_FLAG(param_value);
              COPY_VAR_FLAG(param_value, form_param->local.object);
              form_param->local.object->value.objvalue = param_value;
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
            form_param->local.object->value.objvalue = param_value;
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
#ifdef TRACE_EXEC
    printf("END par_init\n");
#endif
  } /* par_init */



#ifdef ANSI_C

static INLINE void par_restore (loclisttype form_param,
    listtype backup_form_params, listtype evaluated_act_params)
#else

static INLINE void par_restore (form_param, backup_form_params,
    evaluated_act_params)
loclisttype form_param;
listtype backup_form_params;
listtype evaluated_act_params;
#endif

  {
    booltype save_fail_flag;
    errinfotype err_info = OKAY_NO_ERROR;

  /* par_restore */
#ifdef TRACE_EXEC
    printf("BEGIN par_restore\n");
#endif
    save_fail_flag = fail_flag;
    fail_flag = FALSE;
    while (form_param != NULL && err_info == OKAY_NO_ERROR) {
#ifdef OUT_OF_ORDER
      if (trace.actions) {
        prot_cstri("par_restore ");
        if (evaluated_act_params->obj != NULL) {
          printclass(CLASS_OF_OBJ(evaluated_act_params->obj));
          prot_cstri(" ");
          prot_int((inttype) evaluated_act_params->obj);
          prot_cstri(" ");
          trace1(evaluated_act_params->obj);
        } else {
          prot_cstri("NULL");
        } /* if */
        prot_nl();
      } /* if */
#endif
      switch (CLASS_OF_OBJ(form_param->local.object)) {
        case VALUEPARAMOBJECT:
          destroy_local_object(&form_param->local, &err_info);
          FREE_OBJECT(form_param->local.object->value.objvalue);
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
      form_param->local.object->value.objvalue = backup_form_params->obj;
      form_param = form_param->next;
      backup_form_params = backup_form_params->next;
      evaluated_act_params = evaluated_act_params->next;
    } /* while */
    fail_flag = save_fail_flag;
#ifdef TRACE_EXEC
    printf("END par_restore\n");
#endif
  } /* par_restore */



#ifdef ANSI_C

static void loc_init (loclisttype loc_var, listtype *backup_loc_var,
    listtype act_param_list)
#else

static void loc_init (loc_var, backup_loc_var, act_param_list)
loclisttype loc_var;
listtype *backup_loc_var;
listtype act_param_list;
#endif

  {
    listtype *list_insert_place;
    errinfotype err_info = OKAY_NO_ERROR;

  /* loc_init */
#ifdef TRACE_EXEC
    printf("BEGIN loc_init\n");
#endif
    *backup_loc_var = NULL;
    list_insert_place = backup_loc_var;    
    while (loc_var != NULL && !fail_flag) {
      append_to_list(list_insert_place,
          loc_var->local.object->value.objvalue, act_param_list);
      create_local_object(&loc_var->local, loc_var->local.init_value, &err_info);
      loc_var = loc_var->next;
    } /* while */
#ifdef TRACE_EXEC
    printf("END loc_init\n");
#endif
  } /* loc_init */



#ifdef ANSI_C

static void loc_restore (loclisttype loc_var, listtype backup_loc_var)
#else

static void loc_restore (loc_var, backup_loc_var)
loclisttype loc_var;
listtype backup_loc_var;
#endif

  {
    booltype save_fail_flag;
    errinfotype err_info = OKAY_NO_ERROR;

  /* loc_restore */
#ifdef TRACE_EXEC
    printf("BEGIN loc_restore\n");
#endif
    save_fail_flag = fail_flag;
    fail_flag = FALSE;
    while (loc_var != NULL) {
      destroy_local_object(&loc_var->local, &err_info);
      FREE_OBJECT(loc_var->local.object->value.objvalue);
      loc_var->local.object->value.objvalue = backup_loc_var->obj;
      loc_var = loc_var->next;
      backup_loc_var = backup_loc_var->next;
    } /* while */
    fail_flag = save_fail_flag;
#ifdef TRACE_EXEC
    printf("END loc_restore\n");
#endif
  } /* loc_restore */



#ifdef ANSI_C

static INLINE booltype res_init (locobjtype block_result,
    objecttype *backup_block_result)
#else

static INLINE booltype res_init (block_result, backup_block_result)
locobjtype block_result;
objecttype *backup_block_result;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;

  /* res_init */
#ifdef TRACE_EXEC
    printf("BEGIN res_init\n");
#endif
    /* printf("block_result ");
    trace1(block_result->object);
    printf("\n");
    printf("result_init ");
    trace1(block_result->init_value);
    printf("\n"); */
    if (block_result->object != NULL) {
      *backup_block_result = block_result->object->value.objvalue;
      create_local_object(block_result, block_result->init_value, &err_info);
    } /* if */
#ifdef TRACE_EXEC
    printf("END res_init(%ld)\n",
        block_result->object ? ((inttype) block_result->object->value.objvalue) : 0);
#endif
    return(err_info == OKAY_NO_ERROR);
  } /* res_init */



#ifdef ANSI_C

static INLINE void res_restore (locobjtype block_result,
    objecttype backup_block_result, objecttype *result)
#else

static INLINE void res_restore (block_result, backup_block_result, result)
locobjtype block_result;
objecttype backup_block_result;
objecttype *result;
#endif

  {
    errinfotype err_info = OKAY_NO_ERROR;

  /* res_restore */
#ifdef TRACE_EXEC
    printf("BEGIN res_restore\n");
#endif
    if (block_result->object != NULL) {
      if (!fail_flag) {
        *result = block_result->object->value.objvalue;
        /* CLEAR_VAR_FLAG(*result); */
        SET_TEMP_FLAG(*result);
      } /* if */
      block_result->object->value.objvalue = backup_block_result;
    } else if (*result != NULL && !TEMP_OBJECT(*result) &&
        CLASS_OF_OBJ(*result) != ENUMLITERALOBJECT) {
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
#ifdef TRACE_EXEC
    printf("END res_restore(%ld)\n",
        *result ? ((inttype) *result) : 0);
#endif
  } /* res_restore */



#ifdef OUT_OF_ORDER
#ifdef ANSI_C

static void show_arg_list (listtype act_param_list)
#else

static void show_arg_list (act_param_list)
listtype act_param_list;
#endif

  { /* show_arg_list */
    while (act_param_list != NULL) {
      if (act_param_list->obj != NULL) {
#ifdef WITH_PROTOCOL
        if (trace.actions) {
          prot_cstri("show_arg_list ");
          printclass(CLASS_OF_OBJ(act_param_list->obj));
          prot_cstri(" ");
          prot_int((inttype) act_param_list->obj);
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



#ifdef ANSI_C

static objecttype exec_lambda (blocktype block,
    listtype actual_parameters, objecttype object)
#else

static objecttype exec_lambda (block, actual_parameters, object)
blocktype block;
listtype actual_parameters;
objecttype object;
#endif

  {
    objecttype result;
    listtype evaluated_act_params;
    listtype backup_form_params;
    objecttype backup_block_result;
    listtype backup_loc_var;

  /* exec_lambda */
#ifdef TRACE_EXEC
    printf("BEGIN exec_lambda\n");
#endif
    par_init(block->params, &backup_form_params, actual_parameters,
        &evaluated_act_params);
    if (fail_flag) {
      emptylist(backup_form_params);
      emptylist(evaluated_act_params);
      result = fail_value;
    } else {
      loc_init(block->locals, &backup_loc_var, actual_parameters);
      if (fail_flag) {
        emptylist(backup_loc_var);
        result = fail_value;
      } else {
        if (res_init(&block->result, &backup_block_result)) {
          result = exec_call(block->body);
          if (fail_flag) {
            errinfotype err_info;

            /* err_info is not checked since an exception was already raised */
            incl_list(&fail_stack, object, &err_info);
          } /* if */
          res_restore(&block->result, backup_block_result, &result);
        } else {
          result = raise_with_arguments(SYS_MEM_EXCEPTION, actual_parameters);
        } /* if */
        loc_restore(block->locals, backup_loc_var);
        emptylist(backup_loc_var);
      } /* if */
      /* show_arg_list(evaluated_act_params); */
      par_restore(block->params, backup_form_params, evaluated_act_params);
      emptylist(backup_form_params);
      emptylist(evaluated_act_params);
    } /* if */
#ifdef TRACE_EXEC
    printf("END exec_lambda\n");
#endif
    return(result);
  } /* exec_lambda */



#ifdef ANSI_C

static listtype eval_arg_list (register listtype act_param_list)
#else

static listtype eval_arg_list (act_param_list)
register listtype act_param_list;
#endif

  {
    listtype evaluated_act_params;
    register listtype *evaluated_insert_place;

  /* eval_arg_list */
    evaluated_act_params = NULL;
    evaluated_insert_place = &evaluated_act_params;    
    while (act_param_list != NULL && !fail_flag) {
      append_to_list(evaluated_insert_place, exec_object(act_param_list->obj), act_param_list);
      act_param_list = act_param_list->next;
    } /* while */
    return(evaluated_act_params);
  } /* eval_arg_list */



#ifdef ANSI_C

static void dump_arg_list (listtype evaluated_act_params)
#else

static void dump_arg_list (evaluated_act_params)
listtype evaluated_act_params;
#endif

  {
    register listtype list_end;

  /* dump_arg_list */
    if (evaluated_act_params != NULL) {
      list_end = evaluated_act_params;
      while (list_end->next != NULL) {
        if (list_end->obj != NULL &&
            TEMP_OBJECT(list_end->obj)) {
          dump_any_temp(list_end->obj);
        } /* if */
        list_end = list_end->next;
      } /* while */
      if (list_end->obj != NULL &&
          TEMP_OBJECT(list_end->obj)) {
        dump_any_temp(list_end->obj);
      } /* if */
      to_empty_list(evaluated_act_params, list_end);
    } /* if */
  } /* dump_arg_list */



#ifdef ANSI_C

static objecttype exec_action (objecttype act_object,
    listtype act_param_list, objecttype object)
#else

static objecttype exec_action (act_object, act_param_list, object)
objecttype act_object;
listtype act_param_list;
objecttype object;
#endif

  {
    listtype evaluated_act_params;
    objecttype result;

  /* exec_action */
#ifdef TRACE_EXEC
    printf("BEGIN exec_action(%s)\n",
        get_primact(act_object->value.actvalue)->name);
#endif
    evaluated_act_params = eval_arg_list(act_param_list);
    if (fail_flag) {
      emptylist(evaluated_act_params);
      result = fail_value;
    } else {
#ifdef WITH_ACTION_CHECK
      if (trace.check_actions) {
        if (!act_okay(act_object->value.actvalue)) {
          result = raise_with_arguments(SYS_ACT_ILLEGAL_EXCEPTION,
              evaluated_act_params);
        } /* if */
      } /* if */
#endif
#ifdef WITH_PROTOCOL
      if (trace.actions) {
        /* heap_statistic(); */
        prot_heapsize();
        prot_cstri(" ");
        prot_cstri(get_primact(act_object->value.actvalue)->name);
        prot_cstri("(");
        prot_list(evaluated_act_params);
        prot_cstri(") ");
        prot_flush();
        /* curr_action_object = act_object; */
        curr_exec_object = object;
        curr_agument_list = evaluated_act_params;
        result = (*(act_object->value.actvalue))(evaluated_act_params);
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
              prot_cstri(get_primact(act_object->value.actvalue)->name);
            } /* if */
          } else {
            prot_cstri("** act_object->type_of->result_type == NULL ");
          } /* if */
        } else {
          prot_cstri("** act_object->type_of == NULL ");
        } /* if */
        prot_cstri(" ==> ");
        printobject(result);
        prot_cstri(" ");
        prot_heapsize();
        prot_nl();
        prot_flush();
      } else {
#endif
        /* curr_action_object = act_object; */
        curr_exec_object = object;
        curr_agument_list = evaluated_act_params;
        result = (*(act_object->value.actvalue))(evaluated_act_params);
        if (result != NULL && result->type_of == NULL) {
          result->type_of = act_object->type_of->result_type;
        } /* if */
#ifdef WITH_PROTOCOL
      } /* if */
#endif
      dump_arg_list(evaluated_act_params);
      /* emptylist(evaluated_act_params); replaced by to_empty_list */
    } /* if */
#ifdef TRACE_EXEC
    printf("END exec_action\n");
#endif
    return(result);
  } /* exec_action */



#ifdef ANSI_C

static void exec_all_parameters (listtype act_param_list)
#else

static void exec_all_parameters (act_param_list)
listtype act_param_list;
#endif

  { /* exec_all_parameters */
#ifdef TRACE_EXEC
    printf("BEGIN exec_all_parameters\n");
#endif
    while (act_param_list != NULL && !fail_flag) {
      exec_object(act_param_list->obj);
      act_param_list = act_param_list->next;
    } /* while */
#ifdef TRACE_EXEC
    printf("END exec_all_parameters\n");
#endif
  } /* exec_all_parameters */



#ifdef ANSI_C

objecttype exec_call (objecttype object)
#else

objecttype exec_call (object)
objecttype object;
#endif

  {
    objecttype subroutine_object;
    listtype actual_parameters;
    objecttype result;

  /* exec_call */
#ifdef TRACE_EXEC
    printf("BEGIN exec_call ");
    trace1(object);
    printf(" <-> ");
    trace1(object->value.listvalue->obj);
    printf(" (");
    prot_list(object->value.listvalue->next);
    printf(")\n");
#endif
    subroutine_object = object->value.listvalue->obj;
    actual_parameters = object->value.listvalue->next;
/*  if (CLASS_OF_OBJ(subroutine_object) == REFPARAMOBJECT) {
      printf("refparamobject ");
      trace1(subroutine_object);
      printf(" value ");
      trace1(subroutine_object->value.objvalue);
      printf(" params ");
      prot_list(actual_parameters);
      printf("\n");
      printf("\n");
      subroutine_object = subroutine_object->value.objvalue;
    }  if */
    switch (CLASS_OF_OBJ(subroutine_object)) {
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
        result = exec_lambda(subroutine_object->value.blockvalue,
            actual_parameters, object);
        break;
      case CONSTENUMOBJECT:
/*        printf("constenumobject ");
        trace1(subroutine_object);
        printf(" params ");
        prot_list(actual_parameters);
        printf("\n"); */
        exec_all_parameters(actual_parameters);
        result = subroutine_object->value.objvalue;
        break;
      case INTOBJECT:
      case BIGINTOBJECT:
      case CHAROBJECT:
      case STRIOBJECT:
      case ARRAYOBJECT:
      case HASHOBJECT:
      case STRUCTOBJECT:
      case SETOBJECT:
      case FILEOBJECT:
      case LISTOBJECT:
#ifdef WITH_FLOAT
      case FLOATOBJECT:
#endif
      case WINOBJECT:
      case VARENUMOBJECT:
      case ENUMLITERALOBJECT:
      case REFOBJECT:
      case REFLISTOBJECT:
      case TYPEOBJECT:
      case CLASSOBJECT:
      case PROGOBJECT:
/*        printf("int/char/stri/array/file/type ");
        trace1(subroutine_object);
        printf(" params ");
        prot_list(actual_parameters);
        printf("\n"); */
        exec_all_parameters(actual_parameters);
        result = subroutine_object;
        break;
      case REFPARAMOBJECT:
/*        printf("refparamobject ");
        trace1(subroutine_object);
        printf(" value ");
        trace1(subroutine_object->value.objvalue);
        printf(" params ");
        prot_list(actual_parameters);
        printf("\n");
        printf("\n"); */
        result = evaluate(subroutine_object->value.objvalue);
        /* result = exec_object(subroutine_object->value.objvalue); */
        break;
      default:
        printf("class_of_obj: ");
        trace1(object);
        printf("\nsubroutine_object: ");
        trace1(subroutine_object);
        printf("\n");
        printf("\n");
/*        result = exec_dynamic(object->value.listvalue); */
        result = NULL;
        break;
    } /* switch */
#ifdef TRACE_EXEC
    printf("END exec_call ");
    trace1(result);
    printf("\n");
#endif
    return(result);
  } /* exec_call */



#ifdef ANSI_C

objecttype evaluate (objecttype object)
#else

objecttype evaluate (object)
objecttype object;
#endif

  {
    objecttype result;

  /* evaluate */
#ifdef TRACE_EXEC
    printf("BEGIN evaluate\n");
#endif
#ifdef OUT_OF_ORDER
    if (fail_flag) {
      printf("evaluate fail_flag for ");
      trace1(object);
      printf("\n");
    } /* if */
#endif
    switch (CLASS_OF_OBJ(object)) {
      case MATCHOBJECT:
        result = exec_call(object);
        break;
      case VALUEPARAMOBJECT:
      case REFPARAMOBJECT:
      case RESULTOBJECT:
      case CONSTENUMOBJECT:
      case VARENUMOBJECT:
        result = object->value.objvalue;
        break;
      case ENUMLITERALOBJECT:
        result = object;
        break;
      case BLOCKOBJECT:
        result = exec_lambda(object->value.blockvalue, NULL, object);
        break;
      case ACTOBJECT:
        result = exec_action(object, NULL, NULL);
        break;
      default:
        printf("evaluate unknown ");
        trace1(object);
        printf("\n");
        result = object;
        break;
    } /* switch */
#ifdef TRACE_EXEC
    printf("END evaluate ");
    trace1(result);
    printf("\n");
#endif
    return(result);
  } /* evaluate */



#ifdef ANSI_C

objecttype eval_expression (objecttype object)
#else

objecttype eval_expression (object)
objecttype object;
#endif

  {
    objecttype result;
    objecttype matched_expression;
    objecttype matched_object;

  /* eval_expression */
#ifdef TRACE_EXEC
    printf("BEGIN eval_expression\n");
#endif
    if ((matched_expression = match_expression(object)) != NULL) {
      if ((matched_object = match_object(matched_expression)) != NULL) {
/*
        printf("eval expression match succeeded ");
        trace1(matched_object);
        printf("\n"); */
        if (CLASS_OF_OBJ(matched_object) == CALLOBJECT) {
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
#ifdef TRACE_EXEC
    printf("END eval_expression\n");
#endif
    return(result);
  } /* eval_expression */



#ifdef ANSI_C

objecttype exec_dynamic (listtype expr_list)
#else

objecttype exec_dynamic (expr_list)
listtype expr_list;
#endif

  {
    objecttype match_expr;
    listtype actual_element;
    listtype *list_insert_place;
    objecttype element_value;
    booltype value_dereferenced;
    objecttype match_result;
    objecttype result = NULL;
    errinfotype err_info = OKAY_NO_ERROR;

  /* exec_dynamic */
#ifdef TRACE_EXEC
    printf("BEGIN exec_dynamic\n");
#endif
#ifdef WITH_PROTOCOL
    if (trace.dynamic) {
      prot_heapsize();
      prot_cstri(" DYNAMIC ");
      prot_list(expr_list);
      prot_nl();
    } /* if */
#endif
    if (ALLOC_OBJECT(match_expr)) {
      match_expr->type_of = take_type(SYS_EXPR_TYPE);
      match_expr->descriptor.entity = entity.literal;
      match_expr->value.listvalue = NULL;
      list_insert_place = &match_expr->value.listvalue;
      INIT_CLASS_OF_OBJ(match_expr, EXPROBJECT);
      value_dereferenced = FALSE;
      actual_element = expr_list;
      while (actual_element != NULL) {
/* printf("actual_element->obj ");
trace1(actual_element->obj);
printf("\n"); */
        switch (CLASS_OF_OBJ(actual_element->obj)) {
          case VALUEPARAMOBJECT:
          case REFPARAMOBJECT:
          case RESULTOBJECT:
          case LOCALVOBJECT:
          case CONSTENUMOBJECT:
          case VARENUMOBJECT:
          case CLASSOBJECT:
            element_value = actual_element->obj->value.objvalue;
            value_dereferenced = TRUE;
            break;
          default:
            element_value = actual_element->obj;
            break;
        } /* switch */
/* printf("element_value ");
trace1(element_value);
printf("\n"); */
        /* err_info is not checked after append! */
        list_insert_place = append_element_to_list(list_insert_place,
            element_value, &err_info);
        actual_element = actual_element->next;
      } /* while */
/* printf("match_expr ");
trace1(match_expr);
printf("\n"); */
      if (match_expression(match_expr) != NULL &&
          (match_result = match_object(match_expr)) != NULL) {
#ifdef WITH_PROTOCOL
        if (trace.dynamic) {
          prot_cstri("matched ==> ");
          trace1(match_result);
        } /* if */
#endif
        result = exec_call(match_result);

        if (match_result != match_expr) {
          FREE_OBJECT(match_result->value.listvalue->obj);
          emptylist(match_result->value.listvalue);
          FREE_OBJECT(match_result);
        } else {
          emptylist(match_expr->value.listvalue);
          FREE_OBJECT(match_expr);          
        } /* if */
#ifdef WITH_PROTOCOL
        if (trace.dynamic) {
          prot_cstri(" ");
          prot_heapsize();
          prot_nl();
        } /* if */
#endif
      } else {
        return(raise_with_arguments(SYS_MEM_EXCEPTION, expr_list));
      } /* if */
    } else {
      return(raise_with_arguments(SYS_MEM_EXCEPTION, expr_list));
    } /* if */
#ifdef TRACE_EXEC
    printf("END exec_dynamic\n");
#endif
    return(result);
  } /* exec_dynamic */



#ifdef ANSI_C

objecttype exec_expr (progtype currentProg, objecttype object)
#else

objecttype exec_expr (currentProg, object)
progtype currentProg;
objecttype object;
#endif

  {
    progrecord prog_backup;
    objecttype result;

  /* exec_expr */
#ifdef TRACE_EXEC
    printf("BEGIN exec_expr\n");
#endif
    if (currentProg != NULL) {
      fail_flag = FALSE;
      fail_value = (objecttype) NULL;
      fail_expression = (listtype) NULL;
      set_trace(option.exec_trace_level, -1, option.prot_file_name);
      memcpy(&prog_backup, &prog, sizeof(progrecord));
      memcpy(&prog, currentProg, sizeof(progrecord));
      result = exec_object(object);
      if (fail_flag) {
        printf("\n*** Uncaught EXCEPTION ");
        printobject(fail_value);
        printf(" raised with\n");
        prot_list(fail_expression);
        printf("\n");
      } /* if */
      memcpy(&prog, &prog_backup, sizeof(progrecord));
    } else {
      result = NULL;
    } /* if */
#ifdef TRACE_EXEC
    printf("END exec_expr\n");
#endif
    return(result);
  } /* exec_expr */



#ifdef ANSI_C

void interpr (progtype currentProg)
#else

void interpr (currentProg)
progtype currentProg;
#endif

  {
    progrecord prog_backup;
    objecttype resultOfProgram;

  /* interpr */
#ifdef TRACE_EXEC
    printf("BEGIN interpr\n");
#endif
    if (currentProg != NULL) {
      fail_flag = FALSE;
      fail_value = (objecttype) NULL;
      fail_expression = (listtype) NULL;
      fail_stack = NULL;
      set_trace(option.exec_trace_level, -1, option.prot_file_name);
      if (currentProg->main_object != NULL) {
        memcpy(&prog_backup, &prog, sizeof(progrecord));
        memcpy(&prog, currentProg, sizeof(progrecord));
/*        printf("main defined as: ");
        trace1(prog.main_object);
        printf("\n"); */
#ifdef WITH_PROTOCOL
        if (trace.actions) {
          prot_heapsize();
          prot_cstri(" begin main");
          prot_nl();
        } /* if */
#endif
        resultOfProgram = exec_call(prog.main_object);
#ifdef WITH_PROTOCOL
        if (trace.actions) {
          prot_heapsize();
          prot_cstri(" end main");
          prot_nl();
        } /* if */
#endif
#ifdef OUT_OF_ORDER
        shut_drivers();
        if (fail_flag) {
          printf("\n*** Uncaught EXCEPTION ");
          printobject(fail_value);
          printf(" raised with\n");
          prot_list(fail_expression);
          printf("\n");
          write_call_stack(fail_stack);
        } /* if */
#endif
        memcpy(&prog, &prog_backup, sizeof(progrecord));
      } /* if */
    } /* if */
#ifdef TRACE_EXEC
    printf("END interpr\n");
#endif
  } /* interpr */

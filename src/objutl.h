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
/*  Module: Runtime                                                 */
/*  File: seed7/src/objutl.h                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: isit_.. and bld_.. functions for primitive data types. */
/*                                                                  */
/********************************************************************/

#define arg_1(arguments) arguments->obj
#define arg_2(arguments) arguments->next->obj
#define arg_3(arguments) arguments->next->next->obj
#define arg_4(arguments) arguments->next->next->next->obj
#define arg_5(arguments) arguments->next->next->next->next->obj
#define arg_6(arguments) arguments->next->next->next->next->next->obj
#define arg_7(arguments) arguments->next->next->next->next->next->next->obj
#define arg_8(arguments) arguments->next->next->next->next->next->next->next->obj
#define arg_9(arguments) arguments->next->next->next->next->next->next->next->next->obj
#define arg_10(arguments) arguments->next->next->next->next->next->next->next->next->next->obj
#define arg_11(arguments) arguments->next->next->next->next->next->next->next->next->next->next->obj
#define arg_12(arguments) arguments->next->next->next->next->next->next->next->next->next->next->next->obj

#define take_act_obj(arg)   (CATEGORY_OF_OBJ(arg) == MATCHOBJECT ? take_reflist(arg)->obj : (arg))
#define take_action(arg)    take_act_obj(arg)->value.actValue
#define take_array(arg)     (arg)->value.arrayValue
#define take_binary(arg)    (arg)->value.binaryValue
#define take_block(arg)     (arg)->value.blockValue
#define take_bool(arg)      (CATEGORY_OF_OBJ(arg) == CONSTENUMOBJECT || CATEGORY_OF_OBJ(arg) == VARENUMOBJECT ? (arg)->value.objValue : (arg))
#define take_bstri(arg)     (arg)->value.bstriValue
#define take_char(arg)      (arg)->value.charValue
#define take_database(arg)  (arg)->value.databaseValue
#define take_enum(arg)      (CATEGORY_OF_OBJ(arg) == CONSTENUMOBJECT || CATEGORY_OF_OBJ(arg) == VARENUMOBJECT ? (arg)->value.objValue : (arg))
#define take_file(arg)      (arg)->value.fileValue
#define take_float(arg)     (arg)->value.floatValue
#define take_hash(arg)      (arg)->value.hashValue
#define take_int(arg)       (arg)->value.intValue
#define take_interface(arg) (CATEGORY_OF_OBJ(arg) == INTERFACEOBJECT ? (arg)->value.objValue : (arg))
#define take_bigint(arg)    (arg)->value.bigIntValue
#define take_list(arg)      (arg)->value.listValue
#define take_param(arg)     (arg)->value.objValue
#define take_poll(arg)      (arg)->value.pollValue
#define take_prog(arg)      (arg)->value.progValue
#define take_reference(arg) (arg)->value.objValue
#define take_reflist(arg)   (arg)->value.listValue
#define take_set(arg)       (arg)->value.setValue
#define take_socket(arg)    (arg)->value.socketValue
#define take_sqlstmt(arg)   (arg)->value.sqlStmtValue
#define take_stri(arg)      (arg)->value.striValue
#define take_struct(arg)    (arg)->value.structValue
#define take_type(arg)      (arg)->value.typeValue
#define take_win(arg)       (arg)->value.winValue
#define take_pointlist(arg) (arg)->value.bstriValue
#define take_process(arg)   (arg)->value.processValue

#if WITH_TYPE_CHECK
#define run_exception(c,arg) { run_error(c, arg); return NULL; }
#define hasCategory(arg,cat)             if (unlikely(CATEGORY_OF_OBJ(arg) != (cat))) run_exception(cat, arg)
#define hasCategory2(arg,cat1,cat2)      if (unlikely(CATEGORY_OF_OBJ(arg) != (cat1) && \
                                                      CATEGORY_OF_OBJ(arg) != (cat2))) run_exception(cat1, arg)
#define hasCategory3(arg,cat1,cat2,cat3) if (unlikely(CATEGORY_OF_OBJ(arg) != (cat1) && \
                                                      CATEGORY_OF_OBJ(arg) != (cat2) && \
                                                      CATEGORY_OF_OBJ(arg) != (cat3))) run_exception(cat1, arg)
#define isit_action(arg)    hasCategory(take_act_obj(arg), ACTOBJECT)
#define isit_array(arg)     hasCategory(arg, ARRAYOBJECT); \
                            if (unlikely(take_array(arg) == NULL))      { empty_value(arg); return NULL; }
#define isit_binary(arg)    hasCategory(arg, INTOBJECT)
#define isit_bigint(arg)    hasCategory(arg, BIGINTOBJECT)
#define isit_block(arg)     hasCategory(arg, BLOCKOBJECT)
#define isit_bool(arg)      if (take_bool(arg) != SYS_TRUE_OBJECT && \
                                take_bool(arg) != SYS_FALSE_OBJECT) { \
                              run_exception(ENUMLITERALOBJECT, arg); \
                            }
#define isit_bstri(arg)     hasCategory(arg, BSTRIOBJECT); \
                            if (unlikely(take_bstri(arg) == NULL))      { empty_value(arg); return NULL; }
#define isit_call(arg)      hasCategory(arg, CALLOBJECT)
#define isit_char(arg)      hasCategory(arg, CHAROBJECT)
#define isit_database(arg)  hasCategory(arg, DATABASEOBJECT)
#define isit_interface(arg) hasCategory2(arg, INTERFACEOBJECT, STRUCTOBJECT); \
                            if (unlikely(take_interface(arg) == NULL))  { empty_value(arg); return NULL; }
/*      isit_enum(arg)      */
#define isit_file(arg)      hasCategory(arg, FILEOBJECT)
#define isit_float(arg)     hasCategory(arg, FLOATOBJECT)
#define isit_hash(arg)      hasCategory(arg, HASHOBJECT); \
                            if (unlikely(take_hash(arg) == NULL))       { empty_value(arg); return NULL; }
#define isit_int(arg)       hasCategory(arg, INTOBJECT)
/*      isit_list(arg)      */
#define isit_param(arg)     hasCategory(arg, FORMPARAMOBJECT)
#define isit_poll(arg)      hasCategory(arg, POLLOBJECT)
#define isit_proc(arg)      hasCategory3(arg, BLOCKOBJECT, MATCHOBJECT, ACTOBJECT)
#define isit_prog(arg)      hasCategory(arg, PROGOBJECT)
#define isit_reference(arg) hasCategory(arg, REFOBJECT)
#define isit_not_null(arg)  if (unlikely(arg == NULL))                  { empty_value(arg); return NULL; }
#define isit_reflist(arg)   hasCategory3(arg, REFLISTOBJECT, MATCHOBJECT, CALLOBJECT)
#define isit_set(arg)       hasCategory(arg, SETOBJECT); \
                            if (unlikely(take_set(arg) == NULL))        { empty_value(arg); return NULL; }
#define isit_socket(arg)    hasCategory(arg, SOCKETOBJECT)
#define isit_sqlstmt(arg)   hasCategory(arg, SQLSTMTOBJECT)
#define isit_stri(arg)      hasCategory(arg, STRIOBJECT); \
                            if (unlikely(take_stri(arg) == NULL))       { empty_value(arg); return NULL; }
#define isit_struct(arg)    hasCategory(arg, STRUCTOBJECT); \
                            if (unlikely(take_struct(arg) == NULL))     { empty_value(arg); return NULL; }
#define isit_struct_ok(arg) if (unlikely(take_struct(arg) == NULL))     { empty_value(arg); return NULL; }
#define isit_type(arg)      hasCategory(arg, TYPEOBJECT)
#define isit_win(arg)       hasCategory(arg, WINOBJECT)
#define isit_pointlist(arg) hasCategory(arg, POINTLISTOBJECT); \
                            if (unlikely(take_bstri(arg) == NULL))      { empty_value(arg); return NULL; }
#define isit_process(arg)   hasCategory(arg, PROCESSOBJECT)
#define is_variable(arg)    if (unlikely(!VAR_OBJECT(arg)))             { var_required(arg); return NULL; }
#define isit_int2(arg)      if (unlikely(CATEGORY_OF_OBJ(arg) != INTOBJECT)) run_error(INTOBJECT, arg)
#define just_interface(arg) hasCategory(arg, INTERFACEOBJECT); \
                            if (unlikely(take_interface(arg) == NULL))  { empty_value(arg); return NULL; }
#else
#define isit_action(arg)
#define isit_array(arg)
#define isit_bigint(arg)
#define isit_block(arg)
#define isit_bool(arg)
#define isit_bstri(arg)
#define isit_call(arg)
#define isit_char(arg)
#define isit_database(arg)
#define isit_interface(arg)
#define isit_enum(arg)
#define isit_file(arg)
#define isit_float(arg)
#define isit_hash(arg)
#define isit_int(arg)
#define isit_list(arg)
#define isit_param(arg)
#define isit_proc(arg)
#define isit_prog(arg)
#define isit_reference(arg)
#define isit_not_null(arg)
#define isit_reflist(arg)
#define isit_set(arg)
#define isit_socket(arg)
#define isit_sqlstmt(arg)
#define isit_stri(arg)
#define isit_struct(arg)
#define isit_struct_ok(arg)
#define isit_type(arg)
#define isit_win(arg)
#define isit_pointlist(arg)
#define isit_process(arg)
#define is_variable(arg)
#define isit_int2(arg)
#define just_interface(arg)
#endif


#if WITH_TYPE_CHECK
/* void isit_action (objectType argument); */
/* void isit_array (objectType argument); */
/* void isit_block (objectType argument); */
/* void isit_bool (objectType argument); */
/* void isit_bstri (objectType argument); */
/* void isit_call (objectType argument); */
/* void isit_char (objectType argument); */
/* void isit_interface (objectType argument); */
void isit_enum (objectType argument);
/* void isit_file (objectType argument); */
#if WITH_FLOAT
/* void isit_float (objectType argument); */
#endif
/* void isit_hash (objectType argument); */
/* void isit_int (objectType argument); */
void isit_list (objectType argument);
/* void isit_proc (objectType argument); */
/* void isit_prog (objectType argument); */
/* void isit_reference (objectType argument); */
/* void isit_reflist (objectType argument); */
/* void isit_set (objectType argument); */
/* void isit_socket (objectType argument); */
/* void isit_stri (objectType argument); */
/* void isit_struct (objectType argument); */
/* void isit_type (objectType argument); */
/* void isit_win (objectType argument); */
/* void isit_pointlist (objectType argument); */
/* void isit_process (objectType argument); */
#endif
objectType bld_action_temp (actType temp_action);
objectType bld_array_temp (arrayType temp_array);
objectType bld_bigint_temp (bigIntType temp_bigint);
objectType bld_binary_temp (uintType temp_binary);
objectType bld_block_temp (blockType temp_block);
objectType bld_bstri_temp (bstriType temp_bstri);
objectType bld_char_temp (charType temp_char);
objectType bld_database_temp (databaseType temp_database);
objectType bld_interface_temp (objectType temp_interface);
objectType bld_file_temp (fileType temp_file);
objectType bld_float_temp (double temp_float);
objectType bld_hash_temp (hashType temp_hash);
objectType bld_int_temp (intType temp_int);
objectType bld_list_temp (listType temp_list);
objectType bld_param_temp (objectType temp_param);
objectType bld_poll_temp (pollType temp_poll);
objectType bld_prog_temp (progType temp_prog);
objectType bld_reference_temp (objectType temp_reference);
objectType bld_reflist_temp (listType temp_reflist);
objectType bld_set_temp (setType temp_set);
objectType bld_socket_temp (socketType temp_socket);
objectType bld_sqlstmt_temp (sqlStmtType temp_sqlstmt);
objectType bld_stri_temp (striType temp_stri);
objectType bld_struct_temp (structType temp_struct);
objectType bld_type_temp (typeType temp_type);
objectType bld_win_temp (winType temp_win);
objectType bld_pointlist_temp (bstriType temp_pointlist);
objectType bld_process_temp (processType temp_win);
void dump_temp_value (objectType object);
void dump_any_temp (objectType object);
void dump_list (listType list);

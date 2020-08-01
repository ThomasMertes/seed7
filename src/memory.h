/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
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
/*  License along with this program; if not, write to the Free      */
/*  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,  */
/*  MA 02111-1307 USA                                               */
/*                                                                  */
/*  Module: Runtime                                                 */
/*  File: seed7/src/memory.h                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: isit_.. and bld_.. functions for primitive datatypes.  */
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

#define take_action(arg)    (arg)->value.actvalue
#define take_array(arg)     (arg)->value.arrayvalue
#define take_block(arg)     (arg)->value.blockvalue
#define take_bool(arg)      (CLASS_OF_OBJ(arg) == CONSTENUMOBJECT || CLASS_OF_OBJ(arg) == VARENUMOBJECT ? (arg)->value.objvalue : (arg))
#define take_char(arg)      (arg)->value.charvalue
#define take_enum(arg)      (CLASS_OF_OBJ(arg) == CONSTENUMOBJECT || CLASS_OF_OBJ(arg) == VARENUMOBJECT ? (arg)->value.objvalue : (arg))
#define take_file(arg)      (arg)->value.filevalue
#define take_float(arg)     (arg)->value.floatvalue
#define take_hash(arg)      (arg)->value.hashvalue
#define take_int(arg)       (arg)->value.intvalue
#define take_bigint(arg)    (arg)->value.bigintvalue
#define take_list(arg)      (arg)->value.listvalue
#define take_prog(arg)      (arg)->value.progvalue
#define take_reference(arg) (arg)->value.objvalue
#define take_reflist(arg)   (arg)->value.listvalue
#define take_set(arg)       (arg)->value.setvalue
#define take_stri(arg)      (arg)->value.strivalue
#define take_struct(arg)    (arg)->value.structvalue
#define take_type(arg)      (arg)->value.typevalue
#define take_win(arg)       (arg)->value.winvalue

#ifdef WITH_TYPE_CHECK
#define isit_action(arg)    if (CLASS_OF_OBJ(arg) != ACTOBJECT)     run_error(ACTOBJECT, arg)
#define isit_array(arg)     if (CLASS_OF_OBJ(arg) != ARRAYOBJECT)   run_error(ARRAYOBJECT, arg); \
                            if (take_array(arg) == NULL)            empty_value(arg)
#define isit_bigint(arg)    if (CLASS_OF_OBJ(arg) != BIGINTOBJECT)  run_error(BIGINTOBJECT, arg)
#define isit_block(arg)     if (CLASS_OF_OBJ(arg) != BLOCKOBJECT)   run_error(BLOCKOBJECT, arg)
/*      isit_bool(arg)      */
#define isit_call(arg)      if (CLASS_OF_OBJ(arg) != CALLOBJECT)    run_error(CALLOBJECT, arg)
#define isit_char(arg)      if (CLASS_OF_OBJ(arg) != CHAROBJECT)    run_error(CHAROBJECT, arg)
#define isit_class(arg)     if (CLASS_OF_OBJ(arg) != CLASSOBJECT)   run_error(CLASSOBJECT, arg); \
                            if (take_reference(arg) == NULL)        empty_value(arg)
/*      isit_enum(arg)      */
#define isit_file(arg)      if (CLASS_OF_OBJ(arg) != FILEOBJECT)    run_error(FILEOBJECT, arg)
#define isit_float(arg)     if (CLASS_OF_OBJ(arg) != FLOATOBJECT)   run_error(FLOATOBJECT, arg)
#define isit_hash(arg)      if (CLASS_OF_OBJ(arg) != HASHOBJECT)    run_error(HASHOBJECT, arg); \
                            if (take_hash(arg) == NULL)             empty_value(arg)
#define isit_int(arg)       if (CLASS_OF_OBJ(arg) != INTOBJECT)     run_error(INTOBJECT, arg)
/*      isit_list(arg)      */
#define isit_proc(arg)      if (CLASS_OF_OBJ(arg) != BLOCKOBJECT && \
                                CLASS_OF_OBJ(arg) != ACTOBJECT)     run_error(BLOCKOBJECT, arg)
#define isit_prog(arg)      if (CLASS_OF_OBJ(arg) != PROGOBJECT)    run_error(PROGOBJECT, arg)
#define isit_reference(arg) if (CLASS_OF_OBJ(arg) != REFOBJECT)     run_error(REFOBJECT, arg)
#define isit_reflist(arg)   if (CLASS_OF_OBJ(arg) != REFLISTOBJECT) run_error(REFLISTOBJECT, arg)
#define isit_set(arg)       if (CLASS_OF_OBJ(arg) != SETOBJECT)     run_error(SETOBJECT, arg); \
                            if (take_set(arg) == NULL)              empty_value(arg)
#define isit_stri(arg)      if (CLASS_OF_OBJ(arg) != STRIOBJECT)    run_error(STRIOBJECT, arg); \
                            if (take_stri(arg) == NULL)             empty_value(arg)
#define isit_struct(arg)    if (CLASS_OF_OBJ(arg) != STRUCTOBJECT)  run_error(STRUCTOBJECT, arg); \
                            if (take_struct(arg) == NULL)           empty_value(arg)
#define isit_type(arg)      if (CLASS_OF_OBJ(arg) != TYPEOBJECT)    run_error(TYPEOBJECT, arg)
#define isit_win(arg)       if (CLASS_OF_OBJ(arg) != WINOBJECT)     run_error(WINOBJECT, arg)
#define is_variable(arg)    if (!VAR_OBJECT(arg)) var_required(arg);
#else
#define isit_action(arg)
#define isit_array(arg)
#define isit_block(arg)
#define isit_bool(arg)
#define isit_call(arg)
#define isit_char(arg)
#define isit_class(arg)
#define isit_enum(arg)
#define isit_file(arg)
#define isit_float(arg)
#define isit_hash(arg)
#define isit_int(arg)
#define isit_list(arg)
#define isit_proc(arg)
#define isit_prog(arg)
#define isit_reference(arg)
#define isit_reflist(arg)
#define isit_set(arg)
#define isit_stri(arg)
#define isit_struct(arg)
#define isit_type(arg)
#define isit_win(arg)
#define is_variable(arg)
#endif


#ifdef ANSI_C

#ifdef WITH_TYPE_CHECK
/* void isit_action (objecttype); */
/* void isit_array (objecttype); */
/* void isit_block (objecttype); */
void isit_bool (objecttype);
/* void isit_call (objecttype); */
/* void isit_char (objecttype); */
/* void isit_class (objecttype); */
void isit_enum (objecttype);
/* void isit_file (objecttype); */
#ifdef WITH_FLOAT
/* void isit_float (objecttype); */
#endif
/* void isit_hash (objecttype); */
/* void isit_int (objecttype); */
void isit_list (objecttype);
/* void isit_proc (objecttype); */
/* void isit_prog (objecttype); */
/* void isit_reference (objecttype); */
/* void isit_reflist (objecttype); */
/* void isit_set (objecttype); */
/* void isit_stri (objecttype); */
/* void isit_struct (objecttype); */
/* void isit_type (objecttype); */
/* void isit_win (objecttype); */
#endif
objecttype bld_action_temp (acttype);
objecttype bld_array_temp (arraytype);
objecttype bld_bigint_temp (biginttype);
objecttype bld_block_temp (blocktype);
objecttype bld_char_temp (chartype);
objecttype bld_class_temp (objecttype);
objecttype bld_file_temp (filetype);
objecttype bld_float_temp (double);
objecttype bld_hash_temp (hashtype);
objecttype bld_int_temp (inttype);
objecttype bld_list_temp (listtype);
objecttype bld_param_temp (objecttype);
objecttype bld_prog_temp (progtype);
objecttype bld_reference_temp (objecttype);
objecttype bld_reflist_temp (listtype);
objecttype bld_set_temp (settype);
objecttype bld_stri_temp (stritype);
objecttype bld_struct_temp (structtype);
objecttype bld_type_temp (typetype);
objecttype bld_win_temp (wintype);
void dump_any_temp (objecttype);

#else

#ifdef WITH_TYPE_CHECK
/* void isit_action (); */
/* void isit_array (); */
/* void isit_block (); */
void isit_bool ();
/* void isit_call (); */
/* void isit_char (); */
/* void isit_class (); */
void isit_enum ();
/* void isit_file (); */
#ifdef WITH_FLOAT
/* void isit_float (); */
#endif
/* void isit_hash (); */
/* void isit_int (); */
void isit_list ();
/* void isit_proc (); */
/* void isit_prog (); */
/* void isit_reference (); */
/* void isit_reflist (); */
/* void isit_set (); */
/* void isit_stri (); */
/* void isit_struct (); */
/* void isit_type (); */
/* void isit_win (); */
#endif
objecttype bld_action_temp ();
objecttype bld_array_temp ();
objecttype bld_bigint_temp ();
objecttype bld_block_temp ();
objecttype bld_char_temp ();
objecttype bld_class_temp ();
objecttype bld_file_temp ();
objecttype bld_float_temp ();
objecttype bld_hash_temp ();
objecttype bld_int_temp ();
objecttype bld_list_temp ();
objecttype bld_param_temp ();
objecttype bld_prog_temp ();
objecttype bld_reference_temp ();
objecttype bld_reflist_temp ();
objecttype bld_set_temp ();
objecttype bld_stri_temp ();
objecttype bld_struct_temp ();
objecttype bld_type_temp ();
objecttype bld_win_temp ();
void dump_any_temp ();

#endif

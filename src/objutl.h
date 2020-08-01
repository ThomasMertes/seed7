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
#define take_bool(arg)      (CATEGORY_OF_OBJ(arg) == CONSTENUMOBJECT || CATEGORY_OF_OBJ(arg) == VARENUMOBJECT ? (arg)->value.objvalue : (arg))
#define take_bstri(arg)     (arg)->value.bstrivalue
#define take_char(arg)      (arg)->value.charvalue
#define take_database(arg)  (arg)->value.databasevalue
#define take_enum(arg)      (CATEGORY_OF_OBJ(arg) == CONSTENUMOBJECT || CATEGORY_OF_OBJ(arg) == VARENUMOBJECT ? (arg)->value.objvalue : (arg))
#define take_file(arg)      (arg)->value.filevalue
#define take_float(arg)     (arg)->value.floatvalue
#define take_hash(arg)      (arg)->value.hashvalue
#define take_int(arg)       (arg)->value.intvalue
#define take_interface(arg) (CATEGORY_OF_OBJ(arg) == INTERFACEOBJECT ? (arg)->value.objvalue : (arg))
#define take_bigint(arg)    (arg)->value.bigintvalue
#define take_list(arg)      (arg)->value.listvalue
#define take_param(arg)     (arg)->value.objvalue
#define take_poll(arg)      (arg)->value.pollvalue
#define take_prog(arg)      (arg)->value.progvalue
#define take_reference(arg) (arg)->value.objvalue
#define take_reflist(arg)   (arg)->value.listvalue
#define take_set(arg)       (arg)->value.setvalue
#define take_socket(arg)    (arg)->value.socketvalue
#define take_sqlstmt(arg)   (arg)->value.sqlstmtvalue
#define take_stri(arg)      (arg)->value.strivalue
#define take_struct(arg)    (arg)->value.structvalue
#define take_type(arg)      (arg)->value.typevalue
#define take_win(arg)       (arg)->value.winvalue

#ifdef WITH_TYPE_CHECK
#define run_exception(c,arg) { run_error(c, arg); return(NULL); }
#define hasCategory(arg,cat)             if (unlikely(CATEGORY_OF_OBJ(arg) != (cat))) run_exception(cat, arg)
#define hasCategory2(arg,cat1,cat2)      if (unlikely(CATEGORY_OF_OBJ(arg) != (cat1) && \
                                                      CATEGORY_OF_OBJ(arg) != (cat2))) run_exception(cat1, arg)
#define hasCategory3(arg,cat1,cat2,cat3) if (unlikely(CATEGORY_OF_OBJ(arg) != (cat1) && \
                                                      CATEGORY_OF_OBJ(arg) != (cat2) && \
                                                      CATEGORY_OF_OBJ(arg) != (cat3))) run_exception(cat1, arg)
#define isit_action(arg)    hasCategory(arg, ACTOBJECT)
#define isit_array(arg)     hasCategory(arg, ARRAYOBJECT); \
                            if (unlikely(take_array(arg) == NULL))      { empty_value(arg); return(NULL); }
#define isit_bigint(arg)    hasCategory(arg, BIGINTOBJECT)
#define isit_block(arg)     hasCategory(arg, BLOCKOBJECT)
/*      isit_bool(arg)      */
#define isit_bstri(arg)     hasCategory(arg, BSTRIOBJECT); \
                            if (unlikely(take_bstri(arg) == NULL))      { empty_value(arg); return(NULL); }
#define isit_call(arg)      hasCategory(arg, CALLOBJECT)
#define isit_char(arg)      hasCategory(arg, CHAROBJECT)
#define isit_database(arg)  hasCategory(arg, DATABASEOBJECT)
#define isit_interface(arg) hasCategory2(arg, INTERFACEOBJECT, STRUCTOBJECT); \
                            if (unlikely(take_interface(arg) == NULL))  { empty_value(arg); return(NULL); }
/*      isit_enum(arg)      */
#define isit_file(arg)      hasCategory(arg, FILEOBJECT)
#define isit_float(arg)     hasCategory(arg, FLOATOBJECT)
#define isit_hash(arg)      hasCategory(arg, HASHOBJECT); \
                            if (unlikely(take_hash(arg) == NULL))       { empty_value(arg); return(NULL); }
#define isit_int(arg)       hasCategory(arg, INTOBJECT)
/*      isit_list(arg)      */
#define isit_param(arg)     hasCategory(arg, FORMPARAMOBJECT)
#define isit_poll(arg)      hasCategory(arg, POLLOBJECT)
#define isit_proc(arg)      hasCategory3(arg, BLOCKOBJECT, MATCHOBJECT, ACTOBJECT)
#define isit_prog(arg)      hasCategory(arg, PROGOBJECT)
#define isit_reference(arg) hasCategory(arg, REFOBJECT)
#define isit_reflist(arg)   hasCategory3(arg, REFLISTOBJECT, MATCHOBJECT, CALLOBJECT)
#define isit_set(arg)       hasCategory(arg, SETOBJECT); \
                            if (unlikely(take_set(arg) == NULL))        { empty_value(arg); return(NULL); }
#define isit_socket(arg)    hasCategory(arg, SOCKETOBJECT)
#define isit_sqlstmt(arg)   hasCategory(arg, SQLSTMTOBJECT)
#define isit_stri(arg)      hasCategory(arg, STRIOBJECT); \
                            if (unlikely(take_stri(arg) == NULL))       { empty_value(arg); return(NULL); }
#define isit_struct(arg)    hasCategory(arg, STRUCTOBJECT); \
                            if (unlikely(take_struct(arg) == NULL))     { empty_value(arg); return(NULL); }
#define isit_type(arg)      hasCategory(arg, TYPEOBJECT)
#define isit_win(arg)       hasCategory(arg, WINOBJECT)
#define is_variable(arg)    if (unlikely(!VAR_OBJECT(arg)))             { var_required(arg); return(NULL); }
#define isit_int2(arg)      if (unlikely(CATEGORY_OF_OBJ(arg) != INTOBJECT)) run_error(INTOBJECT, arg)
#define just_interface(arg) hasCategory(arg, INTERFACEOBJECT); \
                            if (unlikely(take_interface(arg) == NULL))  { empty_value(arg); return(NULL); }
#else
#define isit_action(arg)
#define isit_array(arg)
#define isit_bigint(arg)
#define isit_block(arg)
#define isit_bool(arg)
#define isit_bstri(arg)
#define isit_call(arg)
#define isit_char(arg)
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
#define isit_reflist(arg)
#define isit_set(arg)
#define isit_socket(arg)
#define isit_stri(arg)
#define isit_struct(arg)
#define isit_type(arg)
#define isit_win(arg)
#define is_variable(arg)
#define isit_int2(arg)
#define just_interface(arg)
#endif


#ifdef ANSI_C

#ifdef WITH_TYPE_CHECK
/* void isit_action (objecttype); */
/* void isit_array (objecttype); */
/* void isit_block (objecttype); */
void isit_bool (objecttype argument);
/* void isit_bstri (objecttype); */
/* void isit_call (objecttype); */
/* void isit_char (objecttype); */
/* void isit_interface (objecttype); */
void isit_enum (objecttype argument);
/* void isit_file (objecttype); */
#ifdef WITH_FLOAT
/* void isit_float (objecttype); */
#endif
/* void isit_hash (objecttype); */
/* void isit_int (objecttype); */
void isit_list (objecttype argument);
/* void isit_proc (objecttype); */
/* void isit_prog (objecttype); */
/* void isit_reference (objecttype); */
/* void isit_reflist (objecttype); */
/* void isit_set (objecttype); */
/* void isit_socket (objecttype); */
/* void isit_stri (objecttype); */
/* void isit_struct (objecttype); */
/* void isit_type (objecttype); */
/* void isit_win (objecttype); */
#endif
objecttype bld_action_temp (acttype temp_action);
objecttype bld_array_temp (arraytype temp_array);
objecttype bld_bigint_temp (biginttype temp_bigint);
objecttype bld_block_temp (blocktype temp_block);
objecttype bld_bstri_temp (bstritype temp_bstri);
objecttype bld_char_temp (chartype temp_char);
objecttype bld_database_temp (databasetype temp_database);
objecttype bld_interface_temp (objecttype temp_interface);
objecttype bld_file_temp (filetype temp_file);
objecttype bld_float_temp (double temp_float);
objecttype bld_hash_temp (hashtype temp_hash);
objecttype bld_int_temp (inttype temp_int);
objecttype bld_list_temp (listtype temp_list);
objecttype bld_param_temp (objecttype temp_param);
objecttype bld_poll_temp (polltype temp_poll);
objecttype bld_prog_temp (progtype temp_prog);
objecttype bld_reference_temp (objecttype temp_reference);
objecttype bld_reflist_temp (listtype temp_reflist);
objecttype bld_set_temp (settype temp_set);
objecttype bld_socket_temp (sockettype temp_socket);
objecttype bld_sqlstmt_temp (sqlstmttype temp_sqlstmt);
objecttype bld_stri_temp (stritype temp_stri);
objecttype bld_struct_temp (structtype temp_struct);
objecttype bld_type_temp (typetype temp_type);
objecttype bld_win_temp (wintype temp_win);
void dump_temp_value (objecttype object);
void dump_any_temp (objecttype object);
void dump_list (listtype list);

#else

#ifdef WITH_TYPE_CHECK
/* void isit_action (); */
/* void isit_array (); */
/* void isit_block (); */
void isit_bool ();
/* void isit_bstri (); */
/* void isit_call (); */
/* void isit_char (); */
/* void isit_interface (); */
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
/* void isit_socket (); */
/* void isit_stri (); */
/* void isit_struct (); */
/* void isit_type (); */
/* void isit_win (); */
#endif
objecttype bld_action_temp ();
objecttype bld_array_temp ();
objecttype bld_bigint_temp ();
objecttype bld_block_temp ();
objecttype bld_bstri_temp ();
objecttype bld_char_temp ();
objecttype bld_interface_temp ();
objecttype bld_file_temp ();
objecttype bld_float_temp ();
objecttype bld_hash_temp ();
objecttype bld_int_temp ();
objecttype bld_list_temp ();
objecttype bld_param_temp ();
objecttype bld_poll_temp ();
objecttype bld_prog_temp ();
objecttype bld_reference_temp ();
objecttype bld_reflist_temp ();
objecttype bld_set_temp ();
objecttype bld_socket_temp ();
objecttype bld_stri_temp ();
objecttype bld_struct_temp ();
objecttype bld_type_temp ();
objecttype bld_win_temp ();
void dump_temp_value ();
void dump_any_temp ();
void dump_list ();

#endif

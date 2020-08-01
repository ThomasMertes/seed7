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
/*  Module: Interpreter                                             */
/*  File: seed7/src/executl.h                                       */
/*  Changes: 1993, 1994  Thomas Mertes                              */
/*  Content: Initalisation operation procedures used at runtime.    */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype get_create_call_obj (objecttype, errinfotype *);
objecttype get_destroy_call_obj (objecttype, errinfotype *);
void do_create (objecttype, objecttype, errinfotype *);
void do_destroy (objecttype, errinfotype *);
inttype do_ord (objecttype, errinfotype *);
booltype do_in (objecttype, objecttype, errinfotype *);
objecttype param1_call (objecttype, objecttype);
objecttype param2_call (objecttype, objecttype, objecttype);
objecttype param3_call (objecttype, objecttype, objecttype, objecttype);
objecttype create_return_object (locobjtype, objecttype, errinfotype *);
void create_local_object (locobjtype, objecttype, errinfotype *);
void destroy_local_object (locobjtype, errinfotype *);
void destroy_object_list (listtype);
booltype any_var_initialisation (objecttype, objecttype);
void destr_struct (objecttype, memsizetype);
void destr_array (objecttype, memsizetype);
booltype crea_array (objecttype, objecttype, memsizetype);
void cpy_array (objecttype, objecttype, memsizetype);

#else

objecttype get_create_call_obj ();
objecttype get_destroy_call_obj ();
void do_create ();
void do_destroy ();
inttype do_ord ();
booltype do_in ();
objecttype param1_call ();
objecttype param2_call ();
objecttype param3_call ();
objecttype create_return_object ();
void create_local_object ();
void destroy_local_object ();
void destroy_object_list ();
booltype any_var_initialisation ();
void destr_struct ();
void destr_array ();
booltype crea_array ();
void cpy_array ();

#endif

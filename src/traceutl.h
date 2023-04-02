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
/*  Module: General                                                 */
/*  File: seed7/src/traceutl.h                                      */
/*  Changes: 1990, 1991, 1992, 1993, 1994  Thomas Mertes            */
/*  Content: Tracing and protocol procedures.                       */
/*                                                                  */
/********************************************************************/

typedef struct traceStruct {
    boolType actions;
    boolType check_actions;
    boolType dynamic;
    boolType exceptions;
    boolType heapsize;
    boolType match;
    boolType executil;
    boolType signals;
  } traceRecord;

#ifdef DO_INIT
traceRecord trace = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
#else
EXTERN traceRecord trace;
#endif

#define prot_ustri(stri) prot_cstri((const_cstriType) stri)
#define prot_uchar(ch) prot_cchar((char) ch)


void prot_flush (void);
void prot_nl (void);
void prot_cstri (const_cstriType cstri);
void prot_cstri8 (const const_cstriType cstri8);
void prot_cchar (char ch);
void prot_writeln (const_cstriType stri);
void prot_int (intType ivalue);
void prot_bigint (const const_bigIntType bintvalue);
#if WITH_FLOAT
void prot_float (floatType floatValue);
#endif
void prot_char (charType cvalue);
void prot_string (const striType stri);
void prot_stri_unquoted (const const_striType stri);
void prot_stri (const const_striType stri);
void prot_bstri (bstriType bstri);
void prot_set (const_setType setValue);
void prot_heapsize (void);
void printcategory (objectCategory category);
void printtype (const_typeType anytype);
void printvalue (const_objectType anyobject);
void printobject (const_objectType anyobject);
void printLocObj (const_locObjType locObj);
void prot_list (const_listType list);
void prot_list_limited (const_listType list, int depthLimit);
void prot_dot_expr (const_listType list);
void prot_params (const_listType list);
void prot_name (const_listType list);
void trace_node (const_nodeType anynode);
void trace_nodes (void);
void printnodes (const_nodeType anynode);
void trace1 (const_objectType traceobject);
void trace_entity (const_entityType anyentity);
void trace_list (const_listType list);
void set_protfile_name (const const_striType protfile_name);
void set_trace (uintType options);
void mapTraceFlags (const_striType trace_level, uintType *options);
void mapTraceFlags2 (const_cstriType ctrace_level, uintType *options);

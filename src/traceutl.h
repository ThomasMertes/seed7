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

typedef struct tracestruct {
    booltype actions;
    booltype check_actions;
    booltype dynamic;
    booltype exceptions;
    booltype heapsize;
    booltype match;
    booltype executil;
  } tracerecord;

#ifdef DO_INIT
tracerecord trace = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
#else
EXTERN tracerecord trace;
#endif


#ifdef ANSI_C

void prot_flush (void);
void prot_nl (void);
void prot_cstri (const_cstritype);
void prot_writeln (const_cstritype);
void prot_int (inttype);
void prot_bigint (const_biginttype);
void prot_stri (const_stritype);
void prot_set (const_settype);
void prot_heapsize (void);
void printcategory (objectcategory);
void printtype (const_typetype);
void printvalue (const_objecttype);
void printobject (const_objecttype);
void prot_list (const_listtype);
void prot_name (const_listtype list);
void trace_node (const_nodetype);
void trace_nodes (void);
void printnodes (const_nodetype);
void trace1 (const_objecttype);
void trace_entity (const_entitytype);
void trace_list (const_listtype);
void traceelements (nodetype);
void trace2 (objecttype);
void set_trace (const_cstritype, int, cstritype);
void set_trace2 (stritype);

#else

void prot_flush ();
void prot_nl ();
void prot_cstri ();
void prot_writeln ();
void prot_int ();
void prot_bigint ();
void prot_stri ();
void prot_set ();
void prot_heapsize ();
void printcategory ();
void printtype ();
void printvalue ();
void printobject ();
void prot_list ();
void prot_name ();
void trace_node ();
void trace_nodes ();
void printnodes ();
void trace1 ();
void trace_entity ();
void trace_list ();
void traceelements ();
void trace2 ();
void set_trace ();
void set_trace2 ();

#endif

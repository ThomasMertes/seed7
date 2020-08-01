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
void prot_cstri (cstritype);
void prot_writeln (cstritype);
void prot_int (inttype);
void prot_bigint (biginttype);
void prot_stri (stritype);
void prot_set (settype);
void prot_heapsize (void);
cstritype category_stri (objectcategory);
objectcategory category_value (cstritype);
void printcategory (objectcategory);
void printtype (typetype);
void printvalue (objecttype);
void printobject (objecttype);
void prot_list (listtype);
void trace_node (nodetype);
void trace_nodes (void);
void printnodes (nodetype);
void trace1 (objecttype);
void trace_entity (entitytype);
void trace_list (listtype);
void traceelements (nodetype);
void trace2 (objecttype);
void set_trace (cstritype, int, cstritype);
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
cstritype category_stri ();
objectcategory category_value ();
void printcategory ();
void printtype ();
void printvalue ();
void printobject ();
void prot_list ();
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

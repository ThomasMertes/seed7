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
/*  File: seed7/src/actutl.h                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: Conversion of strings to ACTIONs and back.             */
/*                                                                  */
/********************************************************************/

typedef struct {
    cstritype name;
    acttype action;
  } primactrecord;

typedef primactrecord *primacttype;

typedef struct {
    int size;
    primacttype primitive;
  } acttabletype;

#ifdef DO_INIT
acttabletype act_table = {0, NULL};
#else
EXTERN acttabletype act_table;
#endif


#ifdef ANSI_C

booltype find_action (stritype, acttype *);
primacttype get_primact (acttype);
#ifdef WITH_ACTION_CHECK
booltype act_okay (acttype);
#endif

#else

booltype find_action ();
primacttype get_primact ();
#ifdef WITH_ACTION_CHECK
booltype act_okay ();
#endif

#endif

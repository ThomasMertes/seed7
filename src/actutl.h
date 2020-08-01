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
/*  File: seed7/src/actutl.h                                        */
/*  Changes: 1992, 1993, 1994  Thomas Mertes                        */
/*  Content: Conversion of strings to ACTIONs and back.             */
/*                                                                  */
/********************************************************************/

typedef struct {
    const_cstriType name;
    actType action;
  } primActRecord;

typedef primActRecord *primActType;
typedef const primActRecord *const_primActType;

typedef struct {
    unsigned int size;
    const_primActType primitive;
  } actTableType;

#ifdef DO_INIT
actTableType act_table = {0, NULL};
#else
EXTERN actTableType act_table;
#endif


boolType find_action (const const_striType action_name, actType *action_found);
const_primActType get_primact (actType action_searched);

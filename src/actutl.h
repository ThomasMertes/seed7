/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2016  Thomas Mertes                        */
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
/*  Changes: 1992, 1993, 1994, 2016  Thomas Mertes                  */
/*  Content: Conversion of strings to ACTIONs and back.             */
/*                                                                  */
/********************************************************************/

typedef struct {
    const_cstriType name;
    actType action;
  } actEntryRecord;

typedef actEntryRecord *actEntryType;
typedef const actEntryRecord *const_actEntryType;

typedef struct {
    unsigned int size;
    const_actEntryType table;
  } actTableType;

#ifdef DO_INIT
actTableType actTable = {0, NULL};
#else
EXTERN actTableType actTable;
#endif


actType findAction (const const_striType actionName);
actType getActIllegal (void);
void freeActPtrTable (void);
const_actEntryType getActEntry (actType actionSearched);

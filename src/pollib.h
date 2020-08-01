/********************************************************************/
/*                                                                  */
/*  hi   Interpreter for Seed7 programs.                            */
/*  Copyright (C) 1990 - 2011  Thomas Mertes                        */
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
/*  Module: Library                                                 */
/*  File: seed7/src/pollib.h                                        */
/*  Changes: 2011  Thomas Mertes                                    */
/*  Content: All primitive actions to support poll.                 */
/*                                                                  */
/********************************************************************/

#ifdef ANSI_C

objecttype pol_addReadCheck     (listtype arguments);
objecttype pol_addWriteCheck    (listtype arguments);
objecttype pol_clear            (listtype arguments);
objecttype pol_cpy              (listtype arguments);
objecttype pol_create           (listtype arguments);
objecttype pol_destr            (listtype arguments);
objecttype pol_empty            (listtype arguments);
objecttype pol_files            (listtype arguments);
objecttype pol_hasNextReadFile  (listtype arguments);
objecttype pol_hasNextWriteFile (listtype arguments);
objecttype pol_nextReadFile     (listtype arguments);
objecttype pol_nextWriteFile    (listtype arguments);
objecttype pol_poll             (listtype arguments);
objecttype pol_readyForRead     (listtype arguments);
objecttype pol_readyForWrite    (listtype arguments);
objecttype pol_removeReadCheck  (listtype arguments);
objecttype pol_removeWriteCheck (listtype arguments);
objecttype pol_value            (listtype arguments);

#else

objecttype pol_addReadCheck ();
objecttype pol_addWriteCheck ();
objecttype pol_clear ();
objecttype pol_cpy ();
objecttype pol_create ();
objecttype pol_destr ();
objecttype pol_empty ();
objecttype pol_files ();
objecttype pol_hasNextReadFile ();
objecttype pol_hasNextWriteFile ();
objecttype pol_nextReadFile ();
objecttype pol_nextWriteFile ();
objecttype pol_poll ();
objecttype pol_readyForRead ();
objecttype pol_readyForWrite ();
objecttype pol_removeReadCheck ();
objecttype pol_removeWriteCheck ();
objecttype pol_value ();

#endif

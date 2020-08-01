/********************************************************************/
/*                                                                  */
/*  vol_drv.h     Get list of available drive letter volumes.       */
/*  Copyright (C) 1989 - 2018  Thomas Mertes                        */
/*                                                                  */
/*  This file is part of the Seed7 Runtime Library.                 */
/*                                                                  */
/*  The Seed7 Runtime Library is free software; you can             */
/*  redistribute it and/or modify it under the terms of the GNU     */
/*  Lesser General Public License as published by the Free Software */
/*  Foundation; either version 2.1 of the License, or (at your      */
/*  option) any later version.                                      */
/*                                                                  */
/*  The Seed7 Runtime Library is distributed in the hope that it    */
/*  will be useful, but WITHOUT ANY WARRANTY; without even the      */
/*  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR */
/*  PURPOSE.  See the GNU Lesser General Public License for more    */
/*  details.                                                        */
/*                                                                  */
/*  You should have received a copy of the GNU Lesser General       */
/*  Public License along with this program; if not, write to the    */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/vol_drv.h                                       */
/*  Changes: 2018  Thomas Mertes                                    */
/*  Content: Get list of available drive letter volumes.            */
/*                                                                  */
/********************************************************************/


typedef struct {
    /* A volumeListType always has a magic value of UINT32TYPE_MAX. */
    uint32Type magicValue;
    uint32Type driveBitmask;
    int currentDrive;
  } volumeListType;

#define IS_VOLUME_LIST(ptr) (ptr != NULL && ((volumeListType *) (ptr))->magicValue == UINT32TYPE_MAX)


volumeListType *openVolumeList (void);

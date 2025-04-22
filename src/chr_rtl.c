/********************************************************************/
/*                                                                  */
/*  chr_rtl.c     Primitive actions for the char type.              */
/*  Copyright (C) 1989 - 2016  Thomas Mertes                        */
/*                2015 Arkadiy Kuleshov                             */
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
/*  File: seed7/src/chr_rtl.c                                       */
/*  Changes: 1992, 1993, 1994, 2005, 2010  Thomas Mertes            */
/*  Content: Primitive actions for the char type.                   */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "int_rtl.h"
#include "str_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "chr_rtl.h"


static const uint64Type unicode_letters_data[] = {
  /* 0x000000-0x0001ff | number of bits: 373 */
  0x0000000000000000, 0x07fffffe07fffffe, 0x0420040000000000, 0xff7fffffff7fffff,
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  /* 0x000200-0x0003ff | number of bits: 342 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0x0000501f0003ffc3,
  0x0000000000000000, 0xbcdf000000000000, 0xfffffffbffffd740, 0xffbfffffffffffff,
  /* 0x000400-0x0005ff | number of bits: 407 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xfffffffffffffc03, 0xffffffffffffffff,
  0xfffeffffffffffff, 0xffffffff027fffff, 0x00000000000001ff, 0x000787ffffff0000,
  /* 0x000600-0x0007ff | number of bits: 310 */
  0xffffffff00000000, 0xfffec000000007ff, 0xffffffffffffffff, 0x9c00c060002fffff,
  0x0000fffffffd0000, 0xffffffffffffe000, 0x0002003fffffffff, 0x043007fffffffc00,
  /* 0x000800-0x0009ff | number of bits: 269 */
  0x00000110043fffff, 0xffff07ff01ffffff, 0xffffffff00007eff, 0x00000000000003ff,
  0x23fffffffffffff0, 0xfffe0003ff010000, 0x23c5fdfffff99fe1, 0x10030003b0004000,
  /* 0x000a00-0x000bff | number of bits: 194 */
  0x036dfdfffff987e0, 0x001c00005e000000, 0x23edfdfffffbbfe0, 0x0200000300010000,
  0x23edfdfffff99fe0, 0x00020003b0000000, 0x03ffc718d63dc7e8, 0x0000000000010000,
  /* 0x000c00-0x000dff | number of bits: 240 */
  0x23fffdfffffddfe0, 0x0000000327000000, 0x23effdfffffddfe1, 0x0006000360000000,
  0x27fffffffffddff0, 0xfc00000380704000, 0x2ffbfffffc7fffe0, 0x000000000000007f,
  /* 0x000e00-0x000fff | number of bits: 163 */
  0x000dfffffffffffe, 0x000000000000007f, 0x200dffaffffff7d6, 0x00000000f000005f,
  0x0000000000000001, 0x00001ffffffffeff, 0x0000000000001f00, 0x0000000000000000,
  /* 0x001000-0x0011ff | number of bits: 417 */
  0x800007ffffffffff, 0xffe1c0623c3f0000, 0xffffffff00004003, 0xf7ffffffffff20bf,
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  /* 0x001200-0x0013ff | number of bits: 434 */
  0xffffffffffffffff, 0xffffffff3d7f3dff, 0x7f3dffffffff3dff, 0xffffffffff7fff3d,
  0xffffffffff3dffff, 0x0000000007ffffff, 0xffffffff0000ffff, 0x3f3fffffffffffff,
  /* 0x001400-0x0015ff | number of bits: 511 */
  0xfffffffffffffffe, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  /* 0x001600-0x0017ff | number of bits: 360 */
  0xffffffffffffffff, 0xffff9fffffffffff, 0xffffffff07fffffe, 0x01fe07ffffffffff,
  0x0003ffff8003ffff, 0x0001dfff0003ffff, 0x000fffffffffffff, 0x0000000010800000,
  /* 0x001800-0x0019ff | number of bits: 335 */
  0xffffffff00000000, 0x01ffffffffffffff, 0xffff05ffffffff9f, 0x003fffffffffffff,
  0x000000007fffffff, 0x001f3fffffff0000, 0xffff0fffffffffff, 0x00000000000003ff,
  /* 0x001a00-0x001bff | number of bits: 208 */
  0xffffffff007fffff, 0x00000000001fffff, 0x0000008000000000, 0x0000000000000000,
  0x000fffffffffffe0, 0x0000000000001fe0, 0xfc00c001fffffff8, 0x0000003fffffffff,
  /* 0x001c00-0x001dff | number of bits: 335 */
  0x0000000fffffffff, 0x3ffffffffc00e000, 0xe7ffffffffff01ff, 0x046fde0000000000,
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0x0000000000000000,
  /* 0x001e00-0x001fff | number of bits: 474 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  0xffffffff3f3fffff, 0x3fffffffaaff3f3f, 0x5fdfffffffffffff, 0x1fdc1fff0fcf1fdc,
  /* 0x002000-0x0021ff | number of bits: 63 */
  0x0000000000000000, 0x8002000000000000, 0x000000001fff0000, 0x0000000000000000,
  0xf3ffbd503e2ffc84, 0x00000000000043e0, 0x0000000000000018, 0x0000000000000000,
  /* 0x002c00-0x002dff | number of bits: 411 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0x000c781fffffffff,
  0xffff20bfffffffff, 0x000080ffffffffff, 0x7f7f7f7f007fffff, 0x000000007f7f7f7f,
  /* 0x002e00-0x002fff | number of bits: 1 */
  0x0000800000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x003000-0x0031ff | number of bits: 377 */
  0x183e000000000060, 0xfffffffffffffffe, 0xfffffffee07fffff, 0xf7ffffffffffffff,
  0xfffeffffffffffe0, 0xffffffffffffffff, 0xffffffff00007fff, 0xffff000000000000,
  /* 0x004c00-0x004dff | number of bits: 448 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0x0000000000000000,
  /* 0x009e00-0x009fff | number of bits: 509 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0x1fffffffffffffff,
  /* 0x00a400-0x00a5ff | number of bits: 443 */
  0xffffffffffffffff, 0xffffffffffffffff, 0x0000000000001fff, 0x3fffffffffff0000,
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  /* 0x00a600-0x00a7ff | number of bits: 377 */
  0x00000c00ffff1fff, 0x80007fffffffffff, 0xffffffff3fffffff, 0x0000003fffffffff,
  0xfffffffcff800000, 0xffffffffffffffff, 0xfffffffffffff9ff, 0xfffc000003eb07ff,
  /* 0x00a800-0x00a9ff | number of bits: 291 */
  0x00000007fffff7bb, 0x000fffffffffffff, 0x000ffffffffffffc, 0x68fc000000000000,
  0xffff003ffffffc00, 0x1fffffff0000007f, 0x0007fffffffffff0, 0x7c00ffdf00008000,
  /* 0x00aa00-0x00abff | number of bits: 357 */
  0x000001ffffffffff, 0xc47fffff00000ff7, 0x3e62ffffffffffff, 0x001c07ff38000005,
  0xffff7f7f007e7e7e, 0xffff03fff7ffffff, 0xffffffffffffffff, 0x00000007ffffffff,
  /* 0x00d600-0x00d7ff | number of bits: 492 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  0xffffffffffffffff, 0xffffffffffffffff, 0xffff000fffffffff, 0x0ffffffffffff87f,
  /* 0x00f800-0x00f9ff | number of bits: 256 */
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  /* 0x00fa00-0x00fbff | number of bits: 415 */
  0xffffffffffffffff, 0xffff3fffffffffff, 0xffffffffffffffff, 0x0000000003ffffff,
  0x5f7ffdffa0f8007f, 0xffffffffffffffdb, 0x0003ffffffffffff, 0xfffffffffff80000,
  /* 0x00fc00-0x00fdff | number of bits: 448 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  0x3fffffffffffffff, 0xffffffffffff0000, 0xfffffffffffcffff, 0x0fff0000000000ff,
  /* 0x00fe00-0x00ffff | number of bits: 302 */
  0x0000000000000000, 0xffdf000000000000, 0xffffffffffffffff, 0x1fffffffffffffff,
  0x07fffffe00000000, 0xffffffc007fffffe, 0x7fffffffffffffff, 0x000000001cfcfcfc,
  /* 0x010000-0x0101ff | number of bits: 211 */
  0xb7ffff7fffffefff, 0x000000003fff3fff, 0xffffffffffffffff, 0x07ffffffffffffff,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x010200-0x0103ff | number of bits: 250 */
  0x0000000000000000, 0x0000000000000000, 0xffffffff1fffffff, 0x000000000001ffff,
  0xffffe000ffffffff, 0x003fffffffff03fd, 0xffffffff3fffffff, 0x000000000000ff0f,
  /* 0x010400-0x0105ff | number of bits: 392 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffff00003fffffff, 0x0fffffffff0fffff,
  0xffff00ffffffffff, 0xf7ff000fffffffff, 0x1bfbfffbffb7f7ff, 0x0000000000000000,
  /* 0x010600-0x0107ff | number of bits: 398 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  0x007fffffffffffff, 0x000000ff003fffff, 0x07fdffffffffffbf, 0x0000000000000000,
  /* 0x010800-0x0109ff | number of bits: 258 */
  0x91bffffffffffd3f, 0x007fffff003fffff, 0x000000007fffffff, 0x0037ffff00000000,
  0x03ffffff003fffff, 0x0000000000000000, 0xc0ffffffffffffff, 0x0000000000000000,
  /* 0x010a00-0x010bff | number of bits: 244 */
  0x003ffffffeef0001, 0x1fffffff00000000, 0x000000001fffffff, 0x0000001ffffffeff,
  0x003fffffffffffff, 0x0007ffff003fffff, 0x000000000003ffff, 0x0000000000000000,
  /* 0x010c00-0x010dff | number of bits: 211 */
  0xffffffffffffffff, 0x00000000000001ff, 0x0007ffffffffffff, 0x0007ffffffffffff,
  0x0000000fffffffff, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x010e00-0x010fff | number of bits: 158 */
  0x0000000000000000, 0x0000000000000000, 0x000303ffffffffff, 0x0000000000000000,
  0xffff00801fffffff, 0xffff00000000003f, 0xffff000000000003, 0x007fffff0000001f,
  /* 0x011000-0x0111ff | number of bits: 254 */
  0x00fffffffffffff8, 0x0026000000000000, 0x0000fffffffffff8, 0x000001ffffff0000,
  0x0000007ffffffff8, 0x0047ffffffff0090, 0x0007fffffffffff8, 0x000000001400001e,
  /* 0x011200-0x0113ff | number of bits: 182 */
  0x80000ffffffbffff, 0x0000000000000001, 0xffff01ffbfffbd7f, 0x000000007fffffff,
  0x23edfdfffff99fe0, 0x00000003e0010000, 0x0000000000000000, 0x0000000000000000,
  /* 0x011400-0x0115ff | number of bits: 162 */
  0x001fffffffffffff, 0x0000000380000780, 0x0000ffffffffffff, 0x00000000000000b0,
  0x0000000000000000, 0x0000000000000000, 0x00007fffffffffff, 0x000000000f000000,
  /* 0x011600-0x0117ff | number of bits: 127 */
  0x0000ffffffffffff, 0x0000000000000010, 0x010007ffffffffff, 0x0000000000000000,
  0x0000000007ffffff, 0x000000000000007f, 0x0000000000000000, 0x0000000000000000,
  /* 0x011800-0x0119ff | number of bits: 202 */
  0x00000fffffffffff, 0x0000000000000000, 0xffffffff00000000, 0x80000000ffffffff,
  0x8000ffffff6ff27f, 0x0000000000000002, 0xfffffcff00000000, 0x0000000a0001ffff,
  /* 0x011a00-0x011bff | number of bits: 163 */
  0x0407fffffffff801, 0xfffffffff0010000, 0xffff0000200003ff, 0x01ffffffffffffff,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x011c00-0x011dff | number of bits: 166 */
  0x00007ffffffffdff, 0xfffc000000000001, 0x000000000000ffff, 0x0000000000000000,
  0x0001fffffffffb7f, 0xfffffdbf00000040, 0x00000000010003ff, 0x0000000000000000,
  /* 0x011e00-0x011fff | number of bits: 68 */
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0007ffff00000000,
  0x000ffffffffdfff4, 0x0000000000000000, 0x0001000000000000, 0x0000000000000000,
  /* 0x012200-0x0123ff | number of bits: 410 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  0xffffffffffffffff, 0xffffffffffffffff, 0x0000000003ffffff, 0x0000000000000000,
  /* 0x012400-0x0125ff | number of bits: 196 */
  0x0000000000000000, 0x0000000000000000, 0xffffffffffffffff, 0xffffffffffffffff,
  0xffffffffffffffff, 0x000000000000000f, 0x0000000000000000, 0x0000000000000000,
  /* 0x012e00-0x012fff | number of bits: 97 */
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  0x0000000000000000, 0x0000000000000000, 0xffffffffffff0000, 0x0001ffffffffffff,
  /* 0x013400-0x0135ff | number of bits: 54 */
  0x0000ffffffffffff, 0x000000000000007e, 0x0000000000000000, 0x0000000000000000,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x014600-0x0147ff | number of bits: 71 */
  0xffffffffffffffff, 0x000000000000007f, 0x0000000000000000, 0x0000000000000000,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x016a00-0x016bff | number of bits: 289 */
  0x01ffffffffffffff, 0xffff00007fffffff, 0x7fffffffffffffff, 0x00003fffffff0000,
  0x0000ffffffffffff, 0xe0fffff80000000f, 0x000000000000ffff, 0x0000000000000000,
  /* 0x016e00-0x016fff | number of bits: 156 */
  0x0000000000000000, 0xffffffffffffffff, 0x0000000000000000, 0x0000000000000000,
  0xffffffffffffffff, 0x00000000000107ff, 0x00000000fff80000, 0x0000000b00000000,
  /* 0x018600-0x0187ff | number of bits: 504 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0x00ffffffffffffff,
  /* 0x018c00-0x018dff | number of bits: 223 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0x00000000003fffff,
  0x00000000000001ff, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x01ae00-0x01afff | number of bits: 13 */
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x6fef000000000000,
  /* 0x01b000-0x01b1ff | number of bits: 444 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  0x00040007ffffffff, 0xffff00f000270000, 0xffffffffffffffff, 0xffffffffffffffff,
  /* 0x01b200-0x01b3ff | number of bits: 252 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0x0fffffffffffffff,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x01bc00-0x01bdff | number of bits: 139 */
  0xffffffffffffffff, 0x1fff07ffffffffff, 0x0000000003ff01ff, 0x0000000000000000,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x01d400-0x01d5ff | number of bits: 488 */
  0xffffffffffffffff, 0xffffffffffdfffff, 0xebffde64dfffffff, 0xffffffffffffffef,
  0x7bffffffdfdfe7bf, 0xfffffffffffdfc5f, 0xffffffffffffffff, 0xffffffffffffffff,
  /* 0x01d600-0x01d7ff | number of bits: 448 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffff3fffffffff, 0xf7fffffff7fffffd,
  0xffdfffffffdfffff, 0xffff7fffffff7fff, 0xfffffdfffffffdff, 0x0000000000000ff7,
  /* 0x01de00-0x01dfff | number of bits: 37 */
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  0x000007e07fffffff, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x01e000-0x01e1ff | number of bits: 115 */
  0xffff000000000000, 0x00003fffffffffff, 0x0000000000000000, 0x0000000000000000,
  0x3f801fffffffffff, 0x0000000000004000, 0x0000000000000000, 0x0000000000000000,
  /* 0x01e200-0x01e3ff | number of bits: 74 */
  0x0000000000000000, 0x0000000000000000, 0x00003fffffff0000, 0x00000fffffffffff,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x01e400-0x01e5ff | number of bits: 28 */
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x00000fffffff0000,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x01e600-0x01e7ff | number of bits: 28 */
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x7fff6f7f00000000,
  /* 0x01e800-0x01e9ff | number of bits: 266 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0x000000000000001f,
  0xffffffffffffffff, 0x000000000000080f, 0x0000000000000000, 0x0000000000000000,
  /* 0x01ee00-0x01efff | number of bits: 141 */
  0x0af7fe96ffffffef, 0x5ef7f796aa96ea84, 0x0ffffbee0ffffbff, 0x0000000000000000,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x02a600-0x02a7ff | number of bits: 480 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0x00000000ffffffff,
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  /* 0x02b600-0x02b7ff | number of bits: 506 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  0x03ffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  /* 0x02b800-0x02b9ff | number of bits: 510 */
  0xffffffff3fffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  /* 0x02ce00-0x02cfff | number of bits: 498 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffff0003ffffffff, 0xffffffffffffffff,
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  /* 0x02ea00-0x02ebff | number of bits: 481 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0x00000001ffffffff,
  /* 0x02fa00-0x02fbff | number of bits: 30 */
  0x000000003fffffff, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x031200-0x0313ff | number of bits: 507 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  0xffffffffffffffff, 0xffffffffffff07ff, 0xffffffffffffffff, 0xffffffffffffffff,
  /* 0x032200-0x0323ff | number of bits: 432 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  0xffffffffffffffff, 0xffffffffffffffff, 0x0000ffffffffffff, 0x0000000000000000,
};

static const signed char unicode_letters_ind[] = {
    0,   1,   2,   3,   4,   5,   6,   7, /* 0x000000-0x000fff */
    8,   9,  10,  11,  12,  13,  14,  15, /* 0x001000-0x001fff */
   16,  -1,  -1,  -1,  -1,  -1,  17,  18, /* 0x002000-0x002fff */
   19,  -1,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x003000-0x003fff */
   -2,  -2,  -2,  -2,  -2,  -2,  20,  -2, /* 0x004000-0x004fff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x005000-0x005fff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x006000-0x006fff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x007000-0x007fff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x008000-0x008fff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  21, /* 0x009000-0x009fff */
   -2,  -2,  22,  23,  24,  25,  -2,  -2, /* 0x00a000-0x00afff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x00b000-0x00bfff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x00c000-0x00cfff */
   -2,  -2,  -2,  26,  -1,  -1,  -1,  -1, /* 0x00d000-0x00dfff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x00e000-0x00efff */
   -1,  -1,  -1,  -1,  27,  28,  29,  30, /* 0x00f000-0x00ffff */
   31,  32,  33,  34,  35,  36,  37,  38, /* 0x010000-0x010fff */
   39,  40,  41,  42,  43,  44,  45,  46, /* 0x011000-0x011fff */
   -2,  47,  48,  -1,  -1,  -1,  -1,  49, /* 0x012000-0x012fff */
   -2,  -2,  50,  -1,  -1,  -1,  -1,  -1, /* 0x013000-0x013fff */
   -1,  -1,  -2,  51,  -1,  -1,  -1,  -1, /* 0x014000-0x014fff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x015000-0x015fff */
   -1,  -1,  -1,  -1,  -2,  52,  -1,  53, /* 0x016000-0x016fff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x017000-0x017fff */
   -2,  -2,  -2,  54,  -2,  -2,  55,  -1, /* 0x018000-0x018fff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x019000-0x019fff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  56, /* 0x01a000-0x01afff */
   57,  58,  -1,  -1,  -1,  -1,  59,  -1, /* 0x01b000-0x01bfff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x01c000-0x01cfff */
   -1,  -1,  60,  61,  -1,  -1,  -1,  62, /* 0x01d000-0x01dfff */
   63,  64,  65,  66,  67,  -1,  -1,  68, /* 0x01e000-0x01efff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x01f000-0x01ffff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x020000-0x020fff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x021000-0x021fff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x022000-0x022fff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x023000-0x023fff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x024000-0x024fff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x025000-0x025fff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x026000-0x026fff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x027000-0x027fff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x028000-0x028fff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x029000-0x029fff */
   -2,  -2,  -2,  69,  -2,  -2,  -2,  -2, /* 0x02a000-0x02afff */
   -2,  -2,  -2,  70,  71,  -2,  -2,  -2, /* 0x02b000-0x02bfff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  72, /* 0x02c000-0x02cfff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x02d000-0x02dfff */
   -2,  -2,  -2,  -2,  -2,  73,  -1,  -1, /* 0x02e000-0x02efff */
   -1,  -1,  -1,  -1,  -2,  74,  -1,  -1, /* 0x02f000-0x02ffff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x030000-0x030fff */
   -2,  75,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x031000-0x031fff */
   -2,  76                                /* 0x032000-0x0323ff */
};

/**
 *  Non-spacing attribute table.
 *  See PropList.txt, or grep '^[^;]*;[^;]*;[^;]*;[^;]*;NSM;' UnicodeData.txt
 *  Control characters are also marked non-spacing here, because they are not
 *  printable.
 */
static const uint64Type nonspacing_table_data[] = {
  /* 0x000000-0x0001ff | number of bits: 65 */
  0x00000000ffffffff, 0x8000000000000000, 0x00000000ffffffff, 0x0000000000000000,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x000200-0x0003ff | number of bits: 82 */
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  0xffffffffffffffff, 0x0000000700007fff, 0x0000000000000000, 0x0000000000000000,
  /* 0x000400-0x0005ff | number of bits: 53 */
  0x0000000000000000, 0x0000000000000000, 0x0000000000000378, 0x0000000000000000,
  0x0000000000000000, 0x0000000000000000, 0xbbfffffbfffe0000, 0x0000000000000016,
  /* 0x000600-0x0007ff | number of bits: 72 */
  0x0000000000000000, 0x00010000003ff800, 0x0000000000000000, 0x00003d9fffc00000,
  0xffff000000020000, 0x00000000000007ff, 0x0001ffc000000000, 0x0000000000000000,
  /* 0x000800-0x0009ff | number of bits: 27 */
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  0x1000000000000006, 0x0000000c001e21fe, 0x1000000000000002, 0x0000000c0000201e,
  /* 0x000a00-0x000bff | number of bits: 33 */
  0x1000000000000004, 0x0003000000003986, 0x1000000000000006, 0x00000000000021be,
  0x9000000000000002, 0x000000000040200e, 0x0000000000000004, 0x0000000000002001,
  /* 0x000c00-0x000dff | number of bits: 25 */
  0xc000000000000000, 0x0000000000603dc1, 0x8000000000000000, 0x0000000000003040,
  0x0000000000000000, 0x000000000000200e, 0x0000000000000000, 0x00000000005c0400,
  /* 0x000e00-0x000fff | number of bits: 102 */
  0x07f2000000000000, 0x0000000000007f80, 0x1bf2000000000000, 0x0000000000003f00,
  0x02a0000003000000, 0x7ffe000000000000, 0x1ffffffffeff00df, 0x0000000000000040,
  /* 0x001000-0x0011ff | number of bits: 10 */
  0x02c5e00000000000, 0x0000000003000000, 0x0000000000000000, 0x0000000000000000,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x001600-0x0017ff | number of bits: 19 */
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  0x0000000000000000, 0x0000000000000000, 0x3f80000000000000, 0x00000000000ffe40,
  /* 0x001800-0x0019ff | number of bits: 1 */
  0x0000000000000000, 0x0000000000000000, 0x0000020000000000, 0x0000000000000000,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x002000-0x0021ff | number of bits: 20 */
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000fffff0000,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x003000-0x0031ff | number of bits: 8 */
  0x0000fc0000000000, 0x0000000000000000, 0x0000000006000000, 0x0000000000000000,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x00fa00-0x00fbff | number of bits: 1 */
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  0x0000000040000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x00fe00-0x00ffff | number of bits: 4 */
  0x0000000f00000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
};

static const signed char nonspacing_table_ind[] = {
   0,  1,  2,  3,  4,  5,  6,  7, /* 0x0000-0x0fff */
   8, -1, -1,  9, 10, -1, -1, -1, /* 0x1000-0x1fff */
  11, -1, -1, -1, -1, -1, -1, -1, /* 0x2000-0x2fff */
  12, -1, -1, -1, -1, -1, -1, -1, /* 0x3000-0x3fff */
  -1, -1, -1, -1, -1, -1, -1, -1, /* 0x4000-0x4fff */
  -1, -1, -1, -1, -1, -1, -1, -1, /* 0x5000-0x5fff */
  -1, -1, -1, -1, -1, -1, -1, -1, /* 0x6000-0x6fff */
  -1, -1, -1, -1, -1, -1, -1, -1, /* 0x7000-0x7fff */
  -1, -1, -1, -1, -1, -1, -1, -1, /* 0x8000-0x8fff */
  -1, -1, -1, -1, -1, -1, -1, -1, /* 0x9000-0x9fff */
  -1, -1, -1, -1, -1, -1, -1, -1, /* 0xa000-0xafff */
  -1, -1, -1, -1, -1, -1, -1, -1, /* 0xb000-0xbfff */
  -1, -1, -1, -1, -1, -1, -1, -1, /* 0xc000-0xcfff */
  -1, -1, -1, -1, -1, -1, -1, -1, /* 0xd000-0xdfff */
  -1, -1, -1, -1, -1, -1, -1, -1, /* 0xe000-0xefff */
  -1, -1, -1, -1, -1, 13, -1, 14  /* 0xf000-0xffff */
};

/**
 *  Sorted array of character indices marking transitions
 *  between East Asian single- or neutral width and double-width
 */
static const charType east_asian_width[] = {
    0x000000, 0x001101, 0x00115b, 0x001160, 0x001161, 0x00232a, 0x00232c, 0x002e81,
    0x002e9b, 0x002e9c, 0x002ef5, 0x002f01, 0x002fd7, 0x002ff1, 0x002ffd, 0x003001,
    0x003040, 0x003042, 0x003098, 0x00309a, 0x003101, 0x003106, 0x00312e, 0x003132,
    0x003190, 0x003191, 0x0031b9, 0x0031f1, 0x003220, 0x003221, 0x003245, 0x003251,
    0x00327f, 0x003280, 0x003300, 0x003301, 0x004db7, 0x004e01, 0x009fa7, 0x00a001,
    0x00a48e, 0x00a491, 0x00a4c8, 0x00ac01, 0x00d7a5, 0x00f901, 0x00fa2f, 0x00fa31,
    0x00fa6c, 0x00fe31, 0x00fe54, 0x00fe55, 0x00fe68, 0x00fe69, 0x00fe6d, 0x00ff02,
    0x00ff62, 0x00ffe1, 0x00ffe8, 0x020001, 0x02ffff, 0x030001, 0x03ffff
};



static inline boolType is_nonspacing (charType ch)

  {
    int ind;

  /* is_nonspacing */
    if (ch <= 0x00ffff) {
      ind = (int) nonspacing_table_ind[ch >> 9];
      if (ind >= 0) {
        return (boolType) (
            (nonspacing_table_data[8 * (unsigned int) ind + ((ch >> 6) & 7)] >> (ch & 63)) & 1);
      } /* if */
    } else if (ch >= 0x10ffff) {
      return TRUE;
    } /* if */
    return FALSE;
  } /* is_nonspacing */



/**
 * Do a binary search of the character index
 * and assume double width is true for each odd block
 * since first block has single or neutral width
 * and the block properties are alternating
 */
static inline boolType is_doublewidth (charType ch)

  {
    int min = 0;
    int mid;
    int max = sizeof(east_asian_width) / sizeof(charType) - 1;

  /* is_doublewidth */
    while (min <= max) {
      mid = min + (max - min) / 2;
      if (ch < east_asian_width[mid]) {
        max = mid - 1;
      } else if (ch > east_asian_width[mid]) {
        min = mid + 1;
      } else {
        /* printf("mid for 0x%06x is %d\n", ch, mid); */
        return (mid % 2 == 1) ? TRUE : FALSE;
      } /* if */
    } /* while */
    /* printf("min for 0x%06x is %d\n", ch, min - 1); */
    return ((min - 1) % 2 == 1) ? TRUE : FALSE;
  } /* is_doublewidth */



striType chrCLit (charType character)

  {
    /* A string literal starts and ends with apostrophe ('): */
    const memSizeType numOfApostrophes = 2;
    memSizeType len;
    striType result;

  /* chrCLit */
    logFunction(printf("chrCLit('\\" FMT_U32 ";')\n", character););
    if (character < 127) {
      if (character < ' ') {
        len = strlen(cstri_escape_sequence[character]);
        if (unlikely(!ALLOC_STRI_SIZE_OK(result, len + numOfApostrophes))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = len + numOfApostrophes;
          result->mem[0] = '\'';
          memcpy_to_strelem(&result->mem[1],
              (const_ustriType) cstri_escape_sequence[character], len);
          result->mem[len + 1] = '\'';
        } /* if */
      } else if (character == '\\' || character == '\'') {
        if (unlikely(!ALLOC_STRI_SIZE_OK(result, 4))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = 4;
          result->mem[0] = '\'';
          result->mem[1] = (strElemType) '\\';
          result->mem[2] = (strElemType) character;
          result->mem[3] = '\'';
        } /* if */
      } else {
        if (unlikely(!ALLOC_STRI_SIZE_OK(result, 3))) {
          raise_error(MEMORY_ERROR);
        } else {
          result->size = 3;
          result->mem[0] = '\'';
          result->mem[1] = (strElemType) character;
          result->mem[2] = '\'';
        } /* if */
      } /* if */
    } else {
      result = intStr((intType) character);
    } /* if */
    return result;
  } /* chrCLit */



#if ALLOW_STRITYPE_SLICES
striType chrCLitToBuffer (charType character, striType buffer)

  { /* chrCLitToBuffer */
    logFunction(printf("chrCLitToBuffer('\\" FMT_U32 ";')\n", character););
    if (character < 127) {
      buffer->mem = buffer->mem1;
      buffer->mem1[0] = (strElemType) '\'';
      if (character < ' ') {
        buffer->mem1[1] = (strElemType) '\\';
        if (cstri_escape_sequence[character][1] == '0') {
          /* Always write three octal digits as strCLit does. */
          buffer->mem1[2] = (strElemType) '0';
          /* Write the character as two octal digits. */
          /* This code is much faster than sprintf(). */
          buffer->mem1[3] = (strElemType) ((character >> 3 & 0x7) + '0');
          buffer->mem1[4] = (strElemType) ((character      & 0x7) + '0');
          buffer->mem1[5] = (strElemType) '\'';
          buffer->size = 6;
        } else {
          buffer->mem1[2] = (strElemType) cstri_escape_sequence[character][1];
          buffer->mem1[3] = (strElemType) '\'';
          buffer->size = 4;
        } /* if */
      } else if (character == '\\' || character == '\'') {
        buffer->mem1[1] = (strElemType) '\\';
        buffer->mem1[2] = (strElemType) character;
        buffer->mem1[3] = (strElemType) '\'';
        buffer->size = 4;
      } else {
        buffer->mem1[1] = (strElemType) character;
        buffer->mem1[2] = (strElemType) '\'';
        buffer->size = 3;
      } /* if */
    } else {
      (void) intStrToBuffer((intType) character, buffer);
    } /* if */
    return buffer;
  } /* chrCLitToBuffer */
#endif



/**
 *  Compare two characters.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType chrCmp (charType char1, charType char2)

  {
    intType signumValue;

  /* chrCmp */
    if (char1 < char2) {
      signumValue = -1;
    } else {
      signumValue = char1 > char2;
    } /* if */
    return signumValue;
  } /* chrCmp */



/**
 *  Reinterpret the generic parameters as charType and call chrCmp.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(charType).
 */
intType chrCmpGeneric (const genericType value1, const genericType value2)

  { /* chrCmpGeneric */
    return chrCmp(((const_rtlObjectType *) &value1)->value.charValue,
                  ((const_rtlObjectType *) &value2)->value.charValue);
  } /* chrCmpGeneric */



/**
 *  Reinterpret the generic parameters as charType and assign source to dest.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(charType).
 */
void chrCpyGeneric (genericType *const dest, const genericType source)

  { /* chrCpyGeneric */
    ((rtlObjectType *) dest)->value.charValue =
        ((const_rtlObjectType *) &source)->value.charValue;
  } /* chrCpyGeneric */



/**
 *  Determines if the specified character 'ch' is a letter.
 *  A character is considered to be a letter if the category
 *  from the Unicode specification is any of the following:
 *  * UPPERCASE_LETTER (Lu)
 *  * LOWERCASE_LETTER (Ll)
 *  * TITLECASE_LETTER (Lt)
 *  * MODIFIER_LETTER  (Lm)
 *  * OTHER_LETTER     (Lo)
 *  @return TRUE if 'ch' is a letter,
 *          FALSE otherwise.
 */
boolType chrIsLetter (charType ch)

  {
    int ind;

  /* chrIsLetter */
    if (ch <= 0x0323ff) {
      ind = (int) unicode_letters_ind[ch >> 9];
      if (ind >= 0) {
        return (boolType) (
            (unicode_letters_data[8 * (unsigned int) ind + ((ch >> 6) & 7)] >> (ch & 63)) & 1);
      } else {
        return ~ind; /* -1 -> FALSE, -2 -> TRUE */
      } /* if */
    } /* if */
    return FALSE;
  } /* chrIsLetter */



/**
 *  Convert a character to lower case.
 *  The conversion uses the default Unicode case mapping,
 *  where each character is considered in isolation.
 *  Characters without case mapping are left unchanged.
 *  The mapping is independent from the locale. Individual
 *  character case mappings cannot be reversed, because some
 *  characters have multiple characters that map to them.
 *  @return the character converted to lower case.
 */
charType chrLow (charType ch)

  { /* chrLow */
    toLower(&ch, 1, &ch);
    return ch;
  } /* chrLow */



/**
 *  Create a string with one character.
 *  @return a string with the character 'ch'.
 */
striType chrStr (charType ch)

  {
    striType result;

  /* chrStr */
    if (unlikely(!ALLOC_STRI_SIZE_OK(result, (memSizeType) 1))) {
      raise_error(MEMORY_ERROR);
      return NULL;
    } else {
      result->size = 1;
      result->mem[0] = (strElemType) ch;
      return result;
    } /* if */
  } /* chrStr */



/**
 *  Convert a character to upper case.
 *  The conversion uses the default Unicode case mapping,
 *  where each character is considered in isolation.
 *  Characters without case mapping are left unchanged.
 *  The mapping is independent from the locale. Individual
 *  character case mappings cannot be reversed, because some
 *  characters have multiple characters that map to them.
 *  @return the character converted to upper case.
 */
charType chrUp (charType ch)

  { /* chrUp */
    toUpper(&ch, 1, &ch);
    return ch;
  } /* chrUp */



/**
 *  Number of screen columns occupied by the Unicode character 'ch'.
 *  Non-spacing characters and control characters have width of 0.
 *  @return 0,1 or 2 depending on the width occupied on a terminal.
 */
intType chrWidth (charType ch)

  { /* chrWidth */
    if (is_nonspacing(ch)) {
      return 0;
    } else if (is_doublewidth(ch)) {
      return 2;
    } else {
      return 1;
    } /* if */
  } /* chrWidth */

/********************************************************************/
/*                                                                  */
/*  chr_rtl.c     Primitive actions for the char type.              */
/*  Copyright (C) 1989 - 2015  Thomas Mertes                        */
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
  /* 0x000200-0x0003ff | number of bits: 301 */
  0x007fffffffffffff, 0xffffffffffff0000, 0xffffffffffffffff, 0x0000401f0003ffc3,
  0x0000000000000000, 0x0400000000000020, 0xfffffffbffffd740, 0x0fbfffffffff7fff,
  /* 0x000400-0x0005ff | number of bits: 380 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xfffffffffffffc03, 0x033fffffffff7fff,
  0xfffe00000000ffff, 0xfffffffe027fffff, 0xbbff0000000000ff, 0x000707ffffff0016,
  /* 0x000600-0x0007ff | number of bits: 282 */
  0x07fffffe003f0000, 0xffffc00000ffffff, 0xffffffffffffffff, 0x9c00e1fe1fefffff,
  0xffffffffffff0000, 0x000000000000e000, 0x0003ffffffffffff, 0x0000000000000000,
  /* 0x000800-0x0009ff | number of bits: 155 */
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  0xe3fffffffffffffe, 0x0000000fff011fff, 0xe3c5fdfffff99fee, 0x0003000fb080199f,
  /* 0x000a00-0x000bff | number of bits: 251 */
  0xc36dfdfffff987ee, 0x001f00005e001987, 0xe3edfdfffffbbfee, 0x0000000f00011bbf,
  0xe3edfdfffff99fee, 0x00020003b0c0198f, 0xc3bfc718d63dc7ec, 0x0000000000801dc7,
  /* 0x000c00-0x000dff | number of bits: 284 */
  0xc3effdfffffddfee, 0x0000000300601ddf, 0xe3effdfffffddfec, 0x0000000340601ddf,
  0xc3fffdfffffddfec, 0x0000000300801dcf, 0x2ffbfffffc7fffec, 0x000c0000ff5f807f,
  /* 0x000e00-0x000fff | number of bits: 224 */
  0x07fffffffffffffe, 0x000000000000207f, 0x3bffecaefef02596, 0x000000003000205f,
  0x0000000000000001, 0xfffe07fffffffeff, 0x1ffffffffeff0f03, 0x0000000000000000,
  /* 0x001000-0x0011ff | number of bits: 379 */
  0x0147f6fbffffffff, 0x0000000003ff0000, 0xffffffff00000000, 0x01ffffffffff003f,
  0xffffffffffffffff, 0xffffffff83ffffff, 0xffffff07ffffffff, 0x03ffffffffffffff,
  /* 0x001200-0x0013ff | number of bits: 402 */
  0xffffffffffffff7f, 0xffffffff3d7f3d7f, 0x7f3d7fffffff3d7f, 0xffff7fffff7f7f3d,
  0xffffffff7f3d7fff, 0x0000000007ffff7f, 0xffffffff00000000, 0x001fffffffffffff,
  /* 0x001400-0x0015ff | number of bits: 511 */
  0xfffffffffffffffe, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  /* 0x001600-0x0017ff | number of bits: 371 */
  0xffffffffffffffff, 0x007f9fffffffffff, 0xffffffff07fffffe, 0x0001c7ffffffffff,
  0x000fffff000fdfff, 0x000ddfff000fffff, 0xffcfffffffffffff, 0x00000000108001ff,
  /* 0x001800-0x0019ff | number of bits: 215 */
  0xffffffff00000000, 0x00ffffffffffffff, 0x000003ffffffffff, 0x0000000000000000,
  0x01ff0fff1fffffff, 0x001f3fffffff0000, 0x0000000000000000, 0x0000000000000000,
  /* 0x001c00-0x001dff | number of bits: 108 */
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  0xffffffffffffffff, 0x00000fffffffffff, 0x0000000000000000, 0x0000000000000000,
  /* 0x001e00-0x001fff | number of bits: 464 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffff0fffffff, 0x03ffffffffffffff,
  0xffffffff3f3fffff, 0x3fffffffaaff3f3f, 0x5fdfffffffffffff, 0x1fdc1fff0fcf1fdc,
  /* 0x002000-0x0021ff | number of bits: 81 */
  0x0000000000000000, 0x8002000000000000, 0x0000000000000000, 0x0000000000000000,
  0xe3fbbd503e2ffc84, 0xffffffff000003e0, 0x000000000000000f, 0x0000000000000000,
  /* 0x003000-0x0031ff | number of bits: 379 */
  0x1f3e03fe000000e0, 0xfffffffffffffffe, 0xfffffffee07fffff, 0xf7ffffffffffffff,
  0xfffe1fffffffffe0, 0xffffffffffffffff, 0x00ffffff00007fff, 0xffff000000000000,
  /* 0x004c00-0x004dff | number of bits: 438 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  0xffffffffffffffff, 0xffffffffffffffff, 0x003fffffffffffff, 0x0000000000000000,
  /* 0x009e00-0x009fff | number of bits: 422 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  0xffffffffffffffff, 0xffffffffffffffff, 0x0000003fffffffff, 0x0000000000000000,
  /* 0x00a400-0x00a5ff | number of bits: 141 */
  0xffffffffffffffff, 0xffffffffffffffff, 0x0000000000001fff, 0x0000000000000000,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x00d600-0x00d7ff | number of bits: 420 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  0xffffffffffffffff, 0xffffffffffffffff, 0x0000000fffffffff, 0x0000000000000000,
  /* 0x00f800-0x00f9ff | number of bits: 256 */
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  /* 0x00fa00-0x00fbff | number of bits: 305 */
  0xffff3fffffffffff, 0x000007ffffffffff, 0x0000000000000000, 0x0000000000000000,
  0x5f7ffdffe0f8007f, 0xffffffffffffffdb, 0x0003ffffffffffff, 0xfffffffffff80000,
  /* 0x00fc00-0x00fdff | number of bits: 448 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
  0x3fffffffffffffff, 0xffffffffffff0000, 0xfffffffffffcffff, 0x0fff0000000000ff,
  /* 0x00fe00-0x00ffff | number of bits: 302 */
  0x0000000000000000, 0xffdf000000000000, 0xffffffffffffffff, 0x1fffffffffffffff,
  0x07fffffe00000000, 0xffffffc007fffffe, 0x7fffffffffffffff, 0x000000001cfcfcfc,
  /* 0x010000-0x0101ff | number of bits: 211 */
  0xb7ffff7fffffefff, 0x000000003fff3fff, 0xffffffffffffffff, 0x07ffffffffffffff,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x010200-0x0103ff | number of bits: 88 */
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  0xffff00007fffffff, 0x00000000000007ff, 0x000000003fffffff, 0x0000000000000000,
  /* 0x010400-0x0105ff | number of bits: 158 */
  0xffffffffffffffff, 0xffffffffffffffff, 0x000000003fffffff, 0x0000000000000000,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x010800-0x0109ff | number of bits: 55 */
  0x91bffffffffffd3f, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x01d400-0x01d5ff | number of bits: 488 */
  0xffffffffffffffff, 0xffffffffffdfffff, 0xebffde64dfffffff, 0xffffffffffffffef,
  0x7bffffffdfdfe7bf, 0xfffffffffffdfc5f, 0xffffffffffffffff, 0xffffffffffffffff,
  /* 0x01d600-0x01d7ff | number of bits: 444 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffff0fffffffff, 0xf7fffffff7fffffd,
  0xffdfffffffdfffff, 0xffff7fffffff7fff, 0xfffffdfffffffdff, 0x00000000000003f7,
  /* 0x02a600-0x02a7ff | number of bits: 215 */
  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0x00000000007fffff,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  /* 0x02fa00-0x02fbff | number of bits: 30 */
  0x000000003fffffff, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
  0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000,
};

static const signed char unicode_letters_ind[] = {
    0,   1,   2,   3,   4,   5,   6,   7, /* 0x000000-0x000fff */
    8,   9,  10,  11,  12,  -1,  13,  14, /* 0x001000-0x001fff */
   15,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x002000-0x002fff */
   16,  -1,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x003000-0x003fff */
   -2,  -2,  -2,  -2,  -2,  -2,  17,  -2, /* 0x004000-0x004fff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x005000-0x005fff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x006000-0x006fff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x007000-0x007fff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x008000-0x008fff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  18, /* 0x009000-0x009fff */
   -2,  -2,  19,  -1,  -1,  -1,  -2,  -2, /* 0x00a000-0x00afff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x00b000-0x00bfff */
   -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2, /* 0x00c000-0x00cfff */
   -2,  -2,  -2,  20,  -1,  -1,  -1,  -1, /* 0x00d000-0x00dfff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x00e000-0x00efff */
   -1,  -1,  -1,  -1,  21,  22,  23,  24, /* 0x00f000-0x00ffff */
   25,  26,  27,  -1,  28,  -1,  -1,  -1, /* 0x010000-0x010fff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x011000-0x011fff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x012000-0x012fff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x013000-0x013fff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x014000-0x014fff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x015000-0x015fff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x016000-0x016fff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x017000-0x017fff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x018000-0x018fff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x019000-0x019fff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x01a000-0x01afff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x01b000-0x01bfff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x01c000-0x01cfff */
   -1,  -1,  29,  30,  -1,  -1,  -1,  -1, /* 0x01d000-0x01dfff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x01e000-0x01efff */
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
   -2,  -2,  -2,  31,  -1,  -1,  -1,  -1, /* 0x02a000-0x02afff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x02b000-0x02bfff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x02c000-0x02cfff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x02d000-0x02dfff */
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, /* 0x02e000-0x02efff */
   -1,  -1,  -1,  -1,  -2,  32            /* 0x02f000-0x02fbff */
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
      ind = nonspacing_table_ind[ch >> 9];
      if (ind >= 0) {
        return (boolType) (
            (nonspacing_table_data[8 * (unsigned int) ind + ((ch >> 6) & 7)] >> (ch & 63)) & 1);
      } /* if */
    } /* if */
    return FALSE;
  } /* is_nonspacing */



/**
 * Do a binary search of the character index
 * and assume double width is true for each odd block
 * since first block has single or neutral width
 * and the block perperties are alternating
 */
static inline boolType is_doublewidth (charType ch)

  {
    int min = 0;
    int mid = 0;
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
 *  when sizeof(genericType) != sizeof(charType).
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
 *  when sizeof(genericType) != sizeof(charType).
 */
void chrCpyGeneric (genericType *const dest, const genericType source)

  { /* chrCpyGeneric */
    ((rtlObjectType *) dest)->value.charValue =
        ((const_rtlObjectType *) &source)->value.charValue;
  } /* chrCpyGeneric */



/**
 *  Check whether 'ch' is an alphabetic Unicode character.
 *  Uses identifier data table to look up a particular code point.
 *  The table includes many Unicode ranges listed below.
 *  @return TRUE if 'ch' is an alphabetic symbol,
 *          FALSE otherwise
 */
boolType chrIsLetter (charType ch)

  {
    int ind;

  /* chrIsLetter */
    if (ch <= 0x02fbff) {
      ind = unicode_letters_ind[ch >> 9];
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
 *  The mapping is independend from the locale. Individual
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
 *  The mapping is independend from the locale. Individual
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

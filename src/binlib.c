/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2016, 2018, 2021  Thomas Mertes            */
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
/*  File: seed7/src/binlib.c                                        */
/*  Changes: 2015  Thomas Mertes                                    */
/*  Content: All primitive actions for the types bin64 and bin32.   */
/*                                                                  */
/********************************************************************/

#include "version.h"

#include "stdlib.h"
#include "stdio.h"

#include "common.h"
#include "data.h"
#include "syvarutl.h"
#include "objutl.h"
#include "runerr.h"
#include "int_rtl.h"
#include "set_rtl.h"
#include "big_drv.h"

#undef EXTERN
#define EXTERN
#include "biglib.h"



/**
 *  Compute a bitwise 'and' of two binary values.
 *  @return the bitwise 'and' of the two values.
 */
objectType bin_and (listType arguments)

  { /* bin_and */
    isit_binary(arg_1(arguments));
    isit_binary(arg_3(arguments));
    return bld_binary_temp(
        take_binary(arg_1(arguments)) & take_binary(arg_3(arguments)));
  } /* bin_and */



/**
 *  Compute a bitwise 'and' and assign the result back.
 */
objectType bin_and_assign (listType arguments)

  {
    objectType binary_variable;

  /* bin_and_assign */
    binary_variable = arg_1(arguments);
    isit_binary(binary_variable);
    is_variable(binary_variable);
    isit_binary(arg_3(arguments));
    binary_variable->value.binaryValue &= take_binary(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* bin_and_assign */



/**
 *  Convert to bigInteger.
 *  @return the unchanged value as integer.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType bin_big (listType arguments)

  { /* bin_big */
    isit_binary(arg_1(arguments));
    return bld_bigint_temp(bigFromUInt64(take_binary(arg_1(arguments))));
  } /* bin_big */



/**
 *  Convert to bin64.
 *  @return the unchanged value as bin64.
 */
objectType bin_binary (listType arguments)

  { /* bin_binary */
    isit_bigint(arg_1(arguments));
    return bld_binary_temp(bigToUInt64(take_bigint(arg_1(arguments))));
  } /* bin_binary */



/**
 *  Determine the number of one bits in a binary value.
 *  @return the number of one bits.
 */
objectType bin_card (listType arguments)

  { /* bin_card */
    isit_binary(arg_1(arguments));
    return bld_int_temp(
        uintCard(take_binary(arg_1(arguments))));
  } /* bin_card */



/**
 *  Compare two binary values.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
objectType bin_cmp (listType arguments)

  {
    uintType binary1;
    uintType binary2;
    intType signumValue;

  /* bin_cmp */
    isit_binary(arg_1(arguments));
    isit_binary(arg_2(arguments));
    binary1 = take_binary(arg_1(arguments));
    binary2 = take_binary(arg_2(arguments));
    if (binary1 < binary2) {
      signumValue = -1;
    } else if (binary1 > binary2) {
      signumValue = 1;
    } else {
      signumValue = 0;
    } /* if */
    return bld_int_temp(signumValue);
  } /* bin_cmp */



/**
 *  Get 64 bits from a bitset starting with lowestBitNum/arg_2.
 *  @return a bit pattern with 64 bits from set1/arg_1.
 */
objectType bin_get_binary_from_set (listType arguments)

  { /* bin_get_binary_from_set */
    isit_set(arg_1(arguments));
    isit_int(arg_2(arguments));
    return bld_binary_temp(
        setToUInt(take_set(arg_1(arguments)), take_int(arg_2(arguments))));
  } /* bin_get_binary_from_set */



/**
 *  Shift a binary value left by lshift bits.
 *  Bits that are shifted beyond the size of a binary value
 *  are lost.
 *  @return the left shifted binary value.
 *  @exception OVERFLOW_ERROR If the shift amount is
 *             negative or greater equal INTTYPE_SIZE.
 */
objectType bin_lshift (listType arguments)

  {
    intType lshift;

  /* bin_lshift */
    isit_binary(arg_1(arguments));
    isit_int(arg_3(arguments));
    lshift = take_int(arg_3(arguments));
#if CHECK_INT_OVERFLOW
    if (unlikely(lshift < 0 || lshift >= INTTYPE_SIZE)) {
      return raise_exception(SYS_OVF_EXCEPTION);
    } /* if */
#endif
    return bld_binary_temp(
        take_binary(arg_1(arguments)) << lshift);
  } /* bin_lshift */



/**
 *  Shift a binary value left by lshift bits and assign the result back.
 *  @exception OVERFLOW_ERROR If the shift amount is
 *             negative or greater equal INTTYPE_SIZE.
 */
objectType bin_lshift_assign (listType arguments)

  {
    objectType binary_variable;
    intType lshift;

  /* bin_lshift_assign */
    binary_variable = arg_1(arguments);
    isit_binary(binary_variable);
    is_variable(binary_variable);
    isit_int(arg_3(arguments));
    lshift = take_int(arg_3(arguments));
#if CHECK_INT_OVERFLOW
    if (unlikely(lshift < 0 || lshift >= INTTYPE_SIZE)) {
      return raise_exception(SYS_OVF_EXCEPTION);
    } /* if */
#endif
    binary_variable->value.binaryValue =
        take_binary(binary_variable) << lshift;
    return SYS_EMPTY_OBJECT;
  } /* bin_lshift_assign */



/**
 *  Convert an unsigned integer into a big-endian encoded string of bytes.
 *  The result uses binary representation with a base of 256.
 *  The result contains chars (bytes) with an ordinal <= 255.
 *  @param number/arg_1 Unsigned integer number to be converted.
 *  @param length/arg_3 Determines the length of the result string.
 *  @return a string of 'length' bytes with the unsigned binary
 *          representation of 'number'.
 *  @exception RANGE_ERROR If ''length'' is negative or zero, or
 *                         if the result would not fit in ''length'' bytes.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType bin_n_bytes_be (listType arguments)

  { /* bin_n_bytes_be */
    isit_binary(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_stri_temp(uintNBytesBe(take_binary(arg_1(arguments)),
                                      take_int(arg_3(arguments))));
  } /* bin_n_bytes_be */



/**
 *  Convert an unsigned integer into a little-endian encoded string of bytes.
 *  The result uses binary representation with a base of 256.
 *  The result contains chars (bytes) with an ordinal <= 255.
 *  @param number/arg_1 Unsigned integer number to be converted.
 *  @param length/arg_3 Determines the length of the result string.
 *  @return a string of 'length' bytes with the unsigned binary
 *          representation of 'number'.
 *  @exception RANGE_ERROR If ''length'' is negative or zero, or
 *                         if the result would not fit in ''length'' bytes.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType bin_n_bytes_le (listType arguments)

  { /* bin_n_bytes_le */
    isit_binary(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_stri_temp(uintNBytesLe(take_binary(arg_1(arguments)),
                                      take_int(arg_3(arguments))));
  } /* bin_n_bytes_le */



/**
 *  Compute a bitwise inclusive 'or' of two integer values.
 *  @return the bitwise inclusive 'or' of the two values.
 */
objectType bin_or (listType arguments)

  { /* bin_or */
    isit_binary(arg_1(arguments));
    isit_binary(arg_3(arguments));
    return bld_binary_temp(
        take_binary(arg_1(arguments)) | take_binary(arg_3(arguments)));
  } /* bin_or */



/**
 *  Compute a bitwise inclusive 'or' and assign the result back.
 */
objectType bin_or_assign (listType arguments)

  {
    objectType binary_variable;

  /* bin_or_assign */
    binary_variable = arg_1(arguments);
    isit_binary(binary_variable);
    is_variable(binary_variable);
    isit_binary(arg_3(arguments));
    binary_variable->value.binaryValue |= take_binary(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* bin_or_assign */



/**
 *  Convert an unsigned integer number to a string using a radix.
 *  The conversion uses the numeral system with the given base.
 *  Digit values from 10 upward are encoded with lower case letters.
 *  E.g.: 10 is encoded with a, 11 with b, etc.
 *  @return the string result of the conversion.
 *  @exception RANGE_ERROR If base < 2 or base > 36 holds.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType bin_radix (listType arguments)

  { /* bin_radix */
    isit_binary(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_stri_temp(
        uintRadix(take_binary(arg_1(arguments)), take_int(arg_3(arguments)), FALSE));
  } /* bin_radix */



/**
 *  Convert an unsigned integer number to a string using a radix.
 *  The conversion uses the numeral system with the given base.
 *  Digit values from 10 upward are encoded with upper case letters.
 *  E.g.: 10 is encoded with A, 11 with B, etc.
 *  @return the string result of the conversion.
 *  @exception RANGE_ERROR If base < 2 or base > 36 holds.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType bin_RADIX (listType arguments)

  { /* bin_RADIX */
    isit_binary(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_stri_temp(
        uintRadix(take_binary(arg_1(arguments)), take_int(arg_3(arguments)), TRUE));
  } /* bin_RADIX */



/**
 *  Shift a binary value right by rshift bits.
 *  Bits that are shifted beyond the size of a binary value
 *  are lost.
 *  @return the right shifted binary value.
 *  @exception OVERFLOW_ERROR If the shift amount is
 *             negative or greater equal INTTYPE_SIZE.
 */
objectType bin_rshift (listType arguments)

  {
    intType rshift;

  /* bin_rshift */
    isit_binary(arg_1(arguments));
    isit_int(arg_3(arguments));
    rshift = take_int(arg_3(arguments));
#if CHECK_INT_OVERFLOW
    if (unlikely(rshift < 0 || rshift >= INTTYPE_SIZE)) {
      return raise_exception(SYS_OVF_EXCEPTION);
    } /* if */
#endif
    return bld_binary_temp(
        take_binary(arg_1(arguments)) >> rshift);
  } /* bin_rshift */



/**
 *  Shift a binary value right by rshift bits and assign the result back.
 *  @exception OVERFLOW_ERROR If the shift amount is
 *             negative or greater equal INTTYPE_SIZE.
 */
objectType bin_rshift_assign (listType arguments)

  {
    objectType binary_variable;
    intType rshift;

  /* bin_rshift_assign */
    binary_variable = arg_1(arguments);
    isit_binary(binary_variable);
    is_variable(binary_variable);
    isit_int(arg_3(arguments));
    rshift = take_int(arg_3(arguments));
#if CHECK_INT_OVERFLOW
    if (unlikely(rshift < 0 || rshift >= INTTYPE_SIZE)) {
      return raise_exception(SYS_OVF_EXCEPTION);
    } /* if */
#endif
    binary_variable->value.binaryValue =
        take_binary(binary_variable) >> rshift;
    return SYS_EMPTY_OBJECT;
  } /* bin_rshift_assign */



/**
 *  Convert an unsigned integer number to a string.
 *  The number is converted to a string with decimal representation.
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType bin_str (listType arguments)

  { /* bin_str */
    isit_binary(arg_1(arguments));
    return bld_stri_temp(
        uintStr(take_binary(arg_1(arguments))));
  } /* bin_str */



/**
 *  Compute a bitwise exclusive or ('xor') of two binary values.
 *  @return the bitwise exclusive or ('xor') of the two values.
 */
objectType bin_xor (listType arguments)

  { /* bin_xor */
    isit_binary(arg_1(arguments));
    isit_binary(arg_3(arguments));
    return bld_binary_temp(
        take_binary(arg_1(arguments)) ^ take_binary(arg_3(arguments)));
  } /* bin_xor */



/**
 *  Compute a bitwise exclusive or ('xor') and assign the result back.
 */
objectType bin_xor_assign (listType arguments)

  {
    objectType binary_variable;

  /* bin_xor_assign */
    binary_variable = arg_1(arguments);
    isit_binary(binary_variable);
    is_variable(binary_variable);
    isit_binary(arg_3(arguments));
    binary_variable->value.binaryValue ^= take_binary(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* bin_xor_assign */

/********************************************************************/
/*                                                                  */
/*  s7   Seed7 interpreter                                          */
/*  Copyright (C) 1990 - 2015  Thomas Mertes                        */
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
#include "big_drv.h"

#undef EXTERN
#define EXTERN
#include "biglib.h"



/**
 *  Add two unsigned integer numbers.
 *  @return the sum of the two numbers.
 */
objectType bin_add (listType arguments)

  { /* bin_add */
    isit_binary(arg_1(arguments));
    isit_binary(arg_3(arguments));
    return bld_binary_temp(
        take_binary(arg_1(arguments)) + take_binary(arg_3(arguments)));
  } /* bin_add */



/**
 *  Increment an unsigned integer variable by an unsigned delta.
 */
objectType bin_add_assign (listType arguments)

  {
    objectType int_variable;
    uintType delta;

  /* bin_add_assign */
    int_variable = arg_1(arguments);
    isit_binary(int_variable);
    is_variable(int_variable);
    isit_binary(arg_3(arguments));
    delta = take_binary(arg_3(arguments));
    int_variable->value.binaryValue += delta;
    return SYS_EMPTY_OBJECT;
  } /* bin_add_assign */



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



objectType bin_big (listType arguments)

  { /* bin_binary */
    isit_binary(arg_1(arguments));
    return bld_bigint_temp(bigFromUInt64(take_binary(arg_1(arguments))));
  } /* bin_binary */



objectType bin_binary (listType arguments)

  { /* bin_binary */
    isit_bigint(arg_1(arguments));
    return bld_binary_temp(bigToUInt64(take_bigint(arg_1(arguments))));
  } /* bin_binary */



objectType bin_lshift (listType arguments)

  {
    intType lshift;

  /* bin_lshift */
    isit_binary(arg_1(arguments));
    isit_int(arg_3(arguments));
    lshift = take_int(arg_3(arguments));
#ifdef CHECK_INT_OVERFLOW
    if (unlikely(lshift < 0 || lshift >= INTTYPE_SIZE)) {
      return raise_exception(SYS_OVF_EXCEPTION);
    } /* if */
#endif
    return bld_binary_temp(
        take_binary(arg_1(arguments)) << lshift);
  } /* bin_lshift */



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
#ifdef CHECK_INT_OVERFLOW
    if (unlikely(lshift < 0 || lshift >= INTTYPE_SIZE)) {
      return raise_exception(SYS_OVF_EXCEPTION);
    } /* if */
#endif
    binary_variable->value.binaryValue =
        take_binary(binary_variable) << lshift;
    return SYS_EMPTY_OBJECT;
  } /* bin_lshift_assign */



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
 *  @exception RANGE_ERROR When base < 2 or base > 36 holds.
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
 *  @exception RANGE_ERROR When base < 2 or base > 36 holds.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
objectType bin_RADIX (listType arguments)

  { /* bin_RADIX */
    isit_binary(arg_1(arguments));
    isit_int(arg_3(arguments));
    return bld_stri_temp(
        uintRadix(take_binary(arg_1(arguments)), take_int(arg_3(arguments)), TRUE));
  } /* bin_RADIX */



objectType bin_rshift (listType arguments)

  {
    intType rshift;

  /* bin_rshift */
    isit_binary(arg_1(arguments));
    isit_int(arg_3(arguments));
    rshift = take_int(arg_3(arguments));
#ifdef CHECK_INT_OVERFLOW
    if (unlikely(rshift < 0 || rshift >= INTTYPE_SIZE)) {
      return raise_exception(SYS_OVF_EXCEPTION);
    } /* if */
#endif
    return bld_binary_temp(
        take_binary(arg_1(arguments)) >> rshift);
  } /* bin_rshift */



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
#ifdef CHECK_INT_OVERFLOW
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

  /* binbin_xor_assign */
    binary_variable = arg_1(arguments);
    isit_binary(binary_variable);
    is_variable(binary_variable);
    isit_binary(arg_3(arguments));
    binary_variable->value.binaryValue ^= take_binary(arg_3(arguments));
    return SYS_EMPTY_OBJECT;
  } /* bin_xor_assign */

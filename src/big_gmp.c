/********************************************************************/
/*                                                                  */
/*  big_gmp.c     Functions for bigInteger using the gmp library.   */
/*  Copyright (C) 1989 - 2019  Thomas Mertes                        */
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
/*  File: seed7/src/big_gmp.c                                       */
/*  Changes: 2008, 2009, 2010, 2013 - 2019  Thomas Mertes           */
/*  Content: Functions for bigInteger using the gmp library.        */
/*                                                                  */
/********************************************************************/

#define LOG_FUNCTIONS 0
#define VERBOSE_EXCEPTIONS 0

#include "version.h"

#if BIGINT_LIB == BIG_GMP_LIBRARY
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "limits.h"
#include "gmp.h"

#include "common.h"
#include "data_rtl.h"
#include "heaputl.h"
#include "striutl.h"
#include "int_rtl.h"
#include "rtl_err.h"

#undef EXTERN
#define EXTERN
#include "big_drv.h"


/* The integers in the gmp library are limited in size.    */
/* If the size of an integer gets too large the message:   */
/* gmp: overflow in mpz type                               */
/* is written and the program is terminated with abort().  */
/* The setting AVOID_OVERFLOW_IN_MPZ_TYPE activates code   */
/* to avoid the gmp overflow error for shift operations.   */
/* Instead the exception OVERFLOW_IN_MPZ_ERROR is raised.  */
#define AVOID_OVERFLOW_IN_MPZ_TYPE 1
#define GMP_MAX_LIMB_SIZE INT_MAX
#define OVERFLOW_IN_MPZ_ERROR MEMORY_ERROR

#define USE_CUSTOM_ALLOCATION 0

#define HEAP_ALLOC_BIG     (bigIntType) malloc(sizeof(__mpz_struct))
#define HEAP_FREE_BIG(var) free(var)

#if WITH_BIGINT_FREELIST

static freeListElemType flist = NULL;
static unsigned int flist_allowed = 100;

#define POP_BIG_OK      flist != NULL
#define PUSH_BIG_OK     flist_allowed > 0

#define POP_BIG(var)    {var = (bigIntType) flist; flist = flist->next; flist_allowed++; }
#define PUSH_BIG(var)   {((freeListElemType) var)->next = flist; flist = (freeListElemType) var; flist_allowed--; }

#define ALLOC_BIG(var)  if (POP_BIG_OK) POP_BIG(var) else var = HEAP_ALLOC_BIG;
#define FREE_BIG(var)   if (PUSH_BIG_OK) PUSH_BIG(var) else HEAP_FREE_BIG(var);

#else

#define ALLOC_BIG(var)  var = HEAP_ALLOC_BIG
#define FREE_BIG(var)   HEAP_FREE_BIG(var)

#endif



#if USE_CUSTOM_ALLOCATION
static void *alloc_func (size_t size)

  {
    void *memory;

  /* alloc_func */
    /* printf("alloc_func(" FMT_U_MEM ")\n", (memSizeType) size); */
    memory = malloc(size);
    if (unlikely(memory == NULL)) {
      raise_error(MEMORY_ERROR);
      printf(" ***** MEMORY_ERROR *****\n");
      exit(0);
    } /* if */
    return memory;
  } /* alloc_func */



static void *realloc_func (void *ptr, size_t old_size, size_t new_size)

  {
    void *memory;

  /* realloc_func */
    /* printf("realloc_func(*, " FMT_U_MEM ", " FMT_U_MEM ")\n",
       (memSizeType) old_size, (memSizeType) new_size); */
    memory = realloc(ptr, new_size);
    if (unlikely(memory == NULL)) {
      raise_error(MEMORY_ERROR);
      printf(" ***** MEMORY_ERROR *****\n");
      exit(0);
    } /* if */
    return memory;
  } /* realloc_func */
#endif



/**
 *  Setup bigInteger computations.
 *  This function must be called before doing any bigInteger computations.
 */
void setupBig (void)

  { /* setupBig */
#if USE_CUSTOM_ALLOCATION
    mp_set_memory_functions(alloc_func, realloc_func, NULL);
#endif
  } /* setupBig */



cstriType bigHexCStri (const const_bigIntType big1)

  {
    size_t sizeInBytes;
    size_t bytesExported;
    size_t pos;
    int sign;
    unsigned int carry;
    const_cstriType stri_ptr;
    ustriType buffer;
    memSizeType charIndex;
    size_t result_size;
    cstriType result;

  /* bigHexCStri */
    if (big1 != NULL) {
      sizeInBytes = (mpz_sizeinbase(big1, 2) + CHAR_BIT - 1) / CHAR_BIT;
      buffer = (ustriType) malloc(sizeInBytes);
      if (unlikely(buffer == NULL)) {
        raise_error(MEMORY_ERROR);
        result = NULL;
      } else {
        mpz_export(buffer, &bytesExported, 1, 1, 0, 0, big1);
        sign = mpz_sgn(big1);
        if (sign < 0) {
          carry = 1;
          pos = bytesExported;
          while (pos > 0) {
            pos--;
            carry += ~buffer[pos] & 0xFF;
            buffer[pos] = (ucharType) (carry & 0xFF);
            carry >>= CHAR_BIT;
          } /* while */
        } /* if */
        result_size = 3 + sizeInBytes * 2;
        if ((sign > 0 && buffer[0] > BYTE_MAX) ||
            (sign < 0 && buffer[0] <= BYTE_MAX)) {
          result_size += 2;
        } /* if */
        if (unlikely(!ALLOC_CSTRI(result, result_size))) {
          raise_error(MEMORY_ERROR);
        } else {
          if (sign == 0) {
            strcpy(result, "16#00");
          } else {
            strcpy(result, "16#");
            charIndex = 3;
            if (sign < 0) {
              if (buffer[0] <= BYTE_MAX) {
                strcpy(&result[charIndex], "ff");
                charIndex += 2;
              } /* if */
            } else {
              if (buffer[0] > BYTE_MAX) {
                strcpy(&result[charIndex], "00");
                charIndex += 2;
              } /* if */
            } /* for */
            for (pos = 0; pos < bytesExported; pos++) {
              sprintf(&result[charIndex], "%02x", buffer[pos]);
              charIndex += 2;
            } /* for */
          } /* if */
        } /* if */
        free(buffer);
      } /* if */
    } else {
      stri_ptr = " *NULL_BIGINT* ";
      result_size = STRLEN(" *NULL_BIGINT* ");
      if (unlikely(!ALLOC_CSTRI(result, result_size))) {
        raise_error(MEMORY_ERROR);
        return NULL;
      } else {
        strcpy(result, stri_ptr);
      } /* if */
    } /* if */
    return result;
  } /* bigHexCStri */



/**
 *  Compute the absolute value of a 'bigInteger' number.
 *  @return the absolute value.
 */
bigIntType bigAbs (const const_bigIntType big1)

  {
    bigIntType absoluteValue;

  /* bigAbs */
    ALLOC_BIG(absoluteValue);
    mpz_init(absoluteValue);
    mpz_abs(absoluteValue, big1);
    return absoluteValue;
  } /* bigAbs */



/**
 *  Compute the absolute value of a 'bigInteger' number.
 *  Big1 is assumed to be a temporary value which is reused.
 *  @return the absolute value.
 */
bigIntType bigAbsTemp (bigIntType big1)

  { /* bigAbsTemp */
    mpz_abs(big1, big1);
    return big1;
  } /* bigAbsTemp */



/**
 *  Add two 'bigInteger' numbers.
 *  @return the sum of the two numbers.
 */
bigIntType bigAdd (const_bigIntType summand1, const_bigIntType summand2)

  {
    bigIntType sum;

  /* bigAdd */
    ALLOC_BIG(sum);
    mpz_init(sum);
    mpz_add(sum, summand1, summand2);
    return sum;
  } /* bigAdd */



/**
 *  Increment a 'bigInteger' variable by a delta.
 *  Adds delta to *big_variable.
 *  @param delta The delta to be added to *big_variable.
 */
void bigAddAssign (bigIntType *const big_variable, const const_bigIntType delta)

  { /* bigAddAssign */
    mpz_add(*big_variable, *big_variable, delta);
  } /* bigAddAssign */



/**
 *  Increment a 'bigInteger' variable by a delta.
 *  Adds delta to *big_variable.
 *  @param delta The delta to be added to *big_variable.
 *         Delta must be in the range of a long int.
 */
void bigAddAssignSignedDigit (bigIntType *const big_variable, const intType delta)

  { /* bigAddAssignSignedDigit */
    if (delta < 0) {
      mpz_sub_ui(*big_variable, *big_variable, (unsigned long int) -delta);
    } else {
      mpz_add_ui(*big_variable, *big_variable, (unsigned long int) delta);
    } /* if */
  } /* bigAddAssignSignedDigit */



/**
 *  Add two 'bigInteger' numbers.
 *  Summand1 is assumed to be a temporary value which is reused.
 *  @return the sum of the two numbers in 'summand1'.
 */
bigIntType bigAddTemp (bigIntType summand1, const const_bigIntType summand2)

  { /* bigAddTemp */
    mpz_add(summand1, summand1, summand2);
    return summand1;
  } /* bigAddTemp */



bigIntType bigAnd (const_bigIntType big1, const_bigIntType big2)

  {
    bigIntType result;

  /* bigAnd */
    ALLOC_BIG(result);
    mpz_init(result);
    mpz_and(result, big1, big2);
    return result;
  } /* bigAnd */



/**
 *  Number of bits in the minimum two's-complement representation.
 *  The high bits equivalent to the sign bit are not part of the
 *  minimum two's-complement representation.
 *  @return the number of bits.
 *  @exception RANGE_ERROR The result does not fit into an integer.
 */
intType bigBitLength (const const_bigIntType big1)

  {
    int sign;
    mpz_t help;
    intType bitLength;

  /* bigBitLength */
    sign = mpz_sgn(big1);
    if (sign < 0) {
      if (mpz_cmp_si(big1, -1) == 0) {
        bitLength = 0;
      } else {
        mpz_init(help);
        mpz_add_ui(help, big1, 1);
        bitLength = (intType) mpz_sizeinbase(help, 2);
        mpz_clear(help);
      } /* if */
    } else if (sign == 0) {
      bitLength = 0;
    } else {
      bitLength = (intType) mpz_sizeinbase(big1, 2);
    } /* if */
    return bitLength;
  } /* bigBitLength */



/**
 *  Compare two 'bigInteger' numbers.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType bigCmp (const const_bigIntType big1, const const_bigIntType big2)

  {
    intType signumValue;

  /* bigCmp */
    signumValue = mpz_cmp(big1, big2);
    if (signumValue < 0) {
      signumValue = -1;
    } else if (signumValue > 0) {
      signumValue = 1;
    } /* if */
    return signumValue;
  } /* bigCmp */



/**
 *  Reinterpret the generic parameters as bigIntType and call bigCmp.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(bigIntType).
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType bigCmpGeneric (const genericType value1, const genericType value2)

  { /* bigCmpGeneric */
    return bigCmp(((const_rtlObjectType *) &value1)->value.bigIntValue,
                  ((const_rtlObjectType *) &value2)->value.bigIntValue);
  } /* bigCmpGeneric */



/**
 *  Compare 'big1' with the bigdigit 'number'.
 *  The range of 'number' is restricted and it is the job of the
 *  compiler to assure that 'number' is within the allowed range.
 *  @param number Number that must be in the range of
 *         a long int.
 *  @return -1, 0 or 1 if the first argument is considered to be
 *          respectively less than, equal to, or greater than the
 *          second.
 */
intType bigCmpSignedDigit (const const_bigIntType big1, intType number)

  {
    intType signumValue;

  /* bigCmpSignedDigit */
    signumValue = mpz_cmp_si(big1, number);
    if (signumValue < 0) {
      signumValue = -1;
    } else if (signumValue > 0) {
      signumValue = 1;
    } /* if */
    return signumValue;
  } /* bigCmpSignedDigit */



/**
 *  Assign source to *dest.
 *  A copy function assumes that *dest contains a legal value.
 */
void bigCpy (bigIntType *const dest, const const_bigIntType source)

  { /* bigCpy */
    mpz_set(*dest, source);
  } /* bigCpy */



/**
 *  Reinterpret the generic parameters as bigIntType and call bigCpy.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(bigIntType).
 */
void bigCpyGeneric (genericType *const dest, const genericType source)

  { /* bigCpyGeneric */
    bigCpy(&((rtlObjectType *) dest)->value.bigIntValue,
           ((const_rtlObjectType *) &source)->value.bigIntValue);
  } /* bigCpyGeneric */



/**
 *  Return a copy of source, that can be assigned to a new destination.
 *  It is assumed that the destination of the assignment is undefined.
 *  Create functions can be used to initialize Seed7 constants.
 *  @return a copy of source.
 */
bigIntType bigCreate (const const_bigIntType source)

  {
    bigIntType result;

  /* bigCreate */
    ALLOC_BIG(result);
    mpz_init_set(result, source);
    return result;
  } /* bigCreate */



/**
 *  Generic Create function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(bigIntType).
 */
genericType bigCreateGeneric (const genericType source)

  {
    rtlObjectType result;

  /* bigCreateGeneric */
    INIT_GENERIC_PTR(result.value.genericValue);
    result.value.bigIntValue =
        bigCreate(((const_rtlObjectType *) &source)->value.bigIntValue);
    return result.value.genericValue;
  } /* bigCreateGeneric */



/**
 *  Decrement a 'bigInteger' variable.
 *  Decrements *big_variable by 1.
 */
void bigDecr (bigIntType *const big_variable)

  { /* bigDecr */
    mpz_sub_ui(*big_variable, *big_variable, 1);
  } /* bigDecr */



/**
 *  Free the memory referred by 'old_bigint'.
 *  After bigDestr is left 'old_bigint' refers to not existing memory.
 *  The memory where 'old_bigint' is stored can be freed afterwards.
 */
void bigDestr (const const_bigIntType old_bigint)

  { /* bigDestr */
    if (old_bigint != NULL) {
      mpz_clear((bigIntType) old_bigint);
      FREE_BIG((bigIntType) old_bigint);
    } /* if */
  } /* bigDestr */



/**
 *  Generic Destr function to be used via function pointers.
 *  Function pointers in C programs generated by the Seed7 compiler
 *  may point to this function. This assures correct behaviour even
 *  if sizeof(genericType) != sizeof(bigIntType).
 */
void bigDestrGeneric (const genericType old_value)

  { /* bigDestrGeneric */
    bigDestr(((const_rtlObjectType *) &old_value)->value.bigIntValue);
  } /* bigDestrGeneric */



/**
 *  Integer division truncated towards zero.
 *  The remainder of this division is computed with bigRem.
 *  @return the quotient of the integer division.
 *  @exception NUMERIC_ERROR If a division by zero occurs.
 */
bigIntType bigDiv (const const_bigIntType dividend, const const_bigIntType divisor)

  {
    bigIntType quotient;

  /* bigDiv */
    logFunction(printf("bigDiv(%s,", bigHexCStri(dividend));
                printf("%s)\n", bigHexCStri(divisor)););
    if (unlikely(mpz_sgn(divisor) == 0)) {
      logError(printf("bigDiv(%s, %s): Division by zero.\n",
                      bigHexCStri(dividend), bigHexCStri(divisor)););
      raise_error(NUMERIC_ERROR);
      quotient = NULL;
    } else {
      ALLOC_BIG(quotient);
      mpz_init(quotient);
      mpz_tdiv_q(quotient, dividend, divisor);
    } /* if */
    logFunction(printf("bigDiv --> %s\n", bigHexCStri(quotient)););
    return quotient;
  } /* bigDiv */



/**
 *  Integer division truncated towards zero.
 *  The memory for the quotient is requested and the normalized
 *  quotient is returned. The memory for the remainder is
 *  requested and the normalized remainder is assigned to
 *  *remainderAddr.
 *  @return the quotient of the integer division.
 *  @exception NUMERIC_ERROR If a division by zero occurs.
 */
bigIntType bigDivRem (const const_bigIntType dividend, const const_bigIntType divisor,
    bigIntType *remainderAddr)

  {
    bigIntType quotient;

  /* bigDivRem */
    logFunction(printf("bigDivRem(%s,", bigHexCStri(dividend));
                printf("%s)\n", bigHexCStri(divisor)););
    if (unlikely(mpz_sgn(divisor) == 0)) {
      logError(printf("bigDiv(%s, %s): Division by zero.\n",
                      bigHexCStri(dividend), bigHexCStri(divisor)););
      raise_error(NUMERIC_ERROR);
      quotient = NULL;
    } else {
      ALLOC_BIG(quotient);
      mpz_init(quotient);
      ALLOC_BIG(*remainderAddr);
      mpz_init(*remainderAddr);
      mpz_tdiv_qr(quotient, *remainderAddr, dividend, divisor);
    } /* if */
    logFunction(printf("bigDivRem --> %s", bigHexCStri(quotient));
                printf(" (%s)\n", bigHexCStri(*remainderAddr)););
    return quotient;
  } /* bigDivRem */



/**
 *  Check if two 'bigInteger' numbers are equal.
 *  @return TRUE if both numbers are equal,
 *          FALSE otherwise.
 */
boolType bigEq (const const_bigIntType big1, const const_bigIntType big2)

  { /* bigEq */
    return mpz_cmp(big1, big2) == 0;
  } /* bigEq */



/**
 *  Check if 'big1' is equal to the bigdigit 'number'.
 *  The range of 'number' is restricted and it is the job of the
 *  compiler to assure that 'number' is within the allowed range.
 *  @param number Number that must be in the range of
 *         a long int.
 *  @return TRUE if 'big1' and 'number' are equal,
 *          FALSE otherwise.
 */
boolType bigEqSignedDigit (const const_bigIntType big1, intType number)

  { /* bigEqSignedDigit */
    return mpz_cmp_si(big1, number) == 0;
  } /* bigEqSignedDigit */



/**
 *  Convert a byte buffer (interpreted as big-endian) to a bigInteger.
 *  @param size Size of the byte buffer to be converted (in bytes).
 *  @param buffer Byte buffer to be converted. The bytes are interpreted
 *         as binary big-endian representation with a base of 256.
 *  @param isSigned Defines if 'buffer' is interpreted as signed value.
 *         If 'isSigned' is TRUE the twos-complement representation
 *         is used. In this case the result is negative if the most
 *         significant byte (the first byte) has an ordinal > BYTE_MAX (=127).
 *  @return a bigInteger created from the big-endian bytes.
 *  @exception RANGE_ERROR If 'size' is zero ('buffer' is empty).
 */
bigIntType bigFromByteBufferBe (const memSizeType size,
    const const_ustriType buffer, const boolType isSigned)

  {
    memSizeType pos;
    ustriType negated_buffer;
    unsigned int carry;
    bigIntType result;

  /* bigFromByteBufferBe */
    logFunction(printf("bigFromByteBufferBe(" FMT_U_MEM ", 0x" FMT_X_MEM ", %d)\n",
                       size, (memSizeType) buffer, isSigned););
    if (unlikely(size == 0)) {
      logError(printf("bigFromByteBufferBe(0, \"\", %d): "
                      "Buffer is empty.\n", isSigned););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      ALLOC_BIG(result);
      mpz_init(result);
      if (isSigned && buffer[0] > BYTE_MAX) {
        negated_buffer = (ustriType) malloc(size);
        carry = 1;
        pos = size;
        while (pos > 0) {
          pos--;
          carry += ~buffer[pos] & 0xFF;
          negated_buffer[pos] = (ucharType) (carry & 0xFF);
          carry >>= CHAR_BIT;
        } /* for */
        mpz_import(result, (size_t) size, 1, 1, 0, 0, negated_buffer);
        free(negated_buffer);
        mpz_neg(result, result);
      } else {
        mpz_import(result, (size_t) size, 1, 1, 0, 0, buffer);
      } /* if */
    } /* if */
    logFunction(printf("bigFromByteBufferBe --> %s\n", bigHexCStri(result)););
    return result;
  } /* bigFromByteBufferBe */



/**
 *  Convert a byte buffer (interpreted as little-endian) to a bigInteger.
 *  @param size Size of the byte buffer to be converted (in bytes).
 *  @param buffer Byte buffer to be converted. The bytes are interpreted
 *         as binary little-endian representation with a base of 256.
 *  @param isSigned Defines if 'buffer' is interpreted as signed value.
 *         If 'isSigned' is TRUE the twos-complement representation
 *         is used. In this case the result is negative if the most
 *         significant byte (the last byte) has an ordinal > BYTE_MAX (=127).
 *  @return a bigInteger created from the little-endian bytes.
 *  @exception RANGE_ERROR If 'size' is zero ('buffer' is empty).
 */
bigIntType bigFromByteBufferLe (const memSizeType size,
    const const_ustriType buffer, const boolType isSigned)

  {
    memSizeType pos;
    ustriType negated_buffer;
    unsigned int carry;
    bigIntType result;

  /* bigFromByteBufferLe */
    logFunction(printf("bigFromByteBufferLe(" FMT_U_MEM ", 0x" FMT_X_MEM ", %d)\n",
                       size, (memSizeType) buffer, isSigned););
    if (unlikely(size == 0)) {
      logError(printf("bigFromByteBufferLe(0, \"\", %d): "
                      "Buffer is empty.\n", isSigned););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      ALLOC_BIG(result);
      mpz_init(result);
      if (isSigned && buffer[size - 1] > BYTE_MAX) {
        negated_buffer = (ustriType) malloc(size);
        carry = 1;
        pos = 0;
        while (pos < size) {
          carry += ~buffer[pos] & 0xFF;
          negated_buffer[pos] = (ucharType) (carry & 0xFF);
          carry >>= CHAR_BIT;
          pos++;
        } /* for */
        mpz_import(result, (size_t) size, -1, 1, 0, 0, negated_buffer);
        free(negated_buffer);
        mpz_neg(result, result);
      } else {
        mpz_import(result, (size_t) size, -1, 1, 0, 0, buffer);
      } /* if */
    } /* if */
    logFunction(printf("bigFromByteBufferLe --> %s\n", bigHexCStri(result)););
    return result;
  } /* bigFromByteBufferLe */



/**
 *  Convert a bstring (interpreted as big-endian) to a bigInteger.
 *  @param bstri Bstring to be converted. The bytes are interpreted
 *         as binary big-endian representation with a base of 256.
 *  @param isSigned Defines if 'bstri' is interpreted as signed value.
 *         If 'isSigned' is TRUE the twos-complement representation
 *         is used. In this case the result is negative if the most
 *         significant byte (the first byte) has an ordinal > BYTE_MAX (=127).
 *  @return a bigInteger created from the big-endian bytes.
 *  @exception RANGE_ERROR If 'bStri' is empty.
 */
bigIntType bigFromBStriBe (const const_bstriType bstri, const boolType isSigned)

  { /* bigFromBStriBe */
    logFunction(printf("bigFromBStriBe(\"%s\", %d)\n",
                       bstriAsUnquotedCStri(bstri), isSigned););
    return bigFromByteBufferBe(bstri->size, bstri->mem, isSigned);
  } /* bigFromBStriBe */



/**
 *  Convert a bstring (interpreted as little-endian) to a bigInteger.
 *  @param bstri Bstring to be converted. The bytes are interpreted
 *         as binary little-endian representation with a base of 256.
 *  @param isSigned Defines if 'bstri' is interpreted as signed value.
 *         If 'isSigned' is TRUE the twos-complement representation
 *         is used. In this case the result is negative if the most
 *         significant byte (the last byte) has an ordinal > BYTE_MAX (=127).
 *  @return a bigInteger created from the little-endian bytes.
 *  @exception RANGE_ERROR If 'bStri' is empty.
 */
bigIntType bigFromBStriLe (const const_bstriType bstri, const boolType isSigned)

  { /* bigFromBStriLe */
    logFunction(printf("bigFromBStriLe(\"%s\", %d)\n",
                       bstriAsUnquotedCStri(bstri), isSigned););
    return bigFromByteBufferLe(bstri->size, bstri->mem, isSigned);
  } /* bigFromBStriLe */



/**
 *  Convert an int32Type number to 'bigInteger'.
 *  @return the bigInteger result of the conversion.
 */
bigIntType bigFromInt32 (int32Type number)

  {
    bigIntType result;

  /* bigFromInt32 */
    ALLOC_BIG(result);
    mpz_init_set_si(result, number);
    return result;
  } /* bigFromInt32 */



#ifdef INT64TYPE
/**
 *  Convert an int64Type number to 'bigInteger'.
 *  @return the bigInteger result of the conversion.
 */
bigIntType bigFromInt64 (int64Type number)

  {
    bigIntType result;

  /* bigFromInt64 */
    ALLOC_BIG(result);
#if LONG_SIZE == 64
    mpz_init_set_si(result, number);
#else
    {
      mpz_t help;

      mpz_init_set_si(result, (int32Type) ((number >> 32) & 0xFFFFFFFF));
      mpz_mul_2exp(result, result, 32);
      mpz_init_set_ui(help, (uint32Type) (number & 0xFFFFFFFF));
      mpz_ior(result, result, help);
      mpz_clear(help);
    }
#endif
    return result;
  } /* bigFromInt64 */
#endif



/**
 *  Convert an uint32Type number to 'bigInteger'.
 *  @return the bigInteger result of the conversion.
 */
bigIntType bigFromUInt32 (uint32Type number)

  {
    bigIntType result;

  /* bigFromUInt32 */
    ALLOC_BIG(result);
    mpz_init_set_ui(result, number);
    return result;
  } /* bigFromUInt32 */



#ifdef INT64TYPE
/**
 *  Convert an uint64Type number to 'bigInteger'.
 *  @return the bigInteger result of the conversion.
 */
bigIntType bigFromUInt64 (uint64Type number)

  {
    mpz_t help;
    bigIntType result;

  /* bigFromUInt64 */
    logFunction(printf("bigFromUInt64(" FMT_U64 ")\n", number););
    ALLOC_BIG(result);
    mpz_init_set_ui(result, (uint32Type) ((number >> 32) & 0xFFFFFFFF));
    mpz_mul_2exp(result, result, 32);
    mpz_init_set_ui(help, (uint32Type) (number & 0xFFFFFFFF));
    mpz_ior(result, result, help);
    mpz_clear(help);
    logFunction(printf("bigFromUInt64 --> %s\n", bigHexCStri(result)););
    return result;
  } /* bigFromUInt64 */
#endif



/**
 *  Compute the greatest common divisor of two 'bigInteger' numbers.
 *  @return the greatest common divisor of the two numbers.
 *          The greatest common divisor is positive or zero.
 */
bigIntType bigGcd (const const_bigIntType big1,
    const const_bigIntType big2)

  {
    bigIntType result;

  /* bigGcd */
    ALLOC_BIG(result);
    mpz_init(result);
    mpz_gcd(result, big1, big2);
    return result;
  } /* bigGcd */



/**
 *  Compute the hash value of a 'bigInteger' number.
 *  @return the hash value.
 */
intType bigHashCode (const const_bigIntType big1)

  {
    size_t size;
    intType result;

  /* bigHashCode */
    size = mpz_size(big1);
    if (size == 0) {
      result = 0;
    } else {
      result = (intType) (mpz_getlimbn(big1, 0) << 5 ^ size << 3 ^ mpz_getlimbn(big1, size - 1));
    } /* if */
    return result;
  } /* bigHashCode */



/**
 *  Increment a 'bigInteger' variable.
 *  Increments *big_variable by 1.
 */
void bigIncr (bigIntType *const big_variable)

  { /* bigIncr */
    mpz_add_ui(*big_variable, *big_variable, 1);
  } /* bigIncr */



/**
 *  Compute the exponentiation of a 'bigInteger' base with an integer exponent.
 *  @return the result of the exponentiation.
 *  @exception NUMERIC_ERROR If the exponent is negative.
 */
bigIntType bigIPow (const const_bigIntType base, intType exponent)

  {
    bigIntType power;

  /* bigIPow */
    logFunction(printf("bigIPow(%s, " FMT_D ")\n",
                       bigHexCStri(base), exponent););
    if (unlikely(exponent < 0)) {
      logError(printf("bigIPow(%s, " FMT_D "): "
                      "Exponent is negative.\n",
                      bigHexCStri(base), exponent););
      raise_error(NUMERIC_ERROR);
      power = NULL;
    } else {
      ALLOC_BIG(power);
      mpz_init(power);
      mpz_pow_ui(power, base, (uintType) exponent);
    } /* if */
    logFunction(printf("bigIPow --> %s\n", bigHexCStri(power)););
    return power;
  } /* bigIPow */



/**
 *  Compute the exponentiation of a bigdigit base with an integer exponent.
 *  @param base Base that must be in the range of a long int.
 *  @return the result of the exponentiation.
 *  @exception NUMERIC_ERROR If the exponent is negative.
 */
bigIntType bigIPowSignedDigit (intType base, intType exponent)

  {
    bigIntType power;

  /* bigIPowSignedDigit */
    if (unlikely(exponent < 0)) {
      logError(printf("bigIPowSignedDigit(" FMT_D ", " FMT_D "): "
                      "Exponent is negative.\n",
                      base, exponent););
      raise_error(NUMERIC_ERROR);
      power = NULL;
    } else {
      ALLOC_BIG(power);
      mpz_init_set_si(power, base);
      mpz_pow_ui(power, power, (uintType) exponent);
    } /* if */
    return power;
  } /* bigIPowSignedDigit */



/**
 *  Compute the truncated base 10 logarithm of a 'bigInteger' number.
 *  The definition of 'log10' is extended by defining log10(0) = -1_.
 *  @return the truncated base 10 logarithm.
 *  @exception NUMERIC_ERROR The number is negative.
 */
bigIntType bigLog10 (const const_bigIntType big1)

  {
    int sign;
    char *cstri;
    bigIntType logarithm;

  /* bigLog10 */
    sign = mpz_sgn(big1);
    if (unlikely(sign < 0)) {
      logError(printf("bigLog10(%s): Number is negative.\n",
                      bigHexCStri(big1)););
      raise_error(NUMERIC_ERROR);
      logarithm = NULL;
    } else if (sign == 0) {
      ALLOC_BIG(logarithm);
      mpz_init_set_si(logarithm, -1);
    } else {
      cstri = mpz_get_str(NULL, 10, big1);
      ALLOC_BIG(logarithm);
      mpz_init_set_ui(logarithm, strlen(cstri) - 1);
      free(cstri);
    } /* if */
    return logarithm;
  } /* bigLog10 */



/**
 *  Compute the truncated base 2 logarithm of a 'bigInteger' number.
 *  The definition of 'log2' is extended by defining log2(0) = -1_.
 *  @return the truncated base 2 logarithm.
 *  @exception NUMERIC_ERROR The number is negative.
 */
bigIntType bigLog2 (const const_bigIntType big1)

  {
    int sign;
    bigIntType logarithm;

  /* bigLog2 */
    sign = mpz_sgn(big1);
    if (unlikely(sign < 0)) {
      logError(printf("bigLog2(%s): Number is negative.\n",
                      bigHexCStri(big1)););
      raise_error(NUMERIC_ERROR);
      logarithm = NULL;
    } else if (sign == 0) {
      ALLOC_BIG(logarithm);
      mpz_init_set_si(logarithm, -1);
    } else {
      ALLOC_BIG(logarithm);
      mpz_init_set_ui(logarithm, mpz_sizeinbase(big1, 2) - 1);
    } /* if */
    return logarithm;
  } /* bigLog2 */



/**
 *  Create a number from the lower bits of big1.
 *  This corresponds to the modulo if the dividend is a power of two:
 *   bigLowerBits(big1, bits)  corresponds to  big1 mod (2_ ** bits)
 *  @param bits Number of lower bits to select from big1.
 *  @return a number in the range 0 .. pred(2_ ** bits).
 *  @exception NUMERIC_ERROR The number of bits is negative.
 */
bigIntType bigLowerBits (const const_bigIntType big1, const intType bits)

  {
    bigIntType result;

  /* bigLowerBits */
    if (unlikely(bits < 0)) {
      logError(printf("bigLowerBits(%s, " FMT_D "): "
                      "Number of bits is negative.\n",
                      bigHexCStri(big1), bits););
      raise_error(NUMERIC_ERROR);
      result = NULL;
    } else {
      ALLOC_BIG(result);
      mpz_init(result);
      mpz_fdiv_r_2exp(result, big1, (uintType) bits);
    } /* if */
    return result;
  } /* bigLowerBits */



/**
 *  Create a number from the lower bits of big1.
 *  Big1 is assumed to be a temporary value which is reused.
 *  This corresponds to the modulo if the dividend is a power of two:
 *   bigLowerBits(big1, bits)  corresponds to  big1 mod (2_ ** bits)
 *  @param bits Number of lower bits to select from big1.
 *  @return a number in the range 0 .. pred(2_ ** bits).
 *  @exception NUMERIC_ERROR The number of bits is negative.
 */
bigIntType bigLowerBitsTemp (const bigIntType big1, const intType bits)

  { /* bigLowerBitsTemp */
    if (unlikely(bits < 0)) {
      FREE_BIG(big1);
      logError(printf("bigLowerBitsTemp(%s, " FMT_D "): "
                      "Number of bits is negative.\n",
                      bigHexCStri(big1), bits););
      raise_error(NUMERIC_ERROR);
      return NULL;
    } else {
      mpz_fdiv_r_2exp(big1, big1, (uintType) bits);
    } /* if */
    return big1;
  } /* bigLowerBitsTemp */



uint64Type bigLowerBits64 (const const_bigIntType big1)

  {
    mpz_t mod64;
    uint64Type result;

  /* bigLowerBits64 */
    logFunction(printf("bigLowerBits64(%s)\n", bigHexCStri(big1)););
    mpz_init(mod64);
    mpz_fdiv_r_2exp(mod64, big1, 64);
#if LONG_SIZE == 64
    result = mpz_get_ui(mod64);
#else
    {
      mpz_t help;

      mpz_init_set(help, mod64);
      mpz_fdiv_q_2exp(help, help, 32);
      result = (uint64Type) (mpz_get_ui(help) & 0xFFFFFFFF) << 32 |
               (uint64Type) (mpz_get_ui(mod64) & 0xFFFFFFFF);
      mpz_clear(help);
    }
#endif
    logFunction(printf("bigLowerBits64(%s) --> " FMT_U64 "\n",
                       bigHexCStri(big1), result););
    return result;
  } /* bigLowerBits64 */



/**
 *  Index of the lowest-order one bit.
 *  For A <> 0 this is equal to the number of lowest-order zero bits.
 *  @return the number of lowest-order zero bits or -1 for lowestSetBit(0).
 */
intType bigLowestSetBit (const const_bigIntType big1)

  { /* bigLowestSetBit */
    return (intType) mpz_scan1(big1, 0);
  } /* bigLowestSetBit */



/**
 *  Shift a 'bigInteger' number left by lshift bits.
 *  If lshift is negative a right shift is done instead.
 *  A << B is equivalent to A * 2_ ** B if B >= 0 holds.
 *  A << B is equivalent to A mdiv 2_ ** -B if B < 0 holds.
 *  @return the left shifted number.
 */
bigIntType bigLShift (const const_bigIntType big1, const intType lshift)

  {
    bigIntType result;

  /* bigLShift */
    logFunction(printf("bigLShift(%s, " FMT_D ")\n",
                       bigHexCStri(big1), lshift););
    ALLOC_BIG(result);
    mpz_init(result);
    if (lshift <= 0) {
      /* The unsigned value is negated to avoid a signed integer */
      /* overflow if the smallest signed integer is negated.     */
      mpz_fdiv_q_2exp(result, big1, -(uintType) lshift);
#if AVOID_OVERFLOW_IN_MPZ_TYPE
    } else if (unlikely((uintType) lshift / GMP_LIMB_BITS + 1 >
                        GMP_MAX_LIMB_SIZE - mpz_size(big1) &&
                        mpz_sgn(big1) != 0)) {
      mpz_clear(result);
      FREE_BIG(result);
      raise_error(OVERFLOW_IN_MPZ_ERROR);
      result = NULL;
#endif
    } else {
      mpz_mul_2exp(result, big1, (uintType) lshift);
    } /* if */
    logFunction(printf("bigLShift --> %s\n", bigHexCStri(result)););
    return result;
  } /* bigLShift */



/**
 *  Shift a number left by lshift bits and assign the result back to number.
 *  If lshift is negative a right shift is done instead.
 */
void bigLShiftAssign (bigIntType *const big_variable, intType lshift)

  {
    bigIntType big1;

  /* bigLShiftAssign */
    big1 = *big_variable;
    logFunction(printf("bigLShiftAssign(%s, " FMT_D ")\n",
                       bigHexCStri(big1), lshift););
    if (lshift < 0) {
      /* The unsigned value is negated to avoid a signed integer */
      /* overflow if the smallest signed integer is negated.     */
      mpz_fdiv_q_2exp(big1, big1, -(uintType) lshift);
    } else if (lshift == 0) {
      /* do nothing */
#if AVOID_OVERFLOW_IN_MPZ_TYPE
    } else if (unlikely((uintType) lshift / GMP_LIMB_BITS + 1 >
                        GMP_MAX_LIMB_SIZE - mpz_size(big1) &&
                        mpz_sgn(big1) != 0)) {
      raise_error(OVERFLOW_IN_MPZ_ERROR);
#endif
    } else {
      mpz_mul_2exp(big1, big1, (uintType) lshift);
    } /* if */
    logFunction(printf("bigLShiftAssign --> %s\n", bigHexCStri(big1)););
  } /* bigLShiftAssign */



/**
 * Shift one left by 'lshift' bits.
 * If 'lshift' is positive or zero this corresponds to
 * the computation of a power of two:
 *  bigLShiftOne(lshift)  corresponds to  2_ ** lshift
 * If 'lshift' is negative the result is zero.
 * @return one shifted left by 'lshift'.
 */
bigIntType bigLShiftOne (const intType lshift)

  {
    mpz_t one;
    bigIntType result;

  /* bigLShiftOne */
    ALLOC_BIG(result);
    if (lshift < 0) {
      mpz_init_set_ui(result, 0);
#if AVOID_OVERFLOW_IN_MPZ_TYPE
    } else if (unlikely((uintType) lshift / GMP_LIMB_BITS + 1 >
                        GMP_MAX_LIMB_SIZE - 1)) {
      FREE_BIG(result);
      raise_error(OVERFLOW_IN_MPZ_ERROR);
      result = NULL;
#endif
    } else {
      mpz_init(result);
      mpz_init_set_ui(one, 1);
      mpz_mul_2exp(result, one, (uintType) lshift);
      mpz_clear(one);
    } /* if */
    return result;
  } /* bigLShiftOne */



/**
 *  Exponentiation if the base is a power of two.
 *  @param log2base Logarithm of the actual base ( =log2(base) )
 *  @return (2 ** log2base) ** exponent
 *  @exception NUMERIC_ERROR If log2base or exponent is negative.
 */
bigIntType bigLog2BaseIPow (const intType log2base, const intType exponent)

  {
    uintType high_shift;
    uintType low_shift;
    bigIntType power;

  /* bigLog2BaseIPow */
    if (unlikely(log2base < 0 || exponent < 0)) {
      logError(printf("bigLog2BaseIPow(" FMT_D ", " FMT_D "): "
                      "Log2base or exponent is negative.\n",
                      log2base, exponent););
      raise_error(NUMERIC_ERROR);
      power = NULL;
    } else if (likely(log2base == 1)) {
      power = bigLShiftOne(exponent);
    } else if (log2base <= 10 && exponent <= MAX_DIV_10) {
      power = bigLShiftOne(log2base * exponent);
    } else {
      low_shift = uintMult((uintType) log2base, (uintType) exponent, &high_shift);
      if (unlikely(high_shift != 0 || (intType) low_shift < 0)) {
        raise_error(MEMORY_ERROR);
        power = NULL;
      } else {
        power = bigLShiftOne((intType) low_shift);
      } /* if */
    } /* if */
    return power;
  } /* bigLog2BaseIPow */



/**
 *  Integer division truncated towards negative infinity.
 *  The modulo (remainder) of this division is computed with bigMod.
 *  Therefore this division is called modulo division (MDiv).
 *  @return the quotient of the integer division.
 *  @exception NUMERIC_ERROR If a division by zero occurs.
 */
bigIntType bigMDiv (const const_bigIntType dividend, const const_bigIntType divisor)

  {
    bigIntType quotient;

  /* bigMDiv */
    logFunction(printf("bigMDiv(%s,", bigHexCStri(dividend));
                printf("%s)\n", bigHexCStri(divisor)););
    if (unlikely(mpz_sgn(divisor) == 0)) {
      logError(printf("bigMDiv(%s, %s): Division by zero.\n",
                      bigHexCStri(dividend), bigHexCStri(divisor)););
      raise_error(NUMERIC_ERROR);
      quotient = NULL;
    } else {
      ALLOC_BIG(quotient);
      mpz_init(quotient);
      mpz_fdiv_q(quotient, dividend, divisor);
    } /* if */
    logFunction(printf("bigMDiv --> %s\n", bigHexCStri(quotient)););
    return quotient;
  } /* bigMDiv */



/**
 *  Compute the modulo (remainder) of the integer division bigMDiv.
 *  The modulo has the same sign as the divisor.
 *  @return the modulo of the integer division.
 *  @exception NUMERIC_ERROR If a division by zero occurs.
 */
bigIntType bigMod (const const_bigIntType dividend, const const_bigIntType divisor)

  {
    bigIntType modulo;

  /* bigMod */
    logFunction(printf("bigMod(%s,", bigHexCStri(dividend));
                printf("%s)\n", bigHexCStri(divisor)););
    if (unlikely(mpz_sgn(divisor) == 0)) {
      logError(printf("bigMod(%s, %s): Division by zero.\n",
                      bigHexCStri(dividend), bigHexCStri(divisor)););
      raise_error(NUMERIC_ERROR);
      modulo = NULL;
    } else {
      ALLOC_BIG(modulo);
      mpz_init(modulo);
      mpz_fdiv_r(modulo, dividend, divisor);
    } /* if */
    logFunction(printf("bigMod --> %s\n", bigHexCStri(modulo)););
    return modulo;
  } /* bigMod */



/**
 *  Multiply two 'bigInteger' numbers.
 *  @return the product of the two numbers.
 */
bigIntType bigMult (const const_bigIntType factor1, const const_bigIntType factor2)

  {
    bigIntType product;

  /* bigMult */
    ALLOC_BIG(product);
    mpz_init(product);
    mpz_mul(product, factor1, factor2);
    return product;
  } /* bigMult */



/**
 *  Multiply a 'bigInteger' number by a factor and assign the result back to number.
 */
void bigMultAssign (bigIntType *const big_variable, const_bigIntType factor)

  { /* bigMultAssign */
    mpz_mul(*big_variable, *big_variable, factor);
  } /* bigMultAssign */



/**
 *  Multiply factor1 with the bigdigit factor2.
 *  The range of factor2 is restricted and it is the job of the
 *  compiler to assure that factor2 is within the allowed range.
 *  @param factor2 Multiplication factor that must be
 *         in the range of a long int.
 *  @return the product of factor1 * factor2.
 */
bigIntType bigMultSignedDigit (const_bigIntType factor1, intType factor2)

  {
    bigIntType product;

  /* bigMultSignedDigit */
    ALLOC_BIG(product);
    mpz_init(product);
    mpz_mul_si(product, factor1, factor2);
    return product;
  } /* bigMultSignedDigit */



/**
 *  Minus sign, negate a 'bigInteger' number.
 *  @return the negated value of the number.
 */
bigIntType bigNegate (const const_bigIntType big1)

  {
    bigIntType negatedValue;

  /* bigNegate */
    ALLOC_BIG(negatedValue);
    mpz_init(negatedValue);
    mpz_neg(negatedValue, big1);
    return negatedValue;
  } /* bigNegate */



/**
 *  Minus sign, negate a 'bigInteger' number.
 *  Big1 is assumed to be a temporary value which is reused.
 *  @return the negated value of the number.
 */
bigIntType bigNegateTemp (bigIntType big1)

  { /* bigNegateTemp */
    mpz_neg(big1, big1);
    return big1;
  } /* bigNegateTemp */



/**
 *  Determine if a 'bigInteger' number is odd.
 *  @return TRUE if the number is odd,
 *          FALSE otherwise.
 */
boolType bigOdd (const const_bigIntType big1)

  { /* bigOdd */
    return mpz_odd_p(big1);
  } /* bigOdd */



bigIntType bigOr (const_bigIntType big1, const_bigIntType big2)

  {
    bigIntType result;

  /* bigOr */
    ALLOC_BIG(result);
    mpz_init(result);
    mpz_ior(result, big1, big2);
    return result;
  } /* bigOr */



/**
 *  Convert a string to a 'bigInteger' number.
 *  The string must contain an integer literal consisting of an
 *  optional + or - sign, followed by a sequence of digits. Other
 *  characters as well as leading or trailing whitespace characters are
 *  not allowed. The sequence of digits is taken to be decimal.
 *  @return the 'bigInteger' result of the conversion.
 *  @exception RANGE_ERROR If the string is empty or does not contain
 *             an integer literal.
 *  @exception MEMORY_ERROR  Not enough memory to represent the result.
 */
bigIntType bigParse (const const_striType stri)

  {
    cstriType cstri;
    int mpz_result;
    errInfoType err_info = OKAY_NO_ERROR;
    bigIntType result;

  /* bigParse */
    cstri = stri_to_cstri(stri, &err_info);
    if (unlikely(cstri == NULL)) {
      logError(printf("bigParse: stri_to_cstri(\"%s\", *) failed:\n"
                      "err_info=%d\n",
                      striAsUnquotedCStri(stri), err_info););
      raise_error(err_info);
      result = NULL;
    } else if (strpbrk(cstri, " \f\n\r\t\v") != NULL) {
      logError(printf("bigParse(\"%s\"): String contains whitespace characters.\n",
                      striAsUnquotedCStri(stri)););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      ALLOC_BIG(result);
      if (cstri[0] == '+' && cstri[1] != '-') {
        mpz_result = mpz_init_set_str(result, &cstri[1], 10);
      } else {
        mpz_result = mpz_init_set_str(result, cstri, 10);
      } /* if */
      free_cstri(cstri, stri);
      if (unlikely(mpz_result != 0)) {
        mpz_clear(result);
        FREE_BIG(result);
        logError(printf("bigParse(\"%s\"): "
                        "mpz_init_set_str(*, \"%s\", 10) failed.\n",
                        striAsUnquotedCStri(stri),
                        cstri[0] == '+' && cstri[1] != '-' ? &cstri[1] : cstri););
        raise_error(RANGE_ERROR);
        result = NULL;
      } /* if */
    } /* if */
    return result;
  } /* bigParse */



/**
 *  Convert a numeric string, with a specified radix, to a 'bigInteger'.
 *  The numeric string must contain the representation of an integer
 *  in the specified radix. It consists of an optional + or - sign,
 *  followed by a sequence of digits in the specified radix. Digit values
 *  from 10 upward can be encoded with upper or lower case letters.
 *  E.g.: 10 can be encoded with A or a, 11 with B or b, etc. Other
 *  characters as well as leading or trailing whitespace characters
 *  are not allowed.
 *  @param base Radix of the integer in the 'stri' parameter.
 *  @return the 'bigInteger' result of the conversion.
 *  @exception RANGE_ERROR If base < 2 or base > 36 holds or
 *             the string is empty or it does not contain an integer
 *             literal with the specified base.
 *  @exception MEMORY_ERROR  Not enough memory to represent the result.
 */
bigIntType bigParseBased (const const_striType stri, intType base)

  {
    cstriType cstri;
    int mpz_result;
    errInfoType err_info = OKAY_NO_ERROR;
    bigIntType result;

  /* bigParseBased */
    if (unlikely(stri->size == 0 || base < 2 || base > 36)) {
      logError(printf("bigParseBased(\"%s\", " FMT_D "): "
                      "String empty or base not in allowed range.\n",
                      striAsUnquotedCStri(stri), base););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      cstri = stri_to_cstri(stri, &err_info);
      if (unlikely(cstri == NULL)) {
        logError(printf("bigParseBased: stri_to_cstri(\"%s\", *) failed:\n"
                        "err_info=%d\n",
                        striAsUnquotedCStri(stri), err_info););
        raise_error(err_info);
        result = NULL;
      } else if (strpbrk(cstri, " \f\n\r\t\v") != NULL) {
        logError(printf("bigParseBased(\"%s\", " FMT_D "): "
                        "String contains whitespace characters.\n",
                        striAsUnquotedCStri(stri), base););
        raise_error(RANGE_ERROR);
        result = NULL;
      } else {
        ALLOC_BIG(result);
        if (cstri[0] == '+' && cstri[1] != '-') {
          mpz_result = mpz_init_set_str(result, &cstri[1], (int) base);
        } else {
          mpz_result = mpz_init_set_str(result, cstri, (int) base);
        } /* if */
        free_cstri(cstri, stri);
        if (unlikely(mpz_result != 0)) {
          mpz_clear(result);
          FREE_BIG(result);
          logError(printf("bigParseBased(\"%s\", " FMT_D "): "
                          "mpz_init_set_str(*, \"%s\", " FMT_D ") failed.\n",
                          striAsUnquotedCStri(stri), base,
                          cstri[0] == '+' && cstri[1] != '-' ? &cstri[1] : cstri,
                          base););
          raise_error(RANGE_ERROR);
          result = NULL;
        } /* if */
      } /* if */
    } /* if */
    return result;
  } /* bigParseBased */



/**
 *  Predecessor of a 'bigInteger' number.
 *  pred(A) is equivalent to A-1 .
 *  @return big1 - 1 .
 */
bigIntType bigPred (const const_bigIntType big1)

  {
    bigIntType predecessor;

  /* bigPred */
    ALLOC_BIG(predecessor);
    mpz_init(predecessor);
    mpz_sub_ui(predecessor, big1, 1);
    return predecessor;
  } /* bigPred */



/**
 *  Returns a signed big integer decremented by 1.
 *  Big1 is assumed to be a temporary value which is reused.
 */
bigIntType bigPredTemp (bigIntType big1)

  { /* bigPredTemp */
    mpz_sub_ui(big1, big1, 1);
    return big1;
  } /* bigPredTemp */



/**
 *  Convert a big integer number to a string using a radix.
 *  The conversion uses the numeral system with the given base.
 *  Digit values from 10 upward are encoded with letters.
 *  For negative numbers a minus sign is prepended.
 *  @param upperCase Decides about the letter case.
 *  @return the string result of the conversion.
 *  @exception RANGE_ERROR If base < 2 or base > 36 holds.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
striType bigRadix (const const_bigIntType big1, intType base,
    boolType upperCase)

  {
    char *cstri;
    striType result;

  /* bigRadix */
    logFunction(printf("bigRadix(%s, " FMT_D ", %d)\n",
                       bigHexCStri(big1), base, upperCase););
    if (unlikely(base < 2 || base > 36)) {
      logError(printf("bigRadix(%s, " FMT_D ", %d): "
                      "Base not in allowed range.\n",
                      bigHexCStri(big1), base, upperCase););
      raise_error(RANGE_ERROR);
      result = NULL;
    } else {
      if (upperCase) {
        cstri = mpz_get_str(NULL, -(int) base, big1);
      } else {
        cstri = mpz_get_str(NULL, (int) base, big1);
      } /* if */
      result = cstri_to_stri(cstri);
      free(cstri);
      if (unlikely(result == NULL)) {
        raise_error(MEMORY_ERROR);
      } /* if */
    } /* if */
    logFunction(printf("bigRadix --> \"%s\"\n", striAsUnquotedCStri(result)););
    return result;
  } /* bigRadix */



/**
 *  Compute pseudo-random number in the range [low, high].
 *  The random values are uniform distributed.
 *  @return a random number such that low <= rand(low, high) and
 *          rand(low, high) <= high holds.
 *  @exception RANGE_ERROR The range is empty (low > high holds).
 */
bigIntType bigRand (const const_bigIntType low,
    const const_bigIntType high)

  {
    static boolType seed_necessary = TRUE;
    static gmp_randstate_t state;
    mpz_t range_limit;
    bigIntType randomNumber;

  /* bigRand */
    mpz_init(range_limit);
    mpz_sub(range_limit, high, low);
    if (unlikely(mpz_sgn(range_limit) < 0)) {
      logError(printf("bigRand(%s, %s): "
                      "The range is empty (low > high holds).\n",
                      bigHexCStri(low), bigHexCStri(high)););
      raise_error(RANGE_ERROR);
      randomNumber = 0;
    } else {
      mpz_add_ui(range_limit, range_limit, 1);
      ALLOC_BIG(randomNumber);
      mpz_init(randomNumber);
      if (seed_necessary) {
        gmp_randinit_default(state);
        seed_necessary = FALSE;
      } /* if */
      mpz_urandomm(randomNumber, state, range_limit);
      mpz_add(randomNumber, randomNumber, low);
    } /* if */
    mpz_clear(range_limit);
    return randomNumber;
  } /* bigRand */



/**
 *  Compute the remainder of the integer division bigDiv.
 *  The remainder has the same sign as the dividend.
 *  @return the remainder of the integer division.
 *  @exception NUMERIC_ERROR If a division by zero occurs.
 */
bigIntType bigRem (const const_bigIntType dividend, const const_bigIntType divisor)

  {
    bigIntType remainder;

  /* bigRem */
    logFunction(printf("bigRem(%s,", bigHexCStri(dividend));
                printf("%s)\n", bigHexCStri(divisor)););
    if (unlikely(mpz_sgn(divisor) == 0)) {
      logError(printf("bigRem(%s, %s): Division by zero.\n",
                      bigHexCStri(dividend), bigHexCStri(divisor)););
      raise_error(NUMERIC_ERROR);
      remainder = NULL;
    } else {
      ALLOC_BIG(remainder);
      mpz_init(remainder);
      mpz_tdiv_r(remainder, dividend, divisor);
    } /* if */
    logFunction(printf("bigRem --> %s\n", bigHexCStri(remainder)););
    return remainder;
  } /* bigRem */



/**
 *  Shift a 'bigInteger' number right by rshift bits.
 *  If rshift is negative a left shift is done instead.
 *  A >> B is equivalent to A mdiv 2_ ** B if B >= 0 holds.
 *  A >> B is equivalent to A * 2_ ** -B if B < 0 holds.
 *  @return the right shifted number.
 */
bigIntType bigRShift (const const_bigIntType big1, const intType rshift)

  {
    bigIntType result;

  /* bigRShift */
    logFunction(printf("bigRShift(%s, " FMT_D ")\n",
                       bigHexCStri(big1), rshift););
    ALLOC_BIG(result);
    mpz_init(result);
    if (rshift >= 0) {
      mpz_fdiv_q_2exp(result, big1, (uintType) rshift);
#if AVOID_OVERFLOW_IN_MPZ_TYPE
    } else if (unlikely((-(uintType) rshift) / GMP_LIMB_BITS + 1 >
                        GMP_MAX_LIMB_SIZE - mpz_size(big1) &&
                        mpz_sgn(big1) != 0)) {
      mpz_clear(result);
      FREE_BIG(result);
      raise_error(OVERFLOW_IN_MPZ_ERROR);
      result = NULL;
#endif
    } else {
      /* The unsigned value is negated to avoid a signed integer */
      /* overflow if the smallest signed integer is negated.     */
      mpz_mul_2exp(result, big1, -(uintType) rshift);
    } /* if */
    logFunction(printf("bigRShift --> %s\n", bigHexCStri(result)););
    return result;
  } /* bigRShift */



/**
 *  Shift a number right by rshift bits and assign the result back to number.
 *  If rshift is negative a left shift is done instead.
 */
void bigRShiftAssign (bigIntType *const big_variable, intType rshift)

  {
    bigIntType big1;

  /* bigRShiftAssign */
    big1 = *big_variable;
    logFunction(printf("bigRShiftAssign(%s, " FMT_D ")\n",
                       bigHexCStri(big1), rshift););
    if (rshift > 0) {
      mpz_fdiv_q_2exp(big1, big1, (uintType) rshift);
    } else if (rshift == 0) {
      /* do nothing */
#if AVOID_OVERFLOW_IN_MPZ_TYPE
    } else if (unlikely((-(uintType) rshift) / GMP_LIMB_BITS + 1 >
                        GMP_MAX_LIMB_SIZE - mpz_size(big1) &&
                        mpz_sgn(big1) != 0)) {
      raise_error(OVERFLOW_IN_MPZ_ERROR);
#endif
    } else {
      /* The unsigned value is negated to avoid a signed integer */
      /* overflow if the smallest signed integer is negated.     */
      mpz_mul_2exp(big1, big1, -(uintType) rshift);
    } /* if */
    logFunction(printf("bigRShiftAssign --> %s\n", bigHexCStri(big1)););
  } /* bigRShiftAssign */



/**
 *  Compute the subtraction of two 'bigInteger' numbers.
 *  @return the difference of the two numbers.
 */
bigIntType bigSbtr (const const_bigIntType minuend, const const_bigIntType subtrahend)

  {
    bigIntType difference;

  /* bigSbtr */
    ALLOC_BIG(difference);
    mpz_init(difference);
    mpz_sub(difference, minuend, subtrahend);
    return difference;
  } /* bigSbtr */



/**
 *  Decrement a 'bigInteger' variable by a delta.
 *  Subtracts delta from *big_variable.
 */
void bigSbtrAssign (bigIntType *const big_variable, const const_bigIntType delta)

  { /* bigSbtrAssign */
    mpz_sub(*big_variable, *big_variable, delta);
  } /* bigSbtrAssign */



/**
 *  Compute the subtraction of two 'bigInteger' numbers.
 *  Minuend is assumed to be a temporary value which is reused.
 *  @return the difference of the two numbers in 'minuend'.
 */
bigIntType bigSbtrTemp (bigIntType minuend, const_bigIntType subtrahend)

  { /* bigSbtrTemp */
    mpz_sub(minuend, minuend, subtrahend);
    return minuend;
  } /* bigSbtrTemp */



/**
 *  Compute the square of a 'bigInteger'.
 *  This function is used by the compiler to optimize
 *  multiplication and exponentiation operations.
 *  @return the square of big1.
 */
bigIntType bigSquare (const_bigIntType big1)

  {
    bigIntType result;

  /* bigSquare */
    ALLOC_BIG(result);
    mpz_init(result);
    mpz_mul(result, big1, big1);
    return result;
  } /* bigSquare */



/**
 *  Convert a 'bigInteger' number to a string.
 *  The number is converted to a string with decimal representation.
 *  For negative numbers a minus sign is prepended.
 *  @return the string result of the conversion.
 *  @exception MEMORY_ERROR  Not enough memory to represent the result.
 */
striType bigStr (const const_bigIntType big1)

  {
    char *cstri;
    striType result;

  /* bigStr */
    logFunction(printf("bigStr(%s)\n", bigHexCStri(big1)););
    cstri = mpz_get_str(NULL, 10, big1);
    result = cstri_to_stri(cstri);
    free(cstri);
    if (unlikely(result == NULL)) {
      raise_error(MEMORY_ERROR);
    } /* if */
    logFunction(printf("bigStr --> \"%s\"\n", striAsUnquotedCStri(result)););
    return result;
  } /* bigStr */



/**
 *  Successor of a 'bigInteger' number.
 *  succ(A) is equivalent to A+1 .
 *  @return big1 + 1 .
 */
bigIntType bigSucc (const const_bigIntType big1)

  {
    bigIntType successor;

  /* bigSucc */
    ALLOC_BIG(successor);
    mpz_init(successor);
    mpz_add_ui(successor, big1, 1);
    return successor;
  } /* bigSucc */



/**
 *  Successor of a 'bigInteger' number.
 *  Big1 is assumed to be a temporary value which is reused.
 *  @return big1 + 1 .
 */
bigIntType bigSuccTemp (bigIntType big1)

  { /* bigSuccTemp */
    mpz_add_ui(big1, big1, 1);
    return big1;
  } /* bigSuccTemp */



/**
 *  Convert a 'bigInteger' into a big-endian 'bstring'.
 *  The result uses binary representation with a base of 256.
 *  @param big1 BigInteger number to be converted.
 *  @param isSigned Determines the signedness of the result.
 *         If 'isSigned' is TRUE the result is encoded with the
 *         twos-complement representation. In this case a negative
 *         'big1' is converted to a result where the most significant
 *         byte (the first byte) has an ordinal > BYTE_MAX (=127).
 *  @return a bstring with the big-endian representation.
 *  @exception RANGE_ERROR If 'big1' is negative and 'isSigned' is FALSE.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bstriType bigToBStriBe (const const_bigIntType big1, const boolType isSigned)

  {
    size_t sizeInBytes;
    size_t bytesExported;
    size_t pos;
    int sign;
    unsigned int carry;
    ustriType buffer;
    memSizeType charIndex;
    memSizeType result_size;
    bstriType result;

  /* bigToBStriBe */
    sizeInBytes = (mpz_sizeinbase(big1, 2) + CHAR_BIT - 1) / CHAR_BIT;
    buffer = (ustriType) malloc(sizeInBytes);
    if (unlikely(buffer == NULL)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      mpz_export(buffer, &bytesExported, 1, 1, 0, 0, big1);
      sign = mpz_sgn(big1);
      if (isSigned) {
        if (sign < 0) {
          carry = 1;
          pos = bytesExported;
          while (pos > 0) {
            pos--;
            carry += ~buffer[pos] & 0xFF;
            buffer[pos] = (ucharType) (carry & 0xFF);
            carry >>= CHAR_BIT;
          } /* while */
        } /* if */
        result_size = sizeInBytes;
        if ((sign > 0 && buffer[0] > BYTE_MAX) ||
            (sign < 0 && buffer[0] <= BYTE_MAX)) {
          result_size++;
        } /* if */
      } else {
        if (unlikely(sign < 0)) {
          logError(printf("bigToBStriBe(%s, %d): "
                          "Number is negative and 'isSigned' is FALSE.\n",
                          bigHexCStri(big1), isSigned););
          raise_error(RANGE_ERROR);
          free(buffer);
          return NULL;
        } else {
          result_size = sizeInBytes;
        } /* if */
      } /* if */
      if (unlikely(!ALLOC_BSTRI_CHECK_SIZE(result, result_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = result_size;
        if (sign == 0) {
          result->mem[0] = 0;
        } else {
          charIndex = 0;
          if (isSigned) {
            if (sign < 0) {
              if (buffer[0] <= BYTE_MAX) {
                result->mem[charIndex] = UBYTE_MAX;
                charIndex++;
              } /* if */
            } else {
              if (buffer[0] > BYTE_MAX) {
                result->mem[charIndex] = 0;
                charIndex++;
              } /* if */
            } /* if */
          } /* if */
          memcpy(&result->mem[charIndex], buffer, bytesExported);
        } /* if */
      } /* if */
      free(buffer);
    } /* if */
    return result;
  } /* bigToBStriBe */



/**
 *  Convert a 'bigInteger' into a little-endian 'bstring'.
 *  The result uses binary representation with a base of 256.
 *  @param big1 BigInteger number to be converted.
 *  @param isSigned Determines the signedness of the result.
 *         If 'isSigned' is TRUE the result is encoded with the
 *         twos-complement representation. In this case a negative
 *         'big1' is converted to a result where the most significant
 *         byte (the last byte) has an ordinal > BYTE_MAX (=127).
 *  @return a bstring with the little-endian representation.
 *  @exception RANGE_ERROR If 'big1' is negative and 'isSigned' is FALSE.
 *  @exception MEMORY_ERROR Not enough memory to represent the result.
 */
bstriType bigToBStriLe (const const_bigIntType big1, const boolType isSigned)

  {
    size_t sizeInBytes;
    size_t bytesExported;
    size_t pos;
    int sign;
    unsigned int carry;
    ustriType buffer;
    memSizeType result_size;
    bstriType result;

  /* bigToBStriLe */
    sizeInBytes = (mpz_sizeinbase(big1, 2) + CHAR_BIT - 1) / CHAR_BIT;
    buffer = (ustriType) malloc(sizeInBytes);
    if (unlikely(buffer == NULL)) {
      raise_error(MEMORY_ERROR);
      result = NULL;
    } else {
      mpz_export(buffer, &bytesExported, -1, 1, 0, 0, big1);
      sign = mpz_sgn(big1);
      if (isSigned) {
        if (sign < 0) {
          carry = 1;
          pos = 0;
          while (pos < bytesExported) {
            carry += ~buffer[pos] & 0xFF;
            buffer[pos] = (ucharType) (carry & 0xFF);
            carry >>= CHAR_BIT;
            pos++;
          } /* while */
        } /* if */
        result_size = sizeInBytes;
        if ((sign > 0 && buffer[bytesExported - 1] > BYTE_MAX) ||
            (sign < 0 && buffer[bytesExported - 1] <= BYTE_MAX)) {
          result_size++;
        } /* if */
      } else {
        if (unlikely(sign < 0)) {
          logError(printf("bigToBStriLe(%s, %d): "
                          "Number is negative and 'isSigned' is FALSE.\n",
                          bigHexCStri(big1), isSigned););
          raise_error(RANGE_ERROR);
          free(buffer);
          return NULL;
        } else {
          result_size = sizeInBytes;
        } /* if */
      } /* if */
      if (unlikely(!ALLOC_BSTRI_CHECK_SIZE(result, result_size))) {
        raise_error(MEMORY_ERROR);
      } else {
        result->size = result_size;
        if (sign == 0) {
          result->mem[0] = 0;
        } else {
          memcpy(result->mem, buffer, bytesExported);
          if (isSigned) {
            if (sign < 0) {
              if (buffer[bytesExported - 1] <= BYTE_MAX) {
                result->mem[bytesExported] = UBYTE_MAX;
              } /* if */
            } else {
              if (buffer[bytesExported - 1] > BYTE_MAX) {
                result->mem[bytesExported] = 0;
              } /* if */
            } /* if */
          } /* if */
        } /* if */
      } /* if */
      free(buffer);
    } /* if */
    return result;
  } /* bigToBStriLe */



/**
 *  Convert a 'bigInteger' to an 'int16Type' number.
 *  @return the int16Type result of the conversion.
 *  @param big1 BigInteger to be converted.
 *  @param err_info Unchanged if the function succeeds or
 *                  RANGE_ERROR The number is too small or too big
 *                  to fit into a int16Type value.
 */
int16Type bigToInt16 (const const_bigIntType big1, errInfoType *err_info)

  {
    long int result;

  /* bigToInt16 */
    if (unlikely(!mpz_fits_slong_p(big1))) {
      logError(printf("bigToInt16(%s): mpz_fits_slong_p failed.\n",
                      bigHexCStri(big1)););
      *err_info = RANGE_ERROR;
      return 0;
    } else {
      result = mpz_get_si(big1);
#if LONG_SIZE > 16
      if (unlikely(result < INT16TYPE_MIN || result > INT16TYPE_MAX)) {
        *err_info = RANGE_ERROR;
        return 0;
      } /* if */
#endif
      return (int16Type) result;
    } /* if */
  } /* bigToInt16 */



/**
 *  Convert a 'bigInteger' to an 'int32Type' number.
 *  @return the int32Type result of the conversion.
 *  @param big1 BigInteger to be converted.
 *  @param err_info Only used if err_info is not NULL.
 *                  Unchanged if the function succeeds or
 *                  RANGE_ERROR The number is too small or too big
 *                  to fit into a int32Type value.
 *  @exception RANGE_ERROR If err_info is NULL and the number is
 *             too small or too big to fit into a int32Type value.
 */
int32Type bigToInt32 (const const_bigIntType big1, errInfoType *err_info)

  {
    long int result;

  /* bigToInt32 */
    logFunction(printf("bigToInt32(%s)\n", bigHexCStri(big1)););
    if (unlikely(!mpz_fits_slong_p(big1))) {
      logError(printf("bigToInt32(%s): mpz_fits_slong_p failed.\n",
                      bigHexCStri(big1)););
      if (err_info == NULL) {
        raise_error(RANGE_ERROR);
      } else {
        *err_info = RANGE_ERROR;
      } /* if */
      result = 0;
    } else {
      result = mpz_get_si(big1);
#if LONG_SIZE > 32
      if (unlikely(result < INT32TYPE_MIN || result > INT32TYPE_MAX)) {
        if (err_info == NULL) {
          raise_error(RANGE_ERROR);
        } else {
          *err_info = RANGE_ERROR;
        } /* if */
        result = 0;
      } /* if */
#endif
    } /* if */
    logFunction(printf("bigToInt32 --> " FMT_D32 "\n", (int32Type) result););
    return (int32Type) result;
  } /* bigToInt32 */



#ifdef INT64TYPE
/**
 *  Convert a 'bigInteger' to an 'int64Type' number.
 *  @return the int64Type result of the conversion.
 *  @param big1 BigInteger to be converted.
 *  @param err_info Only used if err_info is not NULL.
 *                  Unchanged if the function succeeds or
 *                  RANGE_ERROR The number is too small or too big
 *                  to fit into a int64Type value.
 *  @exception RANGE_ERROR If err_info is NULL and the number is
 *             too small or too big to fit into a int64Type value.
 */
int64Type bigToInt64 (const const_bigIntType big1, errInfoType *err_info)

  {
    int64Type result;

  /* bigToInt64 */
    logFunction(printf("bigToInt64(%s)\n", bigHexCStri(big1)););
#if LONG_SIZE == 64
    if (unlikely(!mpz_fits_slong_p(big1))) {
      logError(printf("bigToInt64(%s): mpz_fits_slong_p failed.\n",
                      bigHexCStri(big1)););
      if (err_info == NULL) {
        raise_error(RANGE_ERROR);
      } else {
        *err_info = RANGE_ERROR;
      } /* if */
      result = 0;
    } else {
      result = mpz_get_si(big1);
    } /* if */
#else
    {
      mpz_t help;

      mpz_init_set(help, big1);
      mpz_fdiv_q_2exp(help, help, 32);
      if (unlikely(!mpz_fits_slong_p(help))) {
        logError(printf("bigToInt64(%s): mpz_fits_slong_p failed.\n",
                        bigHexCStri(big1)););
        if (err_info == NULL) {
          raise_error(RANGE_ERROR);
        } else {
          *err_info = RANGE_ERROR;
        } /* if */
        result = 0;
      } else {
        result = (int64Type) (mpz_get_si(help) & 0xFFFFFFFF) << 32 |
                 (int64Type) (mpz_get_si(big1) & 0xFFFFFFFF);
        mpz_clear(help);
      } /* if */
    }
#endif
    logFunction(printf("bigToInt64 --> " FMT_D64 "\n", result););
    return result;
  } /* bigToInt64 */



/**
 *  Convert a 'bigInteger' to an 'uint64Type' number.
 *  @return the uint64Type result of the conversion.
 *  @exception RANGE_ERROR The number is negative or too big to fit
 *             into a uint64Type value.
 */
uint64Type bigToUInt64 (const const_bigIntType big1)

  {
    uint64Type result;

  /* bigToUInt64 */
    logFunction(printf("bigToUInt64(%s)\n", bigHexCStri(big1)););
#if LONG_SIZE == 64
    if (unlikely(!mpz_fits_ulong_p(big1))) {
      logError(printf("bigToUInt64(%s): mpz_fits_ulong_p failed.\n",
                      bigHexCStri(big1)););
      raise_error(RANGE_ERROR);
      result = 0;
    } else {
      result = mpz_get_ui(big1);
    } /* if */
#else
    {
      mpz_t help;

      mpz_init_set(help, big1);
      mpz_fdiv_q_2exp(help, help, 32);
      if (unlikely(!mpz_fits_ulong_p(help))) {
        logError(printf("bigToUInt64(%s): mpz_fits_ulong_p failed.\n",
                        bigHexCStri(big1)););
        raise_error(RANGE_ERROR);
        result = 0;
      } else {
        result = (uint64Type) (mpz_get_ui(help) & 0xFFFFFFFF) << 32 |
                 (uint64Type) (mpz_get_ui(big1) & 0xFFFFFFFF);
        mpz_clear(help);
      } /* if */
    }
#endif
    logFunction(printf("bigToUInt64(%s) --> " FMT_U64 "\n",
                       bigHexCStri(big1), result););
    return result;
  } /* bigToUInt64 */
#endif



bigIntType bigXor (const_bigIntType big1, const_bigIntType big2)

  {
    bigIntType result;

  /* bigXor */
    ALLOC_BIG(result);
    mpz_init(result);
    mpz_xor(result, big1, big2);
    return result;
  } /* bigXor */



bigIntType bigZero (void)

  {
    bigIntType result;

  /* bigZero */
    ALLOC_BIG(result);
    mpz_init_set_ui(result, 0);
    return result;
  } /* bigZero */

#endif

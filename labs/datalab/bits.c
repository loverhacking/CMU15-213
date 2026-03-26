/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  // De Morgan's laws: ~(x | y) = ~x & ~y
  return ~((~(~x & y)) & ~(x & ~y));
}
/*
 * tmin - return minimum two's complement integer
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  return 1 << 31;
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  // note: if x is Tmax(0x7fffffff), ~x = x + 1 = 0x80000000
  // and exclude the special case if x == -1
  return !((x + 1) ^ ~x) & !!(x + 1);
}
/*
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  int mask = 0xAA;
  mask |= (mask << 8); // mask = 0x0000AAAA
  mask |= (mask << 16); // mask = 0xAAAAAAAA
  return !((x & mask) ^ mask);
}
/*
 * negate - return -x
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x + 1;
}
//3
/*
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  int lastOnebyte = x & 0xf;
  int bit12 = lastOnebyte & 6;
  // high byte should be 0x30, and low byte should either the 3th bit be 0, or
  // the 1th bit and 2th bit shouble be 0 (0000 - 1001).
  return (!((x >> 4) ^ 0x3)) & (!(lastOnebyte & 8) | !bit12);
}
/*
 * conditional - same as x ? y : z
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  int con = !!x + (~1 + 1); // x = 0, con = -1(0xffffffff); x !=0, con = 0;
  return (~con & y) | (con & z);
}
/*
 * isLessOrEqual - if x <= y  then return 1, else return 0
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  int signX = (x >> 31) & 1;
  int signY = (y >> 31) & 1;
  int signDiff = signX ^ signY;
  int sub = y + (~x + 1); // y - x;
  int subSign = (sub >> 31) & 1;
  // x and y do not have the same sign, x < 0, then x <= y
  // otherwise, sub >= 0, then x <= y
  return (signDiff & signX) | (!signDiff & !subSign);
}
//4
/*
 * logicalNeg - implement the ! operator, using all of
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4
 */
int logicalNeg(int x) {
  // note: for any nonzero value, either the x or the -x should have the 1
  // at the most significant bit, so (x | negX) >> 31 will be -1 for nonzero value
  // and be 0 for x == 0.
  int negX = ~x + 1;
  int sign = (x | negX) >> 31;
  return sign + 1;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
   int sign = x >> 31;
   int b16, b8, b4, b2, b1, b0;

   // NOTE: for negative value, the bitwise representation like: 111...0..
   // There exist many sign extension 1 which we do not care.
   // We do only care the first value behind the sign extend bit, so we reverse the value bit by bit
   // like, 000...1.., and find the left most 1 position, just like what we deal with positive value.
   x = (sign & ~x) | (~sign & x); // x < 0, x = ~x, otherwise x remains unchanged.

   // Binary search:
   // Is the highest bit (bit 1) located in the high 16 bits? Yes: Add at least 16 bits to the result.
   // In the remaining range, is the highest bit (bit 1) in the high 8 bits? Yes: Add another 8 bits
   // and check the high 4 bits. Yes: Add another 4 bits
   // and check the high 2 bits. Yes: Add another 2 bits
   // and check the high 1 bit. Yes: Add another 1 bit.
   // The sum of these steps determines the position of the highest bit (bit 1).
   b16 = !!(x >> 16) << 4;
   b8  = !!(x >> (b16 + 8)) << 3;
   b4  = !!(x >> (b16 + b8 + 4)) << 2;
   b2  = !!(x >> (b16 + b8 + b4 + 2)) << 1;
   b1  = !!(x >> (b16 + b8 + b4 + b2 + 1));
   b0  = !!(x >> (b16 + b8 + b4 + b2 + b1));

   // do not forget the sign bit.
   return b16 + b8 + b4 + b2 + b1 + b0 + 1;
}
//float
/*
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  // 32bits float point: 1(sign bit) - 8(exp) - 23(frac)
  unsigned int frac = uf & 0x7fffff;
  unsigned int exp = (uf >> 23) & 0xff;
  unsigned int sign = uf >> 31;
  if (exp == 0xff) {
    // uf is Nan or Inf
    return uf;
  }

  // non-nomial number
  if (exp == 0) {
    frac = frac << 1;
    return (sign << 31) | frac;
  }

  exp += 1;
  if (exp == 0xff) {
    frac = 0; // Inf
  }
  return (sign << 31) | (exp << 23) | frac;
}
/*
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
  // 32bits float point: 1(sign bit) - 8(exp) - 23(frac)
  int frac = uf & 0x7fffff;
  int exp = (uf >> 23) & 0xff;
  int sign = uf >> 31;

  // do not forget the bias(2^(8-1)-1=127)
  int pow = exp + (~127 + 1);

  // value = (1.frac) * 2^(pow) = (1frac) * 2^23 * 2^pow = (1frac) * 2^(23-pow);
  int m = (1 << 23) | frac;
  int value = 0;

  if (exp == 0xff) {
    // uf is Nan or Inf
    return 0x80000000u;
  }

  // the abs value < 1
  if (pow < 0) {
    return 0;
  }

  // the 32-bit int [-2^31, 2^31 - 1]
  if (pow >= 31) {
    return 0x80000000u;
  }

  if (pow >= 23) {
    value = m << (pow - 23);
  } else {
    value = m >> (23 - pow);
  }

  return sign ? -value : value;
}
/*
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 *
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatPower2(int x) {
  int exp;
  int frac;
  int sign = 0;

  // note: the small value for 32bit float point:
  // nonimal value 0.0..01 * 2^(1-bias) = 2^(-23) * 2^(-126) = 2^-149
  if (x < -149) {
    return 0;
  }

  // too big for 32bit float point(Inf)
  if (x > 127) {
    exp = 0xff;
    frac = 0;
    return (sign << 31) | (exp << 23);
  }

  // non-nominal: 0.frac * 2^(−126) = frac * 2^(-23) * 2^(-126)
  // = frac * 2^(-149) = 2^x
  // => frac = 2^(x+149)
  if (x < -126) {
    frac = 1 << (x + 149);
    exp = 0;
    return (sign << 31) | (exp << 23) | frac;
  }

  // nominal: 1.0 * 2^x
  frac = 0;
  exp = x + 127;
  return (sign << 31) | (exp << 23) | frac;
}

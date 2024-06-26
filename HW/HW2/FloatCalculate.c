#include "FloatCalculate.h"
#include <stdbool.h>
#include <stdint.h>

const size_t SIGN_BIT = 1;
const size_t EXPONENT_BITS = 8;
const size_t MANTISSA_BITS = 23;

// static int32_t get_norm_bias(void) { return 1 - (1 << (EXPONENT_BITS - 1)); }

// static int32_t get_denorm_bias(void) { return 1 + get_norm_bias(); }

// static bool test_rightmost_all_zeros(uint32_t number, size_t bits)
// {
//   uint32_t mask = (1ull << bits) - 1;
//   return (number & mask) == 0;
// }

// static bool test_rightmost_all_ones(uint32_t number, size_t bits)
// {
//   uint32_t mask = (1ull << bits) - 1;
//   return (number & mask) == mask;
// }

// You can also design a function of your own.
static void build_bitstring(Float input, char *output)
{
  output[0] = input.sign > 0 ? '1' : '0';
  for (int i = 7; i >= 0; --i)
  {
    output[8 - i] = ((input.exponent >> i) & 1) > 0 ? '1' : '0';
  }
  for (int i = 22; i >= 0; --i)
  {
    output[31 - i] = ((input.mantissa >> i) & 1) > 0 ? '1' : '0';
  }
  output[32] = '\0';
}

// You can also design a function of your own.
static Float parse_bitstring(const char *input)
{
  Float f;
  f.sign = input[0] == '1' ? 1 : 0;
  f.exponent = 0;
  for (int i = 1; i <= 8; ++i)
  {
    f.exponent = (f.exponent << 1) | (input[i] == '1' ? 1 : 0);
  }
  f.mantissa = 0;
  for (int i = 9; i < 32; ++i)
  {
    f.mantissa = (f.mantissa << 1) | (input[i] == '1' ? 1 : 0);
  }
  if (f.exponent == 0 && f.mantissa != 0)
  {
    f.mantissa = f.mantissa << 3;
    f.exponent += 1;
    return f;
  }
  uint32_t hide = 0x800000;
  f.mantissa = f.mantissa | hide;
  f.mantissa = f.mantissa << 3;
  return f;
}

// You can also design a function of your own.
static Float float_add_impl(Float a, Float b)
{
  Float res;
  res.sign = 0;
  res.exponent = 0;
  res.mantissa = 0;
  uint32_t temp;
  uint32_t flag = 0;

  // for testcase 10
  // if ((a.exponent == 0 && b.exponent == 1) || (a.exponent == 1 && b.exponent == 0) || (a.exponent == 0 && b.exponent == 0))
  // {
  //   if (a.sign == b.sign)
  //   {
  //     res.mantissa = a.mantissa + b.mantissa;
  //     res.sign = a.sign;
  //   }
  //   else
  //   {
  //     if (a.mantissa > b.mantissa)
  //     {
  //       res.mantissa = a.mantissa - b.mantissa;
  //       res.sign = a.sign;
  //     }
  //     else
  //     {
  //       res.mantissa = b.mantissa - a.mantissa;
  //       res.sign = b.sign;
  //     }
  //   }
  //   res.exponent = 1;

  //   while ((res.mantissa >> 26) > 1)
  //   {
  //     res.mantissa >>= 1;
  //     res.exponent++;
  //   }

  //   while (((res.mantissa >> 26) == 0))
  //   {
  //     res.mantissa <<= 1;
  //     if (res.exponent != 0)
  //     {
  //       res.exponent--;
  //     }
  //     else
  //     {
  //       res.mantissa = res.mantissa >> 1;
  //       return res;
  //     }
  //   }
  //   return res;
  // }

  // shift the exponent
  if (a.exponent > b.exponent)
  {
    temp = a.exponent - b.exponent;
    for (uint32_t i = 0; i < temp; ++i)
    {
      b.mantissa = b.mantissa >> 1;
      if ((b.mantissa & 0x1) == 1)
      {
        flag = 1;
      }
    }
    if (flag == 1)
    {
      b.mantissa = b.mantissa | 0x1;
    }
    res.exponent = a.exponent;
  }
  else if (a.exponent < b.exponent)
  {
    temp = b.exponent - a.exponent;
    for (uint32_t i = 0; i < temp; ++i)
    {
      a.mantissa = a.mantissa >> 1;
      if ((a.mantissa & 0x1) == 1)
      {
        flag = 1;
      }
    }
    if (flag == 1)
    {
      a.mantissa = a.mantissa | 0x1;
    }
    res.exponent = b.exponent;
  }
  else
  {
    res.exponent = a.exponent;
  }

  // add the mantissa
  if (a.sign == b.sign)
  {
    res.mantissa = a.mantissa + b.mantissa;
    res.sign = a.sign;
  }
  else
  {
    if (a.mantissa > b.mantissa)
    {
      res.mantissa = a.mantissa - b.mantissa;
      res.sign = a.sign;
    }
    else
    {
      res.mantissa = b.mantissa - a.mantissa;
      res.sign = b.sign;
    }
  }

  // normalize
  while ((res.mantissa >> 26) > 1)
  {
    res.mantissa >>= 1;
    res.exponent++;
  }

  while (((res.mantissa >> 26) == 0))
  {
    res.mantissa <<= 1;
    if (res.exponent != 0)
    {
      res.exponent--;
    }
    else
    {
      res.mantissa = res.mantissa >> 1;
      return res;
    }
  }
  if(res.exponent==0)
  {
    res.mantissa = res.mantissa >> 1;
  }

  // round
  res.mantissa = res.mantissa >> 3;

  return res;
}

// You should not modify the signature of this function
void float_add(const char *a, const char *b, char *result)
{
  // TODO: Implement this function
  // A possible implementation of the function:
  Float fa = parse_bitstring(a);
  Float fb = parse_bitstring(b);
  Float fresult = float_add_impl(fa, fb);
  build_bitstring(fresult, result);
}

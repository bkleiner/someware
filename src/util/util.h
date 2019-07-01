#pragma once

#include <cstring>

#include "math.h"
#include "vector.h"
#include "buffer.h"
#include "ring_buffer.h"

#define FAST_FTOA_PRECISSION 2

namespace util {

  template <uint8_t... values>
  struct bit_pack;

  template <uint8_t first, uint8_t... others>
  struct bit_pack<first, others...>
  {
    static_assert(sizeof...(others) < 4, "number of types must be less then 4");
    static const uint32_t value = (first << ((sizeof...(others)) * 8)) | bit_pack<others...>::value;
    
    static inline uint8_t get(uint8_t index) {
      static_assert(sizeof...(others) < 4, "number of types must be less then 4");
      //return value & uint32_t(0xFF << ((sizeof...(others) - index) * 8));
      return (value >> ((sizeof...(others) - index) * 8)) & 0xFF;
    }
  };

  template <>
  struct bit_pack<>
  {
    static const uint32_t value = 0;
  };

  template <uint32_t... values>
  struct disjunction;

  template <uint32_t first, uint32_t... others>
  struct disjunction<first, others...>
  {
      static_assert(first < 32, "maximum 32 bits can be used");
      static const uint32_t value = first | disjunction<others...>::value;
  };

  template <>
  struct disjunction<>
  {
      static const uint32_t value = 0;
  };


  template <uint8_t... values>
  struct flag_disjunction;

  template <uint8_t first, uint8_t... others>
  struct flag_disjunction<first, others...>
  {
      static_assert(first < 32, "maximum 32 bits can be used");
      static const uint32_t value = (1 << first) | flag_disjunction<others...>::value;
  };

  template <>
  struct flag_disjunction<>
  {
      static const uint32_t value = 0;
  };

  static void fast_ftoa(char* str, float v) {
    uint32_t value = v * powf(10, FAST_FTOA_PRECISSION);
    uint32_t index = 0;

    while (value != 0) {
      if (index == FAST_FTOA_PRECISSION) {
        str[index++] = '.';
      }

      uint8_t digit = value % 10;
      str[index++] = '0' + digit;
      value /= 10;
    }

    for (uint32_t i = 0; i < index; i++) {
      const uint32_t start = i;
      const uint32_t end = index - i - 1;
      if ((end - start) <= 1) {
        break;
      }
      uint8_t tmp = str[end];
      str[end] = str[start];
      str[start] = tmp;
    }
    str[index+1] = 0;
  }
}
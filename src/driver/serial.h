#pragma once

#include <cstdio>
#include <cstdarg>

#include "util/util.h"

class serial {
public:
  virtual buffer<uint8_t> read() = 0;
  virtual bool write(uint8_t) = 0;
  virtual bool flush() {
    return false;
  };

  void print(const char* str) {
    for(uint32_t pos = 0; str[pos] != 0; ++pos) {
      if (!write(str[pos])) {
        while (!flush()) {}
        pos--;
      }
    }
  }

  void printf(const char* fmt, ...) {
    const size_t size = strlen(fmt) + 128;
    char str[size];

    memset(str, 0, size);

    va_list args;
    va_start(args, fmt);
    vsnprintf(str, size, fmt, args);
    va_end(args);

    print(str);
  }
};
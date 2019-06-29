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

  void print(const char* str);
  void printf(const char* fmt, ...);
};
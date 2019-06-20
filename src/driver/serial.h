#pragma once

#include "util/util.h"

class serial {
public:
  virtual buffer<uint8_t> read() = 0;
  virtual bool write(uint8_t) = 0;
  virtual bool flush() {};
};
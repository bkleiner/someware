#pragma once

#include <cstdint>

#include "util/vector.h"
#include "util/buffer.h"

class flash_memory {
public:
  virtual void write(const buffer<uint32_t>&) = 0;
  virtual uint8_t* pointer();
};
#pragma once

#include <cstdint>

namespace adc {
  
  enum names {
    DEVICE1,
    DEVICE2,
    DEVICE3,
    DEVICE4,
  };

  struct adc {
    virtual uint16_t read() = 0;
  }; 
}

#pragma once

#include <cstdint>

namespace pwm {
  struct pwm {
    virtual void set(float v) = 0;
  }; 
}

#pragma once

#include "util/vector.h"
#include "util/buffer.h"

namespace accel {

  class accel {
  public:
    virtual void calibrate() = 0;
    
    virtual float read_temparture() = 0;
    virtual vector read_gyro() = 0;
    virtual buffer<int16_t> read_accel() = 0;
  };

}
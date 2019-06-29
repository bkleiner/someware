#pragma once

#include "util/vector.h"
#include "util/buffer.h"

namespace accel {

  class accel {
  public:
    virtual vector calibrate_gyro() = 0;
    virtual vector calibrate_accel() = 0;
    
    virtual float read_temparture() = 0;
    virtual vector read_gyro() = 0;
    virtual vector read_accel() = 0;

    virtual void set_gyro_bias(const vector&) = 0;
    virtual void set_accel_bias(const vector&) = 0;
  };

}
#pragma once

#include "driver/spi.h"
#include "driver/gpio.h"
#include "driver/accel.h"

namespace accel {
  class mpu_6000 : public accel {
  public:
    mpu_6000(spi* bus, gpio::pin* cs);

    vector calibrate() override;

    float read_temparture() override;
    vector read_gyro() override;
    buffer<int16_t> read_accel() override;

    void set_bias(const vector& bias) {
      gyro_bias = bias;
    }

  private:
    spi* bus;
    gpio::pin* cs;
    vector gyro_bias;
  };
}


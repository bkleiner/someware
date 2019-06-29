#pragma once

#include "driver/spi.h"
#include "driver/gpio.h"
#include "driver/accel.h"

namespace accel {
  class mpu_6000 : public accel {
  public:
    mpu_6000(spi* bus, gpio::pin* cs);

    vector calibrate_gyro() override;
    vector calibrate_accel() override;

    float read_temparture() override;
    vector read_gyro() override;
    vector read_accel() override;

    void set_gyro_bias(const vector& bias) {
      gyro_bias = bias;
    }

    void set_accel_bias(const vector& bias) {
      accel_bias = bias;
    }

  private:
    spi* bus;
    gpio::pin* cs;
    
    vector gyro_bias;
    vector accel_bias;
  };
}


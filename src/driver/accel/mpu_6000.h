#pragma once

#include "util/util.h"

#include "driver/spi.h"
#include "driver/gpio.h"

class mpu_6000 {
public:
  mpu_6000(spi* bus, gpio::pin* cs);

  void calibrate();

  float read_temparture();
  buffer<float> read_gyro();
  buffer<int16_t> read_accel();
  float gyro_bias[3];

private:
  spi* bus;
  gpio::pin* cs;

};
#pragma once

#include "driver/spi.h"

#include "gpio.h"

namespace stm32_f3 {
  
  class spi : public ::spi {
  public:
    spi();

    void set_divisor(::spi::clock_divider divisor);
    uint8_t transfer(uint8_t v);
  };
  
}


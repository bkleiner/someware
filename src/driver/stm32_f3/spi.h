#pragma once

#include "driver/spi.h"

#include "gpio.h"

namespace stm32_f3 {
  
  class spi : public ::spi {
  public:
    using mode_cs_pin = gpio::mode<
      gpio::modes::output,
      gpio::modes::push_pull,
      gpio::modes::speed_50MHz,
      gpio::modes::no_pull
    >;

    spi();

    void set_divisor(::spi::clock_divider divisor);
    uint8_t transfer(uint8_t v);
  };
  
}


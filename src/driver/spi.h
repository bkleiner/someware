#pragma once

#include <cstdint>

#include "gpio.h"
#include "util/buffer.h"

class spi {
public:
  enum clock_divider {
    SPI_CLOCK_INITIALIZATON = 256,
    SPI_CLOCK_SLOW          = 128, //00.56250 MHz
    SPI_CLOCK_STANDARD      = 4,   //09.00000 MHz
    SPI_CLOCK_FAST          = 2,   //18.00000 MHz
    SPI_CLOCK_ULTRAFAST     = 2    //18.00000 MHz
  };

  virtual void set_divisor(clock_divider divisor) = 0;
  virtual uint8_t transfer(uint8_t) = 0;

  uint8_t bus_read_register(gpio::pin* cs, uint8_t reg);
  uint8_t bus_write_register(gpio::pin* cs, uint8_t reg, uint8_t data);

  buffer<uint8_t> bus_read_register_buffer(gpio::pin* cs, uint8_t reg, size_t size);
};
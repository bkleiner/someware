#pragma once

#include "driver/serial.h"

namespace stm32_f3 {

  class serial_usart : public serial {
  public:
    serial_usart();

    buffer<uint8_t> read();
    bool write(uint8_t);
  };
  
}


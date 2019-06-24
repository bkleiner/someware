#pragma once

#include "driver/serial.h"
#include "util/util.h"

namespace stm32_f3 {

  class serial_usart : public serial {
  public:
    serial_usart();

    buffer<uint8_t> read();
    bool write(uint8_t);

  private:
    ring_buffer<uint8_t> rx_buf;

    void isr();
  };
  
}


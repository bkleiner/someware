#pragma once

#include "driver/serial.h"

namespace stm32_f3 {
  class serial_vcp : public serial {
  public:
    serial_vcp();

    buffer<uint8_t> read();
    bool write(uint8_t);
    bool flush();
    bool is_active();

  private:
    void tx_callback();

    ring_buffer<uint8_t> tx_buf;
  };
}
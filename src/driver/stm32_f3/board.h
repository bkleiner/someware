#pragma once

#include "driver/board.h"

#include "serial_vcp.h"

namespace stm32_f3 {

  class rc_controller {
  public:
    rc_controller();
  };

  class board : public ::board {
  public:
    board();

    bool usb_serial_active() override;
    serial& usb_serial() override {
      return vcp;
    }

    float millis() override;
    uint64_t micros() override;
    
    void reset() override;
    void reset_to_bootloader() override;

  private:
    rc_controller rcc;
    serial_vcp vcp;
  };
}

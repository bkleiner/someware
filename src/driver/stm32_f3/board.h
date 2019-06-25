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
    uint32_t micros() override;
    
    void delay_us(uint64_t us) override;
    
    uint32_t cycles();

    void reset() override;
    void reset_to_bootloader() override;

  private:
    uint64_t last_cycles = 0;

    rc_controller rcc;
    serial_vcp vcp;
  };
}

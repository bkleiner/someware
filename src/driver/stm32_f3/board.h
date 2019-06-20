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

    bool usb_serial_active();
    serial_vcp& usb_serial() {
      return vcp;
    }
    
    void reset() override;
    void reset_to_bootloader() override;

  private:
    rc_controller rcc;
    serial_vcp vcp;
  };
}

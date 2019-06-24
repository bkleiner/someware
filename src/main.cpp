#include "target/betaflight_f3/betaflight_f3.h"

#include "control/console.h"
#include "control/control.h"

#include "rx/sbus.h"

int main() {
  control::console cli;
  control::control ctrl;

  stm32_f3::betaflight_f3 board;

  rx::sbus sbus(172, 1811);

  while (1) {
    sbus.update(board.uart2);
    ctrl.update(board, sbus);

    if (board.usb_serial_active()) {
      cli.update(board, sbus, ctrl);
    }
  }
}
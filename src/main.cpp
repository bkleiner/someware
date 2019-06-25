#include "target/betaflight_f3/betaflight_f3.h"

#include "control/console.h"
#include "control/control.h"

#include "rx/sbus.h"

int main() {
  stm32_f3::betaflight_f3 board;

  control::console cli;
  control::control ctrl(&board);

  rx::sbus sbus(172, 1811);

  float last_time = board.millis();
  while (1) {
    const float dt = float(board.millis() - last_time) / 1000.f;

    sbus.update(board.uart2);
    ctrl.update(dt, sbus);

    if (board.usb_serial_active()) {
      cli.update(dt, board, sbus, ctrl);
    }

    last_time = board.millis();
  }
}
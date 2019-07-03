#include "target/betaflight_f3/betaflight_f3.h"

#include "control/console.h"
#include "control/control.h"

#include "rx/sbus.h"

int main() {
  stm32_f3::betaflight_f3 board;

  control::console cli(&board);
  control::control ctrl(&board);

  rx::sbus sbus;

  uint32_t last_time = board.micros() - LOOP_TIME; // make sure first dt is > 0
  while (true) {
    const uint32_t start = board.micros();
    const uint32_t delta = start - last_time;
    const float dt = float(delta) * 0.001f;
    last_time = start;

    {
      sbus.update(board.uart2);
      ctrl.update(dt, sbus);

      if (board.usb_serial_active()) {
        cli.update(dt, sbus, ctrl);
      }
    }
    {
      const uint32_t now = board.micros();
      const uint32_t delta = (now - start);

      if (delta > 0 && delta < LOOP_TIME)
        board.delay_us(LOOP_TIME - delta);
    }
  }
}
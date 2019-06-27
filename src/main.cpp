#include "target/betaflight_f3/betaflight_f3.h"

#include "control/console.h"
#include "control/control.h"

#include "rx/sbus.h"

#define LOOP_FREQ_HZ 2000
#define LOOP_TIME (1000000 / LOOP_FREQ_HZ)

int main() {
  stm32_f3::betaflight_f3 board;

  control::console cli(&board);
  control::control ctrl(&board);

  rx::sbus sbus(172, 1811);

  auto last_time = board.micros();
  while (true) {
    const auto start = board.micros();
    const float dt = float(start - last_time) * 0.001f;
    last_time = start;

    {
      sbus.update(board.uart2);
      ctrl.update(dt, sbus);

      if (board.usb_serial_active()) {
        cli.update(dt, sbus, ctrl);
      }
    }
    {
      const auto now = board.micros();
      const auto delta = (now - start);

      if (board.usb_serial_active()) {
        // board.usb_serial().printf(
        //   "dt: %fms (%.2fkHz) now: %u start: %u last_time: %u delta: %u delay: %d\r\n",
        //   dt, (1000.0f / dt) * 0.001f, now, start, last_time, delta, LOOP_TIME - delta
        // );
      }

      if (delta > 0 && delta < LOOP_TIME)
        board.delay_us(LOOP_TIME - delta);
    }
  }
}
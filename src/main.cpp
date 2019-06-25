#include "target/betaflight_f3/betaflight_f3.h"

#include "control/console.h"
#include "control/control.h"

#include "rx/sbus.h"

#define LOOPTIME 500

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
      // util::timer tick_timer(&board, "tick");
      sbus.update(board.uart2);
      ctrl.update(dt, sbus);

      if (board.usb_serial_active()) {
        cli.update(dt, sbus, ctrl);
      }
    }
    {
      const auto now = board.micros();
      const auto delta = (now - start);
      // if (board.usb_serial_active() && (start % 25000) == 0)
      //   board.usb_serial().printf(
      //     "dt: %fms (%.2fkHz) now: %u start: %u last_time: %u delta: %u delay: %d\r\n",
      //     dt, (1000.0f / dt) * 0.001f, now, start, last_time, delta, LOOPTIME - delta);
      if (delta > 100 && delta < LOOPTIME)
        board.delay_us(LOOPTIME - delta);
    }
  }
}
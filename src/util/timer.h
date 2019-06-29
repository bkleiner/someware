#pragma once

#include "driver/board.h"
#include "platform/time.h"

namespace util {
  struct timer {
    timer(board* brd, const char* name) 
      : brd(brd)
      , name(name)
      , start(0)
      , end(0)
    {
      start = brd->micros();
    }

    ~timer() {
      end = brd->micros();

      if (brd->usb_serial_active())
        brd->usb_serial().printf(
          "timer: %s took %fms (%dus)\r\n",
          name,
          delta() / 1000.f,
          delta()
        );
    }

    uint64_t delta() {
      return end - start;
    }

    board* brd;
    const char* name;
    uint64_t start;
    uint64_t end;
  };

  template<uint32_t step, class T>
  void trigger_every(uint32_t time, T fn) {
    static uint32_t last_time = 0;
    if ((time - last_time) >= step) {
      fn();
      last_time = time;
    }
  }
}
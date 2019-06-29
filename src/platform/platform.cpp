#include "time.h"

#include "locator.h"
#include "driver/board.h"

using plt = platform::locator;

board* plt::_brd;

namespace platform {

  void print(const char* str) {
    if (plt::brd()->usb_serial_active()) {
      plt::brd()->usb_serial().print(str);
      plt::brd()->usb_serial().print("\r\n");
    }
  }

  void printf(const char* fmt, ...) {
    const size_t size = strlen(fmt) + 64;
    char str[size];

    memset(str, 0, size);

    va_list args;
    va_start(args, fmt);
    vsnprintf(str, size, fmt, args);
    va_end(args);

    print(str);
  }

  namespace time {
    uint32_t now_us() {
      return plt::brd()->micros();
    }

    float now_ms() {
      return plt::brd()->millis();
    }

    void delay_us(uint64_t v) {
      plt::brd()->delay_us(v);
    }

    void delay_ms(uint64_t v) {
      delay_us(v * 1000);
    }
  }
}
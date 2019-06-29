#include "time.h"

#include "locator.h"
#include "driver/board.h"

board* platform::locator::_brd;

namespace platform::time {

  uint32_t now_us() {
    return ::platform::locator::brd()->micros();
  }

  float now_ms() {
    return ::platform::locator::brd()->millis();
  }

  void delay_us(uint32_t v) {
    return ::platform::locator::brd()->delay_us(v);
  }
  void delay_ms(uint32_t v) {
    return ::platform::locator::brd()->delay_us(v * 1000);
  }
  
}
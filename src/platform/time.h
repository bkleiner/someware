#pragma once

#include <cstdint>

namespace platform::time {
  uint32_t now_us();
  float now_ms();

  void delay_us(uint64_t v);
  void delay_ms(uint64_t v);
}

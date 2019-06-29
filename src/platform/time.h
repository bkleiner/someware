#pragma once

#include <cstdint>

namespace platform::time {
  uint32_t now_us();
  float now_ms();

  void delay_us(uint32_t v);
  void delay_ms(uint32_t v);
}

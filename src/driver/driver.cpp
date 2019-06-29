#include "board.h"
#include "spi.h"

#include "platform/locator.h"

board::board() {
  platform::locator::provide(this);
}

void serial::print(const char* str) {
  for(uint32_t pos = 0; str[pos] != 0; ++pos) {
    if (!write(str[pos])) {
      while (!flush()) {}
      pos--;
    }
  }
}

void serial::printf(const char* fmt, ...) {
  const size_t size = strlen(fmt) + 128;
  char str[size];

  memset(str, 0, size);

  va_list args;
  va_start(args, fmt);
  vsnprintf(str, size, fmt, args);
  va_end(args);

  print(str);
}

uint8_t spi::bus_write_register(gpio::pin* cs, uint8_t reg, uint8_t data) {
  cs->low();
  transfer(reg);
  auto val = transfer(data);
  cs->high();

  return val;
}

uint8_t spi::bus_read_register(gpio::pin* cs, uint8_t reg) {
  return bus_write_register(cs, reg | 0x80, 0xFF);
}

buffer<uint8_t> spi::bus_read_register_buffer(gpio::pin* cs, uint8_t reg, size_t size) {
  buffer<uint8_t> buf(size);

  cs->low();
  transfer(reg | 0x80);
  for (size_t i = 0; i < size; i++) {
    buf[i] = transfer(0xFF);
  }
  cs->high();

  return buf;
}
#include <cstdio>
#include <cstdarg>

#include <stm32f30x.h>
#include <stm32f30x_conf.h>

#include "driver/accel/mpu_6000.h"
#include "driver/stm32_f3/board.h"
#include "driver/stm32_f3/spi.h"
#include "driver/stm32_f3/gpio.h"
#include "driver/stm32_f3/timer_channel.h"
#include "driver/stm32_f3/serial_usart.h"

#include "util/util.h"
#include "rx/sbus.h"

void write_string(serial& srl, const char* str) {
  for(uint32_t pos = 0; str[pos] != 0; ++pos) {
    if (!srl.write(str[pos])) {
      while (!srl.flush()) {}
      pos--;
    }
  }
}

void write_printf(serial& srl, const char* fmt, ...) {
  const size_t size = strlen(fmt) + 128;
  char str[size];

  memset(str, 0, size);

  va_list args;
  va_start(args, fmt);
  vsnprintf(str, size, fmt, args);
  va_end(args);

  write_string(srl, str);
}

void print_sbus(serial& srl, rx::sbus& sbus, serial& rx) {
  auto buf = rx.read();
  for (size_t i = 0; i < buf.size(); i++) {
    if (sbus.feed(buf[i])) {
      write_printf(
        srl,
        "THR: %5.2f, AIL: %5.2f, ELE: %5.2f, RUD: %5.2f, AUX1: %5.2f, AUX2: %5.2f\r\n",
        sbus.get(rx::THR), sbus.get(rx::AIL), sbus.get(rx::ELE), sbus.get(rx::RUD), sbus.get(rx::AUX1), sbus.get(rx::AUX2)
      );

      i--;
    }
  }
}

extern volatile uint32_t systick_count;

using mode_cs_pin = stm32_f3::gpio::mode<
  stm32_f3::gpio::modes::output,
  stm32_f3::gpio::modes::push_pull,
  stm32_f3::gpio::modes::speed_50MHz,
  stm32_f3::gpio::modes::no_pull
>;

int main() {
  bool dump_sbus = false;
  bool dump_gyro = false;

  stm32_f3::board board;

  stm32_f3::serial_usart rx;
  rx::sbus sbus(172, 1811);

  stm32_f3::spi spi;
  auto cs_pin = stm32_f3::gpio::pin<stm32_f3::gpio::port<stm32_f3::gpio::A, 15>, mode_cs_pin>();
  mpu_6000 mpu(&spi, &cs_pin);

  stm32_f3::timer::timer<stm32_f3::timer::TIMER1, 0, 32000> timer;

  auto& usb = board.usb_serial();
  while (1) {
    if (!board.usb_serial_active()) {
      continue;
    }

    if (dump_sbus && (systick_count % 250) == 0) {
      print_sbus(usb, sbus, rx);
    }

    if (dump_gyro && (systick_count % 250) == 0) {
      const auto& gyro = mpu.read_gyro();
      write_printf(usb,
        "%2.3f,%2.3f,%2.3f\r\n", 
        gyro[0], gyro[1], gyro[2]
      );
    }

    {
      auto buf = usb.read();
      for (size_t i = 0; i < buf.size(); i++) {
        switch (buf[i]) {
        case 'R':
          board.reset_to_bootloader();
          break;
        case 'C':
          mpu.calibrate();
          break;
        case 'M':
          dump_gyro = !dump_gyro;
          write_printf(usb, "dump_gyro: %d\r\n", dump_gyro ? 1 : 0);
          break;
        case 'B':
          dump_sbus = !dump_sbus;
          write_printf(usb, "dump_sbus: %d\r\n", dump_sbus ? 1 : 0);
          break;
        default:
          usb.write(buf[i]);
          break;
        }
      }
    }

    usb.flush();
  }
}
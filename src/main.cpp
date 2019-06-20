#include <cstdio>
#include <cstdarg>

#include <stm32f30x.h>
#include <stm32f30x_conf.h>

#include "driver/accel/mpu_6000.h"
#include "driver/stm32_f3/board.h"
#include "driver/stm32_f3/spi.h"
#include "driver/stm32_f3/gpio.h"
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
  for (int32_t i = 0; i < buf.size(); i++) {
    if (sbus.feed(buf[i])) {
      auto channels = sbus.channels();

      if (channels.flags & SBUS_FLAG_SIGNAL_LOSS) {
        write_string(srl, "SBUS_FLAG_SIGNAL_LOSS\r\n");
      } else if (channels.flags & SBUS_FLAG_FAILSAFE_ACTIVE) {
        write_string(srl, "SBUS_FLAG_FAILSAFE_ACTIVE\r\n");
      } else {
        write_printf(
          srl,
          "THR: %.2f, AIL: %.2f, ELE: %.2f, RUD: %.2f, AUX1: %.2f, AUX2: %.2f\r\n",
          util::map(channels.chan0, 172, 1811, -1, 1),
          util::map(channels.chan1, 172, 1811, -1, 1),
          util::map(channels.chan2, 172, 1811, -1, 1),
          util::map(channels.chan3, 172, 1811, -1, 1),
          util::map(channels.chan4, 172, 1811, -1, 1),
          util::map(channels.chan5, 172, 1811, -1, 1)
        );
      }

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
  bool dump_sbus = true;
  bool dump_gyro = false;

  stm32_f3::board board;

  stm32_f3::serial_usart rx;
  rx::sbus sbus;

  stm32_f3::spi spi;
  auto cs_pin = stm32_f3::gpio::pin<stm32_f3::gpio::port<stm32_f3::gpio::A, 15>, mode_cs_pin>();
  mpu_6000 mpu(&spi, &cs_pin);

  auto& usb = board.usb_serial();
  while (1) {
    if (board.usb_serial_active()) {
      if (dump_sbus) {
        print_sbus(usb, sbus, rx);
      }

      if (dump_gyro && (systick_count % 250) == 0) {
        const auto& gyro = mpu.read_gyro();
        write_printf(usb,
          "%.3f,%.3f,%.3f\r\n", 
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
}
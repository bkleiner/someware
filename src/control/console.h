#pragma once

#include <cstdio>
#include <cstdarg>

#include "control.h"

#include "rx/rx.h"
#include "driver/board.h"

extern volatile uint32_t systick_count;

namespace control {
  struct console {
    bool dump_sbus = false;
    bool dump_gyro = false;
    bool dump_ctrl = false;

    uint32_t motor_speed = 0;

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

    void update(float dt, board& brd, rx::rx& sbus, control& ctrl) {
      auto& usb = brd.usb_serial();

      if (dump_sbus && (systick_count % 250) == 0) {
        write_printf(
          usb,
          "THR: %5.2f, AIL: %5.2f, ELE: %5.2f, RUD: %5.2f, AUX1: %5.2f, AUX2: %5.2f\r\n",
          sbus.get(rx::THR), sbus.get(rx::AIL), sbus.get(rx::ELE), sbus.get(rx::RUD), sbus.get(rx::AUX1), sbus.get(rx::AUX2)
        );
      }
      if (dump_ctrl && (systick_count % 250) == 0) {
        write_printf(
          usb,
          "DT: %f, ARMED: %d, THR: %5.2f, ROLL: %5.2f, PITCH: %5.2f, YAW: %5.2f\r\n",
          dt,
          ctrl.is_armed(sbus),
          ctrl.demands.throttle,
          ctrl.demands.roll,
          ctrl.demands.pitch,
          ctrl.demands.yaw
        );
      }

      if (dump_gyro && (systick_count % 250) == 0) {
        const auto& gyro = brd.accel().read_gyro();
        write_printf(usb,
          "roll: %2.3f, pitch: %2.3f, yaw: %2.3f\r\n", 
          gyro.roll(), gyro.pitch(), gyro.yaw()
        );
      }

      {
        auto buf = usb.read();
        for (size_t i = 0; i < buf.size(); i++) {
          switch (buf[i]) {
          case 'R':
            brd.reset_to_bootloader();
            break;
          case 'C':
            brd.accel().calibrate();
            break;
          case 'M':
            dump_gyro = !dump_gyro;
            write_printf(usb, "dump_gyro: %d\r\n", dump_gyro ? 1 : 0);
            break;
          case 'B':
            dump_sbus = !dump_sbus;
            write_printf(usb, "dump_sbus: %d\r\n", dump_sbus ? 1 : 0);
            break;
          case 'T':
            dump_ctrl = !dump_ctrl;
            write_printf(usb, "dump_ctrl: %d\r\n", dump_ctrl ? 1 : 0);
            break;
          case 'U':
            motor_speed++;
            write_printf(usb, "motor_speed: %d %d/%d\r\n", motor_speed, uint32_t(motor_speed / 100.f * PWM_PERIOD), PWM_PERIOD);

            // brd.motor0.set(motor_speed / 100.f * PWM_PERIOD);
            // brd.motor1.set(motor_speed / 100.f * PWM_PERIOD);
            // brd.motor2.set(motor_speed / 100.f * PWM_PERIOD);
            // brd.motor3.set(motor_speed / 100.f * PWM_PERIOD);
            break;
          default:
            usb.write(buf[i]);
            break;
          }
        }
      }

      usb.flush();
    }
  };
}
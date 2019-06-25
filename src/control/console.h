#pragma once

#include "control.h"

#include "rx/rx.h"
#include "driver/board.h"

namespace control {

  class console {
  public:
    console(board* brd)
      : brd(brd)
    {}

    void update(float dt, rx::rx& sbus, control& ctrl) {
      auto time = int32_t(brd->millis());
      auto& usb = brd->usb_serial();

      if ((time % 250) == 0) {
        if (dump_sbus) {
          usb.printf(
            "THR: %5.2f, AIL: %5.2f, ELE: %5.2f, RUD: %5.2f, AUX1: %5.2f, AUX2: %5.2f\r\n",
            sbus.get(rx::THR), sbus.get(rx::AIL), sbus.get(rx::ELE), sbus.get(rx::RUD), sbus.get(rx::AUX1), sbus.get(rx::AUX2)
          );
        }
        if (dump_ctrl) {
          usb.printf(
            "ARMED: %d, THR: %5.2f, ROLL: %5.2f, PITCH: %5.2f, YAW: %5.2f\r\n",
            ctrl.is_armed(sbus),
            ctrl.demands.throttle,
            ctrl.demands.roll,
            ctrl.demands.pitch,
            ctrl.demands.yaw
          );
        }
        if (dump_gyro) {
          const auto& gyro = brd->accel().read_gyro();
          usb.printf(
            "roll: %2.3f, pitch: %2.3f, yaw: %2.3f\r\n", 
            gyro.roll(), gyro.pitch(), gyro.yaw()
          );
        }
      }

      {
        auto buf = usb.read();
        for (size_t i = 0; i < buf.size(); i++) {
          switch (buf[i]) {
          case 'R':
            brd->reset_to_bootloader();
            break;
          case 'A':
            ctrl.arm_override = !ctrl.arm_override;
            usb.printf("arm_override: %d\r\n", ctrl.arm_override ? 1 : 0);
            break;
          case 'C':
            brd->accel().calibrate();
            break;
          case 'M':
            dump_gyro = !dump_gyro;
            usb.printf("dump_gyro: %d\r\n", dump_gyro ? 1 : 0);
            break;
          case 'B':
            dump_sbus = !dump_sbus;
            usb.printf("dump_sbus: %d\r\n", dump_sbus ? 1 : 0);
            break;
          case 'T':
            dump_ctrl = !dump_ctrl;
            usb.printf("dump_ctrl: %d\r\n", dump_ctrl ? 1 : 0);
            break;
          default:
            usb.write(buf[i]);
            break;
          }
        }
      }

      usb.flush();
    }

private:
    bool dump_sbus = false;
    bool dump_gyro = false;
    bool dump_ctrl = false;

    board* brd;
  };
}
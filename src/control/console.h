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
      auto time = int64_t(brd->millis());
      auto& usb = brd->usb_serial();

      if ((time % 500) == 0 && dump_ctrl) {
        usb.printf(
            "THR: %5.2f, AIL: %5.2f, ELE: %5.2f, RUD: %5.2f, AUX1: %5.2f, AUX2: %5.2f\r\n",
            sbus.get(rx::THR), sbus.get(rx::AIL), sbus.get(rx::ELE), sbus.get(rx::RUD), sbus.get(rx::AUX1), sbus.get(rx::AUX2)
          );
        usb.printf(
          "CTRL DT: %5.2f, FREQ: %5.2fkHz, ARMED: %d\r\n",
          dt,
          (1000.f / dt) / 1000.f,
          ctrl.armed
        );
        usb.printf(
          "INPUT THR: %5.2f, ROLL: %5.2f, PITCH: %5.2f, YAW: %5.2f\r\n",
          ctrl.input_demands.throttle,
          ctrl.input_demands.roll,
          ctrl.input_demands.pitch,
          ctrl.input_demands.yaw
        );
        usb.printf(
          "GYRO ROLL: %5.2f, PITCH: %5.2f, YAW: %5.2f\r\n", 
          ctrl.gyro.roll(),
          ctrl.gyro.pitch(),
          ctrl.gyro.yaw()
        );
        usb.printf(
          "PTERM ROLL: %5.2f, PITCH: %5.2f, YAW: %5.2f\r\n", 
          ctrl.pid.pterm.roll(),
          ctrl.pid.pterm.pitch(),
          ctrl.pid.pterm.yaw()
        );
        usb.printf(
          "ITERM ROLL: %5.2f, PITCH: %5.2f, YAW: %5.2f\r\n", 
          ctrl.pid.iterm.roll(),
          ctrl.pid.iterm.pitch(),
          ctrl.pid.iterm.yaw()
        );
        usb.printf(
          "DTERM ROLL: %5.2f, PITCH: %5.2f, YAW: %5.2f\r\n", 
          ctrl.pid.dterm.roll(),
          ctrl.pid.dterm.pitch(),
          ctrl.pid.dterm.yaw()
        );
        usb.printf(
          "OUTPUT THR: %5.2f, ROLL: %5.2f, PITCH: %5.2f, YAW: %5.2f\r\n",
          ctrl.output_demands.throttle,
          ctrl.output_demands.roll,
          ctrl.output_demands.pitch,
          ctrl.output_demands.yaw
        );
      }

      {
        auto buf = usb.read();
        for (size_t i = 0; i < buf.size(); i++) {
          switch (buf[i]) {
          case 'R':
            brd->reset_to_bootloader();
            break;
          case 'A':
            ctrl.armed = !ctrl.armed;
            usb.printf("armed: %d\r\n", ctrl.armed ? 1 : 0);
            break;
          case 'C': {
            const auto& bias = ctrl.calibrate_gyro();
            usb.printf(
              "gyro_bias: %5.2f %5.2f %5.2f\r\n",
              bias[0],
              bias[1],
              bias[2]
            );
            break;
          }
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
    bool dump_ctrl = false;

    board* brd;
  };
}
#pragma once

#include "control.h"

#include "rx/rx.h"
#include "driver/board.h"

#include "platform/time.h"
#include "platform/print.h"

namespace control {

  class console {
  public:
    console(board* brd)
      : brd(brd)
    {}

    void update(float dt, rx::rx& sbus, control& ctrl) {
      auto time = platform::time::now_ms();
      auto& usb = brd->usb_serial();


      if (dump_ctrl) {
        util::trigger_every<250>(time, [&] () {
          usb.printf(
            "CTRL DT: %5.2f, FREQ: %5.2fkHz, ARMED: %d, AIRBORN: %d, VBAT: %u\r\n",
            dt,
            (1000.f / dt) / 1000.f,
            ctrl.armed,
            ctrl.is_airborn(),
            ctrl.vbat
          );
          usb.printf(
            "RX THR: %5.2f, AIL: %5.2f, ELE: %5.2f, RUD: %5.2f, AUX1: %5.2f, AUX2: %5.2f\r\n",
            sbus.get(rx::THR),
            sbus.get(rx::AIL),
            sbus.get(rx::ELE),
            sbus.get(rx::RUD),
            sbus.get(rx::AUX1),
            sbus.get(rx::AUX2)
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
            ctrl.imu.gyro.roll(),
            ctrl.imu.gyro.pitch(),
            ctrl.imu.gyro.yaw()
          );
          usb.printf(
            "ACCEL X: %5.2f, Y: %5.2f, Z: %5.2f\r\n", 
            ctrl.imu.accel.roll(),
            ctrl.imu.accel.pitch(),
            ctrl.imu.accel.yaw()
          );
          usb.printf(
            "ANGLE X: %5.2f, Y: %5.2f, Z: %5.2f\r\n", 
            ctrl.imu.angle.roll(),
            ctrl.imu.angle.pitch(),
            ctrl.imu.angle.yaw()
          );
          usb.printf(
            "STICK X: %5.2f, Y: %5.2f, Z: %5.2f\r\n", 
            ctrl.stick_vector.roll(),
            ctrl.stick_vector.pitch(),
            ctrl.stick_vector.yaw()
          );
          usb.printf(
            "PTERM ROLL: %5.2f, PITCH: %5.2f, YAW: %5.2f\r\n", 
            ctrl.rate_pid.pterm.roll(),
            ctrl.rate_pid.pterm.pitch(),
            ctrl.rate_pid.pterm.yaw()
          );
          usb.printf(
            "ITERM ROLL: %5.2f, PITCH: %5.2f, YAW: %5.2f\r\n", 
            ctrl.rate_pid.iterm.roll(),
            ctrl.rate_pid.iterm.pitch(),
            ctrl.rate_pid.iterm.yaw()
          );
          usb.printf(
            "DTERM ROLL: %5.2f, PITCH: %5.2f, YAW: %5.2f\r\n", 
            ctrl.rate_pid.dterm.roll(),
            ctrl.rate_pid.dterm.pitch(),
            ctrl.rate_pid.dterm.yaw()
          );
          usb.printf(
            "ANGLE_PTERM ROLL: %5.2f, PITCH: %5.2f, YAW: %5.2f\r\n", 
            ctrl.angle_pid.pterm.roll(),
            ctrl.angle_pid.pterm.pitch(),
            ctrl.angle_pid.pterm.yaw()
          );
          usb.printf(
            "ANGLE_DTERM ROLL: %5.2f, PITCH: %5.2f, YAW: %5.2f\r\n", 
            ctrl.angle_pid.dterm.roll(),
            ctrl.angle_pid.dterm.pitch(),
            ctrl.angle_pid.dterm.yaw()
          );
          usb.printf(
            "OUTPUT THR: %5.2f, ROLL: %5.2f, PITCH: %5.2f, YAW: %5.2f\r\n",
            ctrl.output_demands.throttle,
            ctrl.output_demands.roll,
            ctrl.output_demands.pitch,
            ctrl.output_demands.yaw
          );
        });
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
          case 'C':
            ctrl.calibrate_gyro();
            // fallthrough
          case 'P': {
            usb.printf(
              "CONFIG GYRO_ROLL_BIAS: %5.2f, GYRO_PITCH_BIAS: %5.2f, GYRO_YAW_BIAS: %5.2f\r\n",
              ctrl.cfg.gyro_bias.roll(),
              ctrl.cfg.gyro_bias.pitch(),
              ctrl.cfg.gyro_bias.yaw()
            );
            usb.printf(
              "CONFIG ACCEL_X_BIAS: %5.2f, ACCEL_Y_BIAS: %5.2f, ACCEL_Z_BIAS: %5.2f\r\n",
              ctrl.cfg.accel_bias.x(),
              ctrl.cfg.accel_bias.y(),
              ctrl.cfg.accel_bias.z()
            );
            usb.printf(
              "CONFIG ROLL_KP: %5.2f, PITCH_KP: %5.2f, YAW_KP: %5.2f\r\n",
              ctrl.cfg.pid_kp.roll(),
              ctrl.cfg.pid_kp.pitch(),
              ctrl.cfg.pid_kp.yaw()
            );
            usb.printf(
              "CONFIG ROLL_KI: %5.2f, PITCH_KI: %5.2f, YAW_KI: %5.2f\r\n",
              ctrl.cfg.pid_ki.roll(),
              ctrl.cfg.pid_ki.pitch(),
              ctrl.cfg.pid_ki.yaw()
            );
            usb.printf(
              "CONFIG ROLL_KD: %5.2f, PITCH_KD: %5.2f, YAW_KD: %5.2f\r\n",
              ctrl.cfg.pid_kd.roll(),
              ctrl.cfg.pid_kd.pitch(),
              ctrl.cfg.pid_kd.yaw()
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
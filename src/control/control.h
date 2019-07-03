#pragma once

#include "mixer.h"
#include "pid.h"
#include "config.h"
#include "gesture.h"
#include "imu.h"

#include "rx/rx.h"
#include "util/timer.h"
#include "driver/board.h"

#include "platform/print.h"

namespace control
{
  class control {
  public:
    control(board* brd)
      : brd(brd)
      , cfg(*((config*)brd->flash().pointer()))
      , mix(brd)
      , imu(brd, &cfg)
      , rate_pid(&cfg)
    {}

    void update(float dt, rx::rx& recv) {
      vbat = brd->vbat_adc().read();
      if (!brd->usb_serial_active() && vbat < battery_safety_min) {
        brd->power_off();
        return;
      }

      imu.update(dt);

      if (recv.get(rx::AUX1) > 0.5f) {
        if (!armed && recv.get(rx::THR) < -0.8f) {
          brd->accel().calibrate_accel();
          if (config_dirty) {
            cfg.save(brd->flash());
            config_dirty = false;
          }
          rate_pid.reset();
          armed = true;
        }
        #ifdef ENABLE_ANGLE_MODE
        if (recv.get(rx::AUX2) > 0.5f)  {
          angle_mode = true;
        }
        #endif
      } else {
        if (gesture.update(dt, recv, cfg)) {
          config_dirty = true;
        }
        armed = false;
      }

      input_demands = {
        rc_filter[0].step(recv.get(rx::THR)) * rc_rate.throttle,
        filter::expo(rc_filter[3].step(recv.get(rx::AIL)), 0.8) * rc_rate.roll,
        filter::expo(rc_filter[2].step(recv.get(rx::ELE)), 0.8) * rc_rate.pitch,
        filter::expo(rc_filter[1].step(recv.get(rx::RUD)), 0.6) * rc_rate.yaw
      };

      vector rates = {0, 0, 0};

      if (angle_mode)
      {
        const float roll_angle = -input_demands.roll * angle_limit_deg * util::deg_to_rad;
        const float pitch_angle = -input_demands.pitch * angle_limit_deg * util::deg_to_rad;

        stick_vector[0] = sin(pitch_angle);
        stick_vector[1] = sin(roll_angle);
        stick_vector[2] = cos(roll_angle) * cos(pitch_angle);

        const auto angle_rates = angle_pid.calc(dt, stick_vector, imu.angle);
        rates = {
          angle_rates[vector::ROLL],
          angle_rates[vector::PITCH],
          input_demands.yaw * rate_limit_deg * util::deg_to_rad
        };
      } else {
        rates = {
          input_demands.roll * rate_limit_deg * util::deg_to_rad,
          input_demands.pitch * rate_limit_deg * util::deg_to_rad,
          input_demands.yaw * rate_limit_deg * util::deg_to_rad
        };
      }

      rates = rate_pid.calc(dt, is_airborn(), rates, imu.gyro);
      output_demands = {
        input_demands.throttle,
        rates.roll(),
        rates.pitch(),
        rates.yaw()
      };

      if (armed) {
        mix.set_demands(output_demands);
      } else {
        mix.set_all(0.0f);
      }
    }

    bool is_airborn() {
      // this should be smarter.
      return armed && output_demands.throttle > -0.6f;
    }

    void calibrate_gyro() {
      cfg.reset();
      cfg.gyro_bias = brd->accel().calibrate_gyro();
      cfg.accel_bias = brd->accel().calibrate_accel();
      cfg.save(brd->flash());
    }

    bool armed = false;
    uint16_t vbat = 0;
    
    vector stick_vector = {0, 0, 0};
    
    demands input_demands = {0, 0, 0, 0};
    demands output_demands = {0, 0, 0, 0};

    board* brd;
    config cfg;
    mixer mix;

    inertial_measurement imu;
    pid::rate_controller rate_pid;
    pid::angle_controller angle_pid;
    gesture_controller gesture;

  private:
    bool angle_mode = false;
    bool config_dirty = false;

    filter::biquad_lowpass rc_filter[4] = {
      { 180, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
      { 180, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
      { 180, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
      { 180, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
    };
  };
}

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
      , imu(brd, &cfg)
      , rate_pid(&cfg)
      , mix(brd)
    {}

    void update(float dt, rx::rx& recv) {
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

        if (recv.get(rx::AUX2) > 0.5f)  {
          angle_mode = true;
        }
      } else {
        if (gesture.update(dt, recv, cfg)) {
          config_dirty = true;
        }
        armed = false;
      }

      input_demands = {
        rc_filter[0].step(recv.get(rx::THR)) * rc_rate.throttle,
        filter::expo(rc_filter[3].step(recv.get(rx::AIL)) * rc_rate.roll, 0.8),
        filter::expo(rc_filter[2].step(recv.get(rx::ELE)) * rc_rate.pitch, 0.8),
        filter::expo(rc_filter[1].step(recv.get(rx::RUD)) * rc_rate.yaw, 0.6)
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

      if (is_armed()) {
        mix.set_demands(output_demands);
      } else {
        disarm();
      }
    }

    void disarm() {
      mix.set_all(0.0f);
    }

    bool is_armed() {
      return armed;
    }

    bool is_airborn() {
      return armed && output_demands.throttle > -0.3f;
    }

    void calibrate_gyro() {
      cfg.reset();
      cfg.gyro_bias = brd->accel().calibrate_gyro();
      cfg.accel_bias = brd->accel().calibrate_accel();
      cfg.save(brd->flash());
    }

    bool armed = false;
    
    vector stick_vector = {0, 0, 0};
    
    demands input_demands = {0, 0, 0, 0};
    demands output_demands = {0, 0, 0, 0};

    board* brd;
    config cfg;

    inertial_measurement imu;
    pid::rate_controller rate_pid;
    pid::angle_controller angle_pid;
    gesture_controller gesture;

  private:
    bool angle_mode = false;
    bool config_dirty = false;

    const float rate_limit_deg = 360.f;
    const float angle_limit_deg = 180.0f;
    const float accel_limits[2] = { 0.7f, 1.3f };

    const demands rc_rate = {
      1.0f, // T
      0.5f, // A
      0.5f, // E
      0.5f  // R
    };
    
    
    mixer mix;

    filter::biquad_lowpass rc_filter[4] = {
      { 120, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
      { 120, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
      { 120, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
      { 120, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
    };
  };
}

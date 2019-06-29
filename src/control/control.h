#pragma once

#include "mixer.h"
#include "pid.h"
#include "config.h"
#include "gesture.h"

#include "rx/rx.h"
#include "util/timer.h"
#include "driver/board.h"

namespace control
{
  class control {
  public:
    control(board* brd)
      : brd(brd)
      , cfg(*((config*)brd->flash().pointer()))
      , rate_pid(&cfg)
      , mix(brd)
    {
      brd->accel().set_bias(cfg.gyro_bias);
    }

    void update(float dt, rx::rx& recv) {
      update_gyro();

      if (recv.get(rx::AUX1) > 0.5f) {
        if (!armed && recv.get(rx::THR) < -0.99f) {
          cfg.save(brd->flash());
          rate_pid.reset();
          armed = true;
        }
      } else {
        gesture.update(dt, recv, cfg);
        armed = false;
      }

      input_demands = {
        rc_filter[0].step(recv.get(rx::THR)) * rc_rate.throttle,
        rc_filter[3].step(recv.get(rx::AIL)) * rc_rate.roll,
        rc_filter[2].step(recv.get(rx::ELE)) * rc_rate.pitch,
        rc_filter[1].step(recv.get(rx::RUD)) * rc_rate.yaw
      };
      const vector rates = {
        input_demands.roll * rate_limit_deg * (util::pi / 180.0f),
        input_demands.pitch * rate_limit_deg * (util::pi / 180.0f),
        input_demands.yaw * rate_limit_deg * (util::pi / 180.0f)
      };
      const vector out = rate_pid.calc(dt, is_airborn(), rates, gyro);
      output_demands = {
        input_demands.throttle,
        out.roll(),
        out.pitch(),
        out.yaw()
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
      return armed && output_demands.throttle > -0.2f;
    }

    void update_gyro() {
      vector gyro_new = brd->accel().read_gyro();
      
      gyro_new[vector::ROLL] = -gyro_new[vector::ROLL];

      for (int i = 0; i < 3; i++) {
        gyro[i] = gyro_new[i] * (util::pi / 180.0f);
        gyro[i] = gyro_filter.step(i, gyro[i]);
			  gyro[i] = gyro_filter2.step(i, gyro[i]);
      }
    }

    vector calibrate_gyro() {
      cfg.reset();
      cfg.gyro_bias = brd->accel().calibrate();
      cfg.save(brd->flash());
      return cfg.gyro_bias;
    }

    bool armed = false;
    
    vector gyro = {0, 0, 0};
    
    demands input_demands = {0, 0, 0, 0};
    demands output_demands = {0, 0, 0, 0};

    board* brd;
    config cfg;
    pid::rate_controller rate_pid;
    gesture_controller gesture;

  private:
    const float rate_limit_deg = 860.f;
    const demands rc_rate = {
      1.0f, // T
      0.5f, // A
      0.5f, // E
      0.5f  // R
    };
    
    
    mixer mix;
    
    filter::kalman gyro_filter;
    filter::kalman gyro_filter2;

    filter::biquad_lowpass rc_filter[4] = {
      { 90, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth},
      { 90, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth},
      { 90, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth},
      { 90, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth},
    };
  };
}

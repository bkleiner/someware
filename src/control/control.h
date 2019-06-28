#pragma once

#include "mixer.h"
#include "pid.h"
#include "config.h"

#include "rx/rx.h"
#include "util/timer.h"
#include "driver/board.h"

namespace control
{
  class control {
  public:
    control(board* brd)
      : did_startup(false)
      , brd(brd)
      , mix(brd)
    {
      cfg = *((config*)brd->flash().pointer());
      brd->accel().set_bias(cfg.gyro_bias);
    }

    void update(float dt, rx::rx& recv) {
      update_gyro();

      input_demands = {
        recv.get(rx::THR) * rc_rate.throttle,
        recv.get(rx::AIL) * rc_rate.roll,
        recv.get(rx::ELE) * rc_rate.pitch,
        recv.get(rx::RUD) * rc_rate.yaw
      };
      const vector rates = {
        input_demands.roll * rate_limit_deg * (util::pi / 180.0f),
        input_demands.pitch * rate_limit_deg * (util::pi / 180.0f),
        input_demands.yaw * rate_limit_deg * (util::pi / 180.0f)
      };
      const vector out = pid.calc(dt, rates, gyro);
      output_demands = {
        input_demands.throttle,
        out.roll(),
        out.pitch(),
        out.yaw()
      };

      if (is_armed(recv)) {
        mix.set_demands(output_demands);
      } else {
        disarm();
      }
    }

    void disarm() {
      mix.set_all(0.0f);
    }

    bool is_armed(rx::rx& recv) {
      return arm_override || recv.get(rx::AUX1) > 0.5f;
    };

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
      cfg.gyro_bias = brd->accel().calibrate();
      cfg.save(brd->flash());
      return cfg.gyro_bias;
    }

    bool arm_override = false;
    
    vector gyro = {0, 0, 0};
    
    demands input_demands = {0, 0, 0, 0};
    demands output_demands = {0, 0, 0, 0};

    pid_controller pid;
    config cfg;

  private:
    const float rate_limit_deg = 860.f;
    const demands rc_rate = {
      1.0f, // T
      0.5f, // A
      0.5f, // E
      0.5f  // R
    };

    bool did_startup;
    
    board* brd;
    mixer mix;
    
    filter::kalman gyro_filter;
    filter::kalman gyro_filter2;
  };
}

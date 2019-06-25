#pragma once

#include "mixer.h"
#include "pid.h"

#include "rx/rx.h"
#include "driver/board.h"

namespace control
{
  class control {
  public:
    control(board* brd)
      : did_startup(false)
      , brd(brd)
      , mix(brd)
    {}

    void update(float dt, rx::rx& recv) {
      if (!is_armed(recv)) {
        disarm();
        pid.reset();
        // did_startup = false;
        return;
      }
      if (!did_startup) {
        brd->accel().calibrate();
        
        did_startup = true;
      }

      demands = {
        recv.get(rx::THR) * rc_rate.throttle,
        recv.get(rx::AIL) * rc_rate.roll,
        recv.get(rx::ELE) * rc_rate.pitch,
        recv.get(rx::RUD) * rc_rate.yaw
      };
      const vector gyro = brd->accel().read_gyro();
      const vector error = {
        rate_limit_deg * demands.roll - gyro.roll(),
        rate_limit_deg * demands.pitch - gyro.pitch(),
        rate_limit_deg * demands.yaw - gyro.yaw()
      };

      const vector out = pid.calc(dt, error, gyro);
      demands.roll = out.roll() / rate_limit_deg;
      demands.pitch = out.pitch() / rate_limit_deg;
      demands.yaw = out.yaw() / rate_limit_deg;

      mix.set_demands(demands);
    }

    void disarm() {
      mix.set_all(0.0f);
    }

    bool is_armed(rx::rx& recv) {
      return recv.get(rx::AUX1) > 0.5f;
    };

    input_demands demands;

  private:
    const float rate_limit_deg = 60.f;
    const input_demands rc_rate = {
      1.0f, // T
      1.0f, // A
      1.0f, // E
      1.0f  // R
    };

    bool did_startup;
    
    board* brd;
    mixer mix;
    pid_controller pid;
  };
}

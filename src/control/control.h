#pragma once

#include "mixer.h"
#include "pid.h"

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
    {}

    void update(float dt, rx::rx& recv) {
      // util::timer gyro_timer(brd, "control::update");
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
        -recv.get(rx::AIL) * rc_rate.roll,
         recv.get(rx::ELE) * rc_rate.pitch,
         recv.get(rx::RUD) * rc_rate.yaw
      };
      const vector gyro = brd->accel().read_gyro();
      const vector rates = {
        rate_limit_deg * demands.roll,
        rate_limit_deg * demands.pitch,
        rate_limit_deg * demands.yaw
      };

      const float dts = dt / 1000.f;
      const vector out = pid.calc(dts, rates, gyro);
      demands.roll = out.roll();
      demands.pitch = out.pitch();
      demands.yaw = out.yaw();

      mix.set_demands(demands);
    }

    void disarm() {
      mix.set_all(0.0f);
    }

    bool is_armed(rx::rx& recv) {
      return arm_override || recv.get(rx::AUX1) > 0.5f;
    };

    bool arm_override = false;
    input_demands demands = {0, 0, 0, 0};

  private:
    const float rate_limit_deg = 180.f;
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

#pragma once

#include <cmath>
#include <cstdint>

#include "config.h"
#include "filter.h"

#include "util/vector.h"

namespace control::pid {

  class rate_controller {
  public:
    rate_controller(const config* cfg)
      : cfg(cfg)
    {}

    

    const vector calc(float dt, bool is_airborn, const vector& rate_setpoint, const vector& rate_actual) {
      const vector error = rate_setpoint - rate_actual;

      return vector{
        calc_axis(dt, is_airborn, 0, error, rate_actual),
        calc_axis(dt, is_airborn, 1, error, rate_actual),
        calc_axis(dt, is_airborn, 2, error, rate_actual)
      };
    }

    float calc_axis(float dt, bool is_airborn, uint8_t axis, const vector& error, const vector& actual) {
      // gyro is in deg/s so we need your timestep in s (vs us)
      const float dts = dt / 1000.f;
      const float one_over_dt = 1.0f / dt;

      // P Term 
      pterm[axis] = pid_axis_weights[axis] * error[axis] * cfg->pid_kp[axis]
        - (1.0f - pid_axis_weights[axis]) * cfg->pid_kp[axis] * actual[axis];

      // I term
      if (!is_airborn) {
        // if we are still on the ground the quad vibrates a lot etc.
        // so iterm winds if self up. we dim it down here for easier take of.
        // todo: maybe more? or for longer? take off should be easier.
        iterm[axis] *= 0.98f;
      }

      iterm[axis] = 
        iterm[axis] + 1.0f/6.0f * 
        (lasterror2[axis] + 4.0f * lasterror[axis] + error[axis])
        * cfg->pid_ki[axis] * dts;
      iterm[axis] = filter::constrain_min_max(iterm[axis], -pid_integral_limit[axis], pid_integral_limit[axis]);

      // D term
      // skip yaw D term if not set
      if (cfg->pid_kd[axis] > 0) {
        dterm[axis] = -(actual[axis] - lastrate[axis]) * one_over_dt * cfg->pid_kd[axis];
        dterm[axis] = filter::lpf2(dterm[axis], axis);
        // dterm[axis] = gyro_filter[axis].step(dterm[axis]);
        lastrate[axis] = actual[axis];
      } else {
        dterm[axis] = 0;
      }

      lasterror2[axis] = lasterror[axis];
      lasterror[axis] = error[axis];

      return filter::constrain_min_max(pterm[axis] + iterm[axis] + dterm[axis], -pid_output_limit[axis], pid_output_limit[axis]);
    };

    void reset() {
      pterm = vector(0);
      iterm = vector(0);
      dterm = vector(0);

      lasterror = vector(0);
      lasterror2 = vector(0);
      lastrate = vector(0);
    }


    vector iterm;
    vector pterm;
    vector dterm;

  private:
    const config* cfg = nullptr;

    filter::biquad_lowpass gyro_filter[3] = {
      { 180.0f, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
      { 180.0f, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
      { 180.0f, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
    };

    vector lasterror;
    vector lasterror2;

    vector lastrate;
  };

  class angle_controller {
  public:

    const vector calc(float dt, const vector& rate_setpoint, const vector& rate_actual) {
      const vector error = vector::cross(rate_setpoint, rate_actual);

      return vector{
        calc_axis(dt, 0, error, rate_actual),
        calc_axis(dt, 1, error, rate_actual),
        calc_axis(dt, 2, error, rate_actual)
      };
    }

    float calc_axis(float dt, uint8_t axis, const vector& error, const vector& actual) {
      const float one_over_dt = 1.0f / dt;
      
      pterm[axis] = error[axis] * pid_kp;
      dterm[axis] = (error[axis] - lasterror[axis]) * pid_kd * one_over_dt;

      lasterror[axis] = error[axis];
      
      return pterm[axis] + dterm[axis];
    }

    vector pterm;
    vector dterm;

  private:
    const float pid_kp = 7.0;
    const float pid_kd = 0.0;

    const float output_limit = pid_kp;

    vector lasterror;
  };
}
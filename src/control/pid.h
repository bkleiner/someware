#pragma once

#include <cmath>
#include <cstdint>

#include "config.h"
#include "filter.h"

#include "util/vector.h"


#ifndef DTERM_LPF_2ND_HZ 
#define DTERM_LPF_2ND_HZ 99
#endif

static constexpr float filter_calc(float sampleperiod, float filtertime) {
  return (1.0f - (6.0f * sampleperiod) / (3.0f * sampleperiod + filtertime));
}

float lpf2(float in, int axis) {
  static const constexpr float two_one_minus_alpha = 2 * filter_calc(0.001, (1.0f / DTERM_LPF_2ND_HZ));
  static const constexpr float one_minus_alpha_sqr = (filter_calc(0.001, (1.0f / DTERM_LPF_2ND_HZ))) * (filter_calc(0.001, (1.0f / DTERM_LPF_2ND_HZ)));
  static const constexpr float alpha_sqr = (1 - filter_calc(0.001, (1.0f / DTERM_LPF_2ND_HZ))) * (1 - filter_calc(0.001, (1.0f / DTERM_LPF_2ND_HZ)));

  static vector last_out, last_out2;

  float ans = in * alpha_sqr + two_one_minus_alpha * last_out[axis]
      - one_minus_alpha_sqr * last_out2[axis];   

  last_out2[axis] = last_out[axis];
  last_out[axis] = ans;

  return ans;
}

namespace control::pid {

  class rate_controller {
  public:
    rate_controller(const config* cfg)
      : cfg(cfg)
    {}

    void reset() {
      pterm = vector(0);
      iterm = vector(0);
      dterm = vector(0);

      lasterror = vector(0);
      lasterror2 = vector(0);
      lastrate = vector(0);
    }

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

      // P Term 
      pterm[axis] = error[axis] * cfg->pid_kp[axis];

      // I term
      if (!is_airborn) {
        // if we are still on the ground the quad vibrates a lot etc.
        // so iterm winds if self up. we dim it down here for easier take of.
        iterm[axis] *= 0.98f;
      }

      iterm[axis] = 
        iterm[axis] + 1/6 * 
        (lasterror2[axis] + 4 * lasterror[axis] + error[axis])
        * cfg->pid_ki[axis] * dts;
      iterm[axis] = filter::constrain_min_max(iterm[axis], -integral_limit[axis], integral_limit[axis]);

      // D term
      // skip yaw D term if not set
      if (cfg->pid_kd[axis] > 0) {
        dterm[axis] = -(actual[axis] - lastrate[axis]) / dt * cfg->pid_kd[axis];
        dterm[axis] = lpf2(dterm[axis], axis);
        // dterm[axis] = gyro_filter[axis].step(dterm[axis]);
        lastrate[axis] = actual[axis];
      } else {
        dterm[axis] = 0;
      }

      lasterror2[axis] = lasterror[axis];
      lasterror[axis] = error[axis];

      return pterm[axis] + iterm[axis] + dterm[axis];
    };


    vector iterm;
    vector pterm;
    vector dterm;

  private:
    const config* cfg = nullptr;
    const vector integral_limit = { 1.7 , 1.7 , 0.5 };
    const filter::biquad_lowpass gyro_filter[3] = {
      {120.0f, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth},
      {120.0f, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth},
      {120.0f, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth},
    };

    vector lasterror;
    vector lasterror2;

    vector lastrate;
  };

  class angle_controller {
  public:

    const vector calc(float dt, const vector& rate_setpoint, const vector& rate_actual) {
      const vector error = rate_setpoint - rate_actual;

      return vector{
        calc_axis(dt, 0, error, rate_actual),
        calc_axis(dt, 1, error, rate_actual),
        calc_axis(dt, 2, error, rate_actual)
      };
    }

    float calc_axis(float dt, uint8_t axis, const vector& error, const vector& actual) {
      const float one_over_dt = 1.0f / dt;

      // P term 1 weighted
      float output1 = (1.0f - fabsf(error[axis])) * error[axis] * pidkp1;
      // D term 1 weighted + P term 1 weighted
      output1 += (error[axis] - lasterror[axis]) * pidkd1 * (1-fabsf(error[axis])) * one_over_dt;
      
      // P term 2 weighted
      float output2 = fabsf(error[axis]) * error[axis] * pidkp2;
      // D term 2 weighted + P term 2 weighted
      output2 += ((error[axis] - lasterror[axis]) * pidkd2 * fabsf(error[axis]) * one_over_dt);
      
      // apidoutput sum
      const float output = output1 + output2;
      lasterror[axis] = error[axis];
      
      return filter::constrain_min_max(output, -output_limit, output_limit);
    }

  private:
    const float pidkp1 = 10.0;
    const float pidkd1 =  3.0;

    const float pidkp2 = 5.0;
    const float pidkd2 = 0.0;

    const float output_limit = pidkp1 + pidkd1;

    vector lasterror;
  };
}
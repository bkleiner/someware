#pragma once

#include <cstdint>

#include "filter.h"

#include "util/vector.h"

#define FILTERCALC(sampleperiod, filtertime) (1.0f - ( 6.0f*(float)sampleperiod) / ( 3.0f *(float)sampleperiod + (float)filtertime))

#ifndef DTERM_LPF_2ND_HZ 
#define DTERM_LPF_2ND_HZ 99
#endif

//the compiler calculates these
static float two_one_minus_alpha = 2*FILTERCALC(0.001, (1.0f/DTERM_LPF_2ND_HZ) );
static float one_minus_alpha_sqr = (FILTERCALC(0.001, (1.0f/DTERM_LPF_2ND_HZ) ) )*(FILTERCALC(0.001, (1.0f/DTERM_LPF_2ND_HZ) ));
static float alpha_sqr = (1 - FILTERCALC(0.001, (1.0f/DTERM_LPF_2ND_HZ) ))*(1 - FILTERCALC(0.001, (1.0f/DTERM_LPF_2ND_HZ) ));

static float last_out[3], last_out2[3];

float lpf2( float in, int num)
 {

  float ans = in * alpha_sqr + two_one_minus_alpha * last_out[num]
      - one_minus_alpha_sqr * last_out2[num];   

  last_out2[num] = last_out[num];
  last_out[num] = ans;
  
  return ans;
 }

namespace control {

  class pid_controller {
  public:
    void reset() {
      pterm = vector(0);
      iterm = vector(0);
      dterm = vector(0);

      lasterror = vector(0);
      lasterror2 = vector(0);
      lastrate = vector(0);
    }

    const vector calc(float dt, const vector& rate_setpoint, const vector& rate_actual) {
      const vector error = {
        rate_setpoint[0] - rate_actual[0],
        rate_setpoint[1] - rate_actual[1],
        rate_setpoint[2] - rate_actual[2]
      };

      return vector{
        calc_axis(dt, 0, error, rate_actual),
        calc_axis(dt, 1, error, rate_actual),
        calc_axis(dt, 2, error, rate_actual)
      };
    }

    float calc_axis(float dt, uint8_t axis, const vector& error, const vector& actual) {
      // gyro is in deg/s so we need your timestep in s (vs us)
      const float dts = dt / 1000.f;

      // P Term 
      pterm[axis] = error[axis] * pidkp[axis];

      // I term
      iterm[axis] = 
        iterm[axis] + 1/6 * 
        (lasterror2[axis] + 4 * lasterror[axis] + error[axis])
        * pidki[axis] * dts;
      // iterm[axis] = filter::constrain_min_max(iterm[axis], -integral_limit[axis], integral_limit[axis]);
      /*
      ierror[axis] = ierror[axis] + error[axis] * pidki[axis] * dt;
      lasterror[axis] = error[axis];
      */
      /*
      ierror[axis] = ierror[axis] + (error[axis] + lasterror[axis]) * 0.5f *  pidki[axis] * dt;
      lasterror[axis] = error[axis];
      */

      // D term
      // skip yaw D term if not set
      if (pidkd[axis] > 0) {
        dterm[axis] = (actual[axis] - lastrate[axis]) / dt * pidkd[axis];
        lastrate[axis] = actual[axis];
      } else {
        dterm[axis] = 0;
      }

      lasterror2[axis] = lasterror[axis];
      lasterror[axis] = error[axis];

      return pterm[axis] + iterm[axis] + dterm[axis];
    };

    //                     ROLL       PITCH     YAW
    const vector pidkp = {     0.2,     0.2,    0.1 };
    const vector pidki = {     1.0,     1.0,    1.0 };	
    const vector pidkd = {     0.5,     0.5,    0.5 };

    const vector integral_limit = { 1.7 , 1.7 , 0.5 };

    vector iterm;
    vector pterm;
    vector dterm;

  private:
    vector lasterror;
    vector lasterror2;

    vector lastrate;
  };
}
#pragma once

#include <cstdint>

#include "filter.h"

#include "util/vector.h"

namespace control {

  class pid_controller {
  public:
    void reset() {
      ierror = vector(0);
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
      // P Term 
      float out = error[axis] * pidkp[axis];

      // I term
      ierror[axis] = 
        ierror[axis] + 0.166666f * 
        (lasterror2[axis] + 4 * lasterror[axis] + error[axis])
        * pidki[axis] * dt;
      /*
      ierror[axis] = ierror[axis] + error[axis] * pidki[axis] * dt;
      lasterror[axis] = error[axis];
      */
      /*
      ierror[axis] = ierror[axis] + (error[axis] + lasterror[axis]) * 0.5f *  pidki[axis] * dt;
      lasterror[axis] = error[axis];
      */
      out += ierror[axis];

      // D term
      // skip yaw D term if not set               
      if (pidkd[axis] > 0) {
        out += ((actual[axis] - lastrate[axis]) / dt) * pidkd[axis];
        lastrate[axis] = actual[axis];
      }

      lasterror2[axis] = lasterror[axis];
      lasterror[axis] = error[axis];

      return filter::constrain_min_max(out, -180.f, 180.f) / 180.f;
    };

  private:
    //                     ROLL       PITCH     YAW
    const vector pidkp = {     0.1,     0.1,     0.1 };
    const vector pidki = {     1.0,     1.0,     1.0 };	
    const vector pidkd = {    0.75,    0.75,    0.75 };

    vector ierror;
    vector lasterror;
    vector lasterror2;
    vector lastrate;
  };
}
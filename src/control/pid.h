#pragma once

#include <cstdint>

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

    const vector calc(float dt, const vector& error, const vector& gyro) {
      return vector{
        calc_axis(dt, 0, error, gyro),
        calc_axis(dt, 1, error, gyro),
        calc_axis(dt, 2, error, gyro)
      };
    }

    float calc_axis(float dt, uint8_t axis, const vector& error, const vector& gyro) {
      // P Term 
      float out = error[axis] * pidkp[axis];

      // I term
      ierror[axis] = 
        ierror[axis] + 0.166666f * 
        (lasterror2[axis] + 4 * lasterror[axis] + error[axis])
        * pidki[axis] * dt;
      lasterror2[axis] = lasterror[axis];
      lasterror[axis] = error[axis];
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
        float dterm = -(gyro[axis] - lastrate[axis]) * pidkd[axis] * dt;
		  	lastrate[axis] = gyro[axis];
        out += dterm;
      }

      return out;
    };

  private:
    //                     ROLL       PITCH     YAW
    const vector pidkp = { 21.5e-2, 21.5e-2, 10.5e-1 };
    const vector pidki = {   14e-1,   15e-1,   15e-1 };	
    const vector pidkd = {  7.4e-1,  7.4e-1,  5.5e-1 };

    vector ierror;
    vector lasterror;
    vector lasterror2;
    vector lastrate;
  };
}
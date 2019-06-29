#pragma once

#include "driver/flash.h"

#include "util/util.h"

#define LOOP_FREQ_HZ 2000
#define LOOP_TIME (1000000 / LOOP_FREQ_HZ)

namespace control {

  struct config {
    vector gyro_bias;

    vector pid_kp;
    vector pid_ki;
    vector pid_kd;

    void reset() {
      //                               ROLL   PITCH    YAW
      const vector default_pid_kp = {  0.195,   0.195,  0.14 };
      const vector default_pid_ki = {   1.13,    1.13,  1.02 };	
      const vector default_pid_kd = {   0.78,    0.78,   0.6 };

      gyro_bias = {};

      pid_kp = default_pid_kp;
      pid_ki = default_pid_ki;
      pid_kd = default_pid_kd;
    }

    void save(flash_memory& flash) {
      buffer<uint32_t> buf(this, sizeof(config));
      flash.write(buf);
    }
  };

}
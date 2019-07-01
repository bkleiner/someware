#pragma once

#include "driver/flash.h"

#include "util/util.h"

#define LOOP_FREQ_HZ 2000
#define LOOP_TIME (1000000 / LOOP_FREQ_HZ)

namespace control {
  struct demands {
    float throttle; // T
    float roll; 	  // A
    float pitch;	  // E
    float yaw;	    // R
  };

  struct motor_mix {
    int8_t throttle; // T
    int8_t roll; 	   // A
    int8_t pitch;	   // E
    int8_t yaw;	     // R
  };

  static const constexpr uint16_t battery_safety_min = 3.2;

  // rate/angle mode limits
  static const constexpr float rate_limit_deg = 360.f;
  static const constexpr float angle_limit_deg = 180.0f;

  // rate pind integral limit
  static const vector integral_limit = { 1.7 , 1.7 , 0.5 };

  // imu accel limit
  static const constexpr float accel_limits_min = 0.7f;
  static const constexpr float accel_limits_max = 1.3f;
  
  //                               ROLL   PITCH    YAW
  static const vector default_pid_kp = {  0.215,   0.215,   0.15 };
  static const vector default_pid_ki = {   1.23,    1.23,   1.04 };	
  static const vector default_pid_kd = {  0.795,   0.795,   0.6  };

  static const demands rc_rate = {
    1.0f, // T
    0.5f, // A
    0.5f, // E
    0.5f  // R
  };

  // mixer motor count and mapping
  static const constexpr uint8_t motor_count = 4;
  static const motor_mix motor_mixes[motor_count] = {
    // T   R   P   Y
    { +1, +1, +1, +1 }, // 1 rear left
    { +1, +1, -1, -1 }, // 2 front left
    { +1, -1, +1, -1 }, // 3 rear right
    { +1, -1, -1, +1 }  // 4 front  right
  };

  struct config {
    vector gyro_bias;
    vector accel_bias;

    vector pid_kp;
    vector pid_ki;
    vector pid_kd;

    void reset() {
      gyro_bias = {};
      accel_bias = {};

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
#pragma once

#include "driver/flash.h"

#include "util/util.h"

#define LOOP_FREQ_HZ 2000
#define LOOP_TIME (1000000 / LOOP_FREQ_HZ)

#define ENABLE_ANGLE_MODE

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

  static const constexpr uint16_t vbat_safety_min = 320;
  static const constexpr uint16_t vbat_arm_min = 370;

  // rate/angle mode limits
  static const constexpr float rate_limit_deg = 360.f;
  static const constexpr float angle_limit_deg = 180.0f;

  // rate pid integral limit
  static const constexpr vector pid_output_limit = { 1.7, 1.7, 0.5 };
  static const constexpr vector pid_integral_limit = { 1.7, 1.7, 0.5 };
  static const constexpr vector pid_axis_weights = { 0.93, 0.93, 0.9 };

  // imu accel limit
  static const constexpr float accel_limits_min = 0.7f;
  static const constexpr float accel_limits_max = 1.3f;
  
  //                               ROLL   PITCH    YAW
  static const constexpr vector default_pid_kp = {    0.3,     0.3,   0.6 };
  static const constexpr vector default_pid_ki = {   1.33,    1.33,   1.40 };	
  static const constexpr vector default_pid_kd = {   0.64,    0.64,   0.60 };

  static const constexpr float angle_pid_kp = 10.0;
  static const constexpr float angle_pid_kd =  3.0;
  static const constexpr float angle_output_limit = angle_pid_kp;

  static const constexpr demands rc_rate = {
    1.0f, // T
    0.5f, // A
    0.5f, // E
    0.5f  // R
  };
  static const constexpr float rc_midpoint = 993;
  static const constexpr float rc_ranges[4][2] = {
    {172, 1815},
    {182, 1815},
    {184, 1815},
    {172, 1815}
  };


  // mixer motor count and mapping
  static const constexpr uint8_t motor_count = 4;
  static const constexpr motor_mix motor_mixes[motor_count] = {
    // T   R   P   Y
    { +1, +1, +1, +1 }, // 1 rear left
    { +1, +1, -1, -1 }, // 2 front left
    { +1, -1, +1, -1 }, // 3 rear right
    { +1, -1, -1, +1 }  // 4 front  right
  };

  // filters
  static const constexpr float rc_input_filter_hz = 100;
  static const constexpr float gyro_filter_hz     = 100;
  static const constexpr float accel_filter_hz    = 100;
  static const constexpr float angle_filter_hz    = 100;
  static const constexpr float dterm_filter_hz    = 100;

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
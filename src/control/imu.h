#pragma once

#include "config.h"
#include "filter.h"

#include "driver/board.h"

namespace control
{
  class inertial_measurement {
  public:
    inertial_measurement(board* brd, config* cfg)
      : brd(brd)
      , cfg(cfg)
      , sensor(&brd->accel())
    {
      sensor->set_gyro_bias(cfg->gyro_bias);
      sensor->set_accel_bias(cfg->accel_bias);
    }

    void update(float dt) {
      const float dts = dt / 1000.f;
      {
        vector gyro_raw = brd->accel().read_gyro();

        gyro_raw[vector::ROLL] = -gyro_raw[vector::ROLL];
        gyro_raw = gyro_raw * util::deg_to_rad;

        // gyro = kalman_gyro_filter.step(gyro_raw);
        // gyro = kalman_gyro_filter2.step(gyro);
        
        gyro = {
          biquad_gyro_filter[0].step(gyro_raw[0]),
          biquad_gyro_filter[1].step(gyro_raw[1]),
          biquad_gyro_filter[2].step(gyro_raw[2]),
        };
      }
      {
        vector accel_raw = brd->accel().read_accel();

        accel = {
          accel_filter[0].step(accel_raw[0]),
          accel_filter[1].step(accel_raw[1]),
          accel_filter[2].step(accel_raw[2]),
        };
      }
      {
        static const auto tau = 2.0f;
        static const auto alpha = 0.02f; // tau / (tau + 1.0f / LOOP_FREQ_HZ);

        angle = (angle + gyro * dts) * (1.0f - alpha);

        const auto magnitude = accel.length();
        if (magnitude > accel_limits_min && magnitude < accel_limits_max) {
          angle = angle + accel * alpha;
        }

        angle = {
          angle_filter[0].step(angle[0]),
          angle_filter[1].step(angle[1]),
          angle_filter[2].step(angle[2]),
        };
      }
    }

    // filtered gyro in rad/s
    vector gyro = {0, 0, 0};
    // filtered accel in g ~ 9.80665 m/s2
    vector accel = {0, 0, 0};
    // current rotatation in rad
    vector angle = {0, 0, 0};

  private:
    board* brd;
    config* cfg;

    accel::accel* sensor;

    filter::kalman kalman_gyro_filter;
    filter::kalman kalman_gyro_filter2;

    filter::biquad_lowpass biquad_gyro_filter[3] = {
      { 90, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
      { 90, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
      { 90, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
    };

    filter::biquad_lowpass accel_filter[3] = {
      { 90, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
      { 90, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
      { 90, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
    };

    filter::biquad_lowpass angle_filter[3] = {
      { 90, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
      { 90, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
      { 90, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
    };
  };
}

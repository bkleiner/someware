#pragma once

#include "mixer.h"
#include "pid.h"
#include "config.h"
#include "gesture.h"

#include "rx/rx.h"
#include "util/timer.h"
#include "driver/board.h"

#include "platform/print.h"

namespace control
{
  class control {
  public:
    control(board* brd)
      : brd(brd)
      , cfg(*((config*)brd->flash().pointer()))
      , rate_pid(&cfg)
      , mix(brd)
    {
      brd->accel().set_gyro_bias(cfg.gyro_bias);
      brd->accel().set_accel_bias(cfg.accel_bias);

      seed_accel();
    }

    void update(float dt, rx::rx& recv) {
      update_gyro(dt);

      if (recv.get(rx::AUX1) > 0.5f) {
        if (!armed && recv.get(rx::THR) < -0.8f) {
          brd->accel().calibrate_accel();
          brd->accel().calibrate_gyro();
          if (config_dirty) {
            cfg.save(brd->flash());
            config_dirty = false;
          }
          rate_pid.reset();
          armed = true;
        }
      } else {
        if (gesture.update(dt, recv, cfg)) {
          config_dirty = true;
        }
        armed = false;
      }

      input_demands = {
        rc_filter[0].step(recv.get(rx::THR)) * rc_rate.throttle,
        rc_filter[3].step(recv.get(rx::AIL)) * rc_rate.roll,
        rc_filter[2].step(recv.get(rx::ELE)) * rc_rate.pitch,
        rc_filter[1].step(recv.get(rx::RUD)) * rc_rate.yaw
      };

      vector rates = {0, 0, 0};

      if (angle_mode)
      {
        const float roll_angle = -input_demands.roll * angle_limit_deg * util::deg_to_rad;
        const float pitch_angle = -input_demands.pitch * angle_limit_deg * util::deg_to_rad;

        stick_vector[0] = sin(pitch_angle);
        stick_vector[1] = sin(roll_angle);
        stick_vector[2] = cos(roll_angle) * cos(pitch_angle);

        const auto angle_rates = angle_pid.calc(dt, stick_vector, angel);
        rates = {
          angle_rates[vector::ROLL] + input_demands.roll * rate_limit_deg * util::deg_to_rad,
          angle_rates[vector::PITCH] + input_demands.pitch * rate_limit_deg * util::deg_to_rad,
          input_demands.yaw * rate_limit_deg * util::deg_to_rad
        };
      } else {
        rates = {
          input_demands.roll * rate_limit_deg * util::deg_to_rad,
          input_demands.pitch * rate_limit_deg * util::deg_to_rad,
          input_demands.yaw * rate_limit_deg * util::deg_to_rad
        };
      }

      rates = rate_pid.calc(dt, is_airborn(), rates, gyro);
      output_demands = {
        input_demands.throttle,
        rates.roll(),
        rates.pitch(),
        rates.yaw()
      };

      if (is_armed()) {
        mix.set_demands(output_demands);
      } else {
        disarm();
      }
    }

    void disarm() {
      mix.set_all(0.0f);
    }

    bool is_armed() {
      return armed;
    }

    bool is_airborn() {
      return armed && output_demands.throttle > -0.2f;
    }

    void seed_accel() {
      for (size_t i = 0; i < 100; i++) {
        const auto& accel = brd->accel().read_accel();
        angel = filter::simple_lowpass(angel, accel, 0.85f);
        platform::time::delay_us(1000);
      }
    }

    float lpfcalc(float sampleperiod, float filtertime) {
      if (sampleperiod <= 0)
        return 0;
      if (filtertime <= 0)
        return 1;
      float ga = expf(-1.0f / ((1.0f / sampleperiod) * (filtertime)));
      if (ga > 1)
        ga = 1;
      return ga;
    }

    void update_gyro(float dt) {
      const float dts = dt / 1000.f;

      vector gyro_new = brd->accel().read_gyro();
      
      gyro_new[vector::ROLL] = -gyro_new[vector::ROLL];

      for (int i = 0; i < 3; i++) {
        gyro[i] = gyro_new[i] * util::deg_to_rad;
        gyro[i] = gyro_filter.step(i, gyro[i]);
			  gyro[i] = gyro_filter2.step(i, gyro[i]);
      }

      accel = brd->accel().read_accel();

      #define ssin(val) (val)
      #define scos(val) 1.0f

      const auto angle_delta = gyro * dts;
      angel[2] = scos(angle_delta[0]) * angel[2] - ssin(angle_delta[0]) * angel[0];
      angel[0] = ssin(angle_delta[0]) * angel[2] + scos(angle_delta[0]) * angel[0];

      angel[1] = scos(angle_delta[1]) * angel[1] + ssin(angle_delta[1]) * angel[2];
      angel[2] = -ssin(angle_delta[1]) * angel[1] + scos(angle_delta[1]) * angel[2];

      angel[0] = scos(angle_delta[2]) * angel[0] - ssin(angle_delta[2]) * angel[1];
      angel[1] = ssin(angle_delta[2]) * angel[0] + scos(angle_delta[2]) * angel[1];

      #undef ssin
      #undef scos

      angel = filter::simple_lowpass(angel, vector::normalize(accel), lpfcalc(dts, 2.0f));
    }

    void calibrate_gyro() {
      cfg.reset();
      cfg.gyro_bias = brd->accel().calibrate_gyro();
      cfg.accel_bias = brd->accel().calibrate_accel();
      cfg.save(brd->flash());
    }

    bool armed = false;
    
    vector gyro = {0, 0, 0};
    vector accel = {0, 0, 0};
    vector angel = {0, 0, 1.0};
    vector stick_vector = {0, 0, 0};
    
    demands input_demands = {0, 0, 0, 0};
    demands output_demands = {0, 0, 0, 0};

    board* brd;
    config cfg;
    pid::rate_controller rate_pid;
    pid::angle_controller angle_pid;
    gesture_controller gesture;

  private:
    bool angle_mode = false;
    bool config_dirty = false;

    const float rate_limit_deg = 860.f;
    const float angle_limit_deg = 65.0f;
    const demands rc_rate = {
      1.0f, // T
      0.5f, // A
      0.5f, // E
      0.5f  // R
    };
    
    
    mixer mix;
    
    filter::kalman gyro_filter;
    filter::kalman gyro_filter2;

    filter::biquad_lowpass rc_filter[4] = {
      { 120, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
      { 120, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
      { 120, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
      { 120, LOOP_FREQ_HZ, filter::biquad_lowpass::butterworth },
    };
  };
}

#pragma once

#include "filter.h"

#include "driver/board.h"

namespace control {

  struct demands {
    float throttle; // T
    float roll; 	  // A
    float pitch;	  // E
    float yaw;	    // R
  };

  class mixer {
  public:
    struct motor_mix {
      int8_t throttle; // T
      int8_t roll; 	   // A
      int8_t pitch;	   // E
      int8_t yaw;	     // R
    };

    mixer(board* brd)
      : brd(brd)
    {}

    void set_all(float v) {
      for (uint8_t i = 0; i < motor_count; i++)      
        brd->motor(motor::motors(i)).set(0.f);
    }

    void set_demands(demands d) {
      float thr = (d.throttle + 1.0f) / 2.0f;
      for (uint8_t i = 0; i < motor_count; i++) {
          motors[i] = 
              (thr     * motor_mixes[i].throttle + 
               d.roll  * motor_mixes[i].roll +     
               d.pitch * motor_mixes[i].pitch +   
               d.yaw   * motor_mixes[i].yaw);      
      }

      float max_motor = motors[0];
      for (uint8_t i = 1; i < motor_count; i++)
        if (motors[i] > max_motor)
          max_motor = motors[i];

      for (uint8_t i = 0; i < motor_count; i++) {

          // This is a way to still have good gyro corrections if at least one motor reaches its max
          if (max_motor > 1) {
              motors[i] -= max_motor - 1;
          }

          // Keep motor values in interval [0,1]
          motors[i] = filter::constrain_min_max(motors[i], 0, 1);
      }

      for (uint8_t i = 0; i < motor_count; i++) {
        brd->motor(motor::motors(i)).set(motors[i]);
      }
    }

  private:
    static const uint8_t motor_count = 4;
    const motor_mix motor_mixes[motor_count] = {
      // T   R   P   Y
      { +1, +1, +1, +1 }, // 1 rear left
      { +1, +1, -1, -1 }, // 2 front left
      { +1, -1, +1, -1 }, // 3 rear right
      { +1, -1, -1, +1 }  // 4 front  right
    };
    float motors[motor_count];

    board* brd;
  };
}
#pragma once

#include "config.h"
#include "rx/rx.h"

namespace control {
  
  struct gesture_controller {
    void update(float dt, rx::rx& recv, config& cfg) {
      vector* term = nullptr;

      if (recv.get(rx::AUX2) < -0.5) {
        term = &cfg.pid_kp;
      } else if (recv.get(rx::AUX2) > -0.5 && recv.get(rx::AUX2) < 0.5) {
        term = &cfg.pid_ki;
      } else if (recv.get(rx::AUX2) > 0.5) {
        term = &cfg.pid_kd;
      }

      if (term && recv.get(rx::THR) > 0.9f) {
        modify_value(recv, &(*term)[vector::ROLL], rx::ELE, vector::ROLL);
        modify_value(recv, &(*term)[vector::PITCH], rx::ELE, vector::PITCH);
        //modify_value(recv, &(*term)[vector::YAW], rx::AIL, vector::YAW);
      }
    }

    void modify_value(rx::rx& recv, float* value, rx::channels input, uint8_t axis) {

      if (recv.get(input) > active_threshold) {
        if (!axis_active[axis])
          *value += 0.01f;

        axis_active[axis] = true;
      } else if (recv.get(input) < -active_threshold) {
        if (!axis_active[axis])
          *value -= 0.01f;

        axis_active[axis] = true;
      } else {
        axis_active[axis] = false;
      }
    }

    const float active_threshold = 0.8f;
    bool axis_active[3] = {
      false,
      false,
      false
    };
  };

}
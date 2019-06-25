#pragma once

namespace control::filter {

  static float constrain_min_max(float val, float min, float max) {
    return (val < min) ? min : ((val > max) ? max : val);
  }
  
}
#pragma once

namespace util {

  static const constexpr float pi = 3.14159265358979323846;
  static const constexpr float deg_to_rad = (pi / 180.0f);
  static const constexpr float rad_to_deg = (180.0f / pi);

  template<class T>
  static T min(const T a, const T b) {
    return a < b ? a : b;
  }

  static float map(float value, float start1, float stop1, float start2, float stop2) {
    return start2 + (stop2 - start2) * ((value - start1) / (stop1 - start1));
  }

}
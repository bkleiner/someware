#pragma once

namespace util {

  template<class T>
  static T min(const T a, const T b) {
    return a < b ? a : b;
  }

  static float map(float value, float start1, float stop1, float start2, float stop2) {
    return start2 + (stop2 - start2) * ((value - start1) / (stop1 - start1));
  }

}
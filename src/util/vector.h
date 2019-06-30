#pragma once

#include <math.h>

class vector {
public:
  enum AXIS {
    ROLL,
    PITCH,
    YAW
  };

  vector()
    : vector(0.f)
  {}

  vector(float v) {
    values[0] = v;
    values[1] = v;
    values[2] = v;
  }

  vector(float x, float y, float z) {
    values[0] = x;
    values[1] = y;
    values[2] = z;
  }

  float& operator[](int index) {
    return values[index];
  }

  const float& operator[](int index) const {
    return values[index];
  }


  float x() const {
    return values[0];
  }

  float y() const {
    return values[1];
  }

  float z() const {
    return values[2];
  }


  float roll() const {
    return values[0];
  }

  float pitch() const {
    return values[1];
  }

  float yaw() const {
    return values[2];
  }

  float length() const {
    return sqrt(x() * x() + y() * y() + z() * z());
  }

  static vector normalize(const vector& v) {
    const auto len = v.length();
    if (len == 0) {
      return {0, 0, 0};
    }
    return v / len;
  }

  static vector cross(const vector& v0, const vector& v1) {
    return vector{
      v0[1] * v1[2] - v0[2] * v1[1],
      v0[0] * v1[2] - v0[2] * v1[0],
      v0[0] * v1[1] - v0[1] * v1[0] 
    };
  }

  vector operator -(const vector& rhs) const {
    return vector{
      values[0] - rhs[0],
      values[1] - rhs[1],
      values[2] - rhs[2]
    };
  }

  vector operator +(const vector& rhs) const {
    return vector{
      values[0] + rhs[0],
      values[1] + rhs[1],
      values[2] + rhs[2]
    };
  }

  vector operator *(float v) const {
    return vector{
      values[0] * v,
      values[1] * v,
      values[2] * v
    };
  }

  vector operator *(const vector& v) const {
    return vector{
      values[0] * v[0],
      values[1] * v[1],
      values[2] * v[2]
    };
  }

  vector operator /(float v) const {
    return vector{
      values[0] / v,
      values[1] / v,
      values[2] / v
    };
  }

private:
  float values[3];
};
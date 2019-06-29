#pragma once

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

  vector operator -(const vector& rhs) const {
    return vector{
      values[0] - rhs[0],
      values[1] - rhs[1],
      values[2] - rhs[2]
    };
  }

  vector operator *(float v) const {
    return vector{
      values[0] * v,
      values[1] * v,
      values[2] * v
    };
  }

private:
  float values[3];
};
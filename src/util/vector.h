#pragma once

class vector {
public:
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

  vector(const vector& other)
    : vector(other.x(), other.y(), other.z())
  {}

  vector& operator=(const vector& other) {
    if (this != &other) {
      values[0] = other.values[0];
      values[1] = other.values[1];
      values[2] = other.values[2];
    }
    return *this;
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

private:
  float values[3];
};
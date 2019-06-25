#pragma once

#include "pwm.h"
#include "accel.h"
#include "serial.h"

namespace motor
{
  enum motors {
    MOTOR0,
    MOTOR1,
    MOTOR2,
    MOTOR3,
  };
}


class board {
public:
  virtual bool usb_serial_active() = 0;
  virtual serial& usb_serial() = 0;

  virtual float millis() = 0;
  virtual uint64_t micros() = 0;

  virtual accel::accel& accel() = 0;
  virtual pwm::pwm& motor(motor::motors) = 0;

  virtual void reset() = 0;
  virtual void reset_to_bootloader() = 0;
};
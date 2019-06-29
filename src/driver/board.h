#pragma once

#include "pwm.h"
#include "accel.h"
#include "serial.h"
#include "flash.h"
#include "gpio.h"

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
  board();

  virtual bool usb_serial_active() = 0;
  virtual serial& usb_serial() = 0;

  virtual float millis() = 0;
  virtual uint32_t micros() = 0;
  virtual void delay_us(uint64_t us) = 0;

  virtual gpio::pin& led() = 0;
  virtual accel::accel& accel() = 0;
  virtual flash_memory& flash() = 0;
  virtual pwm::pwm& motor(motor::motors) = 0;

  virtual void reset() = 0;
  virtual void reset_to_bootloader() = 0;
};
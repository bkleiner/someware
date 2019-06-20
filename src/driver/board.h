#pragma once

#include "serial.h"

class board {
public:
  virtual bool usb_serial_active() = 0;

  virtual void reset() = 0;
  virtual void reset_to_bootloader() = 0;
};
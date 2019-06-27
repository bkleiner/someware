#pragma once

#include "driver/flash.h"

#include "util/util.h"

namespace control {

  struct config {
    vector gyro_bias;

    void save(flash_memory& flash) {
      buffer<uint32_t> buf(this, sizeof(config));
      flash.write(buf);
    }
  };

}
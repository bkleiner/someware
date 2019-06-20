#pragma once

#include "util/util.h"

namespace rx {

  enum channels {
    THR, AIL, ELE, RUD,
    AUX1, AUX2
  };

  class rx {
  public:
    rx(int32_t low, int32_t high)
      : low(low), high(high)
    {}

    void update() {
      if (!read_channels(channel_data)) {
        // failsafe?
      }
    }

    float get(channels chan) {
      update();
      return util::map(channel_data[chan], low, high, -1, 1);
    }

  protected:
    virtual bool read_channels(int32_t* channel_data) = 0;

  private:
    int32_t low, high;
    int32_t channel_data[16];
  };
}
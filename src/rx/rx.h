#pragma once

#include "util/util.h"

namespace rx {

  #define MAX_CHANNELS 16

  enum channels {
    THR, AIL, ELE, RUD,
    AUX1, AUX2
  };

  class rx {
  public:
    rx(uint32_t low, uint32_t high)
      : low(low), high(high)
      , channel_data(MAX_CHANNELS)
    {
      reset();
    }

    void update() {
      if (!read_channels(channel_data)) {
        // failsafe?
        // reset();
      }
    }

    float get(channels chan) {
      return util::map(channel_data[chan], low, high, -1, 1);
    }

  protected:
    virtual bool read_channels(buffer<uint32_t>& channel_data) = 0;

  private:
    uint32_t low, high;
    buffer<uint32_t> channel_data;

    void reset() {
      channel_data[THR] = low;
      for (size_t i = THR+1; i < AUX1+1; i++) {
        channel_data[i] = (high + low) / 2;
      }

      for (size_t i = AUX1+1; i < MAX_CHANNELS; i++) {
        channel_data[i] = low;
      }
    }
  };
}
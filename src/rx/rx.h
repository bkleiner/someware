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
      const auto v = get_raw(chan);
      return (v - rc_midpoint) / 1000.f;
    }

    uint32_t get_raw(channels chan) {
      return channel_data[chan];
    }

  protected:
    virtual bool read_channels(buffer<uint32_t>& channel_data) = 0;

  private:
    uint32_t low, high;
    buffer<uint32_t> channel_data;
    const float rc_midpoint = 993;
    const float rc_ranges[4][2] = {
      {172, 1815},
      {182, 1815},
      {184, 1815},
      {172, 1815}
    };

    void reset() {
      channel_data[THR] = 0;
      for (size_t i = THR+1; i < AUX1+1; i++) {
        channel_data[i] = (rc_ranges[i][0] + rc_ranges[i][1]) / 2;
      }

      for (size_t i = AUX1+1; i < MAX_CHANNELS; i++) {
        channel_data[i] = 0;
      }
    }
  };
}
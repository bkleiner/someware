#pragma once

#include "control/config.h"
#include "util/util.h"

namespace rx {

  #define MAX_CHANNELS 16

  enum channels {
    THR, AIL, ELE, RUD,
    AUX1, AUX2, AUX3, AUX4
  };

  class rx {
  public:
    rx()
      : channel_data(MAX_CHANNELS)
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
      auto v = get_raw(chan);
      if (chan < 4) {
        v = util::map(v, control::rc_ranges[chan][0], control::rc_ranges[chan][1], 0, 2000);
      }
      return (v - control::rc_midpoint) / 1000.f;
    }

    uint32_t get_raw(channels chan) {
      return channel_data[chan];
    }

  protected:
    virtual bool read_channels(buffer<uint32_t>& channel_data) = 0;

  private:
    buffer<uint32_t> channel_data;
    
    void reset() {
      channel_data.fill(0);
    }
  };
}
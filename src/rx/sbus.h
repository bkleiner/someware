#pragma once

#include "rx.h"

namespace rx {

struct __attribute__((__packed__)) sbus_channel_data {
  // 176 bits of data (11 bits per channel * 16 channels) = 22 bytes.
  unsigned int chan0 : 11;
  unsigned int chan1 : 11;
  unsigned int chan2 : 11;
  unsigned int chan3 : 11;
  unsigned int chan4 : 11;
  unsigned int chan5 : 11;
  unsigned int chan6 : 11;
  unsigned int chan7 : 11;
  unsigned int chan8 : 11;
  unsigned int chan9 : 11;
  unsigned int chan10 : 11;
  unsigned int chan11 : 11;
  unsigned int chan12 : 11;
  unsigned int chan13 : 11;
  unsigned int chan14 : 11;
  unsigned int chan15 : 11;
  uint8_t flags;
};

#define SBUS_CHANNEL_DATA_LENGTH sizeof(sbus_channel_data)

#define SBUS_FRAME_SIZE (SBUS_CHANNEL_DATA_LENGTH + 2)
#define SBUS_FRAME_BEGIN_BYTE 0x0F

#define SBUS_FLAG_SIGNAL_LOSS       (1 << 2)
#define SBUS_FLAG_FAILSAFE_ACTIVE   (1 << 3)

class sbus : public rx {
public:
  enum sbus_states {
    IDLE,
    START,
    DONE
  };

  sbus(int32_t low, int32_t high)
    : rx(low, high)
  {}

  bool feed(uint8_t v);

protected:
  bool read_channels(int32_t* channel_data);

private:
  sbus_states state = IDLE;
  uint8_t frame[SBUS_FRAME_SIZE];
  uint8_t frame_length = 0;
};

}
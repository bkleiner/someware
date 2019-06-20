#include "sbus.h"

namespace rx {

bool sbus::feed(uint8_t v) {
  switch (state)
  {
  case IDLE:
    // we are idle, lets look for start
    if (v == SBUS_FRAME_BEGIN_BYTE) {
      frame_length = 0;
      frame[frame_length++] = v;

      state = START;
    }
    return false;
  case START:
    if (frame_length < SBUS_FRAME_SIZE) {
      frame[frame_length++] = v;
      return false;
    } else {
      state = DONE;
      return true;
    }
  case DONE:
    return true;
  }
  
  return false;
}

sbus_channel_data sbus::channels() {
  sbus_channel_data* data = reinterpret_cast<sbus_channel_data*>(frame + 1);
  state = IDLE;
  return *data;
}

}
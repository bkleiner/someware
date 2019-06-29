#include "sbus.h"

#include "platform/time.h"

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
      frame_start_us = platform::time::now_us();
    }
    return false;
  case START:
    if (frame_length < SBUS_FRAME_SIZE) {
      frame[frame_length++] = v;
      return false;
    }
    state = DONE;
    return true;
  case DONE:
    return true;
  }
  
  return false;
}

void sbus::update(serial& srl) {
  auto buf = srl.read();
  for (size_t i = 0; i < buf.size(); i++) {
    if (feed(buf[i])) {
      rx::update();
      i--;
    }
  }
}

bool sbus::read_channels(buffer<uint32_t>& channel_data) {
  if (state != DONE) {
    return false;
  }

  sbus_channel_data* data = reinterpret_cast<sbus_channel_data*>(frame.data() + 1);
  state = IDLE;

  if (data->flags & SBUS_FLAG_SIGNAL_LOSS || data->flags & SBUS_FLAG_FAILSAFE_ACTIVE) {
    return false;
  }

  channel_data[0] = data->chan0;
  channel_data[1] = data->chan1;
  channel_data[2] = data->chan2;
  channel_data[3] = data->chan3;
  channel_data[4] = data->chan4;
  channel_data[5] = data->chan5;
  channel_data[6] = data->chan6;
  channel_data[7] = data->chan7;
  channel_data[8] = data->chan8;
  channel_data[9] = data->chan9;
  channel_data[10] = data->chan10;
  channel_data[11] = data->chan11;
  channel_data[12] = data->chan12;
  channel_data[13] = data->chan13;
  channel_data[14] = data->chan14;
  channel_data[15] = data->chan15;

  frame.fill(0);
  return true;
}

}
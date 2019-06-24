#pragma once

#include "driver/board.h"
#include "rx/rx.h"

namespace control
{
  class control {
  public:
    void update(board& brd, rx::rx& recv) {
      if (!is_armed(recv)) {
        disarm(brd);
        return;
      }
      float thr = (recv.get(rx::THR) + 1.0f) / 2.0f;

      for (uint8_t i = 0; i < 4; i++)      
        brd.motor(motor::motors(i)).set(thr);
    }

    void disarm(board& brd) {
      for (uint8_t i = 0; i < 4; i++)      
        brd.motor(motor::motors(i)).set(0.f);
    }

    bool is_armed(rx::rx& recv) {
      return recv.get(rx::AUX1) > 0.0f;
    };
  };
}

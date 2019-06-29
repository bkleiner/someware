#pragma once

class board;

namespace platform {
  class locator {
  public:
    static board* brd() {
      return _brd;
    }

    static void provide(board* brd) {
      _brd = brd;
    }

  private:
    static board* _brd;
  };
}
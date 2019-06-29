#pragma once

namespace gpio {
  struct pin {
    virtual void low() = 0;
    virtual void high() = 0;
    virtual bool state() = 0;
  };
}

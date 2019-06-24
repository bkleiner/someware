#pragma once

#include "driver/stm32_f3/gpio.h"
#include "driver/stm32_f3/board.h"
#include "driver/stm32_f3/serial_vcp.h"

namespace stm32_f3 {
  class betaflight_f3 : public board {
  public:
    using USB_DM_PORT = gpio::port<gpio::A, 11>;
    using USB_DP_PORT = gpio::port<gpio::A, 12>;

    using UART2_RX_PORT = gpio::port<gpio::A, 3>;
    using UART2_TX_PORT = gpio::port<gpio::A, 4>;

    using SPI1_SCK_PORT  = gpio::port<gpio::B, 3>;
    using SPI1_MISO_PORT = gpio::port<gpio::B, 4>;
    using SPI1_MOSI_PORT = gpio::port<gpio::B, 5>;
    using SPI1_CS_PORT   = gpio::port<gpio::A, 15>;

    using MOTOR0_PORT = gpio::port<gpio::A, 6>;
    using MOTOR1_PORT = gpio::port<gpio::A, 7>;
    using MOTOR2_PORT = gpio::port<gpio::B, 8>;
    using MOTOR3_PORT = gpio::port<gpio::B, 9>;

  };
}

#pragma once

#include "driver/stm32_f3/pwm.h"
#include "driver/stm32_f3/gpio.h"
#include "driver/stm32_f3/spi.h"
#include "driver/stm32_f3/adc.h"
#include "driver/stm32_f3/board.h"
#include "driver/stm32_f3/serial_vcp.h"
#include "driver/stm32_f3/serial_uart.h"

#include "driver/accel/mpu_6000.h"

namespace stm32_f3 {
  class betaflight_f3 : public board {
  public:
    betaflight_f3()
      : mpu(&spi1, &mpu_cs_pin)
    {}

    using USB_DM_PORT = gpio::port<gpio::A, 11>;
    using USB_DP_PORT = gpio::port<gpio::A, 12>;

    using SPI1_SCK_PORT  = gpio::port<gpio::B, 3>;
    using SPI1_MISO_PORT = gpio::port<gpio::B, 4>;
    using SPI1_MOSI_PORT = gpio::port<gpio::B, 5>;
    spi spi1;

    gpio::pin<
      gpio::port<gpio::A, 15>,
      gpio::mode_output_pp
    > mpu_cs_pin;

    accel::mpu_6000 mpu;

    accel::accel& accel() override {
      return mpu;
    }
    
    gpio::pin<
      gpio::port<gpio::C, 15>,
      gpio::mode_output_pp,
      true
    > led_pin;

    ::gpio::pin& led() {
      return led_pin;
    }

    pwm<
      timer::TIMER3, 
      channel::CHANNEL1, 
      gpio::port<gpio::A, 6>,
      GPIO_AF_2
    > motor0;

    pwm<
      timer::TIMER3,
      channel::CHANNEL2,
      gpio::port<gpio::A, 7>,
      GPIO_AF_2
    > motor1;

    pwm<
      timer::TIMER4,
      channel::CHANNEL3,
      gpio::port<gpio::B, 8>,
      GPIO_AF_2
    > motor2;

    pwm<
      timer::TIMER4,
      channel::CHANNEL4,
      gpio::port<gpio::B, 9>,
      GPIO_AF_2
    > motor3;

    ::pwm::pwm& motor(motor::motors m) {
      switch (m) {
      case motor::MOTOR0:
        return motor0;
      case motor::MOTOR1:
        return motor1;
      case motor::MOTOR2:
        return motor2;
      case motor::MOTOR3:
        return motor3;
      }
      return motor0;
    }

    using UART1_TX_PORT = gpio::port<gpio::A, 9>;
    using UART1_RX_PORT = gpio::port<gpio::A, 10>;

    uart::serial<
      uart::UART2,
      gpio::port<gpio::A, 2>,
      gpio::port<gpio::A, 3>
    > uart2;

    using UART3_TX_PORT = gpio::port<gpio::B, 10>;
    using UART3_RX_PORT = gpio::port<gpio::B, 11>;

    adc::adc<
      adc::DEVICE2,
      gpio::port<gpio::A, 4>
    > vbat_adc;
  };
}

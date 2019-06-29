#pragma once

#include <stm32f30x.h>
#include <stm32f30x_conf.h>

#include "driver/gpio.h"

#include "util/util.h"

namespace stm32_f3::gpio {

  enum names {
    A, B, C, D
  };

  template<names _name, uint8_t _index>
  struct port {
    static const names name = _name;
    static const uint8_t index = _index;

    static inline uint16_t value() {
      return 1 << index;
    }

    static inline GPIO_TypeDef* get() {
      switch (name) {
      case A:
        return GPIOA;
      case B:
        return GPIOB;
      case C:
        return GPIOC;
      case D:
        return GPIOD;
      }
    }

    static inline void enable() {
      switch (name)
      {
      case A:
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
        break;
      case B:
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
        break;
      case C:
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
        break;
      case D:
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOD, ENABLE);
        break;
      }
    }
  };

  namespace modes
  {
    enum io_mode
    {
      input = GPIO_Mode_IN,
      output = GPIO_Mode_OUT,
      alternate = GPIO_Mode_AF,
      analog = GPIO_Mode_AN,
    };

    enum out_type
    {
      push_pull = GPIO_OType_PP,
      open_drain = GPIO_OType_OD
    };

    enum speed
    {
      speed_2MHz = GPIO_Speed_2MHz,
      speed_10MHz = GPIO_Speed_10MHz,
      speed_50MHz = GPIO_Speed_50MHz
    };

    enum pull_mode
    {
      no_pull = GPIO_PuPd_NOPULL,
      pull_up = GPIO_PuPd_UP,
      pull_down = GPIO_PuPd_DOWN
    };
  }

  template <
    modes::io_mode mod,
    modes::out_type otype,
    modes::speed speed = modes::speed_50MHz,
    modes::pull_mode pull = modes::no_pull
  >
  using mode = util::bit_pack<mod, otype, speed, pull>;

  using mode_af = mode<
    modes::alternate,
    modes::push_pull,
    modes::speed_50MHz,
    modes::no_pull
  >;

  using mode_output_pp = gpio::mode<
    gpio::modes::output,
    gpio::modes::push_pull,
    gpio::modes::speed_50MHz,
    gpio::modes::no_pull
  >;

  template <
    typename port,
    typename mode,
    uint8_t af = 0,
    bool invert = false
  >
  struct pin : public ::gpio::pin {

    pin() {
      port::enable();

      if (mode::get(0) == modes::alternate) {
        GPIO_PinAFConfig(port::get(), port::index, af);
      }

      GPIO_InitTypeDef gpio_init;
      gpio_init.GPIO_Pin = port::value();
      gpio_init.GPIO_Mode = GPIOMode_TypeDef(mode::get(0));
      gpio_init.GPIO_OType = GPIOOType_TypeDef(mode::get(1));
      gpio_init.GPIO_Speed = GPIOSpeed_TypeDef(mode::get(2));
      gpio_init.GPIO_PuPd = GPIOPuPd_TypeDef(mode::get(3));
      GPIO_Init(port::get(), &gpio_init);

      if (invert) {
        GPIO_SetBits(port::get(), port::value());
      }
    }

    void low() override {
      if (invert) {
        GPIO_SetBits(port::get(), port::value());
      } else  {
        GPIO_ResetBits(port::get(), port::value());
      }
    }

    void high() override {
      if (invert) {
        GPIO_ResetBits(port::get(), port::value());
      } else  {
        GPIO_SetBits(port::get(), port::value());
      }
    }

    bool state() override {
      if (invert) {
        return !GPIO_ReadOutputDataBit(port::get(), port::value());
      }
      return GPIO_ReadOutputDataBit(port::get(), port::value());
    }
  };
}
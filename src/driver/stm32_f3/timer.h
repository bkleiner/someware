#pragma once

#include <stm32f30x.h>
#include <stm32f30x_conf.h>

namespace stm32_f3::timer {
  enum names {
    TIMER1,
    TIMER8
  };

  template<names _name>
  struct hardware {
    static inline TIM_TypeDef* get() {
      switch (_name) {
      case TIMER1:
        return TIM1;
      case TIMER8:
        return TIM8;
      }
    }

    static inline void enable() {
      switch (_name) {
      case TIMER1:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
        break;
      case TIMER8:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
        break;
      }
    }

    static inline uint32_t clock() {
      return SystemCoreClock;
    }
  };

  template<
    names _name,
    uint16_t period, 
    uint32_t hz
  >
  struct timer {
    using hw = hardware<_name>;

    timer() {
      hw::enable();

      TIM_TimeBaseInitTypeDef timer_base_init;
      timer_base_init.TIM_Period = (period - 1) & 0xFFFF;
      timer_base_init.TIM_Prescaler = (hw::clock() / hz) - 1;
      timer_base_init.TIM_ClockDivision = 0;
      timer_base_init.TIM_CounterMode = TIM_CounterMode_Up;
      TIM_TimeBaseInit(hw::get(), &timer_base_init);

      TIM_Cmd(hw::get(), ENABLE);
    }

  };
}


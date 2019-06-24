#pragma once

#include <stm32f30x.h>
#include <stm32f30x_conf.h>

namespace stm32_f3::timer {
  enum names {
    TIMER1,
    TIMER3,
    TIMER4,
    TIMER8,
    TIMER16
  };

  template<names _name>
  struct hardware {
    static inline TIM_TypeDef* get() {
      switch (_name) {
      case TIMER1:
        return TIM1;
      case TIMER3:
        return TIM3;
      case TIMER4:
        return TIM4;
      case TIMER8:
        return TIM8;
      case TIMER16:
        return TIM16;
      }
    }

    static inline void enable() {
      switch (_name) {
      case TIMER1:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
        break;
      case TIMER3:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
        break;
      case TIMER4:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
        break;
      case TIMER8:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
        break;
      case TIMER16:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);
        break;
      }
    }
  };

  template<
    names _name,
    uint16_t period, 
    uint32_t prescaler
  >
  struct timer {
    using hw = hardware<_name>;

    timer() {
      hw::enable();

      TIM_TimeBaseInitTypeDef timer_base_init;
      timer_base_init.TIM_Prescaler = prescaler - 1;
      timer_base_init.TIM_Period = period - 1;
      timer_base_init.TIM_ClockDivision = 0;
      timer_base_init.TIM_CounterMode = TIM_CounterMode_Up;
      TIM_TimeBaseInit(hw::get(), &timer_base_init);

      TIM_Cmd(hw::get(), ENABLE);
    }

  };
}


#pragma once

#include <stm32f30x.h>
#include <stm32f30x_conf.h>

#include "timer.h"

namespace stm32_f3::channel {
  enum names {
    CHANNEL1,
    CHANNEL2,
    CHANNEL3,
    CHANNEL4
  };

  template<names _name>
  struct hardware {
    static inline void init(TIM_TypeDef* tim, TIM_OCInitTypeDef* def) {
      switch (_name) {
      case CHANNEL1:
        TIM_OC1Init(tim, def);
        break;
      case CHANNEL2:
        TIM_OC2Init(tim, def);
        break;
      case CHANNEL3:
        TIM_OC3Init(tim, def);
        break;
      case CHANNEL4:
        TIM_OC4Init(tim, def);
        break;
      }
    }

    static inline void set_compare(TIM_TypeDef* tim, uint32_t v) {
      switch (_name) {
      case CHANNEL1:
        TIM_SetCompare1(tim, v);
        break;
      case CHANNEL2:
        TIM_SetCompare2(tim, v);
        break;
      case CHANNEL3:
        TIM_SetCompare3(tim, v);
        break;
      case CHANNEL4:
        TIM_SetCompare4(tim, v);
        break;
      }
    }
  };

  template<
    names _name,
    typename _timer,
    uint32_t _mode,
    uint32_t _pulse
  >
  struct channel {
    using hw = hardware<_name>;
    using timer = _timer;
    
    channel() {
      TIM_OCInitTypeDef channel_init;
      channel_init.TIM_OCMode = _mode;
      channel_init.TIM_OutputState = TIM_OutputState_Enable;
      channel_init.TIM_OCPolarity = TIM_OCPolarity_High;
      channel_init.TIM_OCIdleState = TIM_OCIdleState_Set;
      channel_init.TIM_Pulse = _pulse;
      hw::init(timer::hw::get(), &channel_init);
    }

    void set_compare(uint32_t v) {
      hw::set_compare(timer::hw::get(), v);
    }
  };
}
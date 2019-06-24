#pragma once

#include <cstdint>
#include <stm32f30x.h>
#include <stm32f30x_conf.h>
#include <delegate/delegate.hpp>

namespace stm32_f3 {
  using irq_delegate_type = delegate<void()>;

  template<uint8_t channel>
  struct irq_handler {
    static void enable() {
      NVIC_InitTypeDef nvic_init;
      nvic_init.NVIC_IRQChannel = channel;
      nvic_init.NVIC_IRQChannelPreemptionPriority = 0;
      nvic_init.NVIC_IRQChannelSubPriority = 0;
      nvic_init.NVIC_IRQChannelCmd = ENABLE;
      NVIC_Init(&nvic_init);
    }

    static irq_delegate_type value;
  };

  template<uint8_t channel>
  irq_delegate_type irq_handler<channel>::value;

}
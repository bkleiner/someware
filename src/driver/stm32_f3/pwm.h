#pragma once

#include "driver/pwm.h"

#include "gpio.h"
#include "timer.h"
#include "channel.h"

#define PWM_FREQ 32000
#define PWM_CLOCK_FREQ 72000000
#define PWM_DIVIDER 1
#define PWM_PERIOD (PWM_CLOCK_FREQ / PWM_FREQ)

namespace stm32_f3 {

  template <
    timer::names _timer,
    channel::names _channel,
    typename _port,
    uint8_t _af
  >
  struct pwm : public ::pwm::pwm {
    using pin_type = gpio::pin<_port, gpio::mode_af, _af>;
    using timer_type = timer::timer<_timer, PWM_PERIOD, PWM_DIVIDER>;
    using channel_type = channel::channel<_channel, timer_type, TIM_OCMode_PWM1, 0>;

    pin_type pin;
    timer_type timer;
    channel_type channel;

    pwm() {
      TIM_CtrlPWMOutputs(timer_type::hw::get(), ENABLE);
    }

    void set(float v) {
      channel.set_compare(uint32_t(v * PWM_PERIOD));
    }
  };
}
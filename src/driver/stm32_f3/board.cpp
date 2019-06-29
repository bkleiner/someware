#include "board.h"

#include <stm32f30x.h>
#include <stm32f30x_conf.h>

#include "usb/usb.h"

volatile uint32_t systick_count = 0;
extern "C" void SysTick_Handler(void) {
  systick_count++;
}

namespace stm32_f3 {

rc_controller::rc_controller() {
  // enable gpio clocks
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

  SysTick_Config(SystemCoreClock/1000);
}

board::board() 
  : rcc()
  , vcp()
{
  if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  }
}
 
bool board::usb_serial_active() {
  return bDeviceState == CONFIGURED;
}

float board::millis() {
  return float(micros()) / 1000.0f;
}

uint32_t board::micros() {
  return cycles() / (SystemCoreClock / 1000000L);
}

void board::delay_us(uint64_t us) {
  volatile uint32_t start = DWT->CYCCNT;
  volatile uint32_t delay = us * (SystemCoreClock / 1000000L);
  while (DWT->CYCCNT - start < delay) {
    asm volatile ("nop");
  }
}

uint32_t board::cycles() {
  return DWT->CYCCNT;
}

void board::reset() {
  NVIC_PriorityGroupConfig(0x04);
}

void board::reset_to_bootloader() {
  // 1FFFF000 -> 20000200 -> SP
  // 1FFFF004 -> 1FFFF021 -> PC

  *((uint32_t *)0x20009FFC) = 0xDEADBEEF; // 40KB SRAM STM32F30X

  reset();
}

}
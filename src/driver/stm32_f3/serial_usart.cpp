#include "serial_usart.h"

#include <stm32f30x.h>
#include <stm32f30x_conf.h>

#include "util/util.h"
#include "gpio.h"

/*
USART1:
  TX: A09
  RX: A10
USART2:
  TX: A02
  RX: A03
USART3:
  TX: B10
  RX: B11
*/
uint8_t rx_buffer[256];
uint8_t rx_length = 0;

extern "C" void USART2_IRQHandler(void) {
  if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
    rx_buffer[rx_length++] = USART_ReceiveData(USART2);

    if (USART_GetFlagStatus(USART2, USART_FLAG_ORE)) {
      USART_ClearFlag(USART2, USART_FLAG_ORE);
    } 
	}
}

namespace stm32_f3 {

serial_usart::serial_usart() {
  // Enable clock for USART2
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

  gpio::pin<gpio::port<gpio::A, 3>, gpio::mode_af, GPIO_AF_7>();

  USART_Cmd(USART2, DISABLE);

  // Initialization of USART2
  USART_InitTypeDef USART_InitStruct;
  USART_InitStruct.USART_BaudRate = 100000;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStruct.USART_Mode =  USART_Mode_Rx;
  USART_InitStruct.USART_Parity = USART_Parity_Even;
  USART_InitStruct.USART_StopBits = USART_StopBits_2;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_Init(USART2, &USART_InitStruct);

  // USART_SWAPPinCmd(USART2, ENABLE);
  USART_InvPinCmd(USART2, USART_InvPin_Rx | USART_InvPin_Tx, ENABLE);
  // USART_HalfDuplexCmd(USART2, ENABLE);

  // Enable USART2
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
  USART_ITConfig(USART2, USART_IT_TXE, DISABLE);

  USART_Cmd(USART2, ENABLE);

  NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

buffer<uint8_t> serial_usart::read() {
  if (rx_length) {
    USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
    buffer<uint8_t> buf(rx_buffer, rx_length);
    rx_length = 0;
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    return buf;
  }
  // USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
  return buffer<uint8_t>();
}

bool serial_usart::write(uint8_t val) {

}

}
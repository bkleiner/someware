#include "serial_usart.h"

#include <stm32f30x.h>
#include <stm32f30x_conf.h>

#include "interrupt.h"
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
namespace stm32_f3 {

serial_usart::serial_usart() 
  : rx_buf(256)
{
  // Enable clock for USART2
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

  gpio::pin<gpio::port<gpio::A, 3>, gpio::mode_af, GPIO_AF_7>();

  USART_Cmd(USART2, DISABLE);

  USART_InitTypeDef usart_init;
  usart_init.USART_BaudRate = 100000;
  usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  usart_init.USART_Mode =  USART_Mode_Rx;
  usart_init.USART_Parity = USART_Parity_Even;
  usart_init.USART_StopBits = USART_StopBits_2;
  usart_init.USART_WordLength = USART_WordLength_8b;
  USART_Init(USART2, &usart_init);

  // USART_SWAPPinCmd(USART2, ENABLE);
  USART_InvPinCmd(USART2, USART_InvPin_Rx | USART_InvPin_Tx, ENABLE);
  // USART_HalfDuplexCmd(USART2, ENABLE);

  // Enable USART2
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
  USART_ITConfig(USART2, USART_IT_TXE, DISABLE);

  USART_Cmd(USART2, ENABLE);

  irq_handler<USART2_IRQn>::enable();
  irq_handler<USART2_IRQn>::value.set<serial_usart, &serial_usart::isr>(*this);
}

void serial_usart::isr() {
  if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
    rx_buf.write(USART_ReceiveData(USART2));

    if (USART_GetFlagStatus(USART2, USART_FLAG_ORE)) {
      USART_ClearFlag(USART2, USART_FLAG_ORE);
    } 
	}
}

buffer<uint8_t> serial_usart::read() {
  if (rx_buf.empty()) {
    return buffer<uint8_t>();
  }

  USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
  
  buffer<uint8_t> buf(rx_buf.count());
  for (size_t i = 0; i < buf.size(); i++) {
    rx_buf.read(buf.data() + i);
  }
  
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
  return buf;
}

bool serial_usart::write(uint8_t val) {
  return false;
}

}
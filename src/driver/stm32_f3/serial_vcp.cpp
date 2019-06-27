#include "serial_vcp.h"

#include <stm32f30x.h>
#include <stm32f30x_conf.h>
#include <delegate/delegate.hpp>

#include "gpio.h"
#include "usb/usb.h"

extern LINE_CODING linecoding;

volatile uint8_t Receive_Buffer[VIRTUAL_COM_PORT_DATA_SIZE];
volatile uint8_t Receive_length = 0;

volatile uint32_t packet_sent = 0;
volatile uint32_t packet_receive = 1;

delegate<void()> tx_delegate;

extern "C" void EP1_IN_Callback (void) {
  tx_delegate();
}

extern "C" void EP3_OUT_Callback(void) {
  Receive_length = GetEPRxCount(ENDP3);
  PMAToUserBufferCopy((unsigned char*)Receive_Buffer, ENDP3_RXADDR, Receive_length);
  packet_receive = 1;
}

namespace stm32_f3 {

serial_vcp::serial_vcp()
  : tx_buf(512)
{
  tx_delegate.set<serial_vcp, &serial_vcp::tx_callback>(*this);

  /*SET PA11,12 for USB: USB_DM,DP*/
  gpio::pin<gpio::port<gpio::A, 11>, gpio::mode_af, GPIO_AF_14>();
  gpio::pin<gpio::port<gpio::A, 12>, gpio::mode_af, GPIO_AF_14>();

  /**********************************************************************/
  /*  Configure the EXTI line 18 connected internally to the USB IP     */
  /**********************************************************************/
  EXTI_ClearITPendingBit(EXTI_Line18);

  EXTI_InitTypeDef EXTI_InitStructure;
  EXTI_InitStructure.EXTI_Line = EXTI_Line18;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  // usb clock
  RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5); /* Select USBCLK source */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE); /* Enable the USB clock */

  // usb interrupts
  NVIC_InitTypeDef NVIC_InitStructure;

  /* 2 bit for pre-emption priority, 2 bits for subpriority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

#if defined(STM32F37X)
  /* Enable the USB interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* Enable the USB Wake-up interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USBWakeUp_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
#else
  /* Enable the USB interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* Enable the USB Wake-up interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USBWakeUp_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_Init(&NVIC_InitStructure);   
#endif

  USB_Init();
}

void serial_vcp::tx_callback() {
  packet_sent = 0;
}

bool serial_vcp::is_active() {
  return bDeviceState == CONFIGURED;
}

buffer<uint8_t> serial_vcp::read() {
  if (is_active() && packet_receive) {
    buffer<uint8_t> buf((uint8_t*)Receive_Buffer, Receive_length);

    SetEPRxValid(ENDP3);
    Receive_length = 0;
    packet_receive = 0;

    return buf;
  }
  return buffer<uint8_t>();
}

bool serial_vcp::flush() {
  if (!is_active() || tx_buf.empty()) {
    return false;
  }

  if (packet_sent) {
    return false;
  }

  static uint8_t send_buffer[64];

  uint32_t count = 0;
  for (size_t i = 0; i < VIRTUAL_COM_PORT_DATA_SIZE - 1; i++) {
    if (!tx_buf.read(send_buffer + i)) {
      break;
    }
    count++;
  }
  if (count) {
    UserToPMABufferCopy((unsigned char*)send_buffer, ENDP1_TXADDR, count);
    SetEPTxCount(ENDP1, count);
    packet_sent = count;
    SetEPTxValid(ENDP1);
  }

  return true;
}

bool serial_vcp::write(uint8_t val) {
  return tx_buf.write(val);
}

}
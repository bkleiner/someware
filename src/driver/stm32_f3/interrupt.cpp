#include "interrupt.h"

#include "usb/usb.h"

extern "C" {
  void NMI_Handler(void) {

  }

  void HardFault_Handler(void) {
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1) {}
  }

  void MemManage_Handler(void) {
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1)
    {}
  }

  void BusFault_Handler(void) {
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {}
  }

  void UsageFault_Handler(void) {
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {}
  }

  void SVC_Handler(void) {

  }

  void DebugMon_Handler(void) {

  }

  void PendSV_Handler(void) {

  }


  #if defined(STM32L1XX_MD) || defined(STM32L1XX_HD)|| defined(STM32L1XX_MD_PLUS)|| defined (STM32F37X)
  void USB_LP_IRQHandler(void)
  #else
  void USB_LP_CAN1_RX0_IRQHandler(void)
  #endif
  {
    USB_Istr();
  }

  #if defined(STM32L1XX_MD) || defined(STM32L1XX_HD)|| defined(STM32L1XX_MD_PLUS)
  void USB_FS_WKUP_IRQHandler(void)
  #else
  void USBWakeUp_IRQHandler(void)
  #endif
  {
    EXTI_ClearITPendingBit(EXTI_Line18);
  }

  void USART1_IRQHandler(void) {
    stm32_f3::irq_handler<USART1_IRQn>::value();
  }

  void USART2_IRQHandler(void) {
    stm32_f3::irq_handler<USART2_IRQn>::value();
  }

  void USART3_IRQHandler(void) {
    stm32_f3::irq_handler<USART3_IRQn>::value();
  }
}

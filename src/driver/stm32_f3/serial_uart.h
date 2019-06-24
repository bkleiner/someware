#pragma once

#include <stm32f30x.h>
#include <stm32f30x_conf.h>

#include "interrupt.h"
#include "gpio.h"

#include "driver/serial.h"
#include "util/util.h"

namespace stm32_f3::uart {

  enum names {
    UART1,
    UART2,
    UART3,
  };

  template<names _name>
  struct hardware {
    static inline USART_TypeDef* get() {
      switch (_name) {
      case UART1:
        return USART1;
      case UART2:
        return USART2;
      case UART3:
        return USART3;
      }
    }

    static constexpr uint8_t interrupt() {
      switch (_name) {
      case UART1:
        return USART1_IRQn;
      case UART2:
        return USART2_IRQn;
      case UART3:
        return USART3_IRQn;
      default:
        return 0;
      }
    }

    static inline void enable() {
      switch (_name) {
      case UART1:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
        break;
      case UART2:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
        break;
      case UART3:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
        break;
      }
    }
  };

  template<
    names _name
  >
  struct uart {
    using hw = hardware<_name>;

    uart() {
      hw::enable();

      USART_Cmd(hw::get(), DISABLE);

      USART_InitTypeDef usart_init;
      usart_init.USART_BaudRate = 100000;
      usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
      usart_init.USART_Mode =  USART_Mode_Rx;
      usart_init.USART_Parity = USART_Parity_Even;
      usart_init.USART_StopBits = USART_StopBits_2;
      usart_init.USART_WordLength = USART_WordLength_8b;
      USART_Init(hw::get(), &usart_init);

      // USART_SWAPPinCmd(hw::get(), ENABLE);
      USART_InvPinCmd(hw::get(), USART_InvPin_Rx | USART_InvPin_Tx, ENABLE);
      // USART_HalfDuplexCmd(hw::get(), ENABLE);

      USART_ITConfig(hw::get(), USART_IT_RXNE, ENABLE);
      USART_ITConfig(hw::get(), USART_IT_TXE, DISABLE);

      USART_Cmd(hw::get(), ENABLE);
    }
  };

  template<
    names _name,
    typename _tx_port,
    typename _rx_port
  >
  class serial : public ::serial {
  public:
    using rx_pin_type = gpio::pin<_rx_port, gpio::mode_af, GPIO_AF_7>;
    using uart_type = uart<_name>;
    using interrupt_type = irq_handler<uart_type::hw::interrupt()>;

    serial()
      : rx_buf(256)
    {
      using serial_type = serial<_name, _tx_port, _rx_port>;
      
      interrupt_type::enable();
      interrupt_type::value
        .template set<
          serial_type,
          &serial_type::isr
        >(*this);
    }

    buffer<uint8_t> read() {
      if (rx_buf.empty()) {
        return buffer<uint8_t>();
      }

      USART_ITConfig(uart_type::hw::get(), USART_IT_RXNE, DISABLE);
      
      buffer<uint8_t> buf(rx_buf.count());
      for (size_t i = 0; i < buf.size(); i++) {
        rx_buf.read(buf.data() + i);
      }
      
      USART_ITConfig(uart_type::hw::get(), USART_IT_RXNE, ENABLE);
      return buf;
    }

    bool write(uint8_t) {
      return false;
    }

  private:
    ring_buffer<uint8_t> rx_buf;

    rx_pin_type rx_pin;
    uart_type device;

    void isr() {
      if (USART_GetITStatus(uart_type::hw::get(), USART_IT_RXNE) == RESET) {
        return;
      }

      rx_buf.write(USART_ReceiveData(uart_type::hw::get()));

      if (USART_GetFlagStatus(uart_type::hw::get(), USART_FLAG_ORE)) {
        USART_ClearFlag(uart_type::hw::get(), USART_FLAG_ORE);
      } 
    }
  };
  
}


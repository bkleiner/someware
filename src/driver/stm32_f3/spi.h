#pragma once

#include <strings.h>

#include <stm32f30x.h>
#include <stm32f30x_conf.h>

#include "driver/spi.h"

#include "gpio.h"

namespace stm32_f3::spi {

  enum names {
    DEVICE1,
    DEVICE2,
    DEVICE3,
  };

  template<names _name>
  struct hardware {
    static inline SPI_TypeDef* get() {
      switch (_name) {
      case DEVICE1:
        return SPI1;
      case DEVICE2:
        return SPI2;
      case DEVICE3:
        return SPI3;
      }
    }

    static inline void enable() {
      switch (_name) {
      case DEVICE1:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
        RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, ENABLE);
        break;
      case DEVICE2:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
        RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2, ENABLE);
        break;
      case DEVICE3:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
        RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI3, ENABLE);
        break;
      }
    }
  };
  
  template<
    names _name,
    typename _sck_port,
    typename _miso_port,
    typename _mosi_port,
    bool leadingEdge = false
  >
  class spi : public ::spi {
  public:
    using hw = hardware<_name>;

    using sck_pin_type = gpio::pin<_sck_port, gpio::mode_af, GPIO_AF_5>;
    using miso_pin_type = gpio::pin<_miso_port, gpio::mode_af, GPIO_AF_5>;
    using mosi_pin_type = gpio::pin<_mosi_port, gpio::mode_af, GPIO_AF_5>;

    spi() {
      hw::enable();

      SPI_I2S_DeInit(hw::get());

      SPI_InitTypeDef spi_init;
      spi_init.SPI_Mode = SPI_Mode_Master;
      spi_init.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
      spi_init.SPI_DataSize = SPI_DataSize_8b;
      spi_init.SPI_NSS = SPI_NSS_Soft;
      spi_init.SPI_FirstBit = SPI_FirstBit_MSB;
      spi_init.SPI_CRCPolynomial = 7;
      spi_init.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
      if (leadingEdge) {
        spi_init.SPI_CPOL = SPI_CPOL_Low;
        spi_init.SPI_CPHA = SPI_CPHA_1Edge;
      } else {
        spi_init.SPI_CPOL = SPI_CPOL_High;
        spi_init.SPI_CPHA = SPI_CPHA_2Edge;
      }

      SPI_Init(hw::get(), &spi_init);

      #ifdef STM32F303xC
      SPI_RxFIFOThresholdConfig(hw::get(), SPI_RxFIFOThreshold_QF);
      #endif

      SPI_CalculateCRC(hw::get(), DISABLE);
      SPI_Cmd(hw::get(), ENABLE);
    }

    void set_divisor(::spi::clock_divider divisor) {
      #define BIT(x) (1 << (x))
      #define BR_BITS ((BIT(5) | BIT(4) | BIT(3)))
      SPI_Cmd(SPI1, DISABLE);

      const uint16_t tempRegister = (SPI1->CR1 & ~BR_BITS);
      SPI1->CR1 = tempRegister | (divisor ? ((ffs(divisor | 0x100) - 2) << 3) : 0);

      SPI_Cmd(SPI1, ENABLE);
      #undef BR_BITS
      #undef BIT
    }

    uint8_t transfer(uint8_t v)  {
      for (uint16_t spiTimeout = 1000; SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET; ) {
        if ((spiTimeout--) == 0)
          return 0xff;
      }

      #ifdef STM32F303xC
      SPI_SendData8(SPI1, v);
      #else
      SPI_I2S_SendData(SPI1, v);
      #endif

      for (uint16_t spiTimeout = 1000; SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET; ) {
        if ((spiTimeout--) == 0)
          return 0xff;
      }

      #ifdef STM32F303xC
      return ((uint8_t)SPI_ReceiveData8(SPI1));
      #else
      return ((uint8_t)SPI_I2S_ReceiveData(SPI1));
      #endif
    }

  private:
    sck_pin_type sck_pin;
    miso_pin_type miso_pin;
    mosi_pin_type mosi_pin;
  };
  
}


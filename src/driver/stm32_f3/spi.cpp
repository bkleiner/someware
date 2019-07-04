#include "spi.h"

#include <strings.h>

#include <stm32f30x.h>
#include <stm32f30x_conf.h>

#include "gpio.h"

namespace stm32_f3 {

  bool leadingEdge = false;

  spi::spi() {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1, ENABLE);
    
    // PB3 = SCK; PB4 = MISO; PB5 = MOSI
    gpio::pin<gpio::port<gpio::B, 3>, gpio::mode_af, GPIO_AF_5>();
    gpio::pin<gpio::port<gpio::B, 4>, gpio::mode_af, GPIO_AF_5>();
    gpio::pin<gpio::port<gpio::B, 5>, gpio::mode_af, GPIO_AF_5>();

    SPI_I2S_DeInit(SPI1);

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


#ifdef STM32F303xC
    // Configure for 8-bit reads.
    SPI_RxFIFOThresholdConfig(SPI1, SPI_RxFIFOThreshold_QF);
#endif

    SPI_Init(SPI1, &spi_init);
    SPI_CalculateCRC(SPI1, DISABLE);
    SPI_Cmd(SPI1, ENABLE);
  }

  uint8_t spi::transfer(uint8_t v) {
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

  void spi::set_divisor(::spi::clock_divider divisor) {
#define BIT(x) (1 << (x))
#define BR_BITS ((BIT(5) | BIT(4) | BIT(3)))
    SPI_Cmd(SPI1, DISABLE);

    const uint16_t tempRegister = (SPI1->CR1 & ~BR_BITS);
    SPI1->CR1 = tempRegister | (divisor ? ((ffs(divisor | 0x100) - 2) << 3) : 0);

    SPI_Cmd(SPI1, ENABLE);
#undef BR_BITS
#undef BIT
  }
}
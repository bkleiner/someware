#pragma once

#include <stm32f30x.h>
#include <stm32f30x_conf.h>

#include "gpio.h"

namespace stm32_f3::adc {
  enum names {
    DEVICE1,
    DEVICE2,
    DEVICE3,
    DEVICE4,
  };

  template<names _name>
  struct hardware {
    static inline ADC_TypeDef* get() {
      switch (_name) {
      case DEVICE1:
        return ADC1;
      case DEVICE2:
        return ADC2;
      case DEVICE3:
        return ADC3;
      case DEVICE4:
        return ADC4;
      }
    }

    static inline void enable() {
      switch (_name) {
      case DEVICE1:
        RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div256);
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC12, ENABLE);
        break;
      case DEVICE2:
        RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div256);
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC12, ENABLE);
        break;
      case DEVICE3:
        RCC_ADCCLKConfig(RCC_ADC34PLLCLK_Div256);
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC34, ENABLE);
        break;
      case DEVICE4:
        RCC_ADCCLKConfig(RCC_ADC34PLLCLK_Div256);
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC34, ENABLE);
        break;
      }
    }
  };

  template<
    names _name,
    typename _port
  >
  struct adc {
    using hw = hardware<_name>;
    using pin_mode = gpio::mode<
      gpio::modes::analog,
      gpio::modes::push_pull,
      gpio::modes::speed_50MHz,
      gpio::modes::no_pull
    >;
    using pin_type = gpio::pin<_port, pin_mode>;

    adc() {
      hw::enable();

      ADC_DeInit(hw::get());

      ADC_VoltageRegulatorCmd(hw::get(), ENABLE);
      for(int i = 0; i < 100000; i++);

      ADC_SelectCalibrationMode(hw::get(), ADC_CalibrationMode_Single);
      ADC_StartCalibration(hw::get());

      while(ADC_GetCalibrationStatus(hw::get()) != RESET );
      calibration_value = ADC_GetCalibrationValue(hw::get());

      ADC_InitTypeDef adc_init;
      adc_init.ADC_ContinuousConvMode    = ADC_ContinuousConvMode_Enable;
      adc_init.ADC_Resolution            = ADC_Resolution_12b;
      adc_init.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;
      adc_init.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
      adc_init.ADC_DataAlign             = ADC_DataAlign_Right;
      adc_init.ADC_OverrunMode           = ADC_OverrunMode_Disable;
      adc_init.ADC_AutoInjMode           = ADC_AutoInjec_Disable;
      adc_init.ADC_NbrOfRegChannel       = 1;
      ADC_Init(hw::get(), &adc_init);

      ADC_RegularChannelConfig(hw::get(), ADC_Channel_1, 1, ADC_SampleTime_1Cycles5);
      
      ADC_Cmd(hw::get(), ENABLE);

      while(!ADC_GetFlagStatus(hw::get(), ADC_FLAG_RDY));
      ADC_StartConversion(hw::get());
    }

    uint16_t read() {
      while(ADC_GetFlagStatus(hw::get(), ADC_FLAG_EOC) == RESET);
      return ((ADC_GetConversionValue(hw::get()) + calibration_value) * 3300) / 0xFFF;
    }

    uint16_t calibration_value;
  };
}


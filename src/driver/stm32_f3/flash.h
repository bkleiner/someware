#pragma once

#include <stm32f30x.h>
#include <stm32f30x_conf.h>

#include "driver/flash.h"
#include "util/buffer.h"

namespace stm32_f3 {
  class flash_memory : public ::flash_memory {
  public:
    static const constexpr uint32_t address = 0x0803F000;
    static const constexpr uint32_t page_size = 1024; // 1KB

    void write(const buffer<uint32_t>& buf) override {
      FLASH_Unlock();

      FLASH_ErasePage(address);
      for (size_t i = 0; i < buf.size(); i++) {
        if (i * sizeof(uint32_t) >= page_size)
          break;

        int status = FLASH_ProgramWord(address + i * sizeof(uint32_t), buf[i]);
        if (status != FLASH_COMPLETE) {
          break;
        }
      }

      FLASH_Lock();
    }

    uint8_t* pointer() override {
      return (uint8_t*)address;
    }

  };
}
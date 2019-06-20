TARGET=someware
OBJECT_DIR=build

CXX=arm-none-eabi-g++
CC=arm-none-eabi-gcc
LD=arm-none-eabi-g++
AR=arm-none-eabi-ar
AS=arm-none-eabi-as
CP=arm-none-eabi-objcopy
OD=arm-none-eabi-objdump
SE=arm-none-eabi-size
DFU=dfu-util

ARCH_FLAGS      = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fsingle-precision-constant -fno-exceptions -fno-non-call-exceptions -ffunction-sections -fdata-sections
DEVICE_FLAGS    = -DSTM32F303xC -DSTM32F303 -DUSE_STDPERIPH_DRIVER

INCLUDE = src src/target/stm32_f3 \
	lib/delegate/include \
	lib/CMSIS/Core/Include \
	lib/STM32_USB-FS-Device_Driver/inc \
	lib/STM32F3/Drivers/CMSIS/Device/ST/STM32F30x \
	lib/STM32F3/Drivers/STM32F30x_StdPeriph_Driver/inc
	

SRC = $(TARGET_STARTUP) $(shell find src -name *.cpp -or -name *.c) \
	$(wildcard lib/STM32_USB-FS-Device_Driver/src/*.c) \
	$(wildcard lib/STM32F3/Drivers/CMSIS/Device/ST/STM32F30x/*.c) \
	$(wildcard lib/STM32F3/Drivers/STM32F30x_StdPeriph_Driver/src/*.c)
	
TARGET_OBJS = $(addsuffix .o,$(addprefix $(OBJECT_DIR)/$(TARGET)/,$(basename $(SRC))))
TARGET_DIR = src/target/stm32_f3
TARGET_STARTUP = $(TARGET_DIR)/startup_stm32f30x_md_gcc.S
TARGET_LD_SCRIPT = $(TARGET_DIR)/stm32_flash_f303_256k.ld

OPTIMIZE = -g  -Wall
CFLAGS   = $(OPTIMIZE) $(ARCH_FLAGS) $(DEVICE_FLAGS)
CXXFLAGS = -fno-rtti -std=c++11 $(OPTIMIZE) $(ARCH_FLAGS) $(DEVICE_FLAGS)
LDFLAGS  = -lm -lc -lnosys --specs=nano.specs -u _printf_float -nostartfiles $(ARCH_FLAGS) $(DEVICE_FLAGS) -static -Wl,-L$(TARGET_DIR) -T$(TARGET_LD_SCRIPT) -Wl,-gc-sections
ASFLAGS  = $(ARCH_FLAGS) -x assembler-with-cpp $(addprefix -I,$(INCLUDE)) -MMD -MP

.PHONY: $(TARGET)

$(TARGET): $(TARGET).bin

$(TARGET).bin: $(TARGET).elf
	$(CP) -O binary $(TARGET).elf $(TARGET).bin

$(TARGET).elf: $(TARGET_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ 

$(OBJECT_DIR)/$(TARGET)/%.o: %.c
	mkdir -p $(@D)
	$(CC) -c -o $@ $(addprefix -I,$(INCLUDE)) $(CFLAGS) $^ 

$(OBJECT_DIR)/$(TARGET)/%.o: %.cpp
	mkdir -p $(@D)
	$(CXX) -c -o $@ $(addprefix -I,$(INCLUDE)) $(CXXFLAGS) $^ 

$(OBJECT_DIR)/$(TARGET)/%.o: %.s
	mkdir -p $(dir $@)
	$(CC) -c -o $@ $(ASFLAGS) $<

$(OBJECT_DIR)/$(TARGET)/%.o: %.S
	mkdir -p $(dir $@)
	$(CC) -c -o $@ $(ASFLAGS) $<

clean:
	rm -rf $(OBJECT_DIR) $(TARGET).elf $(TARGET).bin

flash: $(TARGET).bin
	(echo -n 'R' > /dev/ttyACM0 && sleep 2) || true
	$(DFU) -a 0 -s 0x08000000:leave -R -D $(TARGET).bin
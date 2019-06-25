TARGET=someware

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

BUILD_DIR  = build
PLATFORM_DIR = src/platform/stm32_f3
TARGET_DIR = src/target/betaflight_f3

INCLUDE = src $(PLATFORM_DIR) \
	lib/delegate/include \
	lib/CMSIS/Core/Include \
	lib/STM32_USB-FS-Device_Driver/inc \
	lib/STM32F3/Drivers/CMSIS/Device/ST/STM32F30x \
	lib/STM32F3/Drivers/STM32F30x_StdPeriph_Driver/inc

LIB_SOURCE = $(wildcard lib/STM32_USB-FS-Device_Driver/src/*.c) \
	$(wildcard lib/STM32F3/Drivers/CMSIS/Device/ST/STM32F30x/*.c) \
	$(wildcard lib/STM32F3/Drivers/STM32F30x_StdPeriph_Driver/src/*.c)

COMMON_SOURCE = $(shell find src/* -name '*.cpp' -or -name '*.c')

TARGET_STARTUP   = $(PLATFORM_DIR)/startup_stm32f30x_md_gcc.S
TARGET_LD_SCRIPT = $(PLATFORM_DIR)/stm32_flash_f303_256k.ld

TARGET_SOURCE = src/main.cpp $(TARGET_STARTUP) $(COMMON_SOURCE) $(LIB_SOURCE)
TARGET_OBJS   = $(addsuffix .o,$(addprefix $(BUILD_DIR)/,$(basename $(TARGET_SOURCE))))
TARGET_DEPS   = $(addsuffix .d,$(addprefix $(BUILD_DIR)/,$(basename $(TARGET_SOURCE))))

TEST_FILES  = $(shell find test -name '*.cpp')
TEST_BINS   = $(patsubst test/%.cpp,$(BUILD_DIR)/test/%,$(TEST_FILES))

OPTIMIZE = -g -O2 -Wall
CFLAGS   = -MMD -MP $(OPTIMIZE) $(ARCH_FLAGS) $(DEVICE_FLAGS)
CXXFLAGS = -MMD -MP -fno-rtti -std=c++17 $(OPTIMIZE) $(ARCH_FLAGS) $(DEVICE_FLAGS)
HOST_CXXFLAGS = -fno-rtti -std=c++17 $(OPTIMIZE) $(DEVICE_FLAGS)
LDFLAGS  = -lm -lc -lnosys --specs=nano.specs -u _printf_float -nostartfiles $(ARCH_FLAGS) $(DEVICE_FLAGS) -static -Wl,-L$(PLATFORM_DIR) -T$(TARGET_LD_SCRIPT) -Wl,-gc-sections
ASFLAGS  = $(ARCH_FLAGS) -x assembler-with-cpp $(addprefix -I,$(INCLUDE)) -MMD -MP

.PHONY: all

all: $(TARGET) $(TEST_BINS)

$(TEST_BINS): $(TEST_FILES)
	mkdir -p $(@D)
	g++ -o $@ $(addprefix -I,$(INCLUDE)) $(HOST_CXXFLAGS) $^ 

$(TARGET): $(BUILD_DIR)/$(TARGET).bin

clean:
	rm -rf $(BUILD_DIR)

flash: $(BUILD_DIR)/$(TARGET).bin
	(echo -n 'R' > /dev/ttyACM0 && sleep 2) || true
	$(DFU) -a 0 -s 0x08000000:leave -R -D $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(CP) -O binary $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR)/$(TARGET).elf: $(TARGET_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ 

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(@D)
	$(CC) $(addprefix -I,$(INCLUDE)) $(CFLAGS) -c $< -o $@  

$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(@D)
	$(CXX) $(addprefix -I,$(INCLUDE)) $(CXXFLAGS) -c $< -o $@ 

$(BUILD_DIR)/%.o: %.s
	mkdir -p $(dir $@)
	$(CC) -c -o $@ $(ASFLAGS) $<

$(BUILD_DIR)/%.o: %.S
	mkdir -p $(dir $@)
	$(CC) -c -o $@ $(ASFLAGS) $<

-include $(TARGET_DEPS)
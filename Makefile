# =============================================================================
# OPTIMIZATION #
# =============================================================================
MAKEFLAGS += -j$(shell nproc 2>/dev/null || echo 1)



# =============================================================================
# PROJECT SETTINGS
# =============================================================================
OS := $(shell uname -s)

export PATH := $(PATH):$(shell pwd)/stm32cube/bin
export PATH := $(PATH):/opt/AppImages/ImageMagick

CNT_MNGR ?= podman

TARGET_NAME = cck-roo

ifeq ($(OS),Darwin)
BASE_ARDUINO    = $(HOME)/Library/Arduino15
BASE_USER_LIBS  = $(HOME)/Projects/ArduinoLibs/libraries/
else
BASE_ARDUINO    = $(HOME)/.arduino15
BASE_USER_LIBS  = $(HOME)/Arduino/libraries
endif

SERIAL_PORT ?= /dev/ttyACM0
BAUD_RATE ?= 9600

# Toolchain Path
TOOLCHAIN_PATH  = $(BASE_ARDUINO)/packages/STMicroelectronics/tools/xpack-arm-none-eabi-gcc/14.2.1-1.1/bin
CC      = $(TOOLCHAIN_PATH)/arm-none-eabi-gcc
CXX     = $(TOOLCHAIN_PATH)/arm-none-eabi-g++
AS      = $(TOOLCHAIN_PATH)/arm-none-eabi-gcc
OBJCOPY = $(TOOLCHAIN_PATH)/arm-none-eabi-objcopy
SIZE    = $(TOOLCHAIN_PATH)/arm-none-eabi-size

# Base Paths
ARDUINO_PACKAGES = $(BASE_ARDUINO)/packages/STMicroelectronics
STM32_CORE_PATH  = $(ARDUINO_PACKAGES)/hardware/stm32/2.12.0
USER_LIBS        = $(BASE_USER_LIBS)

# Output Directory
BUILD_DIR = build

# =============================================================================
# SOURCES
# =============================================================================

# Project Sources
PROJECT_SRCS  = $(wildcard *.cpp) $(wildcard *.c)

# Core Sources (Wiring, Variants, etc.)
CORE_SRCS += $(wildcard $(STM32_CORE_PATH)/cores/arduino/*.cpp)
CORE_SRCS += $(wildcard $(STM32_CORE_PATH)/cores/arduino/*.c)
CORE_SRCS += $(wildcard $(STM32_CORE_PATH)/cores/arduino/avr/*.c)
CORE_SRCS += $(wildcard $(STM32_CORE_PATH)/cores/arduino/stm32/*.c)
CORE_SRCS += $(wildcard $(STM32_CORE_PATH)/variants/STM32F4xx/F405RGT_F415RGT/*.cpp)
CORE_SRCS += $(wildcard $(STM32_CORE_PATH)/variants/STM32F4xx/F405RGT_F415RGT/*.c)

# Drivers (SrcWrapper, HAL, LL, USB)
# Note: SrcWrapper contains the system calls (_sbrk) and HAL wrappers
CORE_SRCS += $(wildcard $(STM32_CORE_PATH)/libraries/SrcWrapper/src/*.cpp)
CORE_SRCS += $(wildcard $(STM32_CORE_PATH)/libraries/SrcWrapper/src/*.c)
CORE_SRCS += $(wildcard $(STM32_CORE_PATH)/libraries/SrcWrapper/src/*/*.c)
CORE_SRCS += $(wildcard $(STM32_CORE_PATH)/libraries/SrcWrapper/src/*/*.cpp)
CORE_SRCS += $(wildcard $(STM32_CORE_PATH)/libraries/USBDevice/src/*.cpp)
CORE_SRCS += $(wildcard $(STM32_CORE_PATH)/libraries/USBDevice/src/*.c)
CORE_SRCS += $(wildcard $(STM32_CORE_PATH)/libraries/USBDevice/src/*/*.c)
CORE_SRCS += $(wildcard $(STM32_CORE_PATH)/libraries/Wire/src/*.cpp)
CORE_SRCS += $(wildcard $(STM32_CORE_PATH)/libraries/Wire/src/utility/*.c)
CORE_SRCS += $(wildcard $(STM32_CORE_PATH)/libraries/SPI/src/*.cpp)
CORE_SRCS += $(wildcard $(STM32_CORE_PATH)/libraries/SPI/src/utility/*.c)

# User Libraries
LIB_SRCS += $(wildcard $(USER_LIBS)/Adafruit_NeoPixel/*.cpp) $(wildcard $(USER_LIBS)/Adafruit_NeoPixel/*.c)
LIB_SRCS += $(wildcard $(USER_LIBS)/Adafruit_GFX_Library/*.cpp) $(wildcard $(USER_LIBS)/Adafruit_GFX_Library/*.c)
LIB_SRCS += $(wildcard $(USER_LIBS)/Adafruit_BusIO/*.cpp)
LIB_SRCS += $(wildcard $(USER_LIBS)/Adafruit_EPD/src/*.cpp) \
            $(wildcard $(USER_LIBS)/Adafruit_EPD/src/drivers/*.cpp)
LIB_SRCS += $(wildcard $(USER_LIBS)/SdFat_-_Adafruit_Fork/src/*.cpp) \
            $(wildcard $(USER_LIBS)/SdFat_-_Adafruit_Fork/src/*/*.cpp) \
            $(wildcard $(USER_LIBS)/SdFat_-_Adafruit_Fork/src/*/*/*.cpp) \
            $(wildcard $(USER_LIBS)/SdFat_-_Adafruit_Fork/src/*/*/*/*.cpp)
LIB_SRCS += $(wildcard $(USER_LIBS)/Adafruit_SPIFlash/src/*.cpp) \
            $(wildcard $(USER_LIBS)/Adafruit_SPIFlash/src/*/*.cpp)
LIB_SRCS += $(wildcard $(USER_LIBS)/Adafruit_ImageReader_Library/*.cpp)
LIB_SRCS += $(wildcard $(USER_LIBS)/QRCodeGFX/src/*.cpp) $(wildcard $(USER_LIBS)/QRCodeGFX/src/*.c)
LIB_SRCS += $(wildcard $(USER_LIBS)/PubSubClient/src/*.cpp)
LIB_SRCS += $(wildcard $(USER_LIBS)/STM32duino_LwIP/src/*.cpp) \
			$(wildcard $(USER_LIBS)/STM32duino_LwIP/src/utility/*.c)
# LIB_SRCS += $(wildcard $(USER_LIBS)/STM32duino_STM32Ethernet/src/*.cpp) \
# 			$(wildcard $(USER_LIBS)/STM32duino_STM32Ethernet/src/utility/*.cpp) \
# 			$(wildcard $(USER_LIBS)/STM32duino_STM32Ethernet/src/utility/*.c)
LIB_SRCS += $(wildcard $(USER_LIBS)/Ethernet/src/*.cpp) \
			$(wildcard $(USER_LIBS)/Ethernet/src/utility/*.cpp)
LIB_SRCS += $(wildcard $(USER_LIBS)/CRC/src/*.cpp)
SRCS = $(PROJECT_SRCS) $(CORE_SRCS) $(LIB_SRCS)

# Assembly Startup File (Essential for Reset_Handler)
ASM_SRCS = $(STM32_CORE_PATH)/system/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f405xx.s

# =============================================================================
# INCLUDES
# =============================================================================
INCLUDES  = -I.
INCLUDES += -I$(STM32_CORE_PATH)/cores/arduino/avr
INCLUDES += -I$(STM32_CORE_PATH)/cores/arduino/stm32
INCLUDES += -I$(STM32_CORE_PATH)/cores/arduino
INCLUDES += -I$(STM32_CORE_PATH)/variants/STM32F4xx/F405RGT_F415RGT
INCLUDES += -I$(STM32_CORE_PATH)/libraries/SrcWrapper/inc
INCLUDES += -I$(STM32_CORE_PATH)/libraries/SrcWrapper/inc/LL
INCLUDES += -I$(STM32_CORE_PATH)/system/Drivers/STM32F4xx_HAL_Driver/Inc
INCLUDES += -I$(STM32_CORE_PATH)/system/Drivers/STM32F4xx_HAL_Driver/Src
INCLUDES += -I$(STM32_CORE_PATH)/system/STM32F4xx
INCLUDES += -I$(STM32_CORE_PATH)/libraries/USBDevice/inc
INCLUDES += -I$(STM32_CORE_PATH)/system/Middlewares/ST/STM32_USB_Device_Library/Core/Inc
INCLUDES += -I$(STM32_CORE_PATH)/system/Middlewares/ST/STM32_USB_Device_Library/Core/Src
INCLUDES += -I$(STM32_CORE_PATH)/libraries/VirtIO/inc
INCLUDES += -I$(STM32_CORE_PATH)/system/Middlewares/OpenAMP
INCLUDES += -I$(STM32_CORE_PATH)/system/Middlewares/OpenAMP/open-amp/lib/include
INCLUDES += -I$(STM32_CORE_PATH)/system/Middlewares/OpenAMP/libmetal/lib/include
INCLUDES += -I$(STM32_CORE_PATH)/system/Middlewares/OpenAMP/virtual_driver
INCLUDES += -I$(STM32_CORE_PATH)/libraries/Wire/src
INCLUDES += -I$(STM32_CORE_PATH)/libraries/SPI/src
INCLUDES += -I$(ARDUINO_PACKAGES)/tools/CMSIS/6.2.0/CMSIS/Core/Include
INCLUDES += -I$(STM32_CORE_PATH)/system/Drivers/CMSIS/Device/ST/STM32F4xx/Include
INCLUDES += -I$(ARDUINO_PACKAGES)/tools/CMSIS_DSP/1.16.2/Include
INCLUDES += -I$(ARDUINO_PACKAGES)/tools/CMSIS_DSP/1.16.2/PrivateInclude
# Library Includes
INCLUDES += -I$(USER_LIBS)/Adafruit_NeoPixel
INCLUDES += -I$(USER_LIBS)/Adafruit_GFX_Library
INCLUDES += -I$(USER_LIBS)/Adafruit_BusIO
INCLUDES += -I$(USER_LIBS)/Adafruit_EPD/src
INCLUDES += -I$(USER_LIBS)/SdFat_-_Adafruit_Fork/src
INCLUDES += -I$(USER_LIBS)/Adafruit_SPIFlash/src
INCLUDES += -I$(USER_LIBS)/Adafruit_ImageReader_Library
INCLUDES += -I$(USER_LIBS)/QRCodeGFX/src
INCLUDES += -I$(USER_LIBS)/Ethernet/src
INCLUDES += -I$(USER_LIBS)/PubSubClient/src
INCLUDES += -I$(USER_LIBS)/CRC/src
# INCLUDES += -I$(USER_LIBS)/STM32duino_LwIP/src
# INCLUDES += -I$(USER_LIBS)/STM32duino_STM32Ethernet/src

# =============================================================================
# FLAGS
# =============================================================================

MCU_FLAGS = -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb

# QUOTING FIX: Ensure quotes persist through to the compiler
DEFINES  = -DSTM32F4xx -DARDUINO=10607 -DARDUINO_FEATHER_F405 -DARDUINO_ARCH_STM32
DEFINES += '-DBOARD_NAME="FEATHER_F405"' '-DVARIANT_H="variant_FEATHER_F405.h"'
DEFINES += -DSTM32F405xx
DEFINES += -DUSBCON -DUSBD_VID=0x0483 -DUSBD_PID=0x5740 -DHAL_PCD_MODULE_ENABLED
DEFINES += -DUSBD_USE_CDC -DHAL_UART_MODULE_ENABLED
DEFINES += -DVECT_TAB_OFFSET=0x0 -DUSE_HAL_DRIVER -DUSE_FULL_LL_DRIVER -DNDEBUG

# Compiler Flags
COMMON_FLAGS = $(MCU_FLAGS) -c -Os -w $(DEFINES) $(INCLUDES) \
               -ffunction-sections -fdata-sections --param max-inline-insns-single=500

CFLAGS   = $(COMMON_FLAGS) -std=gnu11
CXXFLAGS = $(COMMON_FLAGS) -std=gnu++17 -fno-threadsafe-statics -fno-rtti -fno-exceptions -fno-use-cxa-atexit
ASFLAGS  = $(MCU_FLAGS) -x assembler-with-cpp $(DEFINES) $(INCLUDES)

# Linker Flags
LDSCRIPT = $(STM32_CORE_PATH)/variants/STM32F4xx/F405RGT_F415RGT/ldscript.ld
LDFLAGS  = $(MCU_FLAGS) -Os -w --specs=nano.specs -Wl,--defsym=LD_FLASH_OFFSET=0x0 \
           -Wl,--defsym=LD_MAX_SIZE=1048576 -Wl,--defsym=LD_MAX_DATA_SIZE=131072 \
           -Wl,--cref -Wl,--check-sections -Wl,--gc-sections -Wl,--entry=Reset_Handler \
           -Wl,--unresolved-symbols=report-all -Wl,--warn-common \
           -Wl,--default-script=$(LDSCRIPT) -Wl,--script=$(STM32_CORE_PATH)/system/ldscript.ld \
           -Wl,-Map,$(BUILD_DIR)/$(TARGET_NAME).map -Wl,--no-warn-rwx-segments \
           -L$(BUILD_DIR) -lm -lgcc -lstdc++

# =============================================================================
# BUILD RULES
# =============================================================================

OBJS = $(addprefix $(BUILD_DIR)/, $(addsuffix .o, $(basename $(SRCS))))
ASM_OBJS = $(addprefix $(BUILD_DIR)/, $(addsuffix .o, $(basename $(ASM_SRCS))))

all: $(BUILD_DIR)/$(TARGET_NAME).elf $(BUILD_DIR)/$(TARGET_NAME).hex $(BUILD_DIR)/$(TARGET_NAME).bin size

$(BUILD_DIR)/$(TARGET_NAME).elf: $(OBJS) $(ASM_OBJS)
	@echo "Linking $@"
	@$(CC) $(OBJS) $(ASM_OBJS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling C++: $<"
	@$(CXX) $(CXXFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "Compiling C: $<"
	@$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	@echo "Compiling ASM: $<"
	@$(AS) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET_NAME).hex: $(BUILD_DIR)/$(TARGET_NAME).elf
	@$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)/$(TARGET_NAME).bin: $(BUILD_DIR)/$(TARGET_NAME).elf
	@$(OBJCOPY) -O binary $< $@

size: $(BUILD_DIR)/$(TARGET_NAME).elf
	@$(SIZE) -A $<

clean:
	rm -rf $(BUILD_DIR)

upload: $(BUILD_DIR)/$(TARGET_NAME).bin stm32cube
	@echo "Uploading..."
	sh $(ARDUINO_PACKAGES)/tools/STM32Tools/2.4.0/stm32CubeProg.sh -i dfu -f "$<" -o 0x0 -v 0x0483 -p 0xdf11 -a 0x8000000 -s 0x8000000

monitor:
	@echo "Opening serial monitor on $(SERIAL_PORT) at $(BAUD_RATE) baud..."
	@stty -F $(SERIAL_PORT) $(BAUD_RATE) raw -clocal -echo
	@cat $(SERIAL_PORT)

# Send: Send a message to the serial port
# Usage: make send MSG="Hello World"
send:
	@if [ -z "$(MSG)" ]; then \
		echo "Usage: make send MSG=\"Your Message\""; \
	else \
		echo "Sending '$(MSG)' to $(SERIAL_PORT)..."; \
		stty -F $(SERIAL_PORT) $(BAUD_RATE) raw -clocal -echo; \
		echo "$(MSG)" > $(SERIAL_PORT); \
	fi

# Term: Interactive session (requires 'screen' installed)
term:
	@echo "Opening interactive terminal on $(SERIAL_PORT)..."
	@echo "Press Ctrl+A then K to exit."
	@screen $(SERIAL_PORT) $(BAUD_RATE)


# =============================================================================
# Build STM32 Cube Programmer
# =============================================================================
CNT_MNGR ?= podman

install-CubePrgr: copy-stm32cube

stm32cube:
ifeq ($(OS),Linux)
	$(MAKE) install-CubePrgr
endif

copy-stm32cube: build-CubePrgr
	$(CNT_MNGR) create --name temp_container org.cirelli.containers/stm32cubeprogrammer
	$(CNT_MNGR) cp temp_container:/app/stm32cube ./stm32cube
	$(CNT_MNGR) rm temp_container

build-CubePrgr: stm32cubeprg-lin.zip
	$(CNT_MNGR) build --platform linux/amd64 -t org.cirelli.containers/stm32cubeprogrammer -f STM32Container .

run-arduino: install-CubePrgr
ifeq ($(OS),Darwin)
	open -a "Arduino IDE"
else ifeq ($(OS),Linux)
	/opt/AppImages/ArduinoIDE/arduino-ide.AppImage
endif
# =============================================================================


.PHONY: all clean size upload monitor send term build-CubePrgr run-arduinoide copy-stm32cube install-CubePrgr build-CubePrgr

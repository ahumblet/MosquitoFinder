# Source files 
SRCS=main.c system_stm32f4xx.c

MIC_SRCS = mic/microphone_functions.c mic/microphone.c mic/sound_loudness.c STM32F4-Discovery_FW_V1.1.0/Libraries/CMSIS/DSP_Lib/Source/StatisticsFunctions/arm_rms_f32.c mic/tm_stm32f4_fft.c mic/tm_stm32f4_adc.c mic/tm_stm32f4_dac_signal.c mic/tm_stm32f4_disco.c  mic/tm_stm32f4_delay.c mic/tm_stm32f4_timer_properties.c #mic/tm_stm32f4_sdram.c
SRCS += $(MIC_SRCS)
# Binary will be generated with this name (with .elf filename extension)
PROJ_NAME=MFind

# You should not need to change anything below this line!
#######################################################################################

BUILDDIR = build

# STM32F4 library code directory
STM_COMMON=STM32F4-Discovery_FW_V1.1.0

# Tools
CC=arm-none-eabi-gcc
LD=arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE=arm-none-eabi-size
ifeq ($(OS),Windows_NT)
MKDIR=busybox mkdir
else
MKDIR=mkdir
endif

#Debug
CFLAGS  = -ggdb -O0 -Wall

CFLAGS += -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -D__FPU_PRESENT=1 -DSTM32F40_41xxx -DUSE_STM32F4_DISCO -DSTM32F4XX

CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork -Xassembler -mimplicit-it=always
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -I.

# Include files from STM32 libraries
CFLAGS += -I$(STM_COMMON)/Utilities/STM32F4-Discovery
CFLAGS += -I$(STM_COMMON)/Libraries/CMSIS/Include -I$(STM_COMMON)/Libraries/CMSIS/ST/STM32F4xx/Include
CFLAGS += -I$(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/inc
#AH, adding for screen
CFLAGS += -I$(STM_COMMON)/Libraries/displayLibs/
CFLAGS += -I$(STM_COMMON)/Libraries/displayLibs/tm_stm32f4_ili9341
CFLAGS += -I$(STM_COMMON)/Libraries/displayLibs/tm_stm32f4_gpio
CFLAGS += -I$(STM_COMMON)/Libraries/displayLibs/tm_stm32f4_spi
CFLAGS += -I$(STM_COMMON)/Libraries/displayLibs/tm_stm32f4_dma
CFLAGS += -I$(STM_COMMON)/Libraries/displayLibs/tm_stm32f4_spi_dma
CFLAGS += -I$(STM_COMMON)/Libraries/displayLibs/tm_stm32f4_fonts

# Linker script
LDSCRIPT = stm32_flash.ld
LDFLAGS += -T$(LDSCRIPT) 

# add startup file to build
SRCS += $(STM_COMMON)/Libraries/CMSIS/ST/STM32F4xx/Source/Templates/TrueSTUDIO/startup_stm32f4xx.s 
# AH, adding for screen
SRCS += $(STM_COMMON)/Libraries/displayLibs/tm_stm32f4_ili9341/tm_stm32f4_ili9341.c
SRCS += $(STM_COMMON)/Libraries/displayLibs/tm_stm32f4_gpio/tm_stm32f4_gpio.c
SRCS += $(STM_COMMON)/Libraries/displayLibs/tm_stm32f4_spi/tm_stm32f4_spi.c
SRCS += $(STM_COMMON)/Libraries/displayLibs/tm_stm32f4_dma/tm_stm32f4_dma.c
SRCS += $(STM_COMMON)/Libraries/displayLibs/tm_stm32f4_spi_dma/tm_stm32f4_spi_dma.c
SRCS += $(STM_COMMON)/Libraries/displayLibs/tm_stm32f4_fonts/tm_stm32f4_fonts.c
SRCS += $(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.c
SRCS += $(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c
SRCS += $(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spi.c
SRCS += $(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c
SRCS += $(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/src/misc.c

OBJS = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(SRCS))))

SEMIHOSTING_FLAGS = --specs=rdimon.specs -lc -lrdimon -D ARM_MATH_CM4

ELF = $(PROJ_NAME).elf
BIN = $(PROJ_NAME).bin


$(BUILDDIR)/%.o: %.c
	$(MKDIR) -p $(dir $@)
	$(CC) -c $(SEMIHOSTING_FLAGS) $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: %.s
	$(MKDIR) -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: all

all: $(ELF) $(BIN)

$(ELF): $(OBJS)
	$(LD) $(LDFLAGS) $(SEMIHOSTING_FLAGS) $(CFLAGS) -o $@ $(OBJS) -L$(STM_COMMON)/Utilities/STM32F4-Discovery -lPDMFilter_IAR
	$(SIZE) $@

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@


clean:
	rm -f $(ELF) $(BIN) $(OBJS)


#######################################################
# Debugging targets
#######################################################
gdb: all
	arm-none-eabi-gdb -tui $(ELF)

# Start OpenOCD GDB server (supports semihosting)
openocd: 
	openocd -f board/stm32f4discovery.cfg 

flash: $(BIN)
	st-flash write $(BIN) 0x8000000


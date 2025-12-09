# Makefile to build and flash avrenv receiver
#
# Simplified version from: https://github.com/hexagon5un/AVR-Programming

MCU = avr64ea28
F_CPU = 10000000
BAUD = 9600
PROGRAMMER_TYPE = atmelice_updi
PROGRAMMER_ARGS = -B 125kHz

LORA = 1

# Display dimensions
DISPLAY_WIDTH = 320
DISPLAY_HEIGHT = 240
# true = BGR, false = RGB
BGR = true
# Invert color
INVERT = false
# Flip image
HFLIP = true
VFLIP = true

MAIN = avrenv.c
SRC = data.c rfm.c spi.c tft.c usart.c utils.c
LIB = -lrfm95 -ltft

CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AVRSIZE = avr-size
AVRDUDE = avrdude

# https://github.com/avrdudes/avr-libc/issues/673
# CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU)UL -DBAUD=$(BAUD)
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU)UL -DBAUDRATE=$(BAUD)
CFLAGS += -DLORA=$(LORA)
CFLAGS += -DDISPLAY_WIDTH=$(DISPLAY_WIDTH) -DDISPLAY_HEIGHT=$(DISPLAY_HEIGHT)
CFLAGS += -DINVERT=$(INVERT) -DBGR=$(BGR) -DHFLIP=$(HFLIP) -DVFLIP=$(VFLIP)
CFLAGS += -O2 -I. -I/usr/local/include/dode -L/usr/local/lib/dode
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums 
CFLAGS += -Wall -Wstrict-prototypes
CFLAGS += -g -ggdb
CFLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections -mrelax
CFLAGS += -std=gnu99
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105523
# CFLAGS += --param=min-pagesize=0

TARGET = $(strip $(basename $(MAIN)))
SRC += $(TARGET).c

OBJ = $(SRC:.c=.o)
OBJ = $(SRC:.S=.o)
	
$(TARGET).elf: data.h spi.h usart.h utils.h Makefile

all: $(TARGET).hex

%.hex: %.elf
	$(OBJCOPY) -R .eeprom -O ihex $< $@

%.elf: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIB) --output $@

disasm: $(TARGET).lst

eeprom: $(TARGET).eeprom

%.lst: %.elf
	$(OBJDUMP) -S $< > $@
 
size:  $(TARGET).elf
	$(AVRSIZE) --mcu=$(MCU) $(TARGET).elf

clean:
	rm -f $(TARGET).elf $(TARGET).hex $(TARGET).obj \
	$(TARGET).o $(TARGET).d $(TARGET).eep $(TARGET).lst \
	$(TARGET).lss $(TARGET).sym $(TARGET).map $(TARGET)~ \
	$(TARGET).eeprom

flash: $(TARGET).hex 
	$(AVRDUDE) -c $(PROGRAMMER_TYPE) -p $(MCU) $(PROGRAMMER_ARGS) -U flash:w:$<

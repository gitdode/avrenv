# Makefile to build and flash avrenv transmitter
#
# Simplified version from: https://github.com/hexagon5un/AVR-Programming

MCU = avr64ea28
F_CPU = 10000000
BAUD = 9600
PROGRAMMER_TYPE = atmelice_updi
PROGRAMMER_ARGS = -B 125kHz

# Supported RFM radio module variants: 69 and 95
RFM = 69
# LoRa available only with RFM95
LORA = 0

MAIN = avrenv.c
SRC = rfm.c spi.c usart.c bme68x/bme68x.c bme688.c ens160.c pa1616s.c

CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AVRSIZE = avr-size
AVRDUDE = avrdude

# https://github.com/avrdudes/avr-libc/issues/673
# CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU)UL -DBAUD=$(BAUD)
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU)UL -DBAUDRATE=$(BAUD)
CFLAGS += -DRFM=$(RFM) -DLORA=$(LORA)
CFLAGS += -O2 -I. -I../../
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums 
CFLAGS += -Wall -Wstrict-prototypes
CFLAGS += -g -ggdb
CFLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections -mrelax
CFLAGS += -std=gnu99
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105523
# CFLAGS += --param=min-pagesize=0

TARGET = $(strip $(basename $(MAIN)))
SRC += $(TARGET).c
SRC += ../../librfm$(RFM)/librfm$(RFM).a

OBJ = $(SRC:.c=.o)
OBJ = $(SRC:.S=.o)
	
$(TARGET).elf: spi.h usart.h utils.h bme68x/bme68x.h bme68x/bme68x_defs.h \
	bme688.h ens160.h pa1616s.h Makefile

all: $(TARGET).hex

%.hex: %.elf
	$(OBJCOPY) -R .eeprom -O ihex $< $@

%.elf: $(SRC)
	$(CC) $(CFLAGS) $(SRC) --output $@ 

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

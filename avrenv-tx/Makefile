# Makefile to build and flash avrenv transmitter
#
# Simplified version from: https://github.com/hexagon5un/AVR-Programming

MCU = avr64ea28
F_CPU = 10000000
BAUD = 9600
PROGRAMMER_TYPE = atmelice_updi
PROGRAMMER_ARGS = -B 125kHz

LORA = 1

MAIN = avrenv.c
SRC = data.c rfm.c sdc.c spi.c i2c.c usart.c utils.c bme68x/bme68x.c \
	bme688.c ens160.c pa1616s.c
LIB = -lrfm95 -lsdc

CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AVRSIZE = avr-size
AVRDUDE = avrdude

# https://github.com/avrdudes/avr-libc/issues/673
# CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU)UL -DBAUD=$(BAUD)
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU)UL -DBAUDRATE=$(BAUD)
CFLAGS += -DLORA=$(LORA)
CFLAGS += -O2 -I. -I/usr/local/include/dode -L/usr/local/lib/dode
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums 
CFLAGS += -Wall -Wstrict-prototypes
CFLAGS += -g -ggdb
CFLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections -mrelax
CFLAGS += -std=gnu23
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105523
# CFLAGS += --param=min-pagesize=0

TARGET = $(strip $(basename $(MAIN)))
SRC += $(TARGET).c

OBJ = $(SRC:.c=.o)
OBJ = $(SRC:.S=.o)
	
$(TARGET).elf: data.h spi.h i2c.h usart.h utils.h bme68x/bme68x.h \
	bme68x/bme68x_defs.h bme688.h ens160.h pa1616s.h Makefile

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

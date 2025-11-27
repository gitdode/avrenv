# Makefile to build avrenv client

MAIN = avrenv.c
SRC = serial.c data.c rest.c
HEADER = serial.h data.h rest.h Makefile

CC  = gcc
OBJDUMP = objdump
SIZE = size

CFLAGS = -std=gnu23 -g -ggdb -Wall -O2 -I. \
	-I/usr/include/x86_64-linux-gnu
LIBS = -lcurl -ljson-c

TARGET = $(strip $(basename $(MAIN)))
SRC += $(TARGET).c

$(TARGET): $(SRC) $(HEADER)
	$(CC) $(CFLAGS) $(SRC) $(LIBS) -o $@
	
disasm: $(TARGET).lst
	
%.lst: %
	$(OBJDUMP) -S $< > $@

size:  $(TARGET)
	$(SIZE) $(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).lst

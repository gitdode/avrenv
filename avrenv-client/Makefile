# Makefile to build and avrenv client

MAIN = avrenv.c
SRC = serial.c data.c
HEADER = serial.h data.h Makefile

CC = gcc
OBJDUMP = objdump

CFLAGS = -std=gnu23 -g -ggdb -Wall -O2 -I.

TARGET = $(strip $(basename $(MAIN)))
SRC += $(TARGET).c

$(TARGET): $(SRC) $(HEADER)
	$(CC) $(CFLAGS) $(SRC) -o $@
	
disasm: $(TARGET).lst
	
%.lst: %
	$(OBJDUMP) -S $< > $@

clean:
	rm -f $(TARGET) $(TARGET).lst

# Makefile to build and avrenv client

MAIN = avrenv.c
SRC = serial.c data.c

CC = gcc
CFLAGS = -std=gnu23 -g -ggdb -Wall -O2 -I.

TARGET = $(strip $(basename $(MAIN)))
SRC += $(TARGET).c

OBJ = $(SRC:.c=.o)
OBJ = $(SRC:.S=.o)
	
$(TARGET).elf: serial.h data.h Makefile

%.elf: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) 

clean:
	rm -f $(TARGET).elf $(TARGET).o $(TARGET)~

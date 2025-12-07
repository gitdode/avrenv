# Makefile to build avrenv client

MAIN = avrenv.c
SRC = serial.c data.c rest.c
TESTSRC = $(SRC) data_test.c
TESTCASES = testcases.c
HEADER = serial.h data.h rest.h Makefile
OBJ = serial.o data.o rest.o
TESTOBJ = data_test.o

CC  = gcc
OBJDUMP = objdump
SIZE = size
ACEUNIT = aceunit

CFLAGS = -std=gnu23 -g -ggdb -Wall -O2 -I. \
	-I/usr/include/x86_64-linux-gnu \
	-I/usr/local/include
LIBS = -lcurl -ljson-c
ACEUNIT_LIB = /usr/local/lib/libaceunit-fork.a

TARGET = $(strip $(basename $(MAIN)))
SRC += $(TARGET).c
TEST_TARGET = $(TARGET)-tests

$(TARGET): $(SRC) $(HEADER)
	$(CC) $(CFLAGS) $(SRC) $(LIBS) -o $@
	
test: $(TEST_TARGET)
	./$^
	
$(TEST_TARGET): $(TESTCASES)
	$(CC) $(CFLAGS) $(OBJ) $(TESTOBJ) $(TESTCASES) $(LIBS) $(ACEUNIT_LIB) -o $@
	
$(TESTCASES): $(TESTOBJ)
	$(ACEUNIT) $(TESTOBJ) > $@
	
$(TESTOBJ): $(TESTSRC) $(HEADER)
	$(CC) $(CFLAGS) $(TESTSRC) $(LIBS) -c
	
disasm: $(TARGET).lst
	
%.lst: %
	$(OBJDUMP) -S $< > $@

size:  $(TARGET)
	$(SIZE) $(TARGET)

clean:
	rm -f $(TESTCASES) *.o $(TARGET) $(TARGET).lst $(TEST_TARGET)

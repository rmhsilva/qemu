# Hello Make
# GDP 17, 2013

# This is the name of the Main C file (*without* .c extension)
TARGET = runTests

# These are files containing tests
TEST_OBJS = lru_access.o

# These are files from the cache system
SYS_OBJS = cache.o mips-cache-opts.o replacement_lru.o replacement_lfu.o replacement_rnd.o


# Set up the C compiler flags
CFLAGS = -DGDP17_TESTBENCH
CFLAGS += -DTARGET_MIPS
CFLAGS += -Wall
CFLAGS += `pkg-config --cflags glib-2.0`

LDFLAGS = `pkg-config --libs glib-2.0`

# Misc...
CC = gcc
RM = rm


# Default (first) target
all: begin $(TARGET) end

# Start and finish messages
begin:
	@echo
	@echo --- Starting ---
end:
	@echo
	@echo --- Done ---


# Compile target
$(TARGET): $(TARGET).o $(SYS_OBJS) $(TEST_OBJS)
	@echo
	@echo + Creating Target
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)


$(TARGET).o: $(TARGET).c $(TARGET).h
	$(CC) -o $(TARGET).o -c $(TARGET).c $(CFLAGS)

$(TEST_OBJS): %.o: %.c $(TARGET).h
	@echo + Creating Test
	$(CC) -o $@ -c $< $(CFLAGS)

$(SYS_OBJS): %.o: ../%.c
	@echo 
	@echo + Creating object files
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	@echo
	@echo + Cleaning up
	$(RM) *.o

.PHONY : all begin end clean
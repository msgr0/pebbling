.PHONY: all debug clean test gfatools_lib

CC = clang
CFLAGS = -Wall -Wextra -std=c11 -Ilib -Ilib/gfatools
DEBUG_FLAGS = -g -O0 -DDEBUG
RELEASE_FLAGS = -O3 -march=native -DNDEBUG

SRCS = main.c
OBJS = main.o
DEBUG_OBJS = main_debug.o

GFATOOLS_LIB = lib/gfatools/libgfa1.a
GFATOOLS_DIR = lib/gfatools

TARGET = pebbling
DEBUG_TARGET = pebbling_debug

# Default target
all: $(TARGET)

# Release build
$(TARGET): $(OBJS) $(GFATOOLS_LIB)
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) -o $@ $(OBJS) $(GFATOOLS_LIB) -lz
	@echo "✓ Built: $(TARGET)"

main.o: main.c
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) -c main.c -o $@

main_debug.o: main.c
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c main.c -o $@

# Debug build
debug: $(DEBUG_TARGET)
	@echo "✓ Built debug version: $(DEBUG_TARGET)"

$(DEBUG_TARGET): $(DEBUG_OBJS) $(GFATOOLS_LIB)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o $@ $(DEBUG_OBJS) $(GFATOOLS_LIB) -lz

# Real gfa test
testr: $(TARGET)
	@echo "Running real GFA test with abau-skesa.gfa..."
	@./$(TARGET) abau-skesa.gfa Contig_10_1640.24
	@echo "✓ Real GFA test completed"
# Test target - create a simple test GFA file and run the parser
test: $(TARGET)
	@echo "Creating test GFA file..."
	@printf "S\ts1\tACGTACGTACGT\tLN:i:12\n" > test.gfa
	@printf "S\ts2\tTGCATGCATGCA\tLN:i:12\n" >> test.gfa
	@printf "L\ts1\t+\ts2\t+\t4M\n" >> test.gfa
	@printf "P\tpath1\ts1+,s2+\t4M\n" >> test.gfa
	@echo "Running parser on test.gfa..."
	@./$(TARGET) test.gfa
	@echo "✓ Test completed"

# Debug test - run with debug build
debug_test: debug
	@echo "Creating test GFA file..."
	@printf "S\tseg1\tACGTACGTACGT\tLN:i:12\n" > test.gfa
	@printf "S\tseg2\tTGCATGCATGCA\tLN:i:12\n" >> test.gfa
	@printf "L\tseg1\t+\tseg2\t+\t4M\n" >> test.gfa
	@printf "P\tpath1\tseg1+,seg2+\t4M\n" >> test.gfa
	@echo "Running debug parser on test.gfa..."
	@./$(DEBUG_TARGET) test.gfa
	@echo "✓ Debug test completed"

# Valgrind memory check (requires valgrind)
memcheck: $(TARGET)
	@echo "Running memory check with valgrind..."
	@printf "S\tseg1\tACGTACGTACGT\tLN:i:12\n" > test.gfa
	@printf "S\tseg2\tTGCATGCATGCA\tLN:i:12\n" >> test.gfa
	@printf "L\tseg1\t+\tseg2\t+\t4M\n" >> test.gfa
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET) test.gfa

# Build gfatools library
$(GFATOOLS_LIB):
	$(MAKE) -C $(GFATOOLS_DIR) libgfa1.a

# Clean build artifacts
clean:
	rm -f $(OBJS) $(DEBUG_OBJS) $(TARGET) $(DEBUG_TARGET) test.gfa
	$(MAKE) -C $(GFATOOLS_DIR) clean
	@echo "✓ Cleaned"

# Print variables
info:
	@echo "CC: $(CC)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "RELEASE_FLAGS: $(RELEASE_FLAGS)"
	@echo "DEBUG_FLAGS: $(DEBUG_FLAGS)"
	@echo "SRCS: $(SRCS)"
	@echo "HEADERS: $(HEADERS)"
	@echo "Targets: all, debug, test, debug_test, memcheck, clean, info"

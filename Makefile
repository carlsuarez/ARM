# Toolchain
CC=arm-none-eabi-gcc
LD=arm-none-eabi-gcc
CFLAGS=-Wall -nostdlib -nostartfiles -ffreestanding -O0 -Iinclude
LDFLAGS=-T linker.ld -nostdlib -nostartfiles -ffreestanding

# Directories
SRC_DIR=src
BUILD_DIR=build
INCLUDE_DIR=include

# Files
OBJS=$(BUILD_DIR)/boot.o $(BUILD_DIR)/main.o $(BUILD_DIR)/uart.o
TARGET=$(BUILD_DIR)/kernel.elf

# Default target
all: $(TARGET)

# Link
$(TARGET): $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS) -lgcc

# Compile sources
$(BUILD_DIR)/boot.o: $(SRC_DIR)/boot.s
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/uart.o: $(SRC_DIR)/uart.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Run in QEMU
run: $(TARGET)
	qemu-system-arm -M integratorcp -cpu arm926 -kernel $(TARGET) -nographic -audiodev none,id=snd0

# Clean
clean:
	rm -rf $(BUILD_DIR)

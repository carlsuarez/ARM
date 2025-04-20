# Toolchain
CC=arm-none-eabi-gcc
LD=arm-none-eabi-gcc
CFLAGS=-Wall -nostdlib -nostartfiles -ffreestanding -O0 -Iinclude
LDFLAGS=-T linker.ld -nostdlib -nostartfiles -ffreestanding

# Directories
SRC_DIR=src
BUILD_DIR=build
INCLUDE_DIR=include

# Source files
C_SRCS := $(wildcard $(SRC_DIR)/*.c)
S_SRCS := $(wildcard $(SRC_DIR)/*.s)

# Object files
OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_SRCS)) \
        $(patsubst $(SRC_DIR)/%.s, $(BUILD_DIR)/%.o, $(S_SRCS))

# Output
TARGET=$(BUILD_DIR)/kernel.elf

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS) -lgcc

# Compile .c files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble .s files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Run in QEMU
run: $(TARGET)
	qemu-system-arm -M integratorcp -cpu arm926 -kernel $(TARGET) -nographic -audiodev none,id=snd0

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Toolchain
CC=arm-none-eabi-gcc
LD=arm-none-eabi-gcc
CFLAGS=-Wall -nostdlib -nostartfiles -ffreestanding -O0 -Iinclude
LDFLAGS=-T linker.ld -nostdlib -nostartfiles -ffreestanding

# Directories
SRC_DIR=src
BUILD_DIR=build

# Files
IMG=image/fat32.img

# Find all .c and .s files recursively
C_SRCS := $(shell find $(SRC_DIR) -name '*.c')
S_SRCS := $(shell find $(SRC_DIR) -name '*.s')

# Generate object file paths in build dir, mirroring src structure
OBJS := $(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/%, \
         $(patsubst %.c, %.o, $(C_SRCS)) $(patsubst %.s, %.o, $(S_SRCS)))

# Target binary
TARGET=$(BUILD_DIR)/kernel.elf

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS) -lgcc

# Compile C sources
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble .s sources
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# QEMU run
run: all
	qemu-system-arm -M integratorcp -cpu arm926 -kernel $(TARGET) -sd $(IMG) -nographic -serial mon:stdio -audiodev none,id=snd0

# Clean build output
clean:
	rm -rf $(BUILD_DIR)

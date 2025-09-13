# Toolchain
CC  = arm-none-eabi-gcc
LD  = arm-none-eabi-gcc

# Directories
BUILD_DIR   := ../build/home
SRC_DIR     := .
INCLUDE_DIR := ../include

# Includes
INCLUDES    := -I$(INCLUDE_DIR)

# Linker
LINKER      := linker.ld

# Libraries
LIBUSER     := ../build/libuser.so

# Output
TARGET      := $(BUILD_DIR)/main.elf

# Flags
CFLAGS := -Wall -nostdlib -fPIC -ffreestanding -O0 \
          $(INCLUDES) \
          -fvisibility=hidden
LDFLAGS := -nostdlib -ffreestanding -Wl,--emit-relocs -Wl,-q

# Sources
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Build rules
.PHONY: all clean

all: $(TARGET)

# Link ELF (now keeps relocations)
$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -T $(LINKER) -o $@ $^ -L../build -l:$(notdir $(LIBUSER)) -lgcc

# Compile C sources
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
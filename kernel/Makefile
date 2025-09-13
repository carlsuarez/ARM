# Toolchain
CC  = arm-none-eabi-gcc
LD  = arm-none-eabi-gcc
AR  = arm-none-eabi-ar
AS  = arm-none-eabi-as

# Directories
SRC_DIR      := .
ULIB_DIR     := $(SRC_DIR)/lib
UCOMMON_DIR  := $(SRC_DIR)/common
SHARED_DIR   := ../common
INCLUDE_DIR  := ../include
BUILD_DIR    := ../build

# Outputs
USER_LIB     := $(BUILD_DIR)/libuser.so
USER_STATIC  := $(BUILD_DIR)/libuser.a
KERNEL_ELF   := $(BUILD_DIR)/kernel.elf

# Flags
CFLAGS := -Wall -nostdlib -nostartfiles -ffreestanding -O0 -g \
          -I$(INCLUDE_DIR) -fPIC

# Kernel-specific flags (no -fPIC for kernel)
KERNEL_CFLAGS := -Wall -nostdlib -nostartfiles -ffreestanding -O0 -g \
                 -I$(INCLUDE_DIR) -I$(INCLUDE_DIR)/kernel

# Assembly flags
ASFLAGS := -I$(INCLUDE_DIR) -I$(INCLUDE_DIR)/kernel -g

# Shared library flags
SHARED_LDFLAGS := -shared -nostdlib -Wl,--emit-relocs -Wl,-q

# Kernel linking flags
KERNEL_LDFLAGS := -nostdlib -nostartfiles -T linker.ld

# USER LIBRARY SOURCES
ALL_CSRC     := $(shell find $(SRC_DIR) -name '*.c' | grep -v -E '(arch|core|drivers|fs)')
ULIB_CSRC    := $(wildcard $(ULIB_DIR)/*.c)
UCOMMON_CSRC := $(wildcard $(UCOMMON_DIR)/*.c)
SHARED_CSRC  := $(wildcard $(SHARED_DIR)/*.c)

# Exclude lib/ and common/ from main user sources
USER_MAIN_CSRC := $(filter-out $(ULIB_CSRC) $(UCOMMON_CSRC), $(ALL_CSRC))

# KERNEL SOURCES
KERNEL_CSRC := $(shell find $(SRC_DIR)/arch $(SRC_DIR)/core $(SRC_DIR)/drivers $(SRC_DIR)/fs -name '*.c' 2>/dev/null)
KERNEL_SSRC := $(shell find $(SRC_DIR)/arch -name '*.s' -o -name '*.S' 2>/dev/null)
KERNEL_LIB_CSRC := $(wildcard $(ULIB_DIR)/*.c)
KERNEL_SHARED_CSRC := $(wildcard $(SHARED_DIR)/*.c)

# USER LIBRARY OBJECTS
USER_MAIN_OBJS := $(patsubst %.c, $(BUILD_DIR)/user/%.o, $(USER_MAIN_CSRC))
ULIB_OBJS      := $(patsubst $(ULIB_DIR)/%.c, $(BUILD_DIR)/user/lib/%.o, $(ULIB_CSRC))
UCOMMON_OBJS   := $(patsubst $(UCOMMON_DIR)/%.c, $(BUILD_DIR)/user/common/%.o, $(UCOMMON_CSRC))
SHARED_OBJS    := $(patsubst $(SHARED_DIR)/%.c, $(BUILD_DIR)/user/shared/%.o, $(SHARED_CSRC))

# All user library objects
ALL_USER_OBJS := $(USER_MAIN_OBJS) $(ULIB_OBJS) $(UCOMMON_OBJS) $(SHARED_OBJS)

# KERNEL OBJECTS
KERNEL_COBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(KERNEL_CSRC))
KERNEL_SOBJS := $(patsubst %.s, $(BUILD_DIR)/%.o, $(patsubst %.S, $(BUILD_DIR)/%.o, $(KERNEL_SSRC)))
KERNEL_LIB_OBJS := $(patsubst $(ULIB_DIR)/%.c, $(BUILD_DIR)/lib/%.o, $(KERNEL_LIB_CSRC))
KERNEL_SHARED_OBJS := $(patsubst $(SHARED_DIR)/%.c, $(BUILD_DIR)/common/%.o, $(KERNEL_SHARED_CSRC))

# All kernel objects
ALL_KERNEL_OBJS := $(KERNEL_SOBJS) $(KERNEL_COBJS) $(KERNEL_LIB_OBJS) $(KERNEL_SHARED_OBJS)

# Targets
.PHONY: all clean shared static kernel user

all: $(KERNEL_ELF) $(USER_LIB)

user: $(USER_LIB)
kernel: $(KERNEL_ELF)
shared: $(USER_LIB)
static: $(USER_STATIC)

# Kernel ELF target
$(KERNEL_ELF): $(ALL_KERNEL_OBJS)
	@mkdir -p $(dir $@)
	$(LD) $(KERNEL_LDFLAGS) -o $@ $^ -lgcc

# Shared library target
$(USER_LIB): $(ALL_USER_OBJS)
	@mkdir -p $(dir $@)
	$(LD) $(SHARED_LDFLAGS) -o $@ $^ -lgcc

# Static library target (alternative)
$(USER_STATIC): $(ALL_USER_OBJS)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^

# Build rules for KERNEL objects

# Assembly files
$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# Kernel C files
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(KERNEL_CFLAGS) -c $< -o $@

# Kernel lib files
$(BUILD_DIR)/lib/%.o: $(ULIB_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(KERNEL_CFLAGS) -c $< -o $@

# Kernel shared/common files
$(BUILD_DIR)/common/%.o: $(SHARED_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(KERNEL_CFLAGS) -c $< -o $@

# Build rules for USER LIBRARY objects

# Main user .c files (excluding lib/common)
$(BUILD_DIR)/user/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# User lib
$(BUILD_DIR)/user/lib/%.o: $(ULIB_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# User common (NOT kcommon)
$(BUILD_DIR)/user/common/%.o: $(UCOMMON_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Shared user/kernel common
$(BUILD_DIR)/user/shared/%.o: $(SHARED_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
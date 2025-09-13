# Toolchain
CC  = arm-none-eabi-gcc
LD  = arm-none-eabi-gcc

# Directories
SRC_DIR      := .
ULIB_DIR     := $(SRC_DIR)/lib
UCOMMON_DIR  := $(SRC_DIR)/common
USHARED_DIR  := ../common
INCLUDE_DIR  := ../include
BUILD_DIR    := ../build
USER_SO      := $(BUILD_DIR)/libuser.so

# Flags
CFLAGS    := -Wall -ffreestanding -nostdlib -fPIC -O0 \
             -I$(INCLUDE_DIR)

SO_LDFLAGS := -shared -nostdlib -Wl,--emit-relocs -Wl,-q

# Sources
ULIB_CSRC     := $(wildcard $(ULIB_DIR)/*.c)
UCOMMON_CSRC  := $(wildcard $(UCOMMON_DIR)/*.c)
USHARED_CSRC  := $(wildcard $(USHARED_DIR)/*.c)
UMAIN_CSRC    := $(filter-out $(ULIB_CSRC) $(UCOMMON_CSRC), $(wildcard $(SRC_DIR)/*.c))

# Objects
ULIB_OBJS     := $(patsubst $(ULIB_DIR)/%.c, $(BUILD_DIR)/user/lib/%.o, $(ULIB_CSRC))
UCOMMON_OBJS  := $(patsubst $(UCOMMON_DIR)/%.c, $(BUILD_DIR)/user/common/%.o, $(UCOMMON_CSRC))
USHARED_OBJS  := $(patsubst $(USHARED_DIR)/%.c, $(BUILD_DIR)/user/shared/%.o, $(USHARED_CSRC))
UMAIN_OBJS    := $(patsubst %.c, $(BUILD_DIR)/user/%.o, $(UMAIN_CSRC))

ALL_OBJS := $(ULIB_OBJS) $(UCOMMON_OBJS) $(USHARED_OBJS) $(UMAIN_OBJS)

# Tell make where to find .c sources
vpath %.c $(SRC_DIR) $(ULIB_DIR) $(UCOMMON_DIR) $(USHARED_DIR)

# Targets
.PHONY: all clean

all: $(USER_SO)

$(USER_SO): $(ALL_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(SO_LDFLAGS) -o $@ $^ -lgcc

# Specific build rules for each directory
$(BUILD_DIR)/user/lib/%.o: $(ULIB_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/user/common/%.o: $(UCOMMON_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/user/shared/%.o: $(USHARED_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Main user files (root directory)
$(BUILD_DIR)/user/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)/user $(USER_SO)
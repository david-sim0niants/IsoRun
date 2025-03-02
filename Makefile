CC := gcc
CFLAGS := -Wall -Werror=implicit-function-declaration -Wfatal-errors -Wextra -Wpedantic -Wconversion -Wshadow

SRC_DIR := src
BUILD_DIR := build

INCLUDE_DIRS := include
INCLUDES := $(addprefix -I, $(INCLUDE_DIRS))

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:%.o=%.d)

TARGET := isorun

DEFS_OF_$(SRC_DIR)/isorun.c = -D_GNU_SOURCE

MODE ?= Debug
ifeq ($(MODE), Debug)
	CFLAGS += -DDEBUG -ggdb -O0
else
	CFLAGS += -DNDEBUG
endif

ifeq ($(MODE), Release)
	CFLAGS += -O3
endif

all: $(TARGET)

-include $(DEPS)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFS_OF_$<) -MMD -o $@ -c $<

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean

CC := gcc
CFLAGS := -Wall -Wfatal-errors -Wextra -Wpedantic -Wconversion -Wshadow

SRC_DIR := src
BUILD_DIR := build

INCLUDE_DIRS := include
INCLUDES := $(addprefix -I, $(INCLUDE_DIRS))

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:%.o=%.d)

TARGET := isorun

all: $(TARGET)

-include $(DEPS)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -o $@ -c $<

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean

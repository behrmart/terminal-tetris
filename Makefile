CC := gcc
PKG_CONFIG ?= pkg-config
CPPFLAGS += -Iinclude
CFLAGS += -std=c11 -Wall -Wextra -Wpedantic -Wconversion -g

BUILD_DIR := build
TARGET := $(BUILD_DIR)/terminal-tetris
CORE_SOURCES := src/board.c src/tetromino.c src/game.c
SOURCES := src/main.c $(CORE_SOURCES) src/input.c src/display.c
OBJECTS := $(SOURCES:src/%.c=$(BUILD_DIR)/%.o)
TEST_TARGETS := $(BUILD_DIR)/test-game $(BUILD_DIR)/test-board $(BUILD_DIR)/test-tetromino
NCURSES_CFLAGS = $(shell $(PKG_CONFIG) --cflags ncurses)
NCURSES_LIBS = $(shell $(PKG_CONFIG) --libs ncurses)

.PHONY: all run debug test clean check-ncurses
all: $(TARGET)

check-ncurses:
	@$(PKG_CONFIG) --exists ncurses || { echo "Install pkg-config and libncurses-dev."; exit 1; }

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR) check-ncurses
	$(CC) $(CPPFLAGS) $(NCURSES_CFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(TARGET): $(OBJECTS) | check-ncurses
	$(CC) $(LDFLAGS) $(OBJECTS) $(NCURSES_LIBS) $(LDLIBS) -o $@

$(BUILD_DIR)/test-%: tests/test_%.c $(CORE_SOURCES) $(wildcard include/*.h) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $< $(CORE_SOURCES) $(LDLIBS) -o $@

run: $(TARGET)
	./$(TARGET)

debug: $(TARGET)
	gdb --args ./$(TARGET)

test: $(TEST_TARGETS)
	@set -e; for test_bin in $(TEST_TARGETS); do ./$$test_bin; done

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJECTS:.o=.d)

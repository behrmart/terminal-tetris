CC := gcc
PKG_CONFIG ?= pkg-config
CPPFLAGS += -Iinclude
CFLAGS += -std=c11 -Wall -Wextra -Wpedantic -Wconversion -g

BUILD_DIR := build
TARGET := $(BUILD_DIR)/terminal-tetris
CORE_SOURCES := src/board.c src/tetromino.c src/game.c
SOURCES := src/main.c $(CORE_SOURCES) src/input.c src/display.c
OBJECTS := $(SOURCES:src/%.c=$(BUILD_DIR)/%.o)
TEST_TARGET := $(BUILD_DIR)/test-game
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

$(TEST_TARGET): tests/test_game.c $(CORE_SOURCES) $(wildcard include/*.h) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) tests/test_game.c $(CORE_SOURCES) $(LDLIBS) -o $@

run: $(TARGET)
	./$(TARGET)

debug: $(TARGET)
	gdb --args ./$(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJECTS:.o=.d)

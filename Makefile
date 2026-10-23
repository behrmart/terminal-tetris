CC := gcc
PKG_CONFIG ?= pkg-config
CPPFLAGS += -Iinclude
CFLAGS += -std=c11 -Wall -Wextra -Wpedantic -Wconversion -g

BUILD_DIR := build
TARGET := $(BUILD_DIR)/terminal-tetris
TARGET_GAMES := $(BUILD_DIR)/terminal-games
CORE_SOURCES := src/board.c src/tetromino.c src/game.c
SOURCES := src/main.c $(CORE_SOURCES) src/input.c src/display.c
OBJECTS := $(SOURCES:src/%.c=$(BUILD_DIR)/%.o)
TEST_TARGETS := $(BUILD_DIR)/test-game $(BUILD_DIR)/test-board $(BUILD_DIR)/test-tetromino
TARGET_2048 := $(BUILD_DIR)/terminal-2048
CORE_2048_SOURCES := src/game2048/board.c src/game2048/game.c
SOURCES_2048 := src/game2048/main.c $(CORE_2048_SOURCES) src/game2048/input.c src/game2048/display.c
OBJECTS_2048 := $(SOURCES_2048:src/%.c=$(BUILD_DIR)/%.o)
TEST_2048_TARGETS := $(BUILD_DIR)/test-2048-game $(BUILD_DIR)/test-2048-board
DW_SOURCES := src/drugwars/game.c src/drugwars/main.c src/drugwars/input.c src/drugwars/display.c
DW_OBJECTS := $(DW_SOURCES:src/%.c=$(BUILD_DIR)/%.o)
NCURSES_CFLAGS = $(shell $(PKG_CONFIG) --cflags ncurses)
NCURSES_LIBS = $(shell $(PKG_CONFIG) --libs ncurses)

.PHONY: all run debug test test-terminal package clean check-ncurses run-2048 test-2048 test-2048-terminal package-2048 test-2048-package check run-tetris test-menu package-games test-scores test-login test-drugwars test-drugwars-terminal
all: $(TARGET_GAMES) $(TARGET) $(TARGET_2048) $(BUILD_DIR)/terminal-drugwars

check-ncurses:
	@$(PKG_CONFIG) --exists ncurses || { echo "Install pkg-config and libncurses-dev."; exit 1; }

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR) check-ncurses
	$(CC) $(CPPFLAGS) $(NCURSES_CFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(TARGET_GAMES): $(OBJECTS) $(OBJECTS_2048) $(DW_OBJECTS) $(BUILD_DIR)/launcher.o $(BUILD_DIR)/menu_theme.o $(BUILD_DIR)/scores.o | check-ncurses
	$(CC) $(LDFLAGS) $(OBJECTS) $(OBJECTS_2048) $(DW_OBJECTS) $(BUILD_DIR)/launcher.o $(BUILD_DIR)/menu_theme.o $(BUILD_DIR)/scores.o $(NCURSES_LIBS) $(LDLIBS) -o $@

$(TARGET): $(TARGET_GAMES)
	cp $< $@

$(BUILD_DIR)/test-%: tests/test_%.c $(CORE_SOURCES) $(wildcard include/*.h) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $< $(CORE_SOURCES) $(LDLIBS) -o $@

$(BUILD_DIR)/test-terminal: tests/test_terminal.c src/display.c src/input.c $(CORE_SOURCES) $(wildcard include/*.h) | $(BUILD_DIR) check-ncurses
	$(CC) $(CPPFLAGS) $(NCURSES_CFLAGS) $(CFLAGS) $(LDFLAGS) tests/test_terminal.c src/display.c src/input.c $(CORE_SOURCES) $(NCURSES_LIBS) $(LDLIBS) -o $@

test-terminal: $(TARGET) $(BUILD_DIR)/test-terminal
	python3 tests/test_terminal.py

package: $(TARGET) packaging/README.txt
	install -d $(BUILD_DIR)/package/terminal-tetris
	install -m 755 $(TARGET) $(BUILD_DIR)/package/terminal-tetris/terminal-tetris
	install -m 644 packaging/README.txt $(BUILD_DIR)/package/terminal-tetris/README.txt
	tar -czf $(BUILD_DIR)/terminal-tetris-linux.tar.gz -C $(BUILD_DIR)/package terminal-tetris

$(BUILD_DIR)/game2048/%.o: src/game2048/%.c | check-ncurses
	mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(NCURSES_CFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(TARGET_2048): $(TARGET_GAMES)
	cp $< $@

$(BUILD_DIR)/test-2048-%: tests/test_2048_%.c $(CORE_2048_SOURCES) $(wildcard include/game2048/*.h) | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $< $(CORE_2048_SOURCES) $(LDLIBS) -o $@

$(BUILD_DIR)/test-2048-terminal: tests/test_2048_terminal.c src/game2048/display.c src/game2048/input.c src/menu_theme.c $(CORE_2048_SOURCES) $(wildcard include/game2048/*.h) include/menu_theme.h | $(BUILD_DIR) check-ncurses
	$(CC) $(CPPFLAGS) $(NCURSES_CFLAGS) $(CFLAGS) $(LDFLAGS) tests/test_2048_terminal.c src/game2048/display.c src/game2048/input.c src/menu_theme.c $(CORE_2048_SOURCES) $(NCURSES_LIBS) $(LDLIBS) -o $@

test-2048-terminal: $(TARGET_2048) $(BUILD_DIR)/test-2048-terminal
	python3 tests/test_2048_terminal.py

run-2048: $(TARGET_2048)
	./$(TARGET_2048)

test-2048: $(TEST_2048_TARGETS)
	@set -e; for test_bin in $(TEST_2048_TARGETS); do ./$$test_bin; done

run: $(TARGET_GAMES)
	./$(TARGET_GAMES)

run-tetris: $(TARGET)
	./$(TARGET)

debug: $(TARGET)
	gdb --args ./$(TARGET)

test: $(TEST_TARGETS)
	@set -e; for test_bin in $(TEST_TARGETS); do ./$$test_bin; done

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJECTS:.o=.d) $(OBJECTS_2048:.o=.d) $(BUILD_DIR)/launcher.d $(BUILD_DIR)/menu_theme.d $(BUILD_DIR)/scores.d $(DW_OBJECTS:.o=.d)

# Complete regression suite including the extracted 2048 package.
check: test test-2048 test-terminal test-2048-terminal test-2048-package test-menu test-scores test-login test-drugwars test-drugwars-terminal

package-2048: $(TARGET_2048) packaging/README-2048.txt
	install -d $(BUILD_DIR)/package/terminal-2048
	install -m 755 $(TARGET_2048) $(BUILD_DIR)/package/terminal-2048/terminal-2048
	install -m 644 packaging/README-2048.txt $(BUILD_DIR)/package/terminal-2048/README.txt
	tar -czf $(BUILD_DIR)/terminal-2048-linux.tar.gz -C $(BUILD_DIR)/package terminal-2048/terminal-2048 terminal-2048/README.txt

test-2048-package: package-2048
	python3 tests/test_2048_package.py

test-menu: all
	python3 tests/test_menu.py

package-games: $(TARGET_GAMES) packaging/README-games.txt
	install -d $(BUILD_DIR)/package/terminal-games
	install -m 755 $(TARGET_GAMES) $(BUILD_DIR)/package/terminal-games/terminal-games
	install -m 644 packaging/README-games.txt $(BUILD_DIR)/package/terminal-games/README.txt
	tar -czf $(BUILD_DIR)/terminal-games-linux.tar.gz -C $(BUILD_DIR)/package terminal-games/terminal-games terminal-games/README.txt

$(BUILD_DIR)/test-scores: tests/test_scores.c src/scores.c include/scores.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) tests/test_scores.c src/scores.c $(LDLIBS) -o $@

test-scores: $(BUILD_DIR)/test-scores
	./$(BUILD_DIR)/test-scores

test-login: all
	python3 tests/test_login.py

.PHONY: run-drugwars
$(BUILD_DIR)/drugwars/%.o: src/drugwars/%.c | check-ncurses
	mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(NCURSES_CFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/terminal-drugwars: $(TARGET_GAMES)
	cp $< $@

run-drugwars: $(BUILD_DIR)/terminal-drugwars
	./$(BUILD_DIR)/terminal-drugwars

$(BUILD_DIR)/test-drugwars: tests/test_drugwars.c src/drugwars/game.c include/drugwars/game.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) tests/test_drugwars.c src/drugwars/game.c $(LDLIBS) -o $@

test-drugwars: $(BUILD_DIR)/test-drugwars
	./$(BUILD_DIR)/test-drugwars

$(BUILD_DIR)/test-drugwars-terminal: tests/test_drugwars_terminal.c src/drugwars/display.c src/drugwars/input.c src/drugwars/game.c src/display.c $(CORE_SOURCES) $(wildcard include/drugwars/*.h) | $(BUILD_DIR) check-ncurses
	$(CC) $(CPPFLAGS) $(NCURSES_CFLAGS) $(CFLAGS) $(LDFLAGS) tests/test_drugwars_terminal.c src/drugwars/display.c src/drugwars/input.c src/drugwars/game.c src/display.c $(CORE_SOURCES) $(NCURSES_LIBS) $(LDLIBS) -o $@

test-drugwars-terminal: all $(BUILD_DIR)/test-drugwars-terminal
	python3 tests/test_drugwars_terminal.py

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
SDL_CFLAGS = $(shell sdl2-config --cflags)
SDL_LIBS = $(shell sdl2-config --libs)
BUILD_DIR = build
DIST_DIR = dist

SRCS = chip8.c hal.c
HDRS = chip8.h hal.h
OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)
TARGET = $(DIST_DIR)/chip8

.PHONY: all clean format lint

all: $(TARGET)

$(TARGET): $(OBJS) | $(DIST_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(SDL_LIBS)

$(BUILD_DIR)/hal.o: hal.c $(HDRS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.c $(HDRS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(DIST_DIR):
	mkdir -p $(DIST_DIR)

clean:
	rm -rf $(BUILD_DIR) $(DIST_DIR)

format:
	clang-format -i $(SRCS) $(HDRS)

lint:
	cppcheck --enable=all --suppress=missingIncludeSystem $(SRCS) $(HDRS)

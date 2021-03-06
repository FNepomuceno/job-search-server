.PHONY: all, debug, clean

CC = gcc
DFLAGS = -g -fsanitize=address -Werror
CFLAGS = -Wall -ansi -pedantic -std=c11
LDFLAGS = -lsqlite3
SRC_DIR = src/
BUILD_DIR = build
TARGET = $(BUILD_DIR)/jobhuntserver
DTARGET = $(BUILD_DIR)/test
FILES = $(shell find $(SRC_DIR) -name '*.c')
RM = rm -rf

all:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(FILES) $(LDFLAGS)

debug:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DFLAGS) -o $(DTARGET) $(FILES) $(LDFLAGS)

clean:
	$(RM) $(BUILD_DIR)

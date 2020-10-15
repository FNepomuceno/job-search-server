.PHONY: all, debug, clean

CC = gcc
DFLAGS = -g -Werror
CFLAGS = -Wall -ansi -pedantic -std=c11
LDFLAGS = -lsqlite3
SRC_DIR = src/
BUILD_DIR = build
TARGET = $(BUILD_DIR)/jobhuntserver
DTARGET = $(BUILD_DIR)/test
RAW_FILES = database.c main.c webserver.c
FILES = $(addprefix $(SRC_DIR), $(RAW_FILES))
RM = rm -rf

all:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(FILES) $(LDFLAGS)

debug:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DFLAGS) -o $(DTARGET) $(FILES) $(LDFLAGS)
	./$(DTARGET)

clean:
	$(RM) $(BUILD_DIR)

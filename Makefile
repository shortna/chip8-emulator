.SHELL := /bin/sh
CC := clang

BUILD_DIR := build
SOURCE_DIR := src
INCLUDE_DIR := include

CFLAGS_DEBUG := -g -DDEBUG -fsanitize=address

CFLAGS += -std=c23 -O1 -I $(INCLUDE_DIR)
CFLAGS += -Wall -Wextra -Wpedantic -Werror -Wno-gnu
CFLAGS += `pkg-config --cflags ncurses`
LDFLAGS += `pkg-config --libs ncurses`

SOURCE_FILES := $(subst $(SOURCE_DIR)/,,$(wildcard $(SOURCE_DIR)/*.c))
OBJ_FILES := $(SOURCE_FILES:%.c=$(BUILD_DIR)/%.o)

TARGET := chip-8

TARGET_DEBUG := debug
OBJ_FILES_DEBUG := $(SOURCE_FILES:%.c=$(BUILD_DIR)/%_d.o)

.PHONY: clean all

all: $(TARGET)

$(BUILD_DIR):
	@mkdir $@

$(OBJ_FILES): $(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ_FILES) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $(BUILD_DIR)/$@

$(OBJ_FILES_DEBUG): $(BUILD_DIR)/%_d.o: $(SOURCE_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CFLAGS_DEBUG) -c $< -o $@

$(TARGET_DEBUG): $(OBJ_FILES_DEBUG) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CFLAGS_DEBUG) $(LDFLAGS) $^ -o $(BUILD_DIR)/$@

clean:
	$(RM) -r $(BUILD_DIR) $(wildcard log*)

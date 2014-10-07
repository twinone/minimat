CC := gcc
SRC_DIR := src
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)


all:
	$(CC) $(SRC_FILES) -o minimat -lm

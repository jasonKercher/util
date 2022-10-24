CC          := gcc
CFLAGS      += -Wall -Wextra -g -O0 -std=gnu11
LDFLAGS     += -pthread
PROJECT     := utiltest
OBJECT_DIR  := objects/
MACRO_DIR   := macro/
TARGET      := $(PROJECT)
SRC     := $(wildcard ./*.c)

OBJECTS := $(SRC:%.c=$(OBJECT_DIR)%.o)
M_OBJECTS := $(SRC:%.c=$(MACRO_DIR)%.c)

$(OBJECT_DIR)%.o: %.c
	@mkdir -p $(@D)
	$(CC) -D DEBUG $(CFLAGS) -o $@ -c $<

.PHONY: all build clean macro

build: $(OBJECTS)
	@mkdir -p $(OBJECT_DIR)
	$(CC) -o $(TARGET) $(OBJECTS) -g -O0 $(LDFLAGS)

all: build

$(MACRO_DIR)%.c: %.c
	@mkdir -p $(@D)
	$(CC) -D DEBUG -E $(CFLAGS) -o $@ -c $<

macro: $(M_OBJECTS)

clean:
	-@rm -rfv $(OBJECT_DIR)
	-@rm -rfv $(MACRO_DIR)

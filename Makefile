
CC      := cc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -O2
INCLUDE := -Iinclude
LDFLAGS :=
BUILD   := build
SRC     := src
TESTS   := tests

SOURCES := $(SRC)/main.c $(SRC)/db.c $(SRC)/crypto.c $(SRC)/ui.c
OBJECTS := $(patsubst $(SRC)/%.c,$(BUILD)/%.o,$(SOURCES))

TEST_SRC := $(TESTS)/test_crypto.c
TEST_OBJ := $(BUILD)/test_crypto.o

BIN := $(BUILD)/pwdmgr
TEST_BIN := $(BUILD)/tests

.PHONY: all clean test

all: $(BIN)

$(BUILD)/%.o: $(SRC)/%.c
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(BIN): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test: all $(TEST_BIN)

$(BUILD)/test_crypto.o: $(TEST_SRC)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(TEST_BIN): $(TEST_OBJ) $(SRC)/crypto.c $(SRC)/ui.c
	$(CC) $(CFLAGS) $(INCLUDE) $(TEST_OBJ) $(SRC)/crypto.c $(SRC)/ui.c -o $@ $(LDFLAGS)

clean:
	rm -rf $(BUILD)/*.o $(BIN) $(TEST_BIN)

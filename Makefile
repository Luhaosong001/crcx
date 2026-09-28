CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -Werror -O2 -Iinclude
BUILD   = build

.PHONY: test clean

test: $(BUILD)/test_crcx
	./$(BUILD)/test_crcx

$(BUILD)/test_crcx: tests/test_crcx.c src/crcx.c include/crcx/crcx.h
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) tests/test_crcx.c src/crcx.c -o $@

clean:
	rm -rf $(BUILD)

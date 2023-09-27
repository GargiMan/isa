# Makefile ISA Project
# Marek Gergel (xgerge01)

LOGIN := xgerge01
PROG_NAME := dns
# -g for debug , -O2 for optimization (0 - disabled, 1 - less, 2 - more)
CCFLAGS := -g -O0 -Wall -Wextra -std=c17 -pedantic
SRC_FILES := $(wildcard *.c)

.PHONY: $(PROG_NAME) test clean zip 

$(PROG_NAME): $(SRC_FILES)
	gcc $(CCFLAGS) $(SRC_FILES) -o $(PROG_NAME)

test:
	./$(PROG_NAME)

clean:
	rm -rf $(PROG_NAME)
	rm -rf $(LOGIN).zip

zip: clean
	zip -r $(LOGIN).zip *.h *.c *.md Makefile LICENSE
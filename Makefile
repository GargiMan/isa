# Makefile ISA Project
# Marek Gergel (xgerge01)

LOGIN := xgerge01
PROG_NAME := dns
CC = gcc
# -g for debug , -O2 for optimization (0 - disabled, 1 - less, 2 - more)
CCFLAGS := -g -O0 -Wall -Wextra -std=c17 -pedantic
SRC_FILES := main.c error.c dns.c

.PHONY: $(PROG_NAME) test clean zip 

$(PROG_NAME): $(SRC_FILES)
	$(CC) $(CCFLAGS) $(SRC_FILES) -o $@

test:
	./test.sh

clean:
	rm -rf $(PROG_NAME) $(LOGIN).zip

zip: clean
	zip -r $(LOGIN).zip *.h *.c *.md Makefile LICENSE
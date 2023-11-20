# Makefile ISA Project
# Marek Gergel (xgerge01)

LOGIN := xgerge01
PROG_NAME := dns
CC = g++
# -g for debug , -O2 for optimization (0 - disabled, 1 - less, 2 - more)
CCFLAGS := -g -O0 -Wall -Wextra -std=c++17 -pedantic
SRC_FILES := main.cpp error.cpp dns.cpp

.PHONY: all $(PROG_NAME) test pdf clean zip tar

all: $(PROG_NAME)

$(PROG_NAME): $(SRC_FILES)
	$(CC) $(CCFLAGS) $(SRC_FILES) -o $@

test:
	./test.sh

pdf:
	pandoc -V geometry:margin=1in manual.md -o manual.pdf

clean:
	rm -rf $(PROG_NAME) $(LOGIN).zip $(LOGIN).tar manual.pdf

zip: clean pdf
	zip -r $(LOGIN).zip *.h *.cpp README* *.sh Makefile manual.pdf

tar: clean pdf
	tar -cf $(LOGIN).tar *.h *.cpp README* *.sh Makefile manual.pdf
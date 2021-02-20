.SUFFIXES:
.SUFFIXES: .c .o

CFILES!= find src/ -name "*.c"
CWD!= pwd

CC?= gcc

ifeq ($(shell $(CC) -v 2>&1 | grep -c "clang version"), 1)
	COMPILER=clang
else
	COMPILER=gcc
endif

ifeq ($(COMPILER), clang)
	CFLAGS+=-Weverything
else
	CFLAGS+=-Wall
endif

CFLAGS+=-O2 -Wpedantic -Werror
CFLAGS+=-fno-strict-aliasing
IFLAGS+= -Iinclude/

.c.o:
	$(CC) -c $(CFLAGS) $(IFLAGS) $< -o $@

.tsgen: src/tsgen/frand.o src/tsgen/financial.o
.info: src/info/info.o

all: compile_commands .info .tsgen src/main.o
	$(CC) $(CFLAGS) $(IFLAGS) $(shell find . -type f -name "*.o") -o gtsd.out

.PHONY: clean
clean:
	rm **/*.o

.PHONY: compile_commands
compile_commands:
	@echo "[" > compile_commands.json
	@for f in ${CFILES}; do \
		echo "  {" >> compile_commands.json ; \
		echo "    \"directory\": \"${CWD}\"," >> compile_commands.json ; \
		echo "    \"command\":   \"${CC} -c ${CFLAGS} ${IFLAGS} $${f}\"," >> compile_commands.json; \
		echo "    \"file\":      \"$${f}\"" >> compile_commands.json ; \
		echo "  }," >> compile_commands.json ; \
	done
	@truncate -s-2 compile_commands.json
	@echo "" >> compile_commands.json
	@echo "]" >> compile_commands.json

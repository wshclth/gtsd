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

CFLAGS+=-Wpedantic
CFLAGS+=-Werror

IFLAGS+= -Iinclude/

.c.o:
	$(CC) -c $(CFLAGS) $(IFLAGS) $< -o $@

.info: src/info/info.o

all: compile_commands .info src/main.o
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

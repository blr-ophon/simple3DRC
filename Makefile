CC = gcc
OPT = -O2
CFLAGS = -std=c99 -fPIC -g -Wall -Wextra -pedantic $(OPT)

INCLUDES= -I ./include
LIBRARIES= -lSDL2 -lm

CFILES_DIR := ./src
BUILD_DIR := ./build

CFILES := $(wildcard ./src/*.c)
CFILES := $(filter-out src/test.c, $(CFILES))
OBJECTS := $(CFILES:$(CFILES_DIR)/%.c=$(BUILD_DIR)/%.o)

EXEC := ./simple3DRC

${EXEC}: ${OBJECTS}
	gcc ${CFLAGS} ${INCLUDES} ${LIBRARIES} $^ -o $@

${BUILD_DIR}/%.o: ${CFILES_DIR}/%.c
	mkdir -p $(dir $@)
	$(CC) ${CFLAGS} ${INCLUDES} ${LIBRARIES} -c $< -o $@

clean:
	rm -rf ${OBJECTS}
	rm -rf ${EXEC}
	rm -rf ${MERGED_OBJECT}

run: ${EXEC}
	${EXEC}

debug: ${EXEC}
	cgdb ${EXEC}

testleak: ${EXEC}
	valgrind --leak-check=full --show-leak-kinds=all ./$^


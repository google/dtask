SRC := $(wildcard *.c)
HEADERS := $(wildcard *.h) all_tasks.h
OBJS := $(patsubst %.c, %.o, $(SRC))
CFLAGS := -g -O0 -std=c99

.PHONY: all
all: test

test: $(OBJS)
	gcc $(OBJS) -o test

all_tasks.h: $(SRC)
	python generate_task_header.py

%.o: %.c $(HEADERS)
	gcc -c $*.c -o $*.o $(CFLAGS)

.PHONY: clean
clean:
	rm -f *.o all_tasks.h test

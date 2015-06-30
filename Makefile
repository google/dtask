SRC := $(wildcard *.c)
DTASK_TARGETS := stage1_tasks stage2_tasks
DTASK_HEADERS := $(patsubst %, %.h, $(DTASK_TARGETS))
HEADERS := $(wildcard *.h) $(DTASK_HEADERS)
OBJS := $(patsubst %.c, %.o, $(SRC))
CFLAGS := -g -O3 -std=c99

.PHONY: all
all: test

print-%:
	@echo $($*)

test: $(OBJS)
	gcc $(OBJS) -o test

$(DTASK_HEADERS): %.h : $(SRC)
	python generate_task_header.py --target $* $(SRC)

%.o: %.c $(HEADERS)
	gcc -c $*.c -o $*.o $(CFLAGS) $(CFLAGS_EXT)

.PHONY: clean
clean:
	rm -f *.o $(DTASK_HEADERS) test *.pyc

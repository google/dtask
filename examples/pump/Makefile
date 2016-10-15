BUILD_DIR := build
DTASK_ROOT := ../..
DTASK_SRC := $(DTASK_ROOT)/src
DTASK_TOOLS := $(DTASK_ROOT)/tools

ARCH = MSP430
MCU = msp430g2231
CFLAGS = -std=gnu99 -D__$(ARCH)__ -g -Os -mmcu=$(MCU) -DNO_CLZ -DNO_CTZ
CC = msp430-gcc
MCU_INCLUDE = /usr/msp430/include

SRC := $(wildcard *.c)
DTASK_TARGETS := pump_tasks
DTASK_GENERATED_HEADERS := $(patsubst %, $(BUILD_DIR)/%.h, $(DTASK_TARGETS))
HEADERS := $(wildcard *.h) $(DTASK_GENERATED_HEADERS)
OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRC)) $(BUILD_DIR)/dtask.o
CFLAGS += -I $(DTASK_SRC) -I $(BUILD_DIR)

.PHONY: all
all: pump.elf

print-%:
	@echo $($*)

pump.elf: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(DTASK_GENERATED_HEADERS): $(BUILD_DIR)/%.h : $(SRC)
	@mkdir -p $(BUILD_DIR)
	PYTHONPATH=$(DTASK_ROOT) python $(DTASK_TOOLS)/generate_task_header.py -I $(DTASK_SRC) -I $(MCU_INCLUDE) -b 16 --target $* $(SRC)
	mv $*.h $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c $(HEADERS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/dtask.o: $(DTASK_SRC)/dtask.c $(DTASK_SRC)/*.h
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: debug
debug:
	msp430-gdb pump.elf --eval-command="target remote localhost:2000"

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -f pump.elf

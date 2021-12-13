BUILD_DIR := ./build

BUILD := $(BUILD_DIR)/arobase
BUILD_LIB := $(BUILD_DIR)/libarobase.so

SRCS := $(shell find src/ -name *.c)
SRCS_LIB := $(shell find core/ -name *.c)
SRCS_LIB_A := $(shell find core/ -name *.aro)

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
OBJS_LIB := $(SRCS_LIB:%=$(BUILD_DIR)/%.o)
OBJS_LIB_A := $(SRCS_LIB_A:%=$(BUILD_DIR)/%.o)


HEADERS_DIR := /usr/local/include/arobase

HEADERS := $(shell find core/ -name *.aroh)
DEST_HEADERS := $(addprefix $(HEADERS_DIR)/,$(HEADERS))

C_FLAGS := -Iincludes -O3 -Wall

LD_FLAGS := -lm # math.h

CC := gcc

all: $(BUILD) $(BUILD_LIB)
	@echo "Done."

$(BUILD): $(OBJS)
	$(CC) $(OBJS) -o $@ $(C_FLAGS) $(LD_FLAGS) 

$(BUILD_DIR)/src/%.c.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(C_FLAGS) -c $< -o $@ 

$(BUILD_LIB): $(OBJS_LIB) $(OBJS_LIB_A)
	$(CC) $(OBJS_LIB) $(OBJS_LIB_A) -o $@ -shared -fno-stack-protector -mincoming-stack-boundary=3

$(BUILD_DIR)/core/%.c.o: core/%.c
	@mkdir -p $(dir $@)
	$(CC) -Wall -fpic -c $< -o $@ -mincoming-stack-boundary=3

$(BUILD_DIR)/core/%.aro.o: core/%.aro
	@mkdir -p $(dir $@)
	build/arobase -s $< -o $@.s --no-start --assembly
	as $@.s -o $@ -msyntax=intel -mnaked-reg

.PHONY: clean install uninstall
clean:
	rm -r $(BUILD_DIR)

$(DEST_HEADERS): $(HEADERS)
	@for v in $(HEADERS); do \
		install -D $$v /usr/local/include/arobase/$$v; \
	done

install: $(DEST_HEADERS)
	cp build/libarobase.so /lib
	cp build/arobase /usr/local/bin/arobase

	@if test -f /lib/ld-linux-x86-64.so.2; then \
		ln -sf /lib/ld-linux-x86-64.so.2 /lib/ld64.so.1; \
	else \
		if test -f /lib/x86_64-linux-gnu/ld-linux-x86-64.so.2; then \
			ln -sf /lib/x86_64-linux-gnu/ld-linux-x86-64.so.2 /lib/ld64.so.1; \
		else \
			if test -f /lib64/ld-linux-x86-64.so.2; then \
				ln -sf /lib64/ld-linux-x86-64.so.2 /lib/ld64.so.1; \
			else \
				error Did not find any matching loader...; \
			fi; \
		fi; \
	fi; \


uninstall:
	rm -r /lib/libarobase.so
	rm -rf /usr/local/include/arobase/
	rm /usr/local/bin/arobase
PLATFORM ?= qemu-virt
CROSS ?= riscv64-elf-

CC := $(CROSS)gcc

INCLUDES := -Ikernel/include -Ikernel/drivers

CFLAGS := -march=rv64imac_zicsr_zifencei -mabi=lp64 -mcmodel=medany -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -Wall -Wextra -O2 -g -MMD -MP $(INCLUDES)
LDFLAGS := -nostdlib -static -T platform/$(PLATFORM)/linker.ld

SRCS := kernel/arch/riscv64/entry.s \
		kernel/main.c \
		kernel/console.c \
		kernel/drivers/uart/ns16550.c \
		platform/$(PLATFORM)/platform.c

OBJS := $(SRCS:%=build/%.o)
DEPS := $(OBJS:.o=.d)

kernel.elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

build/%.o: %
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

run: kernel.elf
	qemu-system-riscv64 -machine virt -smp 1 -m 8G -bios none -display none -serial stdio -kernel $<

clean:
	rm -rf build kernel.elf

-include $(DEPS)

.PHONY: run clean

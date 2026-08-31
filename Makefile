PLATFORM ?= qemu-virt

ifndef CROSS
ifneq (,$(shell which riscv64-elf-gcc 2>/dev/null))
CROSS := riscv64-elf-
else ifneq (,$(shell which riscv64-unknown-elf-gcc 2>/dev/null))
CROSS := riscv64-unknown-elf-
else ifneq (,$(shell which riscv64-linux-gnu-gcc 2>/dev/null))
CROSS := riscv64-linux-gnu-
else
$(error No RISC-V toolchain found. Install riscv64-elf-gcc, riscv64-unknown-elf-gcc, or riscv64-linux-gnu-gcc)
endif
endif

CC := $(CROSS)gcc

INCLUDES := -Ikernel/include -Ikernel/drivers

CFLAGS := -march=rv64imac_zicsr_zifencei_sstc -mabi=lp64 -mcmodel=medany -ffreestanding -nostdlib -fno-builtin -fno-stack-protector -Wall -Wextra -O2 -g -MMD -MP $(INCLUDES)
LDFLAGS := -nostdlib -static -T platform/$(PLATFORM)/linker.ld

SRCS := kernel/arch/riscv64/entry.s \
		kernel/arch/riscv64/trap_entry.s \
		kernel/arch/riscv64/trap.c \
		kernel/main.c \
		kernel/console.c \
		kernel/panic.c \
		kernel/drivers/uart/ns16550.c \
		kernel/drivers/irq/plic.c \
		kernel/mm/pmm.c \
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

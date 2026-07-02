# walaos

A hobby operating system, written from scratch in C for x86 (i386). I work on it in my spare time for fun and to learn more about how operating systems work under the hood.

## Requirements

To build and run walaos you'll need:

- A `i686-elf` cross-compiler toolchain (`i686-elf-gcc`, `i686-elf-ar`, `i686-elf-as`)
- `grub-mkrescue` and `grub-file` (for building the bootable ISO)
- `xorriso` (used by `grub-mkrescue`)
- `qemu` (to run the OS, `qemu-system-i386`)

## Building

```
./build.sh   # builds libc + kernel into sysroot
./iso.sh     # builds a bootable ISO
./qemu.sh    # builds and boots the ISO in QEMU
```

## Checklist

Done:
- [x] Cross-compiler toolchain and build system
- [x] GRUB multiboot boot + bootable ISO
- [x] GDT setup
- [x] IDT / ISR setup
- [x] PIC remapping
- [x] PIT timer
- [x] Serial logging
- [x] VGA text mode terminal (with cursor)
- [x] Keyboard input
- [x] Kernel panic handling
- [x] Formatted printing (printf-style)

To do:
- [ ] Physical/virtual memory management (paging)
- [ ] Heap allocator
- [ ] Multitasking / scheduler
- [ ] System calls
- [ ] Userspace processes
- [ ] Filesystem

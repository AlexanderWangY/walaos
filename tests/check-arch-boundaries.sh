#!/bin/sh
# Enforce the portability boundary between generic kernel code and per-arch
# code. Universal invariants run once; a per-arch contract is applied to every
# kernel/arch/<arch>; arch-instruction-specific rules (port I/O) stay guarded
# to the arches that actually have that concept (x86), since e.g. ARM is
# MMIO-only and must not be forced to declare inb/outb.
set -eu

fail() {
  echo "check-arch-boundaries: $*" >&2
  exit 1
}

# --- Universal invariants (architecture-independent) ---

# Generic kernel code must reach arch code only through kernel/include/kernel/*
# interfaces, never by including arch-private headers directly.
if grep -R '#include <arch/' kernel/kernel kernel/include 2>/dev/null; then
  fail "generic kernel code should not include arch headers"
fi

# Port I/O is an arch detail; it must not leak out as a generic kernel header.
[ ! -f kernel/include/kernel/io.h ] ||
  fail "port I/O should not be exposed as a generic kernel header"

# --- Per-arch contract (applied to every kernel/arch/<arch>) ---
for archdir in kernel/arch/*/; do
  [ -d "$archdir" ] || continue
  arch=$(basename "$archdir")
  config="$archdir/make.config"

  [ -f "$config" ] ||
    fail "$arch: missing make.config"

  # If the arch ships private headers under include/arch, its build must add
  # that include directory so its own sources can find them.
  if [ -d "$archdir/include/arch" ]; then
    grep -q 'KERNEL_ARCH_CPPFLAGS=.*-I$(ARCHDIR)/include' "$config" ||
      fail "$arch: build should add its arch include directory (-I\$(ARCHDIR)/include)"
  fi

  # Every object listed in KERNEL_ARCH_OBJS must have a matching source file,
  # so the obj list can't drift from reality as the arch grows.
  for obj in $(sed -n 's/.*\$(ARCHDIR)\/\([A-Za-z0-9_]*\)\.o.*/\1/p' "$config"); do
    [ -f "$archdir/$obj.c" ] || [ -f "$archdir/$obj.S" ] ||
      fail "$arch: KERNEL_ARCH_OBJS lists $obj.o but no $obj.c/$obj.S exists"
  done
done

# --- x86 port-I/O contract (only arches that have port I/O) ---
for arch in i386 i686 x86_64; do
  archdir="kernel/arch/$arch"
  [ -d "$archdir" ] || continue

  io_h="$archdir/include/arch/io.h"
  [ -f "$io_h" ] ||
    fail "$arch: missing arch I/O header ($io_h)"
  grep -q 'uint8_t inb(uint16_t port);' "$io_h" ||
    fail "$arch: arch I/O header should declare inb"
  grep -q 'void outb(uint16_t port, uint8_t val);' "$io_h" ||
    fail "$arch: arch I/O header should declare outb"
  grep -q 'void io_wait(void);' "$io_h" ||
    fail "$arch: arch I/O header should declare io_wait with a void parameter list"

  for src in io keyboard; do
    [ -f "$archdir/$src.c" ] || continue
    grep -q '#include <arch/io.h>' "$archdir/$src.c" ||
      fail "$arch: $src.c should include the arch I/O header"
  done
done

echo "check-arch-boundaries: ok"

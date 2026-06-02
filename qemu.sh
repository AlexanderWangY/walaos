#!/bin/sh
# Build the image and boot it under QEMU. How a given arch is run (machine
# type, BIOS/UEFI, -cdrom vs -kernel, ...) is arch-specific and lives in
# kernel/arch/$ARCH/qemu.sh. iso.sh already computes HOSTARCH in this shell.
set -e
. ./iso.sh

ARCH_QEMU="kernel/arch/$HOSTARCH/qemu.sh"

if [ ! -f "$ARCH_QEMU" ]; then
  echo "qemu.sh: no run recipe for arch '$HOSTARCH' (expected $ARCH_QEMU)" >&2
  exit 1
fi

. "$ARCH_QEMU"

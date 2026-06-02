#!/bin/sh
# Build a bootable image for the current HOST's architecture. The actual
# imaging recipe (GRUB ISO, raw kernel, UEFI, ...) is arch-specific and lives
# in kernel/arch/$ARCH/iso.sh so this top-level script stays portable.
set -e
. ./build.sh

HOSTARCH="$(./target-triplet-to-arch.sh "$HOST")"
ARCH_ISO="kernel/arch/$HOSTARCH/iso.sh"

if [ ! -f "$ARCH_ISO" ]; then
  echo "iso.sh: no image recipe for arch '$HOSTARCH' (expected $ARCH_ISO)" >&2
  exit 1
fi

# Sourced (not exec'd) so it inherits SYSROOT, GRUB_MKRESCUE, etc. from config.
. "$ARCH_ISO"

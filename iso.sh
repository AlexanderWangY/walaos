#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/walaos.kernel isodir/boot/walaos.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "walaos" {
	multiboot /boot/walaos.kernel
}
EOF
"$GRUB_MKRESCUE" -o walaos.iso isodir

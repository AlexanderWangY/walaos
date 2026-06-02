# i386 image recipe: a GRUB El Torito rescue ISO booting our multiboot kernel.
# Sourced by the top-level iso.sh with SYSROOT and GRUB_MKRESCUE already set.
mkdir -p isodir/boot/grub

cp sysroot/boot/walaos.kernel isodir/boot/walaos.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "walaos" {
	multiboot /boot/walaos.kernel
}
EOF

"$GRUB_MKRESCUE" -o walaos.iso isodir

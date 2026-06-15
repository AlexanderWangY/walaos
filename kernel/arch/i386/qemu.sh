# i386 run recipe: boot the GRUB rescue ISO via the default BIOS machine.
# Sourced by the top-level qemu.sh after the image is built.
qemu-system-i386 -cdrom walaos.iso -serial stdio

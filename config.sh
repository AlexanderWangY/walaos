SYSTEM_HEADER_PROJECTS="libc kernel"
PROJECTS="libc kernel"

export MAKE=${MAKE:-make}
export HOST=${HOST:-$(./default-host.sh)}

export AR=${HOST}-ar
export AS=${HOST}-as
export CC=${HOST}-gcc

export PREFIX=/usr
export EXEC_PREFIX=$PREFIX
export BOOTDIR=/boot
export LIBDIR=$EXEC_PREFIX/lib
export INCLUDEDIR=$PREFIX/include

export CFLAGS='-O2 -g'
export CPPFLAGS=''

if command -v grub-file >/dev/null 2>&1; then
  export GRUB_FILE=${GRUB_FILE:-grub-file}
else
  export GRUB_FILE=${GRUB_FILE:-${HOST}-grub-file}
fi

if command -v grub-mkrescue >/dev/null 2>&1; then
  export GRUB_MKRESCUE=${GRUB_MKRESCUE:-grub-mkrescue}
else
  export GRUB_MKRESCUE=${GRUB_MKRESCUE:-${HOST}-grub-mkrescue}
fi

# Configure the cross-compiler to use the desired system root.
export SYSROOT="$(pwd)/sysroot"
export CC="$CC --sysroot=$SYSROOT"

# Work around that the -elf gcc targets doesn't have a system include directory
# because it was configured with --without-headers rather than --with-sysroot.
if echo "$HOST" | grep -Eq -- '-elf($|-)'; then
  export CC="$CC -isystem=$INCLUDEDIR"
fi

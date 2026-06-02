#!/bin/sh
# Generate a per-arch-accurate compile_commands.json for clangd by intercepting
# the real build. We wrap $CC with a shim that records every compiler
# invocation (with the exact flags the Makefiles use) before execing the real
# compiler. This means the database never drifts from the build and is correct
# for whatever HOST/arch you build, instead of hardcoding one arch's flags.
set -e

. ./config.sh

CDB_DIR="$(mktemp -d)"
trap 'rm -rf "$CDB_DIR"' EXIT
export CDB_DIR

# The real (composed) compiler, including --sysroot and friends.
REAL_CC="$CC"
export REAL_CC

# Shim that records the invocation as one JSON fragment, then compiles.
cat > "$CDB_DIR/cc-shim.sh" <<'SHIM'
#!/bin/sh
src=""
for a in "$@"; do
  case "$a" in
    *.c|*.S) src="$a" ;;
  esac
done
if [ -n "$src" ] && [ -n "$CDB_DIR" ]; then
  frag="$CDB_DIR/$$-$(echo "$src" | tr '/.' '__').json"
  # Feed argv to jq as NUL-delimited data on stdin so compiler flags (-c, -o,
  # ...) are never mistaken for jq's own options. $REAL_CC is unquoted on
  # purpose so its tokens (compiler + --sysroot=...) become separate argv
  # entries, which is what clangd expects.
  printf '%s\0' $REAL_CC "$@" |
    jq -sR --arg dir "$PWD" --arg file "$src" \
      '{directory:$dir, file:$file, arguments:(rtrimstr("\u0000") | split("\u0000"))}' > "$frag"
fi
exec $REAL_CC "$@"
SHIM
chmod +x "$CDB_DIR/cc-shim.sh"

# Point the build at the shim. Sub-makes inherit this via the environment.
export CC="$CDB_DIR/cc-shim.sh"

# Headers must be staged so cross-includes resolve, then do a clean rebuild so
# every translation unit is actually compiled (and thus recorded).
mkdir -p "$SYSROOT"
for PROJECT in $SYSTEM_HEADER_PROJECTS; do
  (cd "$PROJECT" && DESTDIR="$SYSROOT" $MAKE install-headers)
done
for PROJECT in $PROJECTS; do
  (cd "$PROJECT" && $MAKE clean && DESTDIR="$SYSROOT" $MAKE)
done

# Assemble fragments into a single array.
if ls "$CDB_DIR"/*.json >/dev/null 2>&1; then
  jq -s '.' "$CDB_DIR"/*.json > compile_commands.json
  echo "wrote compile_commands.json ($(jq 'length' compile_commands.json) entries) for HOST=$HOST"
else
  echo "gen-compile-commands: no compiler invocations captured" >&2
  exit 1
fi

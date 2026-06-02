#!/bin/sh
# Run every static check under tests/. Kept separate from the build so it can
# run standalone (e.g. in CI), but build.sh invokes it so a portability
# violation fails the build loudly instead of silently shipping.
set -e

for t in tests/*.sh; do
  [ -f "$t" ] || continue
  echo "check: $t"
  sh "$t"
done

echo "check: all checks passed"

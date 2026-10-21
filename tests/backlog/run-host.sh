#!/bin/bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
OUT=$(mktemp -d)
trap 'rm -rf -- "$OUT"' EXIT
cc -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined \
  -I"$ROOT/launcher/source" "$ROOT/tests/backlog/banner_library_test.c" \
  "$ROOT/launcher/source/banner_format.c" "$ROOT/launcher/source/library_model.c" \
  -lz -o "$OUT/banner-library-test"
(cd "$OUT" && ./banner-library-test)

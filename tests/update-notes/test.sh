#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
OUT="$ROOT/.test-build/update-notes"
mkdir -p "$OUT"
${CC:-cc} -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all \
    -I"$ROOT/launcher/source" "$ROOT/tests/update-notes/test.c" \
    "$ROOT/launcher/source/update_format.c" -o "$OUT/parser-test"
"$OUT/parser-test"
if [ "$#" -ne 0 ]; then "$OUT/parser-test" "$@"; fi
${CC:-cc} -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all -pthread \
    -I"$ROOT/tests/update-notes/mock" -I"$ROOT/launcher/source" \
    "$ROOT/tests/update-notes/transport_test.c" "$ROOT/launcher/source/app_update.c" \
    "$ROOT/launcher/source/update_format.c" "$ROOT/launcher/source/update_store.c" -o "$OUT/transport-test"
"$OUT/transport-test"

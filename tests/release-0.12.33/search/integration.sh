set -eu
ROOT=/home/maxim/codex-3ds-work/toolchain
TASK=/mnt/c/Users/subje/Documents/Codex/2026-10-13/kan
cp "$TASK/work/search-v01225/integration.c" "$ROOT/search2-test/"
cp "$TASK/outputs/3ds-library/source/library_model.c" "$ROOT/search2-test/"
cp "$TASK/outputs/3ds-library/source/"*.h "$ROOT/search2-test/"
chroot "$ROOT" /bin/bash -c 'set -e; cd /search2-test; gcc -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined integration.c library_model.c game_search.c -o integration; ./integration'

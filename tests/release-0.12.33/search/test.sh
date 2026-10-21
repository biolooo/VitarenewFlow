set -eu
ROOT=/home/maxim/codex-3ds-work/toolchain
TASK=/mnt/c/Users/subje/Documents/Codex/2026-10-13/kan
mkdir -p "$ROOT/search2-test"
cp "$TASK/work/search-v01225/test.c" "$TASK/outputs/3ds-library/source/game_search.c" "$TASK/outputs/3ds-library/source/game_search.h" "$TASK/outputs/3ds-library/source/home_controls.c" "$TASK/outputs/3ds-library/source/home_controls.h" "$ROOT/search2-test/"
chroot "$ROOT" /bin/bash -c 'set -e; cd /search2-test; gcc -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined test.c game_search.c home_controls.c -o test; ./test'

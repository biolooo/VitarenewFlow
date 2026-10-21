set -eu
ROOT=/home/maxim/codex-3ds-work/toolchain
TASK=/mnt/c/Users/subje/Documents/Codex/2026-10-13/kan
mkdir -p "$ROOT/power-rearm-test"
cp "$TASK/work/power-v01227/test.c" "$ROOT/power-rearm-test/"
chroot "$ROOT" /bin/bash -c 'set -e; cd /power-rearm-test; gcc -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined test.c -o test; ./test'

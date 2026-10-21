set -eu
TASK=/mnt/c/Users/subje/Documents/Codex/2026-10-13/kan
ROOT=/home/maxim/codex-3ds-work/toolchain
cp "$TASK/work/v0136/twl_test.c" "$ROOT/project/twl_test.c"
mkdir -p "$ROOT/project/twl-mock"
cp "$TASK/work/v0132/mock/3ds.h" "$ROOT/project/twl-mock/3ds.h"
chroot "$ROOT" /bin/bash -c 'cd /project; gcc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all -Itwl-mock -Isource twl_test.c -o twl-test && ./twl-test'
python3 "$TASK/work/v0135/test-launch.py"
python3 "$TASK/work/v0114/test-system.py"

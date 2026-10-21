set -eu
ROOT=/home/maxim/codex-3ds-work/toolchain
TASK=/mnt/c/Users/subje/Documents/Codex/2026-10-13/kan
mkdir -p "$ROOT/launch-test"
cp "$TASK/work/launch-v01223/test.c" "$TASK/outputs/3ds-library/source/launch_problem.h" "$TASK/outputs/3ds-library/source/library_model.h" "$ROOT/launch-test/"
chroot "$ROOT" /bin/bash -c 'set -e; cd /launch-test; gcc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined test.c -o test; ./test'

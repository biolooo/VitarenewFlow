set -eu
TASK=/mnt/c/Users/subje/Documents/Codex/2026-10-13/kan
ROOT=/home/maxim/codex-3ds-work/toolchain
cp "$TASK/work/v0137/launch_test.c" "$ROOT/project/launch-v128.c"
chroot "$ROOT" /bin/bash -c 'set -e;cd /project;gcc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all -Imock-sdk -Isource launch-v128.c -o launch-v128-test;./launch-v128-test'

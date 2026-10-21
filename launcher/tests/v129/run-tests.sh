set -eu
TASK=/mnt/c/Users/subje/Documents/Codex/2026-10-13/kan
ROOT=/home/maxim/codex-3ds-work/toolchain
cp "$TASK/work/v0138/launch_test.c" "$ROOT/project/launch-v129.c"
chroot "$ROOT" /bin/bash -c 'set -e;cd /project;gcc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all -Imock-sdk -Isource launch-v129.c -o launch-v129-test;./launch-v129-test'

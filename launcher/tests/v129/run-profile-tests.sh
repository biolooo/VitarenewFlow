set -eu
TASK=/mnt/c/Users/subje/Documents/Codex/2026-10-13/kan
ROOT=/home/maxim/codex-3ds-work/toolchain
cp "$TASK/work/v0138/ds_profile_test.c" "$ROOT/project/"
cp "$TASK/outputs/3ds-library/source/"ds_profile.{c,h} "$ROOT/project/source/"
chroot "$ROOT" /bin/bash -c 'set -e;cd /project;gcc -D_DEFAULT_SOURCE -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all -Imock-sdk -Isource ds_profile_test.c -o ds-profile-v129-test;./ds-profile-v129-test'

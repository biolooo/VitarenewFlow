set -eu
TASK=/mnt/c/Users/subje/Documents/Codex/2026-10-13/kan
ROOT=/home/maxim/codex-3ds-work/toolchain
cp "$TASK/work/v0138/"{ds_preflight_test.c,log_test.c} "$ROOT/project/"
chroot "$ROOT" /bin/bash -c 'set -e;cd /project;gcc -D_DEFAULT_SOURCE -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -Imock-sdk -Isource ds_preflight_test.c source/ds_path.c -o ds-preflight-v129-test;./ds-preflight-v129-test;gcc -D_DEFAULT_SOURCE -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -Isource log_test.c -o log-v129-test;./log-v129-test'

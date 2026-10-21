set -eu
TASK=/mnt/c/Users/subje/Documents/Codex/2026-10-13/kan
ROOT=/home/maxim/codex-3ds-work/toolchain
cp "$TASK/work/v0138/"{home_test.c,autoboot_controls_test.c} "$ROOT/project/"
cp "$TASK/work/v0132/l10n_test.c" "$ROOT/project/"
chroot "$ROOT" /bin/bash -c 'set -e;cd /project;gcc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all home_test.c -o home-v129-test;./home-v129-test;gcc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all autoboot_controls_test.c -o autoboot-controls-v129-test;./autoboot-controls-v129-test;gcc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -Isource l10n_test.c -o l10n-v129-test;./l10n-v129-test;./autoboot-test;./twl-test'

set -eu
TASK=/mnt/c/Users/subje/Documents/Codex/2026-10-13/kan
ROOT=/home/maxim/codex-3ds-work/toolchain
mkdir -p "$ROOT/project/v127-mock" "$ROOT/project/v127-audio-mock"
cp "$TASK/work/v0135/"{autoboot_test.c,dispatcher_test.c,home_controls_test.c,music_test.c} "$ROOT/project/"
cp "$TASK/work/v0135/mock/3ds.h" "$ROOT/project/v127-mock/"
cp "$TASK/work/v0135/audio-mock/3ds.h" "$ROOT/project/v127-audio-mock/"
cp "$TASK/outputs/3ds-library/boot-dispatcher/source/main.c" "$ROOT/project/dispatcher-main.c"
chroot "$ROOT" /bin/bash -c 'set -e;cd /project;gcc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -Isource autoboot_test.c -o autoboot-test;./autoboot-test;gcc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -Wno-pointer-to-int-cast -Iv127-mock dispatcher_test.c -o dispatcher-test;./dispatcher-test;gcc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined home_controls_test.c -o home-controls-test;./home-controls-test;gcc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -Iv127-audio-mock -Isource music_test.c source/bgm_format.c -o music-test;./music-test'


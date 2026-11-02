#!/bin/bash
set -euo pipefail
mkdir -p .test-build
cc -D_POSIX_C_SOURCE=200809L -Dmalloc=title_cache_test_malloc -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -Ilauncher/source -c launcher/source/title_cache.c -o .test-build/title-cache-unit.o
cc -D_POSIX_C_SOURCE=200809L -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -Ilauncher/source tests/title-cache/test.c .test-build/title-cache-unit.o -o .test-build/title-cache-test
.test-build/title-cache-test

python3 tests/title-cache/test_scan.py

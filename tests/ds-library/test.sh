#!/bin/bash
set -euo pipefail
cc -D_POSIX_C_SOURCE=200809L -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -Ilauncher/source tests/ds-library/test.c launcher/source/ds_library.c -Wl,--wrap=rename -o .test-build/ds-library-test
.test-build/ds-library-test

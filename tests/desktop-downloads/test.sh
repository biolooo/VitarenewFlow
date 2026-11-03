#!/bin/bash
set -euo pipefail
python3 tests/desktop-downloads/test.py
mkdir -p .test-build
cc -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -Ilauncher/source tests/desktop-downloads/test_l10n.c -o .test-build/desktop-l10n-test
.test-build/desktop-l10n-test

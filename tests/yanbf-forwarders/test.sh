#!/bin/bash
set -euo pipefail
mkdir -p .test-build
cc -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -Ilauncher/source tests/yanbf-forwarders/test.c launcher/source/library_model.c launcher/source/l10n.c -o .test-build/yanbf-classifier-test
.test-build/yanbf-classifier-test
python3 tests/yanbf-forwarders/test_main.py

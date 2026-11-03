#!/bin/bash
set -euo pipefail
mkdir -p .test-build
cc -D_DARWIN_C_SOURCE -D_POSIX_C_SOURCE=200809L -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -Ilauncher/source tests/rom-discovery/test.c launcher/source/rom_library.c launcher/source/ds_library.c launcher/source/library_model.c -o .test-build/rom-discovery-test
.test-build/rom-discovery-test
python3 tests/rom-discovery/test_scan.py
cc -D_DARWIN_C_SOURCE -D_POSIX_C_SOURCE=200809L -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -Ilauncher/source -Dlstat=rom_test_lstat -Dopendir=rom_test_opendir -Dreaddir=rom_test_readdir -Dcalloc=rom_test_calloc -c launcher/source/rom_library.c -o .test-build/rom-discovery-io.o
cc -D_DARWIN_C_SOURCE -D_POSIX_C_SOURCE=200809L -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -Ilauncher/source tests/rom-discovery/test_io.c .test-build/rom-discovery-io.o launcher/source/ds_library.c launcher/source/library_model.c -o .test-build/rom-discovery-io-test
.test-build/rom-discovery-io-test

#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p .test-build
cc -std=c11 -Wall -Wextra -Werror -Wno-misleading-indentation -fsanitize=undefined -Ilauncher/tests -Ilauncher/source launcher/source/l10n.c launcher/source/native_handoff.c launcher/source/catalog_format.c launcher/tests/features_test.c -o .test-build/features
(cd .test-build && ./features)
cc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -Ilauncher/tests -Ilauncher/source launcher/source/game_seed.c launcher/source/catalog_format.c launcher/tests/game_seed_test.c -o .test-build/seeds
(cd .test-build && ./seeds)

#!/bin/bash
set -euo pipefail
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=/opt/devkitpro/devkitARM
export PATH=/opt/devkitpro/tools/bin:/opt/devkitpro/devkitARM/bin:/usr/bin:/bin
python3 check-fixture-source.py
make -C launcher -j2
bash tests/component-runtime/test.sh
bash tests/twl/test.sh
bash tests/home-loading/test.sh
cc -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -Ilauncher/source tests/search/test.c launcher/source/game_search.c launcher/source/home_controls.c -o tests/search/search-test
tests/search/search-test
# These release fixtures are source-anchored below; run in scratch directories.
cc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -Ilauncher/source tests/launch/test.c -o tests/launch/launch-test
(cd tests/launch && ./launch-test)
cc -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined tests/power/test.c -o tests/power/power-test
tests/power/power-test
echo 'PASS: native build and existing component, TWL, HOME-loading, search, launch, POWER fixtures; not hardware acceptance'

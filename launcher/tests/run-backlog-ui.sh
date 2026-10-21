#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/../.."
mkdir -p .test-build
cc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -Ilauncher/source \
  launcher/tests/backlog_ui_test.c launcher/source/settings_menu.c \
  launcher/source/l10n.c launcher/source/update_format.c -o .test-build/backlog-ui-test
.test-build/backlog-ui-test

cc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -Ilauncher/source \
  launcher/tests/update_details_test.c launcher/source/update_details.c -o .test-build/update-details-test
.test-build/update-details-test
python3 launcher/tests/update_notes_layout_test.py

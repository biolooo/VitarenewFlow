#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/../.."
mkdir -p .test-build
cc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -Ilauncher/source \
  launcher/tests/backlog_ui_test.c launcher/source/settings_menu.c \
  launcher/source/l10n.c launcher/source/update_format.c -o .test-build/backlog-ui-test
.test-build/backlog-ui-test

#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
OUT="$ROOT/.test-build/error-report"
mkdir -p "$OUT"
${CC:-cc} -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all \
    -I"$ROOT/launcher/source" "$ROOT/launcher/source/error_report.c" \
    "$ROOT/tests/error-report/test.c" -o "$OUT/test"
"$OUT/test" > "$OUT/vectors.jsonl"
python3 - "$OUT/vectors.jsonl" <<'PY'
import json, sys
from pathlib import Path
reports = [json.loads(line) for line in Path(sys.argv[1]).read_text(encoding='utf-8').splitlines()]
fields = {'schema', 'version', 'area', 'code', 'model', 'region', 'autoboot', 'context', 'activity', 'description'}
assert len(reports) == 216
assert all(set(report) == fields and report['schema'] == 1 for report in reports)
assert reports[0]['description'] == '' and reports[0]['code'] == -(2**31)
assert reports[1]['description'] == 'He said "Hallo".\\\n\t\r\b\f\x01 Grüße 🎮'
assert reports[1]['code'] == 2**31 - 1
assert reports[2]['description'] == '\x01' * 512
assert all(len(report['description'].encode('utf-8')) <= 512 for report in reports)
state = 789
for trial, report in enumerate(reports[-200:]):
    expected = ''
    for _ in range(trial % 129):
        state = (state * 1664525 + 1013904223) & 0xffffffff
        expected += chr(1 + state % 127)
    assert report['description'] == expected
print('PASS: independent Python JSON decode and 200 exact escaping roundtrips')
PY

${CC:-cc} -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -I"$ROOT/launcher/source" "$ROOT/tests/error-report/l10n_test.c" -o "$OUT/l10n-test"
"$OUT/l10n-test"

${CC:-cc} -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -I"$ROOT/launcher/source" "$ROOT/tests/error-report/controls_test.c" "$ROOT/launcher/source/report_controls.c" -o "$OUT/controls-test"
"$OUT/controls-test"

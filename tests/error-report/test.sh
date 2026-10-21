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
assert len(reports) == 239
assert all(set(report) == fields and report['schema'] == 1 for report in reports[:216])
assert reports[0]['description'] == '' and reports[0]['code'] == -(2**31)
assert reports[1]['description'] == 'He said "Hallo".\\\n\t\r\b\f\x01 Grüße 🎮'
assert reports[1]['code'] == 2**31 - 1
assert reports[2]['description'] == '\x01' * 512
assert all(len(report['description'].encode('utf-8')) <= 512 for report in reports)
state = 789
for trial, report in enumerate(reports[16:216]):
    expected = ''
    for _ in range(trial % 129):
        state = (state * 1664525 + 1013904223) & 0xffffffff
        expected += chr(1 + state % 127)
    assert report['description'] == expected
print('PASS: independent Python JSON decode and 200 exact escaping roundtrips')
diagnostic_fields = {'validity', 'uptime_seconds', 'sd_free_mib', 'linear_free_kib', 'trace'}
activities = ['unknown', 'browsing', 'starting-game', 'installing', 'checking-update',
              'downloading', 'checking-components', 'checking-ds', 'opening-home',
              'power-menu', 'loading-theme', 'loading-cover', 'importing']
for report in reports[216:]:
    assert set(report) == fields | {'diagnostics'} and report['schema'] == 2
    diagnostics = report['diagnostics']
    assert set(diagnostics) == diagnostic_fields
    assert 0 <= diagnostics['validity'] <= 7
    for key, flag, maximum in [('uptime_seconds', 1, 604800), ('sd_free_mib', 2, 4194304), ('linear_free_kib', 4, 1048576)]:
        assert type(diagnostics[key]) is int and 0 <= diagnostics[key] <= maximum
        assert diagnostics['validity'] & flag or diagnostics[key] == 0
    assert len(diagnostics['trace']) <= 8 and all(value in activities for value in diagnostics['trace'])
assert reports[216]['diagnostics'] == dict(validity=0, uptime_seconds=0, sd_free_mib=0, linear_free_kib=0, trace=[])
assert reports[217]['description'] == '\x01' * 512
assert reports[217]['diagnostics'] == dict(validity=7, uptime_seconds=604800, sd_free_mib=4194304, linear_free_kib=1048576,
    trace=['browsing', 'checking-update', 'downloading', 'checking-components', 'checking-ds', 'opening-home', 'loading-theme', 'starting-game'])
assert [r['diagnostics']['validity'] for r in reports[218:226]] == list(range(8))
assert [r['diagnostics']['trace'] for r in reports[226:]] == [[activity] for activity in activities]
print('PASS: independent schema 2 JSON decode, privacy allowlist and exact bounded diagnostic snapshots')
PY

${CC:-cc} -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -I"$ROOT/launcher/source" "$ROOT/tests/error-report/l10n_test.c" -o "$OUT/l10n-test"
"$OUT/l10n-test"

${CC:-cc} -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -I"$ROOT/launcher/source" "$ROOT/tests/error-report/controls_test.c" "$ROOT/launcher/source/report_controls.c" -o "$OUT/controls-test"
"$OUT/controls-test"

${CC:-cc} -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all -I"$ROOT/launcher/source" "$ROOT/tests/error-report/history_test.c" "$ROOT/launcher/source/report_history.c" -o "$OUT/history-test"
"$OUT/history-test"

#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
OUT="$ROOT/.test-build/report-transport"
mkdir -p "$OUT"
for MODE in configured unconfigured; do
  if [ "$MODE" = configured ]; then DEFINE='-DFLOW_REPORT_ENDPOINT="https://receiver.invalid/v1/reports"'; else DEFINE='-DFLOW_REPORT_ENDPOINT=""'; fi
  cc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all "$DEFINE" -I"$ROOT/tests/report-transport/mock" -I"$ROOT/launcher/source" "$ROOT/tests/report-transport/test.c" "$ROOT/launcher/source/report_transport.c" "$ROOT/launcher/source/error_report.c" -o "$OUT/$MODE"
  "$OUT/$MODE"
done

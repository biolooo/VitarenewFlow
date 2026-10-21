#!/bin/bash
# Isolated verification of this checkout using an existing SDK, no legacy writes.
set -euo pipefail
PROJECT=$(cd "$(dirname "$0")/.." && pwd)
VERSION=$(sed -n 's/^APP_DESCRIPTION[[:space:]]*:=[[:space:]]*Game Launcher - Version //p' "$PROJECT/launcher/Makefile")
python3 "$PROJECT/scripts/generate-release-notes.py" --root "$PROJECT" --version "$VERSION" --check
SDK=${FLOW_TOOLCHAIN_ROOT:-/home/maxim/codex-3ds-work/toolchain}
test -x "$SDK/bin/bash" || { echo "Missing existing SDK: set FLOW_TOOLCHAIN_ROOT" >&2; exit 1; }
OUT="$PROJECT/.test-build/verification-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$OUT"
(
cd "$PROJECT"
python3 scripts/check-component-packages.py
python3 -m unittest discover -s tests/backlog -p 'test_*.py'
) 2>&1 | tee "$OUT/package-verification.log"
STAGE=$(mktemp -d "$SDK/flow-check-XXXXXX")
mkdir -p "$STAGE/launcher" "$STAGE/tests"
cp "$PROJECT/launcher/Makefile" "$STAGE/launcher/"
cp -a "$PROJECT/launcher/source" "$PROJECT/launcher/branding" "$PROJECT/launcher/data" "$STAGE/launcher/"
cp -a "$PROJECT/tests/release-0.12.33/." "$STAGE/tests/"
cp -a "$PROJECT/tests/error-report" "$STAGE/tests/"
cp -a "$PROJECT/tests/update-notes" "$STAGE/tests/"
cp -a "$PROJECT/tests/report-transport" "$STAGE/tests/"
cp -a "$PROJECT/tests/backlog" "$STAGE/tests/"
cp -a "$PROJECT/launcher/tests" "$STAGE/launcher/"
mkdir -p "$STAGE/scripts"
cp "$PROJECT/scripts/generate-release-notes.py" "$STAGE/scripts/"
cp -a "$PROJECT/release-notes" "$STAGE/"
cp "$PROJECT/scripts/verify-stage.sh" "$STAGE/"
cp "$PROJECT/scripts/check-fixture-source.py" "$STAGE/"
python3 - "$STAGE" "$OUT/source-hashes.json" <<'PY'
from pathlib import Path
import sys,json,hashlib
p=Path(sys.argv[1]); roots=[p/'launcher/source',p/'launcher/branding',p/'launcher/data']
files=[p/'launcher/Makefile']+[f for root in roots for f in root.rglob('*') if f.is_file()]
Path(sys.argv[2]).write_text(json.dumps({f.relative_to(p).as_posix():hashlib.sha256(f.read_bytes()).hexdigest() for f in files},indent=2))
PY
REL=${STAGE#"$SDK"}
printf '%s\n' "$STAGE" > "$OUT/staging-path.txt"
chroot "$SDK" /bin/bash -c "cd '$REL'; bash verify-stage.sh" 2>&1 | tee "$OUT/verification.log"
cp "$STAGE/launcher/3ds-library.3dsx" "$STAGE/launcher/3ds-library.smdh" "$OUT/"
sha256sum "$OUT/3ds-library.3dsx" "$OUT/3ds-library.smdh" > "$OUT/artifact-hashes.txt"
echo "PASS: checkout verification; output $OUT"

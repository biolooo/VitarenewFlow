#!/bin/sh
# Run as the account owning this dedicated deployment directory, not via sudo.
set -eu
cd "$(dirname "$0")"
if [ "$(id -u)" = 0 ]; then
    echo 'Run as the owning service account, not root.' >&2
    exit 1
fi
umask 077
mkdir -p data
test -w data || { echo 'Dedicated data directory is not writable.' >&2; exit 1; }
chmod 700 data
FLOW_REPORT_UID=$(id -u)
FLOW_REPORT_GID=$(id -g)
export FLOW_REPORT_UID FLOW_REPORT_GID
docker compose up -d --build
docker compose ps

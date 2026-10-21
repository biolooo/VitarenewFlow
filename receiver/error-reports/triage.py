#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Read-only, bounded report grouping; never export comments or full reports."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import sqlite3
import time

from server import RETENTION_SECONDS, validate_report


def summarize(rows):
    groups = {}
    for created_at, payload in rows:
        report = validate_report(json.loads(payload))
        key = tuple(report[name] for name in ("version", "area", "code", "activity"))
        fingerprint = hashlib.sha256(json.dumps(key).encode()).hexdigest()[:16]
        group = groups.setdefault(fingerprint, {
            "id": fingerprint, "version": key[0], "area": key[1], "code": key[2],
            "activity": key[3], "count": 0, "first_seen": created_at,
            "last_seen": created_at, "models": Counter(), "contexts": Counter(),
            "with_diagnostics": 0, "with_comment": 0,
            "classification": "error" if key[2] < 0 else "needs-triage",
        })
        group["count"] += 1
        group["first_seen"] = min(group["first_seen"], created_at)
        group["last_seen"] = max(group["last_seen"], created_at)
        group["models"][report["model"]] += 1
        group["contexts"][report["context"]] += 1
        group["with_comment"] += bool(report["description"])
        group["with_diagnostics"] += report["schema"] == 2
    return sorted(groups.values(), key=lambda g: (-g["count"], g["id"]))


def read_database(path, now=None):
    now = int(time.time()) if now is None else now
    uri = Path(path).resolve().as_uri() + "?mode=ro"
    with sqlite3.connect(uri, uri=True, timeout=5) as connection:
        connection.execute("PRAGMA query_only=ON")
        rows = connection.execute(
            "SELECT created_at,payload FROM reports WHERE created_at>=? ORDER BY created_at LIMIT 10001",
            (now - RETENTION_SECONDS,),
        ).fetchall()
    if len(rows) > 10000:
        raise ValueError("Report volume exceeds safe limit; explicit batched triage required")
    return {"generated_at": now, "window_days": 30, "total": len(rows), "groups": summarize(rows)}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--database", required=True)
    args = parser.parse_args()
    print(json.dumps(read_database(args.database), ensure_ascii=True, sort_keys=True))

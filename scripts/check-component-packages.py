#!/usr/bin/env python3
"""Offline bytes/companion inventory; not a license or corresponding-source approval."""
from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path, PurePosixPath
from urllib.parse import unquote, urlsplit

COMPANIONS = {
    "components-1": (
        "gba-source.tar.gz", "gba-LICENSE", "n64-source.tar.gz",
        "n64-UPSTREAM-copying.txt", "picaGL-revamp-source.tar.gz",
        "imgui-picagl-main-source.tar.gz", "ds-ntr-forwarder-source.zip",
        "ds-bootstrap-source.zip", "ds-yanbf-source.zip", "ds-LICENSE.txt",
    ),
    "retro-1": (
        "snes-source.tar.gz", "snes-NOTICES.txt", "nes-source.tar.gz",
        "nes-runtime-source.tar.gz", "nes-NOTICES.txt", "vb-source.tar.gz",
        "vb-NOTICES.txt",
    ),
    "desktop-1": ("VitarenewFlow-Desktop-source.zip", "LICENSE.txt"),
}


def inspect_manifest(manifest: Path, companions: tuple[str, ...] = ()) -> tuple[int, list[str]]:
    errors: list[str] = []
    payload = json.loads(manifest.read_text(encoding="utf-8-sig"))
    packages = payload if isinstance(payload, list) else [payload]
    if not packages:
        return 0, ["empty package manifest"]
    names: set[str] = set()
    targets: set[str] = set()
    count = 0
    for package in packages:
        if (not isinstance(package, dict) or not isinstance(package.get("files"), list)
                or not package["files"]):
            errors.append("invalid package/files schema")
            continue
        for entry in package["files"]:
            if not isinstance(entry, dict):
                errors.append("invalid file entry")
                continue
            name = entry.get("name")
            if (not isinstance(name, str) or not name or name in {".", ".."}
                    or any(c in name for c in "/\\\r\n")):
                errors.append("invalid local basename")
                continue
            count += 1
            if name.casefold() in names:
                errors.append(f"duplicate filename: {name}")
            names.add(name.casefold())
            target = entry.get("target", "")
            if (not isinstance(target, str) or not target.startswith("sdmc:/")
                    or ".." in PurePosixPath(target).parts or "\\" in target
                    or any(c in target for c in "\r\n")):
                errors.append(f"invalid SD target: {name}")
            elif target.casefold() in targets:
                errors.append(f"duplicate SD target: {name}")
            else:
                targets.add(target.casefold())
            url = urlsplit(entry.get("url", ""))
            expected = f"/VitarenewFlow/updates/{manifest.parent.name}/{name}"
            if (url.scheme != "https" or url.netloc != "biolooo.github.io"
                    or unquote(url.path) != expected or url.query or url.fragment):
                errors.append(f"unexpected download URL: {name}")
            local = manifest.parent / name
            if local.is_symlink() or not local.is_file():
                errors.append(f"missing/nonregular payload: {name}")
                continue
            expected_size = entry.get("bytes")
            if (type(expected_size) is not int or expected_size <= 0
                    or local.stat().st_size != expected_size):
                errors.append(f"size mismatch: {name}")
            expected_hash = entry.get("sha256")
            if (not isinstance(expected_hash, str)
                    or re.fullmatch(r"[0-9a-f]{64}", expected_hash) is None
                    or hashlib.sha256(local.read_bytes()).hexdigest() != expected_hash):
                errors.append(f"SHA256 mismatch: {name}")
    for name in companions:
        local = manifest.parent / name
        if local.is_symlink() or not local.is_file() or local.stat().st_size == 0:
            errors.append(f"missing/empty source or notice companion: {name}")
    return count, errors


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    args = parser.parse_args()
    total = 0
    errors: list[str] = []
    for directory, companions in COMPANIONS.items():
        filename = "package.json" if directory == "desktop-1" else "packages.json"
        path = args.root / "docs" / "updates" / directory / filename
        try:
            count, found = inspect_manifest(path, companions)
            total += count
            errors.extend(f"{directory}: {error}" for error in found)
            print(f"{directory}: {count} payload entries, {len(companions)} companion paths inventoried")
        except (OSError, ValueError, TypeError) as exc:
            errors.append(f"{directory}: {exc}")
    if errors:
        for error in errors:
            print("FAIL:", error)
        return 1
    print(f"PASS: {total} local payload sizes/SHA256 and named source/notice companions present")
    print("LIMIT: archive contents, binary/source reproducibility and redistribution rights are NOT approved by this check")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

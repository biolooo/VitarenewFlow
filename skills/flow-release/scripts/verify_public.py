#!/usr/bin/env python3
"""Compare public Flow release and channel bytes against the prepared local package."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import urllib.request


def verify(root, version):
    if not re.fullmatch(r"[0-9]+\.[0-9]+\.[0-9]+", version):
        raise ValueError("Invalid version")
    updates = Path(root) / "docs/updates"
    manifest = json.loads((updates / version / "SHA256.json").read_bytes())
    if not manifest or any(Path(name).name != name or "/" in name or "\\" in name for name in manifest):
        raise ValueError("Invalid manifest paths")
    latest = (updates / "latest.txt").read_text().splitlines()
    metadata = dict(line.split("=", 1) for line in latest[1:] if "=" in line)
    if latest[0] != "VFUPDATE1" or metadata.get("version") != version:
        raise ValueError("Channel does not target requested version")
    binary = (updates / version / "VitarenewFlow.3dsx").read_bytes()
    expected_url = "https://biolooo.github.io/VitarenewFlow/updates/" + version + "/VitarenewFlow.3dsx"
    if (metadata.get("url") != expected_url or metadata.get("size") != str(len(binary))
            or metadata.get("sha256") != hashlib.sha256(binary).hexdigest()):
        raise ValueError("Updater URL, size or hash mismatch")
    store = json.loads((updates / "VitarenewFlow.unistore").read_bytes())
    info = json.loads((updates / "VitarenewFlow-info.json").read_bytes())
    if (store["storeInfo"] != info or len(store["storeContent"]) != 1
            or store["storeContent"][0]["info"]["version"] != version):
        raise ValueError("UniStore/info channels disagree")
    for name, digest in manifest.items():
        if hashlib.sha256((updates / version / name).read_bytes()).hexdigest() != digest:
            raise ValueError("Local manifest mismatch: " + name)
    names = [version + "/" + name for name in manifest]
    names += [version + "/SHA256.json", "latest.txt", "VitarenewFlow.unistore", "VitarenewFlow-info.json", "install.html"]
    evidence = {"version": version, "checked_at": datetime.now(timezone.utc).isoformat(), "files": {}}
    for name in names:
        url = "https://biolooo.github.io/VitarenewFlow/updates/" + name
        request = urllib.request.Request(url, headers={"User-Agent": "Flow-release-verifier", "Cache-Control": "no-cache"})
        with urllib.request.urlopen(request, timeout=25) as response:
            if response.geturl() != url:
                raise ValueError("Unexpected redirect: " + name)
            expected = (updates / name).read_bytes()
            actual = response.read(len(expected) + 1)
        if actual != expected:
            raise ValueError("Public bytes differ: " + name)
        evidence["files"][name] = {"size": len(actual), "sha256": hashlib.sha256(actual).hexdigest()}
        print("PASS", name)
    return evidence


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--version", required=True)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    result = verify(args.root, args.version)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")

"""Isolated native verification in the pinned ephemeral Linux SDK container.

FLOW_SOURCE_VERSION optionally requests reproduction of an existing release ZIP.
This script neither packages releases nor updates channels, commits, or deploys.
"""
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import subprocess
import tempfile
import zipfile


def run(command, cwd, log):
    with subprocess.Popen(command, cwd=cwd, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, text=True) as process:
        for line in process.stdout:
            print(line, end="", flush=True)
            log.write(line)
            log.flush()
        if process.wait():
            raise RuntimeError("Verification command failed: " + command[0])


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    root = Path(__file__).resolve().parents[1]
    version = os.environ.get("FLOW_SOURCE_VERSION", "")
    if version and not re.fullmatch(r"[0-9]+\.[0-9]+\.[0-9]+", version):
        raise ValueError("Invalid source version")
    output = root / ".test-build/ci-verification" / ("source-" + version if version else "checkout")
    output.mkdir(parents=True, exist_ok=False)
    stage = Path(tempfile.mkdtemp(prefix="flow-ci-"))
    (output / "staging-path.txt").write_text(str(stage) + "\n")
    with (output / "verification.log").open("w") as log:
        if version:
            release = root / "docs/updates" / version
            archive_path = release / "VitarenewFlow-source.zip"
            manifest = json.loads((release / "SHA256.json").read_text())
            if digest(archive_path) != manifest[archive_path.name]:
                raise ValueError("Source ZIP does not match its release manifest")
            with zipfile.ZipFile(archive_path) as archive:
                for entry in archive.infolist():
                    path = PurePosixPath(entry.filename)
                    if (path.is_absolute() or ".." in path.parts or "\\" in entry.filename
                            or (entry.external_attr >> 16) & 0o170000 == 0o120000):
                        raise ValueError("Unsafe source ZIP entry")
                archive.extractall(stage)
            (output / "source-zip.sha256").write_text(digest(archive_path) + "\n")
        else:
            for name in ("launcher", "tests", "scripts", "release-notes"):
                shutil.copytree(root / name, stage / name,
                                ignore=shutil.ignore_patterns("build", "__pycache__", ".test-build"))
            run(["python3", "scripts/check-component-packages.py"], root, log)
            run(["python3", "-m", "unittest", "discover", "-s", "tests/backlog", "-p", "test_*.py"], root, log)
        sources = [stage / "launcher/Makefile"]
        for folder in ("source", "branding", "data"):
            sources += [p for p in (stage / "launcher" / folder).rglob("*") if p.is_file()]
        hashes = {p.relative_to(stage).as_posix(): digest(p) for p in sorted(sources)}
        (output / "source-hashes.json").write_text(json.dumps(hashes, indent=2) + "\n")
        shutil.copytree(stage / "tests/release-0.12.33", stage / "tests", dirs_exist_ok=True)
        shutil.copyfile(stage / "scripts/check-fixture-source.py", stage / "check-fixture-source.py")
        shutil.copyfile(stage / "scripts/verify-stage.sh", stage / "verify-stage.sh")
        # Match verify-stage's native environment exactly, but surface compiler
        # failures before unrelated host fixtures. Its later make remains a no-op.
        run(["env", "DEVKITPRO=/opt/devkitpro", "DEVKITARM=/opt/devkitpro/devkitARM",
             "PATH=/opt/devkitpro/tools/bin:/opt/devkitpro/devkitARM/bin:/usr/bin:/bin",
             "make", "-C", "launcher", "-j2"], stage, log)
        stack_output = output / "stack-usage"
        stack_output.mkdir()
        for report in (stage / "launcher/build").glob("*.su"):
            shutil.copyfile(report, stack_output / report.name)
        size = subprocess.check_output(["/opt/devkitpro/devkitARM/bin/arm-none-eabi-size", str(stage / "launcher/3ds-library.elf")], text=True)
        (output / "native-size.txt").write_text(size)
        run(["bash", "verify-stage.sh"], stage, log)
        artifacts = {}
        for name in ("3ds-library.3dsx", "3ds-library.smdh"):
            source = stage / "launcher" / name
            shutil.copyfile(source, output / name)
            artifacts[name] = digest(source)
            if version:
                delivered = release / name.replace("3ds-library", "VitarenewFlow")
                if digest(delivered) != artifacts[name]:
                    raise ValueError("Exact source ZIP failed to reproduce " + name)
        (output / "artifact-hashes.json").write_text(json.dumps(artifacts, indent=2) + "\n")
        (output / "verification-mode.json").write_text(json.dumps({
            "source_version": version or None,
            "sdk_image": "devkitpro/devkitarm@sha256:15b79ce75822c289538d8153da5fa7aafe5e6adc32ad8a575a197beca0f0761b",
            "hardware_acceptance": False,
        }, indent=2) + "\n")
        log.write("PASS: isolated CI verification" + (" and exact source ZIP reproduction" if version else "") + "\n")


if __name__ == "__main__":
    main()

"""Prepare immutable local 0.12.37 layout test assets and four channels; never deploy.

Run only after configuring the approved public HTTPS endpoint and completing
scripts/verify-local.sh: python scripts/prepare-report-release.py --verification PATH
All validation and archive assembly happen before any release/channel mutation.
"""
import argparse
import hashlib
import io
import json
import os
from pathlib import Path, PurePosixPath
import re
import subprocess
from datetime import datetime, timezone
from urllib.parse import urlsplit
import zipfile

VERSION = "0.12.37"
PREVIOUS = "0.12.36"
PUBLIC_BASE = "https://biolooo.github.io/VitarenewFlow/updates"
COMPANIONS = ("VitarenewFlow-Boot.3dsx", "VitarenewFlow-Shortcut.cia", "VitarenewFlow-Copy.gm9")
DOWNLOADS = ("VitarenewFlow.3dsx", "VitarenewFlow.smdh", *COMPANIONS)
CHANNELS = ("latest.txt", "VitarenewFlow.unistore", "VitarenewFlow-info.json", "install.html")
FONT_INPUTS = ("launcher/tools/make-font.py", "launcher/tools/fonts/InterVariable.ttf", "launcher/tools/fonts/Inter-LICENSE.txt")

NOTES = '''# VitarenewFlow 0.12.37 — Unterstützungslayout als Testversion

- „Über & Unterstützung“ gliedert Titel, Version, Kurztext und Website übersichtlich.
- „Problem melden“ besitzt eine eigene klar abgegrenzte Schaltfläche.
- Der Rückweg bleibt als separate Schaltfläche sichtbar; X/Touch öffnet die Meldung.

Die vorhandene Meldefunktion und ihre begrenzte technische Diagnose bleiben erhalten:
- Technische Angaben prüfen und optional selbst beschreiben, was zuvor geschah.
- Erst die ausdrückliche Zustimmung und „Senden“ übertragen einen Bericht per HTTPS.
- Übertragen werden nur Appversion, Fehlerbereich/-code, Modell, Region, Autoboot,
  Startkontext, die begrenzte Diagnosemomentaufnahme und die freiwillige Beschreibung.
- Keine automatischen Rohlogs, Spieltitel/-pfade, Seriennummern, ROMs, Saves oder
  Schlüssel. Bitte auch in den freiwilligen Text keine privaten Daten eingeben.
- Kein automatisches Nachsenden. Ein Sendeversuch kann abgebrochen werden.
- Empfangsdienst: geprüfte JSON-Felder, begrenzte Anfragen, SQLite-Speicherung;
  reguläre Aufbewahrung 30 Tage, keine dauerhaften IP-/Header-/Anfrageprotokolle.

## Auf echter Konsole prüfen

Nach Veröffentlichung: Menü → Konsole & Start → Updates → Aktualisieren → Neu öffnen.
Die installierte Version muss 0.12.37 anzeigen. Kein Konsolenpass ist vorweggenommen.
Pro Fall Modell, Version, Startkontext, Region und Autoboot an/aus festhalten.

1. Deutsch/Englisch: „Über & Unterstützung“ öffnen. Titel, Version, Kurztext und
   Website müssen getrennt lesbar bleiben. „Problem melden“ und „A / B Zurück“
   besitzen getrennte Flächen. X/Touch öffnet die Meldung; A/B führt zurück.
   Diagnosewerte und eingefrorene Aktivitätsspur unverändert prüfen.
   Auf gesunder Konsole keine NAND-Beschädigung oder TWL-Reparatur provozieren.
2. Vor dem Senden technische Vorschau prüfen, optionalen Text eingeben/ändern.
   Zurück vor der Zustimmung sendet nichts. Ein Abbruch kann bereits gespeicherte Berichte nicht zurücknehmen. Ohne Zustimmung ist kein Versand möglich.
3. Mit WLAN genau einen eigenen Testbericht senden, Erfolgsanzeige und tatsächlichen
   Empfang im Betreiberzugriff abgleichen. Erfolg erst nach gespeicherter Annahme.
4. WLAN aus, falsches Datum/HTTPS, Serverfehler und begrenzte Anfragen prüfen:
   verständlicher Fehler, kein falscher Erfolg und kein automatischer Wiederholversand.
5. Während des Sendens abbrechen und erneut öffnen; Bedienung bleibt reaktionsfähig.
   Nach einem Netzabbruch kann der Server den Bericht bereits gespeichert haben.
6. HOME/POWER, Suspend/Resume, Spielstart und bestehende Reparatur-/Updateaktionen
   mit Autoboot an/aus prüfen. Ein Bericht darf keine gefährliche Aktion bestätigen.
7. Eingabegrenze und Umlaute prüfen. Keine echten privaten Daten für Tests verwenden.

Die .36-Verbesserungen bleiben erhalten. Originale HOME-Standardmusik, N64 und
Save-Sync werden mit dieser Version nicht neu implementiert. Keine Stable-1.0-Freigabe.
Das 3DSX-Update verändert keine Spiele, Saves oder Bootkonfiguration. Nicht während
der Installation ausschalten oder SD entfernen; vorherige 3DSX als manuellen Rückweg
unter /3ds/3ds-library/3ds-library.3dsx.previous.3dsx erhalten.

## Quellen und Begleiter

Nativer Build und Hostprüfungen müssen vor dem Packen bestanden sein. Dazu gehören
Berichtformat/UTF-8/Puffergrenzen, Zustimmung/Touch, HTTPS-Transport und bestehende
Komponenten-/TWL-/HOME-/POWER-/Suche-/Banner-/Menü-Fixtures. Hostmocks ersetzen keine
echte OS-/WLAN-/Konsolenabnahme. Receiver-Unittests separat ausführen.

Boot-3DSX, Shortcut-CIA und GM9-Skript stammen unverändert aus .36. Es gibt keine
neue Boot-/NAND-Funktion. Das Quellpaket enthält aktuelle Launcherquellen, Scripts,
Tests, Lizenzen, Projektdokumentation und den getrennten Empfangsdienst samt Compose.
Empfangsdienst nicht ungeprüft öffentlich starten. SDK/Emulator-SDKs separat;
RELEASE-BUILD.md und docs/PC2-SETUP.md beschreiben den Quellpaket-Build.
GPL-3.0-only für eigenen Code; Fremdkomponenten behalten ihre Bedingungen.
Keine Spiele, Saves, Schlüssel, Datenbanken, private Musik oder lokalen Nutzerdaten.
'''

BUILD_NOTES = '''# Build this exact 0.12.37 source package

Use the pinned devkitARM linux/amd64 chroot in docs/PC2-SETUP.md with host GCC,
UBSan, Python and zlib1g-dev. Extract into a NEW directory inside that chroot,
never over an existing SDK project. Enter that directory inside the chroot.
The native receiver endpoint is already included in launcher/source/report_transport.h;
changing it changes the application and requires a new build/verification.

Flatten only the archived historical release fixtures for the current stage runner:

```sh
cp -a tests/release-0.12.33/. tests/
cp scripts/check-fixture-source.py check-fixture-source.py
cp scripts/verify-stage.sh verify-stage.sh
bash verify-stage.sh
```

Keep tests/error-report, tests/report-transport and tests/backlog in place: they are
current root-relative tests, not additional fixtures to flatten. The stage script
sets DEVKITPRO/DEVKITARM/PATH and builds launcher/3ds-library.3dsx and .smdh.
Compare both artifacts byte-for-byte with the delivered files before accepting
reproducibility. No real console, WLAN, HOME/POWER OS transfer or NAND is tested.

The full repository's verify-local.sh also inventories separate historical emulator
distributions under docs/updates, intentionally omitted from this source-only ZIP.
Use the stage runner above for this package rather than claiming that inventory
passed with missing distributions.

Receiver tests use temporary SQLite databases and loopback sockets only:
`python3 -m unittest discover -s receiver/error-reports -p test_server.py -v`.
Receiver deployment, TLS/proxy configuration and a real stored HTTPS report need
separate operator validation. Never expose a database/admin endpoint or copy an
existing deployment's data directory into this package.

Font generator and original licensed input are retained unchanged from the .33
source archive if absent from the checkout; the supplied font header is used.
No SDK, private media, report database, ROM, save, key or local account is bundled.
'''


def digest(data):
    return hashlib.sha256(data).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def read_regular(path):
    require(path.is_file() and not path.is_symlink(), "Expected regular nonsymlink file: " + str(path))
    return path.read_bytes()


def verify_sources(root, directory):
    hashes = json.loads(read_regular(directory / "source-hashes.json"))
    require(type(hashes) is dict and hashes, "Empty or invalid source hash inventory")
    expected = {"launcher/Makefile"}
    for folder in ("source", "branding", "data"):
        expected.update(path.relative_to(root).as_posix() for path in (root / "launcher" / folder).rglob("*") if path.is_file())
    require(set(hashes) == expected, "Verification source inventory is incomplete or unexpected")
    for name, sha in hashes.items():
        path = PurePosixPath(name)
        require(not path.is_absolute() and ".." not in path.parts and "\\" not in name, "Unsafe verification path")
        require(type(sha) is str and re.fullmatch(r"[0-9a-f]{64}", sha), "Invalid source hash")
        require(digest(read_regular(root / name)) == sha, "Source changed after verification: " + name)
    return hashes


def check_versions(root):
    checks = {
        "launcher/Makefile": r"(?m)^APP_DESCRIPTION\s*:=\s*Game Launcher - Version " + re.escape(VERSION) + r"\s*$",
        "launcher/source/main.c": r'\.version\s*=\s*"' + re.escape(VERSION) + r'"',
        "launcher/source/app_update.c": r'(?m)^#define RUNNING_VERSION "' + re.escape(VERSION) + r'"\s*$',
        "launcher/source/components.c": r'(?m)^#define RUNNING_VERSION "' + re.escape(VERSION) + r'"\s*$',
        "launcher/source/app_log.c": r'VitarenewFlow ' + re.escape(VERSION) + r'\\n',
    }
    for name, pattern in checks.items():
        text = read_regular(root / name).decode("utf-8")
        require(re.search(pattern, text), "Runtime version mismatch: " + name)
        require(not re.search(r'(?:RUNNING_VERSION|\.version)\s*=?\s*"0\.12\.(?:33|34|35|36)"', text), "Stale runtime version: " + name)
    log = read_regular(root / "launcher/source/app_log.c").decode("utf-8")
    require("Session VitarenewFlow " + VERSION in log, "Handoff log version mismatch")
    header = read_regular(root / "launcher/source/report_transport.h").decode("utf-8")
    matches = re.findall(r'(?m)^#define FLOW_REPORT_ENDPOINT "([^"\r\n]*)"\s*$', header)
    require(len(matches) == 1 and matches[0], "Configure the reviewed HTTPS endpoint before building")
    endpoint = matches[0]
    url = urlsplit(endpoint)
    require(url.scheme == "https" and url.hostname and not url.username and not url.password
            and not url.query and not url.fragment and url.path in ("/v1/reports", "/flow-reports/v1/reports")
            and url.port in (None, 443, 8443, 10000) and len(endpoint) < 256
            and not any(c.isspace() or c == "\\" for c in endpoint), "Invalid configured HTTPS endpoint")
    require(not url.hostname.endswith(".invalid") and url.hostname not in ("localhost", "127.0.0.1"), "Test endpoint cannot be released")


def include_source(name):
    path = PurePosixPath(name)
    if path.is_absolute() or ".." in path.parts or "\\" in name:
        return False
    excluded = {".git", ".test-build", "__pycache__", "build", "SD-Karte", "cache", "private", "node_modules", "local-development-snapshot"}
    if any(part in excluded or part.startswith(".env") for part in path.parts):
        return False
    if path.suffix.lower() in (".db", ".sqlite", ".sqlite3", ".log", ".sav", ".keys", ".pem", ".key", ".pfx", ".mp3", ".bcstm", ".o", ".elf", ".map", ".3dsx", ".smdh", ".cia", ".zip", ".pyc"):
        return False
    if name in ("LICENSE", "README.md", "AGENTS.md", "PROJECT-COPY.md", "PLANS.md", ".gitattributes", ".gitignore"):
        return True
    if name.startswith("receiver/error-reports/"):
        return len(path.parts) == 3 and path.name in ("server.py", "test_server.py", "Dockerfile", "compose.yaml", "start-local.sh")
    if name.startswith("licenses/"):
        return True
    if name.startswith("docs/"):
        return not name.startswith(("docs/updates/", "docs/assets/", "docs/migration-evidence/")) and path.suffix in (".md", ".txt")
    if name.startswith("launcher/branding/"):
        return path.name in ("application.rsf", "banner.png", "silence.wav", "title.png", "icon.png", "banner.bnr") and len(path.parts) == 3
    if name.startswith("launcher/data/"):
        return path.name in ("README.md", "update_ca.bin", "fallback_bgm.bin") and len(path.parts) == 3
    if name in ("launcher/forwarder/icon.png", "launcher/gm9/scripts/VitarenewFlow-Copy.gm9"):
        return True
    if name in FONT_INPUTS or name.endswith("/Makefile"):
        return name.startswith("launcher/")
    return name.startswith(("launcher/", "scripts/", "tests/")) and path.suffix in (".c", ".h", ".py", ".sh", ".ps1", ".md", ".txt", ".json")


def source_archive(root, updates):
    names = subprocess.check_output(["git", "ls-files", "-z", "--cached", "--others", "--exclude-standard"], cwd=root).decode("utf-8").split("\0")
    selected = {name: read_regular(root / name) for name in sorted(set(names)) if name and include_source(name)}
    required = {"launcher/Makefile", "launcher/source/report_transport.h", "launcher/source/error_report.c", "launcher/source/report_transport.c",
                "scripts/verify-stage.sh", "tests/error-report/test.sh", "tests/report-transport/test.sh",
                "receiver/error-reports/server.py", "receiver/error-reports/test_server.py",
                "receiver/error-reports/Dockerfile", "receiver/error-reports/compose.yaml", "receiver/error-reports/start-local.sh"}
    require(required <= set(selected), "Source package is missing required reporting/build/receiver sources")
    with zipfile.ZipFile(updates / "0.12.33/VitarenewFlow-source.zip") as previous:
        for name in FONT_INPUTS:
            if name not in selected:
                selected[name] = previous.read(name)
    selected["RELEASE-BUILD.md"] = BUILD_NOTES.encode("utf-8")
    output = io.BytesIO()
    with zipfile.ZipFile(output, "w", zipfile.ZIP_DEFLATED) as archive:
        for name, data in sorted(selected.items()):
            info = zipfile.ZipInfo(name, date_time=(2026, 10, 21, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = 0o100644 << 16
            archive.writestr(info, data)
    return output.getvalue()


def prepare(root, verification):
    updates = root / "docs/updates"
    release = updates / VERSION
    require(not release.exists(), "Release directory exists; immutable artifacts must not be overwritten")
    log = read_regular(verification / "verification.log").decode("utf-8")
    for marker in ("PASS: native build and existing component", "PASS: independent Python JSON decode and 200 exact escaping roundtrips",
                   "PASS: actual HTTPS transport consent", "PASS: actual modal consent/review/edit/back priorities"):
        require(marker in log, "Missing required verification: " + marker)
    hashes = verify_sources(root, verification)
    check_versions(root)
    binary = read_regular(verification / "3ds-library.3dsx")
    smdh = read_regular(verification / "3ds-library.smdh")
    require(binary[:4] == b"3DSX" and 32 <= len(binary) <= 8 * 1024 * 1024, "Invalid 3DSX")
    require(len(smdh) == 14016 and smdh[:4] == b"SMDH" and VERSION.encode("utf-16le") in smdh, "Invalid SMDH/version")
    before = {name: read_regular(updates / name) for name in CHANNELS}
    latest = dict(line.split("=", 1) for line in before["latest.txt"].decode("ascii").splitlines()[1:] if "=" in line)
    require(before["latest.txt"].startswith(b"VFUPDATE1") and latest.get("version") == PREVIOUS, "Expected .36 updater channel")
    store = json.loads(before["VitarenewFlow.unistore"].decode("utf-8-sig"))
    require(json.loads(before["VitarenewFlow-info.json"].decode("utf-8-sig")) == store["storeInfo"], "Info/UniStore channels disagree")
    require(len(store["storeContent"]) == 1, "Unexpected store entries")
    entry = store["storeContent"][0]
    require(entry["info"]["version"] == PREVIOUS, "Expected .36 store entry")
    revision = store["storeInfo"]["revision"]
    require(type(revision) is int and revision >= 0, "Invalid UniStore revision")
    old_release = updates / PREVIOUS
    old_manifest = json.loads(read_regular(old_release / "SHA256.json"))
    expected_artifacts = set(DOWNLOADS) | {"FEATURES-DE.md", "README.md", "VitarenewFlow-source.zip"}
    require(set(old_manifest) == expected_artifacts, "Invalid .36 manifest")
    for name, sha in old_manifest.items():
        require(digest(read_regular(old_release / name)) == sha, "Existing .36 artifact hash mismatch: " + name)
    assets = {"VitarenewFlow.3dsx": binary, "VitarenewFlow.smdh": smdh,
              "README.md": NOTES.encode("utf-8"), "FEATURES-DE.md": NOTES.encode("utf-8")}
    for name in COMPANIONS:
        assets[name] = read_regular(old_release / name)
    assets["VitarenewFlow-source.zip"] = source_archive(root, updates)
    manifest = {name: digest(data) for name, data in sorted(assets.items())}
    assets["SHA256.json"] = (json.dumps(manifest, indent=2) + "\n").encode("ascii")
    store["storeInfo"]["revision"] = revision + 1
    entry["info"]["version"] = VERSION
    entry["info"]["description"] = "Clear About and support layout with a separate report button and back action (DE/EN)."
    entry["info"]["last_updated"] = datetime.now(timezone.utc).isoformat(timespec="seconds").replace("+00:00", "Z")
    install = entry["Installieren / Install"]
    downloads = [action for action in install["script"] if action.get("type") == "downloadFile"]
    require(len(downloads) == 5, "Expected exactly five installation downloads")
    seen = set()
    for action in downloads:
        name = action["file"].rsplit("/", 1)[-1]
        require(name in DOWNLOADS and name not in seen and action["file"] == PUBLIC_BASE + "/" + PREVIOUS + "/" + name,
                "Unexpected/stale installation download URL")
        require(action.get("output") == "/3ds/3ds-library/updates/universal-updater/" + name, "Unexpected installation output path")
        action["file"] = PUBLIC_BASE + "/" + VERSION + "/" + name
        seen.add(name)
    require(seen == set(DOWNLOADS), "Incomplete installation downloads")
    install["size"] = str(sum(len(assets[name]) for name in DOWNLOADS)) + " B"
    html = before["install.html"]
    require(PREVIOUS.encode() in html, "Installation page does not reference .36")
    channels = {
        "latest.txt": ("VFUPDATE1\nversion=" + VERSION + "\nsize=" + str(len(binary)) + "\nsha256=" + digest(binary)
                       + "\nurl=" + PUBLIC_BASE + "/" + VERSION + "/VitarenewFlow.3dsx\n").encode("ascii"),
        "VitarenewFlow.unistore": (json.dumps(store, ensure_ascii=False, indent=2) + "\n").encode("utf-8"),
        "VitarenewFlow-info.json": (json.dumps(store["storeInfo"], ensure_ascii=False, indent=2) + "\n").encode("utf-8"),
        "install.html": html.replace(PREVIOUS.encode(), VERSION.encode()),
    }
    # Refuse a concurrent edit between validation/ZIP assembly and mutation.
    require(verify_sources(root, verification) == hashes, "Source inventory changed while packaging")
    for name, data in before.items():
        require(read_regular(updates / name) == data, "Channel changed while packaging: " + name)
    release.mkdir()  # Atomic refusal if another process created this immutable version.
    for name, data in assets.items():
        with (release / name).open("xb") as output:
            output.write(data)
    # Stage replacement bytes inside the new release, then replace only four
    # selected channels atomically per file. An interrupted run needs inspection;
    # it is deliberately never repaired by overwriting an immutable directory.
    for name, data in channels.items():
        staged = release / (".channel-" + name + ".tmp")
        with staged.open("xb") as output:
            output.write(data)
        os.replace(staged, updates / name)
    print("PREPARED locally:", release)
    print("Nine assets and four .37 channels; no push, publication or hardware acceptance performed.")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--verification", required=True, type=Path)
    args = parser.parse_args()
    try:
        prepare(Path(__file__).resolve().parents[1], args.verification.resolve())
    except (ValueError, KeyError, TypeError, OSError, zipfile.BadZipFile) as error:
        parser.error(str(error))


if __name__ == "__main__":
    main()

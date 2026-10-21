import importlib.util
import json
import re
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("release_notes_generator", ROOT / "scripts/generate-release-notes.py")
VERSION = re.search(r"Game Launcher - Version ([0-9.]+)", (ROOT / "launcher/Makefile").read_text())[1]
notes = importlib.util.module_from_spec(spec)
spec.loader.exec_module(notes)


class ReleaseNotesTests(unittest.TestCase):
    def test_current_source_and_generated_header_match(self):
        value = notes.generate(ROOT, VERSION, check=True)
        text = notes.sidecar(value).decode("utf-8")
        self.assertIn("version=" + VERSION + "\n", text)
        for row in value["highlights"]:
            self.assertIn("de=" + row["de"] + "\n", text)
            self.assertIn("en=" + row["en"] + "\n", text)
        self.assertLessEqual(len(text.encode()), 1024)

    def test_missing_mismatched_incomplete_and_unsafe_notes_fail(self):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "notes.json"
            cases = [
                {"version": "0.12.39", "highlights": [{"de": "A", "en": "B"}]},
                {"version": "0.12.38", "highlights": []},
                {"version": "0.12.38", "highlights": [{"de": "A"}]},
            ]
            for value in cases:
                path.write_text(json.dumps(value))
                with self.assertRaises(ValueError): notes.load_notes(path, "0.12.38")
            for text in (" ", "X" * 121, "Hallo\nWelt", "X\u202eY", "X\u200bY", "X\ufeffY"):
                path.write_text(json.dumps({"version": "0.12.38", "highlights": [{"de": text, "en": "OK"}]}))
                with self.assertRaises(ValueError): notes.load_notes(path, "0.12.38")

    def test_one_and_three_entries_and_stale_header(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / "release-notes").mkdir()
            (root / "launcher/source").mkdir(parents=True)
            path = root / "release-notes/0.12.38.json"
            for count in (1, 3):
                path.write_text(json.dumps({"version": "0.12.38", "highlights": [{"de": "Neu", "en": "New"}] * count}))
                notes.generate(root, "0.12.38")
                notes.generate(root, "0.12.38", check=True)
            path.write_text(json.dumps({"version": "0.12.38", "highlights": [{"de": "Anders", "en": "Different"}]}))
            with self.assertRaises(ValueError): notes.generate(root, "0.12.38", check=True)

    def test_package_document_uses_the_same_localized_source(self):
        spec = importlib.util.spec_from_file_location("flow_pack", ROOT / "scripts/prepare-report-release.py")
        pack = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(pack)
        value = notes.load_notes(ROOT / "release-notes" / (VERSION + ".json"), VERSION)
        document = pack.release_document(value)
        for row in value["highlights"]:
            self.assertIn("- " + row["de"], document)
            self.assertIn("- " + row["en"], document)
        self.assertTrue(pack.include_source("release-notes/" + VERSION + ".json"))
        self.assertFalse(pack.include_source("release-notes/private.sqlite3"))

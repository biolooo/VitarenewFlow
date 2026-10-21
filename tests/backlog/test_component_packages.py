"""Error-path tests for the local integrity checker; no network or real payload mutation."""
import copy
import hashlib
import importlib.util
import json
import tempfile
import unittest
from pathlib import Path

spec = importlib.util.spec_from_file_location(
    "component_packages", Path(__file__).resolve().parents[2] / "scripts/check-component-packages.py")
checker = importlib.util.module_from_spec(spec)
spec.loader.exec_module(checker)


class IntegrityTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="flow-package-check-")
        self.addCleanup(self.temp.cleanup)
        self.folder = Path(self.temp.name) / "components-1"
        self.folder.mkdir()
        self.manifest = self.folder / "packages.json"
        self.payload = b"synthetic payload; no executable"
        (self.folder / "example.bin").write_bytes(self.payload)
        self.entry = dict(name="example.bin", target="sdmc:/3ds/example.bin",
                          bytes=len(self.payload), sha256=hashlib.sha256(self.payload).hexdigest(),
                          url="https://biolooo.github.io/VitarenewFlow/updates/components-1/example.bin")

    def check(self, entries=None, companions=()):
        self.manifest.write_text(json.dumps([{"files": entries or [self.entry]}]), encoding="utf-8")
        return checker.inspect_manifest(self.manifest, companions)[1]

    def test_valid_bytes(self):
        self.assertEqual(self.check(), [])

    def test_empty_manifests_fail_closed(self):
        for payload in ([], [{"files": []}], {"files": []}):
            with self.subTest(payload=payload):
                self.manifest.write_text(json.dumps(payload), encoding="utf-8")
                count, errors = checker.inspect_manifest(self.manifest)
                self.assertEqual(count, 0)
                self.assertTrue(errors)

    def test_bad_size_hash_and_url(self):
        for field, value, message in (
                ("bytes", 1, "size mismatch"), ("sha256", "0" * 64, "SHA256 mismatch"),
                ("url", "http://example.com/example.bin", "unexpected download URL")):
            with self.subTest(field=field):
                entry = copy.deepcopy(self.entry)
                entry[field] = value
                self.assertTrue(any(message in error for error in self.check([entry])))

    def test_missing_payload(self):
        (self.folder / "example.bin").unlink()
        self.assertTrue(any("missing/nonregular" in error for error in self.check()))

    def test_duplicates_and_traversal(self):
        errors = self.check([self.entry, copy.deepcopy(self.entry)])
        self.assertTrue(any("duplicate filename" in error for error in errors))
        self.assertTrue(any("duplicate SD target" in error for error in errors))
        entry = copy.deepcopy(self.entry)
        entry["name"] = "../outside.bin"
        self.assertIn("invalid local basename", self.check([entry]))
        entry = copy.deepcopy(self.entry)
        entry["target"] = "sdmc:/../outside.bin"
        self.assertTrue(any("invalid SD target" in error for error in self.check([entry])))

    def test_companion_presence_is_separate(self):
        self.assertTrue(any("companion" in error for error in self.check(companions=("source.zip",))))
        (self.folder / "source.zip").write_bytes(b"not a license approval")
        self.assertEqual(self.check(companions=("source.zip",)), [])


if __name__ == "__main__":
    unittest.main()

# SPDX-License-Identifier: GPL-3.0-only
import json
from pathlib import Path
import sqlite3
import tempfile
import unittest
import triage
from test_server import sample, sample_v2


class TriageTests(unittest.TestCase):
    def test_grouping_privacy_and_missing_diagnostics(self):
        one = sample()
        two = {**sample_v2(), "description": "PRIVATE USER TEXT"}
        groups = triage.summarize([(10, json.dumps(one)), (20, json.dumps(two))])
        self.assertEqual(len(groups), 1)
        self.assertEqual(groups[0]["count"], 2)
        self.assertEqual(groups[0]["with_diagnostics"], 1)
        self.assertEqual(groups[0]["classification"], "error")
        self.assertNotIn("PRIVATE", json.dumps(groups))
        self.assertEqual(groups[0]["first_seen"], 10)

    def test_zero_code_is_not_proof_of_no_error(self):
        report = {**sample(), "code": 0}
        group = triage.summarize([(10, json.dumps(report))])[0]
        self.assertEqual(group["classification"], "needs-triage")

    def test_read_only_retention_and_no_creation(self):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "reports.sqlite3"
            with self.assertRaises(sqlite3.OperationalError):
                triage.read_database(path)
            self.assertFalse(path.exists())
            with sqlite3.connect(path) as db:
                db.execute("CREATE TABLE reports(created_at INTEGER,payload TEXT)")
                db.executemany("INSERT INTO reports VALUES (?,?)", [(0,json.dumps(sample())),(3000000,json.dumps(sample()))])
            before = path.read_bytes()
            self.assertEqual(triage.read_database(path, now=3000001)["total"], 1)
            self.assertEqual(path.read_bytes(), before)

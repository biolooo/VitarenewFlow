# SPDX-License-Identifier: GPL-3.0-only
from contextlib import closing, redirect_stderr
import http.client
import io
import json
from pathlib import Path
import sqlite3
import tempfile
import threading
import unittest
from unittest.mock import patch

import server


def sample():
    return {"schema": 1, "version": "0.12.34", "area": "launch", "code": -20,
            "model": "new-3ds-xl", "region": "eur", "autoboot": "on",
            "context": "homebrew", "activity": "starting-game",
            "description": "Nach dem Start zurück. Danach erneut versucht."}


class ReceiverTests(unittest.TestCase):
    def setUp(self):
        self.scratch = tempfile.TemporaryDirectory(prefix="flow-receiver-test-")
        self.addCleanup(self.scratch.cleanup)
        self.path = Path(self.scratch.name) / "reports.sqlite3"
        self.now = 2_000_000_000
        self.store = server.ReportStore(self.path, clock=lambda: self.now)
        self.httpd = server.ReportServer(("127.0.0.1", 0), self.store,
                                         server.RateLimiter(limit=1000))
        self.thread = threading.Thread(target=self.httpd.serve_forever, kwargs={"poll_interval": 0.01}, daemon=True)
        self.thread.start()
        self.addCleanup(self.stop_server)

    def stop_server(self):
        self.httpd.shutdown()
        self.httpd.server_close()
        self.thread.join(timeout=2)

    def request(self, body=None, path="/v1/reports", content_type="application/json", extra=None, method="POST"):
        if body is None:
            body = json.dumps(sample(), ensure_ascii=False).encode("utf-8")
        elif type(body) is dict:
            body = json.dumps(body, ensure_ascii=False).encode("utf-8")
        connection = http.client.HTTPConnection(*self.httpd.server_address, timeout=3)
        headers = {"Content-Type": content_type}
        headers.update(extra or {})
        try:
            connection.request(method, path, body, headers)
            response = connection.getresponse()
            return response.status, json.loads(response.read())
        finally:
            connection.close()

    def stored(self):
        with closing(sqlite3.connect(self.path)) as connection:
            return connection.execute("SELECT created_at, payload FROM reports ORDER BY rowid").fetchall()

    def test_health_is_minimal_and_does_not_create_report(self):
        self.assertEqual(self.request(body=b"",path="/healthz",method="GET"),(200,{"ok":True}))
        self.assertFalse(self.stored())

    def test_valid_report_is_committed_before_201(self):
        status, body = self.request()
        self.assertEqual((status, body), (201, {"accepted": True}))
        rows = self.stored()
        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0][0], self.now)
        self.assertEqual(json.loads(rows[0][1]), sample())
        with closing(sqlite3.connect(self.path)) as connection:
            columns = [row[1] for row in connection.execute("PRAGMA table_info(reports)")]
        self.assertEqual(columns, ["created_at", "payload"])

    def test_malformed_json_utf8_empty_duplicate_and_constants_fail(self):
        valid = json.dumps(sample()).encode()
        cases = [b"", b"{", b"\xff", b"[]", b"null", b"true", b'{"schema":1,"schema":1}',
                 valid.replace(b'"code": -20', b'"code": NaN'),
                 valid.replace(b'"description":', b'"description":', 1) + b"junk"]
        for body in cases:
            with self.subTest(body=body[:30]):
                self.assertEqual(self.request(body)[0], 400)
        self.assertFalse(self.stored())

    def test_unknown_sensitive_fields_are_rejected_not_stored(self):
        for key in ["path", "logs", "serial", "title_id", "rom", "save", "headers", "ip", "token"]:
            report = sample()
            report[key] = "must not be persisted"
            with self.subTest(key=key):
                self.assertEqual(self.request(report)[0], 400)
        self.assertFalse(self.stored())

    def test_invalid_types_and_category_values_fail(self):
        changes = [("schema", True), ("code", False), ("code", 0.5), ("code", 2**31),
                   ("code", -(2**31)-1), ("version", "1.2"), ("version", "1.2.3\n"),
                   ("version", "١.٢.٣"), ("description", 5), ("description", "a\0b")]
        changes += [(name, "sdmc:/private") for name in server.CATEGORIES]
        for name, value in changes:
            report = sample()
            report[name] = value
            with self.subTest(name=name, value=value):
                self.assertEqual(self.request(report)[0], 400)
        for key in sample():
            report = sample()
            del report[key]
            self.assertEqual(self.request(report)[0], 400)
        self.assertFalse(self.stored())

    def test_description_utf8_byte_limit_and_surrogates(self):
        report = sample()
        report["description"] = "ä" * 256
        self.assertEqual(self.request(report)[0], 201)
        report["description"] += "a"
        self.assertEqual(self.request(report)[0], 400)
        body = json.dumps({**sample(), "description": "\ud800"}).encode()
        self.assertEqual(self.request(body)[0], 400)

    def test_oversize_and_wrong_content_types(self):
        self.assertEqual(self.request(b" " * (server.MAX_BODY + 1))[0], 413)
        for content_type in ["text/plain", "application/json; charset=latin-1", "application/json; boundary=x"]:
            self.assertEqual(self.request(content_type=content_type)[0], 415)
        self.assertEqual(self.request(content_type='application/json; charset="utf-8"')[0], 201)

    def test_content_length_and_encoded_requests_rejected(self):
        self.assertEqual(self.request(extra={"Content-Length": "-1"})[0], 411)
        self.assertEqual(self.request(extra={"Content-Length": "99999999999"})[0], 411)
        self.assertEqual(self.request(extra={"Content-Encoding": "gzip"})[0], 400)
        self.assertEqual(self.request(extra={"Transfer-Encoding": "chunked"})[0], 400)
        connection = http.client.HTTPConnection(*self.httpd.server_address, timeout=3)
        try:
            connection.putrequest("POST", "/v1/reports")
            connection.putheader("Content-Type", "application/json")
            connection.putheader("Content-Length", "1")
            connection.putheader("Content-Length", "2")
            connection.endheaders()
            response = connection.getresponse()
            self.assertEqual(response.status, 411)
            response.read()
        finally:
            connection.close()
        self.assertFalse(self.stored())

    def test_routes_do_not_expose_reports(self):
        self.assertEqual(self.request(path="/v1/reports?dump=1")[0], 404)
        self.assertEqual(self.request(method="GET")[0], 404)
        self.assertEqual(self.request(path="/admin")[0], 404)

    def test_sql_injection_is_stored_as_explicit_description_only(self):
        report = sample()
        report["description"] = "'); DROP TABLE reports; -- \" \\ \n Grüße 🎮"
        self.assertEqual(self.request(report)[0], 201)
        self.assertEqual(json.loads(self.stored()[0][1]), report)
        self.assertEqual(self.request()[0], 201)
        self.assertEqual(len(self.stored()), 2)

    def test_storage_failure_returns_503_without_success(self):
        with patch.object(self.store, "save", side_effect=sqlite3.OperationalError("private DB detail")):
            self.assertEqual(self.request(), (503, {"error": "storage_unavailable"}))
        self.assertFalse(self.stored())

    def test_failed_transaction_rolls_back_cleanup_and_insert(self):
        self.store.save(sample())
        self.now += server.RETENTION_SECONDS + 1
        with closing(sqlite3.connect(self.path)) as connection:
            connection.execute("CREATE TRIGGER fail BEFORE INSERT ON reports BEGIN SELECT RAISE(FAIL,'synthetic failure'); END")
            connection.commit()
        self.assertEqual(self.request()[0], 503)
        self.assertEqual(len(self.stored()), 1)  # DELETE rolled back with failed INSERT.

    def test_retention_on_insert_and_idle_cleanup(self):
        self.store.save(sample())
        self.now += server.RETENTION_SECONDS + 1
        self.store.save(sample())
        rows = self.stored()
        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0][0], self.now)
        self.now += server.RETENTION_SECONDS + 1
        self.httpd._last_cleanup = -1000
        self.httpd.service_actions()
        self.assertFalse(self.stored())

    def test_rate_limit_uses_peer_not_forwarded_header(self):
        self.httpd.limiter = server.RateLimiter(limit=1)
        self.assertEqual(self.request(extra={"X-Forwarded-For": "192.0.2.1"})[0], 201)
        self.assertEqual(self.request(extra={"X-Forwarded-For": "192.0.2.2"})[0], 429)
        self.assertEqual(list(self.httpd.limiter._peers), ["127.0.0.1"])
        self.assertNotIn("192.0.2", self.path.read_bytes().decode("latin-1"))

    def test_no_request_or_error_log_contains_user_data(self):
        output = io.StringIO()
        with redirect_stderr(output):
            self.assertEqual(self.request()[0], 201)
            self.assertEqual(self.request(b"invalid")[0], 400)
        self.assertEqual(output.getvalue(), "")

    def test_loopback_binding_only(self):
        with self.assertRaises(ValueError):
            server.ReportServer(("0.0.0.0", 0), self.store)

    def test_explicit_prefix_path_only(self):
        self.httpd.report_path = "/flow-reports/v1/reports"
        self.assertEqual(self.request()[0], 404)
        self.assertEqual(self.request(path="/flow-reports/v1/reports")[0], 201)
        with self.assertRaises(ValueError):
            server.ReportServer(("127.0.0.1", 0), self.store, report_path="/anything")


class LimiterTests(unittest.TestCase):
    def test_window_expiry_and_peer_memory_bound(self):
        now = [100.0]
        limiter = server.RateLimiter(limit=2, window=60, maximum_peers=2, clock=lambda: now[0])
        self.assertTrue(limiter.allow("one"))
        self.assertTrue(limiter.allow("one"))
        self.assertFalse(limiter.allow("one"))
        self.assertTrue(limiter.allow("two"))
        self.assertFalse(limiter.allow("three"))
        self.assertEqual(len(limiter._peers), 2)
        now[0] += 60
        self.assertTrue(limiter.allow("three"))
        self.assertEqual(list(limiter._peers), ["three"])


if __name__ == "__main__":
    unittest.main()

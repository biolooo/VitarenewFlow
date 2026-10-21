#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Local receiver; put a separately configured TLS proxy in front for deployment.

Run: python3 server.py --database /private/path/reports.sqlite3 --port 4395
Loopback is the default; container binding requires explicit --host 0.0.0.0.
Reports are read locally through SQLite; no
listing/admin endpoint exists. Transport IPs/headers are never written to disk.
"""
import argparse
from collections import deque
from contextlib import contextmanager
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
import os
from pathlib import Path
import re
import socket
import sqlite3
import threading
import time

MAX_BODY = 4095
RETENTION_SECONDS = 30 * 24 * 60 * 60
FIELDS = frozenset(("schema", "version", "area", "code", "model", "region",
                    "autoboot", "context", "activity", "description"))
DIAGNOSTIC_FIELDS = frozenset(("validity", "uptime_seconds", "sd_free_mib", "linear_free_kib", "trace"))
CATEGORIES = {
    "area": frozenset(("general", "launch", "install", "update", "component", "ds",
                       "home", "power", "audio", "cover", "storage", "desktop")),
    "model": frozenset(("unknown", "3ds", "3ds-xl", "2ds", "new-3ds", "new-3ds-xl",
                        "new-2ds-xl", "pc")),
    "region": frozenset(("unknown", "jpn", "usa", "eur", "aus", "chn", "kor", "twn")),
    "autoboot": frozenset(("unknown", "off", "on")),
    "context": frozenset(("unknown", "homebrew", "native", "pc")),
    "activity": frozenset(("unknown", "browsing", "starting-game", "installing",
                           "checking-update", "downloading", "checking-components",
                           "checking-ds", "opening-home", "power-menu", "loading-theme",
                           "loading-cover", "importing")),
}


def validate_report(report):
    """No implicit device information or free-form technical fields."""
    if type(report) is not dict:
        raise ValueError("invalid fields")
    schema = report.get("schema")
    if type(schema) is not int or schema not in (1, 2):
        raise ValueError("invalid schema")
    expected = FIELDS if schema == 1 else FIELDS | {"diagnostics"}
    if set(report) != expected:
        raise ValueError("invalid fields")
    if type(report["code"]) is not int or not -(2**31) <= report["code"] < 2**31:
        raise ValueError("invalid code")
    version = report["version"]
    if type(version) is not str or not 1 <= len(version) <= 31 or not re.fullmatch(r"[0-9]+\.[0-9]+\.[0-9]+", version):
        raise ValueError("invalid version")
    for field, allowed in CATEGORIES.items():
        if type(report[field]) is not str or report[field] not in allowed:
            raise ValueError("invalid category")
    description = report["description"]
    if type(description) is not str:
        raise ValueError("invalid description")
    try:
        encoded = description.encode("utf-8")
    except UnicodeEncodeError:
        raise ValueError("invalid description") from None
    if len(encoded) > 512 or "\0" in description:
        raise ValueError("invalid description")
    if schema == 2:
        validate_diagnostics(report["diagnostics"])
    return report


def validate_diagnostics(diagnostics):
    if type(diagnostics) is not dict or set(diagnostics) != DIAGNOSTIC_FIELDS:
        raise ValueError("invalid diagnostic fields")
    validity = diagnostics["validity"]
    if type(validity) is not int or not 0 <= validity <= 7:
        raise ValueError("invalid diagnostic validity")
    for name, flag, maximum in (("uptime_seconds", 1, 604800), ("sd_free_mib", 2, 4194304), ("linear_free_kib", 4, 1048576)):
        value = diagnostics[name]
        if type(value) is not int or not 0 <= value <= maximum:
            raise ValueError("invalid diagnostic counter")
        if not validity & flag and value != 0:
            raise ValueError("unknown diagnostic counter must be zero")
    trace = diagnostics["trace"]
    if type(trace) is not list or len(trace) > 8 or any(type(item) is not str or item not in CATEGORIES["activity"] for item in trace):
        raise ValueError("invalid diagnostic trace")


def _unique_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError("duplicate key")
        result[key] = value
    return result


def parse_report(body):
    if not 0 < len(body) <= MAX_BODY:
        raise ValueError("invalid size")
    def reject_constant(_value):
        raise ValueError("invalid JSON constant")
    value = json.loads(body.decode("utf-8"), object_pairs_hook=_unique_object,
                       parse_constant=reject_constant)
    return validate_report(value)


class ReportStore:
    """Commit with FULL durability before acknowledging an accepted report."""
    def __init__(self, path, clock=time.time):
        self.path = Path(path)
        self.clock = clock
        self.path.parent.mkdir(mode=0o700, parents=True, exist_ok=True)
        try:
            fd = os.open(self.path, os.O_CREAT | os.O_EXCL | os.O_RDWR, 0o600)
        except FileExistsError:
            pass
        else:
            os.close(fd)
        with self._connect() as connection:
            connection.execute("CREATE TABLE IF NOT EXISTS reports (created_at INTEGER NOT NULL, payload TEXT NOT NULL)")
            connection.execute("CREATE INDEX IF NOT EXISTS reports_created_at ON reports(created_at)")
        self.cleanup()

    @contextmanager
    def _connect(self):
        connection = sqlite3.connect(self.path, timeout=5)
        try:
            connection.execute("PRAGMA synchronous=FULL")
            connection.execute("PRAGMA secure_delete=ON")
            with connection:
                yield connection
        finally:
            connection.close()

    def cleanup(self):
        cutoff = int(self.clock()) - RETENTION_SECONDS
        with self._connect() as connection:
            connection.execute("DELETE FROM reports WHERE created_at < ?", (cutoff,))

    def save(self, report):
        validate_report(report)
        payload = json.dumps(report, ensure_ascii=False, separators=(",", ":"), allow_nan=False)
        now = int(self.clock())
        # Context-manager exit commits. No success can escape before it returns.
        with self._connect() as connection:
            connection.execute("BEGIN IMMEDIATE")
            connection.execute("DELETE FROM reports WHERE created_at < ?", (now - RETENTION_SECONDS,))
            connection.execute("INSERT INTO reports(created_at, payload) VALUES (?, ?)", (now, payload))


class RateLimiter:
    """Bounded memory only. Peer addresses are transient and never logged."""
    def __init__(self, limit=10, window=60, maximum_peers=1024, clock=time.monotonic):
        self.limit = limit
        self.window = window
        self.maximum_peers = maximum_peers
        self.clock = clock
        self._peers = {}
        self._lock = threading.Lock()

    def allow(self, peer):
        now = self.clock()
        with self._lock:
            # Pruning every request also bounds storage in idle low-volume use.
            for address, events in list(self._peers.items()):
                while events and events[0] <= now - self.window:
                    events.popleft()
                if not events:
                    del self._peers[address]
            events = self._peers.get(peer)
            if events is None:
                if len(self._peers) >= self.maximum_peers:
                    return False
                events = self._peers[peer] = deque()
            if len(events) >= self.limit:
                return False
            events.append(now)
            return True


class ReportHandler(BaseHTTPRequestHandler):
    server_version = "FlowReportReceiver"
    sys_version = ""

    def setup(self):
        super().setup()
        self.connection.settimeout(5)

    def log_message(self, _format, *_args):
        pass  # No default request/IP logging, including validation errors.

    def _response(self, status, payload):
        # Drain only a small, unambiguously framed body before closing. Closing
        # an unread socket can discard the HTTP error response (notably Windows).
        if not getattr(self, "_body_consumed", False):
            lengths = self.headers.get_all("Content-Length", [])
            if len(lengths) == 1 and re.fullmatch(r"[0-9]{1,10}", lengths[0]) and not self.headers.get_all("Transfer-Encoding"):
                length = int(lengths[0])
                if length <= MAX_BODY + 1:
                    try:
                        self.rfile.read(length)
                    except (OSError, socket.timeout):
                        pass
            self._body_consumed = True
        data = json.dumps(payload, separators=(",", ":")).encode("ascii")
        self.close_connection = True
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(data)))
        self.send_header("Cache-Control", "no-store")
        self.send_header("Connection", "close")
        if status == 429:
            self.send_header("Retry-After", "60")
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        self._response(200, {"ok": True}) if self.path == "/healthz" else self._response(404, {"error": "not_found"})

    def do_POST(self):
        # X-Forwarded-For and all other claimed client identifiers are ignored.
        if not self.server.limiter.allow(self.client_address[0]):
            self._response(429, {"error": "rate_limited"})
            return
        if self.path != self.server.report_path:
            self._response(404, {"error": "not_found"})
            return
        if self.headers.get_all("Transfer-Encoding") or self.headers.get_all("Content-Encoding"):
            self._response(400, {"error": "invalid_request"})
            return
        lengths = self.headers.get_all("Content-Length", [])
        if len(lengths) != 1 or not re.fullmatch(r"[0-9]{1,10}", lengths[0]):
            self._response(411, {"error": "length_required"})
            return
        length = int(lengths[0])
        if length > MAX_BODY:
            self._response(413, {"error": "too_large"})
            return
        content_types = self.headers.get_all("Content-Type", [])
        if len(content_types) != 1 or not re.fullmatch(r'application/json(?:\s*;\s*charset=(?:utf-8|"utf-8"))?', content_types[0], re.IGNORECASE):
            self._response(415, {"error": "content_type"})
            return
        try:
            body = self.rfile.read(length)
            self._body_consumed = True
            if len(body) != length:
                raise ValueError("incomplete body")
            report = parse_report(body)
        except (ValueError, UnicodeError, RecursionError, socket.timeout):
            self._response(400, {"error": "invalid_report"})
            return
        try:
            self.server.store.save(report)
        except (sqlite3.Error, OSError):
            self._response(503, {"error": "storage_unavailable"})
            return
        self._response(201, {"accepted": True})


class ReportServer(ThreadingHTTPServer):
    daemon_threads = True
    block_on_close = True

    def __init__(self, address, store, limiter=None, report_path="/v1/reports", allow_container_bind=False):
        if address[0] != "127.0.0.1" and not (allow_container_bind and address[0] == "0.0.0.0"):
            raise ValueError("non-loopback binding requires explicit container mode")
        if report_path not in ("/v1/reports", "/flow-reports/v1/reports"):
            raise ValueError("unsupported report path")
        self.report_path = report_path
        self.store = store
        self.limiter = limiter or RateLimiter()
        self._slots = threading.BoundedSemaphore(16)
        self._last_cleanup = time.monotonic()
        super().__init__(address, ReportHandler)

    def process_request(self, request, client_address):
        if not self._slots.acquire(blocking=False):
            self.shutdown_request(request)
            return
        try:
            super().process_request(request, client_address)
        except BaseException:
            self._slots.release()
            raise

    def process_request_thread(self, request, client_address):
        try:
            super().process_request_thread(request, client_address)
        finally:
            self._slots.release()

    def handle_error(self, _request, _client_address):
        pass  # Unexpected transport failures must not dump client metadata.

    def service_actions(self):
        if time.monotonic() - self._last_cleanup >= 60:
            self._last_cleanup = time.monotonic()
            try:
                self.store.cleanup()
            except (sqlite3.Error, OSError):
                pass  # Storage failures fail closed on POST; no report body log.


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--database", type=Path, required=True)
    parser.add_argument("--port", type=int, default=4395)
    parser.add_argument("--host", choices=("127.0.0.1", "0.0.0.0"), default="127.0.0.1",
                        help="Use 0.0.0.0 only inside a container with host port bound to loopback")
    parser.add_argument("--path", choices=("/v1/reports", "/flow-reports/v1/reports"), default="/v1/reports")
    args = parser.parse_args()
    store = ReportStore(args.database)
    with ReportServer((args.host, args.port), store, report_path=args.path,
                      allow_container_bind=args.host == "0.0.0.0") as server:
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            pass


if __name__ == "__main__":
    main()

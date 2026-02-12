#!/usr/bin/env python3
"""UDP syslog test sender for syslog_viewer.

Sends RFC3164-formatted syslog messages to a UDP port so the
syslog_viewer application can be tested interactively.

Usage:
    # Send default batch to localhost:514
    python3 tests/test_sender.py

    # Custom host, port, and options
    python3 tests/test_sender.py --host 127.0.0.1 --port 8514

    # Continuous mode: send a message every 2 seconds
    python3 tests/test_sender.py --continuous --interval 2

    # Burst mode: send N messages as fast as possible
    python3 tests/test_sender.py --burst 100

    # Send a single custom message
    python3 tests/test_sender.py --message "Custom test message"

    # Send from a specific facility/severity
    python3 tests/test_sender.py --facility local0 --severity error --message "Disk full"

    # Replay a log file preserving timestamp delays
    python3 tests/test_sender.py --replay /var/log/app.log

    # Replay at 10x speed
    python3 tests/test_sender.py --replay /var/log/app.log --speed 10
"""

import argparse
import csv
import io
import random
import re
import socket
import string
import sys
import time
from datetime import datetime, timezone, timedelta

# RFC3164 facility codes
FACILITIES = {
    "kern": 0,
    "user": 1,
    "mail": 2,
    "daemon": 3,
    "auth": 4,
    "syslog": 5,
    "lpr": 6,
    "news": 7,
    "uucp": 8,
    "cron": 9,
    "authpriv": 10,
    "ftp": 11,
    "local0": 16,
    "local1": 17,
    "local2": 18,
    "local3": 19,
    "local4": 20,
    "local5": 21,
    "local6": 22,
    "local7": 23,
}

# RFC3164 severity codes
SEVERITIES = {
    "emerg": 0,
    "alert": 1,
    "crit": 2,
    "error": 3,
    "warning": 4,
    "notice": 5,
    "info": 6,
    "debug": 7,
}

# Aliases to map short/alternate severity names to SEVERITIES keys
SEVERITY_ALIASES = {
    "emergency": "emerg",
    "err": "error",
    "warn": "warning",
    "critical": "crit",
    "information": "info",
}

# Sample hostnames, applications, and messages for realistic test data
HOSTNAMES = [
    "webserver",
    "appserver",
    "dbserver",
    "gateway",
    "loadbalancer",
    "buildroot",
    "mailserver",
    "fileserver",
]

APPLICATIONS = [
    "nginx",
    "apache",
    "sshd",
    "postfix",
    "mysql",
    "redis",
    "cron",
    "systemd",
    "kernel",
    "ssb-mk2",
]

SAMPLE_MESSAGES = {
    "emerg": [
        "Kernel panic - not syncing: Fatal exception",
        "System halted due to critical hardware failure",
        "Out of memory: kill process or sacrifice child",
    ],
    "alert": [
        "Filesystem /dev/sda1 has reached 100% capacity",
        "RAID array degraded: disk 2 failed",
        "Temperature critical: CPU0 at 105C",
    ],
    "crit": [
        "Database corruption detected in table users",
        "SSL certificate expired for domain example.com",
        "Segmentation fault in worker process 4821",
    ],
    "error": [
        "Connection refused to upstream server 10.0.0.5:3306",
        "Failed to open /var/log/app.log: Permission denied",
        "Timeout waiting for response from auth service",
        "DNS resolution failed for api.example.com",
        "Failed to allocate buffer: insufficient memory",
    ],
    "warning": [
        "Disk usage at 85% on /var/log",
        "Connection pool near capacity: 95/100 used",
        "Slow query detected: 4.2s for SELECT on orders table",
        "Certificate expires in 7 days",
        "Memory usage at 90%, consider restarting service",
    ],
    "notice": [
        "Service restarted after configuration change",
        "User admin logged in from 192.168.1.50",
        "Backup completed successfully: 2.4GB in 45s",
        "New worker process started with PID 12345",
    ],
    "info": [
        "GET /index.html 200 OK (12ms)",
        "POST /api/users 201 Created (45ms)",
        "Connection accepted from 192.168.1.100:54321",
        "Configuration reloaded successfully",
        "Health check passed: all services operational",
        "Session started for user john.doe",
        "Email sent to admin@example.com",
    ],
    "debug": [
        "Parsed request headers: Content-Type=application/json",
        "Cache hit for key: user:1234:profile",
        "SQL: SELECT * FROM users WHERE id = 42",
        "Thread pool stats: active=3 idle=7 queued=0",
        "GC completed: freed 128MB in 15ms",
    ],
}


def normalize_severity(sev: str) -> str:
    """Normalize a severity string to a SEVERITIES key."""
    s = sev.strip().lower()
    if s in SEVERITIES:
        return s
    return SEVERITY_ALIASES.get(s, "info")


def make_priority(facility: str, severity: str) -> int:
    """Calculate RFC3164 priority value."""
    return FACILITIES[facility] * 8 + SEVERITIES[severity]


def make_rfc3164_message(
    facility: str = "user",
    severity: str = "info",
    hostname: str = "testhost",
    application: str = "testapp",
    pid: int | None = None,
    message: str = "Test message",
) -> str:
    """Build a RFC3164 syslog message string."""
    priority = make_priority(facility, severity)
    timestamp = datetime.now().strftime("%b %d %H:%M:%S")

    if pid is not None:
        tag = f"{application}[{pid}]"
    else:
        tag = application

    return f"<{priority}>{timestamp} {hostname} {tag}: {message}"


def send_message(sock: socket.socket, host: str, port: int, message: str) -> None:
    """Send a single UDP message."""
    sock.sendto(message.encode("utf-8"), (host, port))


def send_default_batch(sock: socket.socket, host: str, port: int) -> None:
    """Send a representative batch of messages covering all severities."""
    print(f"Sending default test batch to {host}:{port}...")

    messages = [
        ("kern", "emerg", "buildroot", "kernel", None, "Kernel panic - not syncing: Fatal exception"),
        ("auth", "alert", "gateway", "sshd", 1234, "Brute force attack detected from 10.0.0.99"),
        ("daemon", "crit", "dbserver", "mysql", 5678, "InnoDB: Fatal error: cannot allocate memory"),
        ("user", "error", "appserver", "nginx", 2345, "upstream timed out (110: Connection timed out)"),
        ("user", "error", "webserver", "apache", 3456, "File does not exist: /var/www/missing.html"),
        ("daemon", "warning", "fileserver", "redis", 6789, "Memory usage above 80% threshold"),
        ("auth", "notice", "gateway", "sshd", 1234, "Accepted publickey for admin from 192.168.1.50"),
        ("local0", "info", "webserver", "nginx", 2345, "GET /api/health 200 OK (3ms)"),
        ("local0", "info", "webserver", "nginx", 2345, "POST /api/login 200 OK (125ms)"),
        ("local0", "info", "loadbalancer", "nginx", 7890, "upstream server 10.0.0.2 is up"),
        ("cron", "info", "appserver", "cron", None, "Job backup-daily completed successfully"),
        ("daemon", "debug", "dbserver", "mysql", 5678, "Query cache hit ratio: 94.2%"),
        ("user", "debug", "appserver", "ssb-mk2", None, "Thread pool: active=2 idle=8 queued=0"),
    ]

    for facility, severity, hostname, app, pid, msg in messages:
        raw = make_rfc3164_message(facility, severity, hostname, app, pid, msg)
        send_message(sock, host, port, raw)
        print(f"  [{severity.upper():7s}] {hostname}/{app}: {msg}")
        time.sleep(0.1)  # Small delay so messages arrive in order

    print(f"\nSent {len(messages)} messages.")


def send_random_message(sock: socket.socket, host: str, port: int) -> None:
    """Send a single random realistic message."""
    severity = random.choice(list(SEVERITIES.keys()))
    facility = random.choice(["user", "daemon", "auth", "local0", "cron", "kern"])
    hostname = random.choice(HOSTNAMES)
    application = random.choice(APPLICATIONS)
    pid = random.randint(100, 65535) if random.random() > 0.3 else None
    message = random.choice(SAMPLE_MESSAGES[severity])

    raw = make_rfc3164_message(facility, severity, hostname, application, pid, message)
    send_message(sock, host, port, raw)
    print(f"  [{severity.upper():7s}] {hostname}/{application}: {message}")


def send_burst(sock: socket.socket, host: str, port: int, count: int) -> None:
    """Send N messages as fast as possible for throughput testing."""
    print(f"Sending burst of {count} messages to {host}:{port}...")
    start = time.monotonic()

    for i in range(count):
        severity = random.choice(list(SEVERITIES.keys()))
        facility = random.choice(["user", "daemon", "local0"])
        hostname = random.choice(HOSTNAMES)
        application = random.choice(APPLICATIONS)
        pid = random.randint(100, 65535)
        message = f"Burst message {i + 1}/{count}: " + "".join(
            random.choices(string.ascii_lowercase + " ", k=random.randint(20, 80))
        )

        raw = make_rfc3164_message(facility, severity, hostname, application, pid, message)
        send_message(sock, host, port, raw)

    elapsed = time.monotonic() - start
    rate = count / elapsed if elapsed > 0 else float("inf")
    print(f"Sent {count} messages in {elapsed:.3f}s ({rate:.0f} msg/s)")


def parse_timestamp_from_line(line: str) -> datetime | None:
    """Extract a timestamp from a log line in any supported format.

    Returns a timezone-aware datetime (UTC) or a naive datetime if no
    timezone info is present. Returns None if no timestamp is found.
    """
    stripped = line.strip()
    if not stripped:
        return None

    # Raw syslog: strip <priority> prefix
    raw_match = re.match(r"^<\d+>(.*)$", stripped)
    if raw_match:
        stripped = raw_match.group(1)

    # Try ISO 8601: 2026-01-15T14:33:02.756342+00:00
    iso_match = re.match(
        r"(\d{4}-\d{2}-\d{2})[T ](\d{2}:\d{2}:\d{2})"
        r"(?:\.(\d+))?"
        r"(?:([Zz])|([+-]\d{2}):?(\d{2}))?",
        stripped,
    )
    if iso_match:
        date_str, time_str = iso_match.group(1), iso_match.group(2)
        dt = datetime.strptime(f"{date_str} {time_str}", "%Y-%m-%d %H:%M:%S")
        # Apply fractional seconds
        frac_str = iso_match.group(3)
        if frac_str:
            microseconds = int(frac_str[:6].ljust(6, "0"))
            dt = dt.replace(microsecond=microseconds)
        # Apply timezone
        tz_z = iso_match.group(4)
        tz_sign = iso_match.group(5)
        if tz_z:
            dt = dt.replace(tzinfo=timezone.utc)
        elif tz_sign:
            tz_minutes = int(iso_match.group(6) or "0")
            tz_hours = int(tz_sign)
            offset = timedelta(hours=tz_hours, minutes=tz_minutes if tz_hours >= 0 else -tz_minutes)
            dt = dt.replace(tzinfo=timezone(offset))
        return dt

    # Try RFC3164 timestamp: "Jan 15 14:33:02" (no year — assume current year)
    rfc3164_match = re.match(
        r"([A-Z][a-z]{2})\s+(\d{1,2})\s+(\d{2}:\d{2}:\d{2})", stripped
    )
    if rfc3164_match:
        month_str = rfc3164_match.group(1)
        day = rfc3164_match.group(2)
        time_str = rfc3164_match.group(3)
        year = datetime.now().year
        try:
            dt = datetime.strptime(
                f"{month_str} {day} {time_str} {year}", "%b %d %H:%M:%S %Y"
            )
            return dt
        except ValueError:
            pass

    return None


def parse_log_fields(line: str) -> dict | None:
    """Parse a log line into fields for RFC3164 reconstruction.

    Returns a dict with keys: severity, facility, hostname, application,
    message (and optionally pid, source_ip). Returns None if parsing fails.
    """
    stripped = line.strip()
    if not stripped:
        return None

    # Already raw syslog — send as-is
    if stripped.startswith("<"):
        return None

    # Dash-delimited: "TIMESTAMP hostname app - - - SEVERITY [message]"
    marker = " - - - "
    marker_pos = stripped.find(marker)
    if marker_pos != -1:
        after = stripped[marker_pos + len(marker):]
        space = after.find(" ")
        if space != -1:
            severity = after[:space].strip().lower()
            message = after[space + 1:]
        else:
            severity = after.strip().lower()
            message = ""
        # Parse before marker to get hostname and app
        before = stripped[:marker_pos]
        tokens = before.split()
        hostname = "unknown"
        application = "unknown"
        if tokens:
            # ISO 8601 single-token timestamp
            if "T" in tokens[0] and len(tokens[0]) > 10:
                if len(tokens) > 1:
                    hostname = tokens[1]
                if len(tokens) > 2:
                    application = tokens[2]
            # Space-separated date time
            elif len(tokens) >= 2 and ":" in tokens[1]:
                if len(tokens) > 2:
                    hostname = tokens[2]
                if len(tokens) > 3:
                    application = tokens[3]
        return {
            "severity": normalize_severity(severity),
            "facility": "user",
            "hostname": hostname,
            "application": application,
            "message": message,
        }

    # Pipe-delimited: "Timestamp|Severity|Facility|Source IP|Hostname|Application|Message"
    if "|" in stripped:
        parts = stripped.split("|", 6)
        if len(parts) >= 7:
            sev = normalize_severity(parts[1])
            fac = parts[2].strip().lower()
            fac = fac if fac in FACILITIES else "user"
            return {
                "severity": sev,
                "facility": fac,
                "hostname": parts[4].strip(),
                "application": parts[5].strip(),
                "message": parts[6],
                "source_ip": parts[3].strip(),
            }

    # CSV: "Timestamp,Severity,Facility,Source IP,Hostname,Application,Message"
    if "," in stripped and stripped[0].isdigit():
        try:
            reader = csv.reader(io.StringIO(stripped))
            parts = next(reader)
            if len(parts) >= 7:
                sev = normalize_severity(parts[1])
                fac = parts[2].strip().lower()
                fac = fac if fac in FACILITIES else "user"
                return {
                    "severity": sev,
                    "facility": fac,
                    "hostname": parts[4].strip(),
                    "application": parts[5].strip(),
                    "message": parts[6],
                    "source_ip": parts[3].strip(),
                }
        except (csv.Error, StopIteration):
            pass

    return None


def line_to_rfc3164(line: str) -> str:
    """Convert a log line to RFC3164 format for UDP transmission.

    If the line is already raw syslog (<priority>...), return as-is.
    Otherwise, parse fields and reconstruct.
    """
    stripped = line.strip()
    if stripped.startswith("<"):
        return stripped

    fields = parse_log_fields(stripped)
    if fields:
        return make_rfc3164_message(
            facility=fields["facility"],
            severity=fields["severity"],
            hostname=fields["hostname"],
            application=fields["application"],
            message=fields["message"],
        )

    # Fallback: wrap as-is in a user.info RFC3164 message
    return make_rfc3164_message(message=stripped)


def replay_file(
    sock: socket.socket, host: str, port: int, filepath: str, speed: float
) -> None:
    """Replay a log file via UDP, preserving timestamp-based delays."""
    try:
        with open(filepath, "r") as f:
            lines = f.readlines()
    except OSError as e:
        print(f"Error opening file: {e}", file=sys.stderr)
        sys.exit(1)

    # Filter out empty lines, comments, and CSV header
    entries: list[tuple[str, datetime | None]] = []
    for line in lines:
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):
            continue
        if stripped.startswith("Timestamp,Severity,Facility"):
            continue
        ts = parse_timestamp_from_line(stripped)
        entries.append((stripped, ts))

    if not entries:
        print("No log entries found in file.")
        return

    print(f"Replaying {len(entries)} messages from {filepath} to {host}:{port}")
    if speed != 1.0:
        print(f"  Speed: {speed}x")
    print()

    sent = 0
    prev_ts: datetime | None = None

    for stripped, ts in entries:
        # Calculate delay from timestamp difference
        if ts is not None and prev_ts is not None:
            # Make both aware or both naive for comparison
            if ts.tzinfo is not None and prev_ts.tzinfo is not None:
                delta = (ts - prev_ts).total_seconds()
            elif ts.tzinfo is None and prev_ts.tzinfo is None:
                delta = (ts - prev_ts).total_seconds()
            else:
                # Mixed aware/naive — skip delay
                delta = 0.0

            if delta > 0 and speed > 0:
                sleep_time = delta / speed
                if sleep_time > 0.001:
                    print(f"  (waiting {sleep_time:.3f}s)")
                    time.sleep(sleep_time)

        raw = line_to_rfc3164(stripped)
        send_message(sock, host, port, raw)
        sent += 1

        # Print a summary of what was sent
        fields = parse_log_fields(stripped)
        if fields:
            sev = fields["severity"].upper()
            host_name = fields["hostname"]
            app = fields["application"]
            msg = fields["message"]
            print(f"  [{sev:7s}] {host_name}/{app}: {msg[:80]}")
        else:
            print(f"  {stripped[:90]}")

        if ts is not None:
            prev_ts = ts

    print(f"\nReplayed {sent} messages.")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="UDP syslog test sender for syslog_viewer",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""\
examples:
  %(prog)s                           Send default test batch
  %(prog)s --port 8514               Send to custom port
  %(prog)s --continuous              Send random messages continuously
  %(prog)s --continuous --interval 1 Send every 1 second
  %(prog)s --burst 1000              Send 1000 messages rapidly
  %(prog)s --message "Test msg"      Send a single custom message
  %(prog)s --facility local0 --severity error --message "Disk full"
  %(prog)s --replay app.log          Replay a log file with original timing
  %(prog)s --replay app.log --speed 5 Replay at 5x speed
""",
    )
    parser.add_argument("--host", default="127.0.0.1", help="target host (default: 127.0.0.1)")
    parser.add_argument("--port", type=int, default=514, help="target UDP port (default: 514)")
    parser.add_argument("--message", "-m", help="send a single custom message")
    parser.add_argument(
        "--facility", default="user", choices=sorted(FACILITIES.keys()), help="syslog facility (default: user)"
    )
    parser.add_argument(
        "--severity", default="info", choices=sorted(SEVERITIES.keys()), help="syslog severity (default: info)"
    )
    parser.add_argument("--hostname", default="testhost", help="hostname in syslog message (default: testhost)")
    parser.add_argument("--app", default="testapp", help="application name (default: testapp)")
    parser.add_argument("--pid", type=int, default=None, help="process ID (optional)")
    parser.add_argument("--continuous", "-c", action="store_true", help="send random messages continuously")
    parser.add_argument("--interval", type=float, default=2.0, help="seconds between messages in continuous mode (default: 2.0)")
    parser.add_argument("--burst", "-b", type=int, help="send N messages as fast as possible")
    parser.add_argument("--replay", "-r", metavar="FILE", help="replay a log file via UDP, preserving timestamp delays")
    parser.add_argument("--speed", "-s", type=float, default=1.0, help="replay speed multiplier (default: 1.0, e.g. 10 = 10x faster)")

    args = parser.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    try:
        if args.replay:
            replay_file(sock, args.host, args.port, args.replay, args.speed)

        elif args.message:
            raw = make_rfc3164_message(
                args.facility, args.severity, args.hostname, args.app, args.pid, args.message
            )
            send_message(sock, args.host, args.port, raw)
            print(f"Sent to {args.host}:{args.port}: {raw}")

        elif args.burst:
            send_burst(sock, args.host, args.port, args.burst)

        elif args.continuous:
            print(f"Sending random messages to {args.host}:{args.port} every {args.interval}s (Ctrl+C to stop)...")
            count = 0
            while True:
                send_random_message(sock, args.host, args.port)
                count += 1
                time.sleep(args.interval)

        else:
            send_default_batch(sock, args.host, args.port)

    except KeyboardInterrupt:
        print(f"\nStopped. Sent {count if args.continuous else 0} messages.")
    finally:
        sock.close()


if __name__ == "__main__":
    main()

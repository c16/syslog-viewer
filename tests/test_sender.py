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
"""

import argparse
import random
import socket
import string
import sys
import time
from datetime import datetime

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

    args = parser.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    try:
        if args.message:
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

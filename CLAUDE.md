# CLAUDE.md

## Project Overview

Syslog Viewer is a GTKmm 3.0 (C++) application for receiving and displaying syslog messages in real-time via UDP. It can also be embedded as a reusable widget in other GTKmm applications.

## Build Instructions

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

Run tests:
```bash
cd build
./syslog_message_test
./udp_listener_test
```

Requires: `libgtkmm-3.0-dev`, CMake 3.10+, C++17 compiler.

Install dependencies (Ubuntu/Debian): `sudo apt-get install build-essential cmake libgtkmm-3.0-dev`

## Project Structure

- `include/` — Header files
  - `syslog_message.h` — Message model, parsing (RFC3164/RFC5424), severity/facility enums
  - `udp_listener.h` — UDP socket listener with threading, ephemeral port support
  - `syslog_dialog.h` — Main GTKmm UI component (reusable widget, inherits `Gtk::Box`)
- `src/` — Implementation files
  - `syslog_message.cpp` — Message parsing, timestamp/severity/facility string conversions
  - `udp_listener.cpp` — UDP listener with separate thread, POSIX sockets, `getsockname` for ephemeral ports
  - `syslog_dialog.cpp` — UI implementation: controls, filters, tree view, import/export, config
  - `main.cpp` — Standalone application entry point (`SyslogWindow` wraps `SyslogDialog`)
- `tests/` — Unit tests (Google Test via CMake FetchContent)
  - `syslog_message_test.cpp` — 41 tests: parsing, severity/facility/timestamp roundtrips
  - `udp_listener_test.cpp` — 10 tests: lifecycle, port config, message reception via UDP
- `example_integration.cpp` — Example of embedding SyslogDialog in a multi-tab app
- `CMakeLists.txt` — Builds static library `libsyslog_dialog_lib.a`, `syslog_viewer` binary, and test executables

## Key Architecture Decisions

- **Threading**: UDP listener runs in a separate thread. UI updates go through `Glib::Dispatcher` for thread safety.
- **Message queue**: Incoming messages are queued in a `std::deque` (not a single slot) to prevent message loss under high throughput. `on_message_dispatch()` drains the entire queue.
- **Mutexes**: `messages_mutex_` (message storage), `pending_mutex_` (dispatcher queue), `log_file_mutex_` (file I/O).
- **Filtering**: Uses `Gtk::TreeModelFilter` with a custom filter function. Text filter is case-insensitive. Severity checkboxes persist to config; text filter does not trigger config saves.
- **Config persistence**: Saved to `~/.syslog_viewer.conf` as key=value pairs. Auto-loaded on startup. Only saved on checkbox toggles and explicit actions (not on every keystroke).
- **Ephemeral ports**: `UdpListener` supports port 0 — after `bind()`, `getsockname()` updates the port to the actual bound value.

## File Formats

- **Pipe-delimited log**: `Timestamp|Severity|Facility|Source IP|Hostname|Application|Message`
- **CSV export**: RFC 4180 compliant — fields containing `"`, `,`, or newlines are quoted/escaped
- **Dash-delimited log**: `Timestamp hostname app - - - SEVERITY message` (ISO 8601 timestamps supported)
- **Raw syslog**: RFC3164 `<priority>MMM DD HH:MM:SS hostname app[pid]: message`

## Timestamp Formats

- Standard: `YYYY-MM-DD HH:MM:SS`
- ISO 8601: `YYYY-MM-DDTHH:MM:SS.ffffff+00:00` (normalized on parse: T→space, fractional/timezone stripped)

## Code Style

- C++17 with STL containers and algorithms
- Snake_case for variables and methods, trailing underscore for member variables
- GTKmm signal/slot pattern via `sigc::mem_fun`
- Use `static_cast<unsigned char>` when calling `<cctype>` functions to avoid UB on signed chars
- No external dependencies beyond GTKmm, POSIX, and Google Test (fetched at build time)

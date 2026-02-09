# CLAUDE.md

## Project Overview

Syslog Viewer is a GTKmm 3.0 (C++) application for receiving and displaying syslog messages in real-time via UDP. It can also be embedded as a reusable widget in other GTKmm applications.

## Build Instructions

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

Requires: `libgtkmm-3.0-dev`, CMake 3.10+, C++17 compiler.

Install dependencies (Ubuntu/Debian): `sudo apt-get install build-essential cmake libgtkmm-3.0-dev`

## Project Structure

- `include/` — Header files
  - `syslog_message.h` — Message model, parsing (RFC3164/RFC5424), severity/facility enums
  - `udp_listener.h` — UDP socket listener with threading
  - `syslog_dialog.h` — Main GTKmm UI component (reusable widget, inherits `Gtk::Box`)
- `src/` — Implementation files
  - `syslog_message.cpp` — Message parsing, timestamp/severity/facility string conversions
  - `udp_listener.cpp` — UDP listener with separate thread, POSIX sockets
  - `syslog_dialog.cpp` — UI implementation: controls, filters, tree view, import/export, config
  - `main.cpp` — Standalone application entry point (`SyslogWindow` wraps `SyslogDialog`)
- `example_integration.cpp` — Example of embedding SyslogDialog in a multi-tab app
- `CMakeLists.txt` — Builds static library `libsyslog_dialog_lib.a` and `syslog_viewer` binary

## Key Architecture Decisions

- **Threading**: UDP listener runs in a separate thread. UI updates go through `Glib::Dispatcher` for thread safety.
- **Mutexes**: `messages_mutex_` (message storage), `pending_mutex_` (dispatcher message), `log_file_mutex_` (file I/O).
- **Filtering**: Uses `Gtk::TreeModelFilter` with a custom filter function. Filters apply to severity checkboxes and text search.
- **Config persistence**: Saved to `~/.syslog_viewer.conf` as key=value pairs. Auto-loaded on startup.

## File Formats

- **Pipe-delimited log**: `Timestamp|Severity|Facility|Source IP|Hostname|Application|Message`
- **CSV export**: `Timestamp,Severity,Facility,Source IP,Hostname,Application,"Message"`
- **Raw syslog**: RFC3164 `<priority>MMM DD HH:MM:SS hostname app[pid]: message`

## Code Style

- C++17 with STL containers and algorithms
- Snake_case for variables and methods, trailing underscore for member variables
- GTKmm signal/slot pattern via `sigc::mem_fun`
- No external dependencies beyond GTKmm and POSIX

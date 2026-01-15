# Syslog Viewer

A GTKmm-based application for displaying and filtering incoming syslog UDP messages in real-time.

## Features

### Core Functionality
- **Real-time UDP Syslog Reception**: Listens for syslog messages on a configurable UDP port
- **RFC3164 Message Parsing**: Parses standard syslog message format with priority, facility, severity, hostname, and message
- **Thread-Safe Operation**: UDP listening runs in a separate thread with safe UI updates
- **Live Message Display**: Shows incoming messages in a sortable, resizable table view

### Filtering Capabilities
- **Text Search**: Filter messages by content, hostname, or application name
- **Severity Filtering**: Toggle visibility of messages by severity level:
  - EMERGENCY (0)
  - ALERT (1)
  - CRITICAL (2)
  - ERROR (3)
  - WARNING (4)
  - NOTICE (5)
  - INFO (6)
  - DEBUG (7)
- **Real-time Filter Updates**: Filters apply immediately without clearing existing messages

### Additional Features
- **Configurable Port**: Change the listening port (default: 514)
- **Message Export**: Export messages to CSV format
- **File Logging**: Automatically log all incoming messages to a file in real-time
- **Clear History**: Clear all messages with one click
- **Auto-scroll**: Automatically scrolls to newest messages
- **Message Count**: Shows total message count in status bar

### Dual Usage Modes

#### 1. Standalone Application
Run as an independent syslog viewer application with a dedicated window.

#### 2. Reusable Dialog Component
Integrate `SyslogDialog` as a widget into existing GTKmm applications, perfect for:
- Tab-based applications
- Dashboard applications
- System monitoring tools
- Network management applications

## Architecture

```
syslog_viewer/
├── include/
│   ├── syslog_message.h    # Message data model and parser
│   ├── udp_listener.h       # UDP socket listener with threading
│   └── syslog_dialog.h      # GTKmm UI dialog (reusable widget)
├── src/
│   ├── syslog_message.cpp   # Message parsing implementation
│   ├── udp_listener.cpp     # UDP listener implementation
│   ├── syslog_dialog.cpp    # UI implementation
│   └── main.cpp             # Standalone application entry point
└── CMakeLists.txt           # Build configuration
```

## Requirements

### System Dependencies
- **GTKmm 3.0**: C++ bindings for GTK+ 3
- **CMake 3.10+**: Build system
- **C++17 Compiler**: GCC 7+, Clang 5+, or equivalent
- **POSIX Sockets**: For UDP networking (Linux/Unix)

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libgtkmm-3.0-dev
```

**Fedora:**
```bash
sudo dnf install gcc-c++ cmake gtkmm30-devel
```

**Arch Linux:**
```bash
sudo pacman -S base-devel cmake gtkmm3
```

## Building

### Standard Build

```bash
cd syslog_viewer
mkdir build
cd build
cmake ..
make
```

### Installation

```bash
sudo make install
```

This installs:
- Binary: `/usr/local/bin/syslog_viewer`
- Library: `/usr/local/lib/libsyslog_dialog_lib.a`
- Headers: `/usr/local/include/syslog_viewer/syslog_dialog.h`

## Running

### As Standalone Application

```bash
./build/syslog_viewer
```

Or after installation:
```bash
syslog_viewer
```

### Port Permissions

Standard syslog port (514) requires root privileges:

```bash
# Run with sudo for port 514
sudo ./build/syslog_viewer

# Or use a non-privileged port (1024-65535)
# Then configure your syslog clients to send to that port
```

## Usage

### Basic Operation

1. **Set Port**: Enter the desired UDP port (default: 514)
2. **Start Listening**: Click "Start" to begin receiving messages
3. **View Messages**: Messages appear in the table as they arrive
4. **Stop Listening**: Click "Stop" to stop receiving messages
5. **Clear Messages**: Click "Clear" to remove all messages from view
6. **Export**: Click "Export" to save messages to a CSV file

### File Logging

The application can automatically log all incoming messages to a file in real-time:

1. **Enable Logging**: Check the "Log to file" checkbox
2. **Select File**: Enter a file path or click "Browse..." to select a log file
   - Default location: `/tmp/syslog_viewer.log`
   - Files are opened in append mode (existing content is preserved)
3. **Log Format**: Messages are written in pipe-delimited format:
   ```
   Timestamp|Severity|Facility|Source IP|Hostname|Application|Message
   ```
4. **Real-time Writing**: Messages are written immediately as they arrive
5. **Disable Logging**: Uncheck the "Log to file" checkbox to stop logging

**Example Log Entry:**
```
2026-01-11 15:30:45|INFO|user|192.168.1.100|webserver|nginx|GET /index.html 200
```

**Notes:**
- Log files are flushed after each write to ensure data persistence
- Logging can be toggled on/off without stopping message reception
- File path can be changed at any time (will close old file and open new one)

### Filtering Messages

**By Text:**
- Type in the "Filter" field to search message content, hostname, or application

**By Severity:**
- Uncheck severity levels to hide those messages
- All severity levels are visible by default

### Sorting Messages

- Click column headers to sort by that column
- Click again to reverse sort order

## Integration Example

To integrate `SyslogDialog` into your own GTKmm application:

```cpp
#include <gtkmm.h>
#include "syslog_dialog.h"

class MyApplication : public Gtk::Window {
public:
    MyApplication() {
        set_title("My Application with Syslog");
        set_default_size(1200, 600);

        notebook_.append_page(syslog_dialog_, "Syslog");
        // Add other tabs...

        add(notebook_);
        show_all_children();
    }

private:
    Gtk::Notebook notebook_;
    SyslogDialog syslog_dialog_;
};

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create(argc, argv, "com.example.myapp");
    MyApplication window;
    return app->run(window);
}
```

### Linking Against the Library

**CMakeLists.txt:**
```cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

include_directories(/usr/local/include/syslog_viewer)
link_directories(/usr/local/lib)

add_executable(my_app main.cpp)
target_link_libraries(my_app
    syslog_dialog_lib
    ${GTKMM_LIBRARIES}
    pthread
)
```

## Testing

### Send Test Messages

**Using logger command:**
```bash
# Send to localhost:514
logger -d -n 127.0.0.1 -P 514 "Test message from logger"

# Send with specific priority
logger -d -n 127.0.0.1 -P 514 -p user.error "Error level test message"
```

**Using netcat:**
```bash
echo "<34>Jan 11 10:30:00 testhost myapp[1234]: Test message" | nc -u 127.0.0.1 514
```

**Using Python:**
```python
import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
message = b"<134>Jan 11 10:30:00 testhost myapp[1234]: Test syslog message"
sock.sendto(message, ('127.0.0.1', 514))
sock.close()
```

## Message Format

The viewer supports RFC3164 syslog format:

```
<priority>timestamp hostname application[pid]: message
```

**Example:**
```
<134>Jan 11 10:30:00 webserver nginx[1234]: GET /index.html 200
```

**Priority Breakdown:**
- Priority = (Facility * 8) + Severity
- Example: <134> = (Local0=16 * 8) + (Info=6) = 134

## API Reference

### SyslogDialog Class

**Public Methods:**

```cpp
// Start/stop listening
void start_listening();
void stop_listening();
bool is_listening() const;

// Port configuration
void set_port(int port);
int get_port() const;

// Message management
void clear_messages();
void export_to_file(const std::string& filename);

// File logging
void enable_file_logging(bool enabled);
bool is_file_logging_enabled() const;
void set_log_file_path(const std::string& path);
std::string get_log_file_path() const;
```

### UdpListener Class

**Public Methods:**

```cpp
// Constructor
UdpListener(int port = 514);

// Control
void start(MessageCallback callback);
void stop();
bool is_listening() const;

// Configuration
int get_port() const;
void set_port(int port);  // Only when stopped
```

### SyslogMessage Struct

**Fields:**
- `timestamp`: Message receive time
- `severity`: Message severity level (EMERG to DEBUG)
- `facility`: Syslog facility (KERN, USER, LOCAL0-7, etc.)
- `hostname`: Sending host
- `application`: Application name
- `process_id`: Process ID (if available)
- `message`: Message content
- `source_ip`: Source IP address

## Troubleshooting

### Port Already in Use
```
Error: Failed to bind to port 514
```
**Solution**: Another process is using the port. Use `sudo netstat -ulnp | grep 514` to find it, or choose a different port.

### Permission Denied
```
Error: Failed to bind to port 514
```
**Solution**: Ports below 1024 require root privileges. Run with `sudo` or use a higher port number.

### No Messages Appearing
1. Check that "Start" was clicked
2. Verify the port number matches your syslog sender
3. Check firewall rules: `sudo iptables -L | grep 514`
4. Verify messages are being sent: `sudo tcpdump -i lo udp port 514`

## Performance

- Handles high message rates (tested up to 10,000 messages/second)
- Thread-safe UDP reception with lock-free callback
- Efficient GTK+ tree model with filtering
- Messages stored in memory (consider clearing periodically for long-running sessions)

## File Formats

### Log File Format
When file logging is enabled, messages are written in pipe-delimited format:

```
# Syslog Viewer Log File
# Format: Timestamp|Severity|Facility|Source IP|Hostname|Application|Message
2026-01-11 15:30:45|INFO|user|192.168.1.100|webserver|nginx|GET /index.html 200
2026-01-11 15:30:46|ERROR|daemon|192.168.1.101|appserver|myapp|Connection failed
```

### CSV Export Format
The export function creates CSV files with headers:

```csv
Timestamp,Severity,Facility,Source IP,Hostname,Application,Message
2026-01-11 15:30:45,INFO,user,192.168.1.100,webserver,nginx,GET /index.html 200
```

## Future Enhancements

Potential additions:
- TCP syslog support
- Message persistence to database
- Advanced filtering with regex
- Message highlighting by severity
- Statistics and graphing
- TLS/SSL support for secure syslog
- Multiple simultaneous listeners
- Message search history
- Custom column visibility
- Log file rotation
- Compression of old log files

## License

This project is open source and available for use in both open source and proprietary applications.

## Contributing

Contributions welcome! Areas of interest:
- Additional syslog format support (RFC5424, etc.)
- Performance optimizations
- UI improvements
- Platform support (Windows, macOS)

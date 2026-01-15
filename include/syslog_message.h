#ifndef SYSLOG_MESSAGE_H
#define SYSLOG_MESSAGE_H

#include <chrono>
#include <ctime>
#include <string>

enum class SyslogSeverity {
  EMERGENCY = 0,
  ALERT = 1,
  CRITICAL = 2,
  ERROR = 3,
  WARNING = 4,
  NOTICE = 5,
  INFO = 6,
  DEBUG = 7
};

enum class SyslogFacility {
  KERN = 0,
  USER = 1,
  MAIL = 2,
  DAEMON = 3,
  AUTH = 4,
  SYSLOG = 5,
  LPR = 6,
  NEWS = 7,
  UUCP = 8,
  CRON = 9,
  AUTHPRIV = 10,
  FTP = 11,
  LOCAL0 = 16,
  LOCAL1 = 17,
  LOCAL2 = 18,
  LOCAL3 = 19,
  LOCAL4 = 20,
  LOCAL5 = 21,
  LOCAL6 = 22,
  LOCAL7 = 23
};

struct SyslogMessage {
  std::chrono::system_clock::time_point timestamp;
  SyslogSeverity severity;
  SyslogFacility facility;
  std::string hostname;
  std::string application;
  std::string process_id;
  std::string message;
  std::string source_ip;

  // Parse RFC3164 or RFC5424 syslog message
  static SyslogMessage parse(const std::string& raw_message,
                             const std::string& source_ip);

  // Get severity as string
  std::string severity_string() const;

  // Get facility as string
  std::string facility_string() const;

  // Get formatted timestamp
  std::string timestamp_string() const;

  // Get priority value
  int priority() const;
};

#endif  // SYSLOG_MESSAGE_H

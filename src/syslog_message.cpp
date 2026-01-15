#include "syslog_message.h"

#include <iomanip>
#include <regex>
#include <sstream>

std::string SyslogMessage::severity_string() const {
  switch (severity) {
    case SyslogSeverity::EMERGENCY:
      return "EMERG";
    case SyslogSeverity::ALERT:
      return "ALERT";
    case SyslogSeverity::CRITICAL:
      return "CRIT";
    case SyslogSeverity::ERROR:
      return "ERROR";
    case SyslogSeverity::WARNING:
      return "WARN";
    case SyslogSeverity::NOTICE:
      return "NOTICE";
    case SyslogSeverity::INFO:
      return "INFO";
    case SyslogSeverity::DEBUG:
      return "DEBUG";
    default:
      return "UNKNOWN";
  }
}

std::string SyslogMessage::facility_string() const {
  switch (facility) {
    case SyslogFacility::KERN:
      return "kern";
    case SyslogFacility::USER:
      return "user";
    case SyslogFacility::MAIL:
      return "mail";
    case SyslogFacility::DAEMON:
      return "daemon";
    case SyslogFacility::AUTH:
      return "auth";
    case SyslogFacility::SYSLOG:
      return "syslog";
    case SyslogFacility::LPR:
      return "lpr";
    case SyslogFacility::NEWS:
      return "news";
    case SyslogFacility::UUCP:
      return "uucp";
    case SyslogFacility::CRON:
      return "cron";
    case SyslogFacility::AUTHPRIV:
      return "authpriv";
    case SyslogFacility::FTP:
      return "ftp";
    case SyslogFacility::LOCAL0:
      return "local0";
    case SyslogFacility::LOCAL1:
      return "local1";
    case SyslogFacility::LOCAL2:
      return "local2";
    case SyslogFacility::LOCAL3:
      return "local3";
    case SyslogFacility::LOCAL4:
      return "local4";
    case SyslogFacility::LOCAL5:
      return "local5";
    case SyslogFacility::LOCAL6:
      return "local6";
    case SyslogFacility::LOCAL7:
      return "local7";
    default:
      return "unknown";
  }
}

std::string SyslogMessage::timestamp_string() const {
  auto time_t_val = std::chrono::system_clock::to_time_t(timestamp);
  std::tm tm_val{};
  localtime_r(&time_t_val, &tm_val);

  std::ostringstream oss;
  oss << std::put_time(&tm_val, "%Y-%m-%d %H:%M:%S");
  return oss.str();
}

int SyslogMessage::priority() const {
  return static_cast<int>(facility) * 8 + static_cast<int>(severity);
}

SyslogMessage SyslogMessage::parse(const std::string& raw_message,
                                   const std::string& source_ip) {
  SyslogMessage msg;
  msg.timestamp = std::chrono::system_clock::now();
  msg.source_ip = source_ip;
  msg.severity = SyslogSeverity::INFO;
  msg.facility = SyslogFacility::USER;

  if (raw_message.empty()) {
    msg.message = raw_message;
    return msg;
  }

  // Parse priority (RFC3164/RFC5424 format: <priority>...)
  std::regex priority_regex(R"(^<(\d+)>(.*)$)");
  std::smatch priority_match;

  std::string remaining = raw_message;

  if (std::regex_match(raw_message, priority_match, priority_regex)) {
    int priority = std::stoi(priority_match[1].str());
    msg.facility = static_cast<SyslogFacility>(priority / 8);
    msg.severity = static_cast<SyslogSeverity>(priority % 8);
    remaining = priority_match[2].str();
  }

  // Try to parse RFC3164 format: MMM DD HH:MM:SS hostname tag: message
  std::regex rfc3164_regex(
      R"(^([A-Z][a-z]{2}\s+\d{1,2}\s+\d{2}:\d{2}:\d{2})\s+(\S+)\s+(\S+?)(?:\[(\d+)\])?\s*:\s*(.*)$)");
  std::smatch rfc3164_match;

  if (std::regex_match(remaining, rfc3164_match, rfc3164_regex)) {
    // timestamp is in rfc3164_match[1] but we use current time
    msg.hostname = rfc3164_match[2].str();
    msg.application = rfc3164_match[3].str();
    if (rfc3164_match[4].matched) {
      msg.process_id = rfc3164_match[4].str();
    }
    msg.message = rfc3164_match[5].str();
  } else {
    // Try to extract hostname and message from simple format
    std::regex simple_regex(R"(^\s*(\S+)\s+(.*)$)");
    std::smatch simple_match;

    if (std::regex_match(remaining, simple_match, simple_regex)) {
      msg.hostname = simple_match[1].str();
      msg.message = simple_match[2].str();
    } else {
      msg.message = remaining;
    }
  }

  return msg;
}

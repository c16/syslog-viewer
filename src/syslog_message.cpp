#include "syslog_message.h"

#include <iomanip>
#include <regex>
#include <sstream>
#include <vector>

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

SyslogSeverity SyslogMessage::parse_severity(const std::string& str) {
  if (str == "EMERG") return SyslogSeverity::EMERGENCY;
  if (str == "ALERT") return SyslogSeverity::ALERT;
  if (str == "CRIT") return SyslogSeverity::CRITICAL;
  if (str == "ERROR") return SyslogSeverity::ERROR;
  if (str == "WARN") return SyslogSeverity::WARNING;
  if (str == "NOTICE") return SyslogSeverity::NOTICE;
  if (str == "INFO") return SyslogSeverity::INFO;
  if (str == "DEBUG") return SyslogSeverity::DEBUG;
  return SyslogSeverity::INFO;
}

SyslogFacility SyslogMessage::parse_facility(const std::string& str) {
  if (str == "kern") return SyslogFacility::KERN;
  if (str == "user") return SyslogFacility::USER;
  if (str == "mail") return SyslogFacility::MAIL;
  if (str == "daemon") return SyslogFacility::DAEMON;
  if (str == "auth") return SyslogFacility::AUTH;
  if (str == "syslog") return SyslogFacility::SYSLOG;
  if (str == "lpr") return SyslogFacility::LPR;
  if (str == "news") return SyslogFacility::NEWS;
  if (str == "uucp") return SyslogFacility::UUCP;
  if (str == "cron") return SyslogFacility::CRON;
  if (str == "authpriv") return SyslogFacility::AUTHPRIV;
  if (str == "ftp") return SyslogFacility::FTP;
  if (str == "local0") return SyslogFacility::LOCAL0;
  if (str == "local1") return SyslogFacility::LOCAL1;
  if (str == "local2") return SyslogFacility::LOCAL2;
  if (str == "local3") return SyslogFacility::LOCAL3;
  if (str == "local4") return SyslogFacility::LOCAL4;
  if (str == "local5") return SyslogFacility::LOCAL5;
  if (str == "local6") return SyslogFacility::LOCAL6;
  if (str == "local7") return SyslogFacility::LOCAL7;
  return SyslogFacility::USER;
}

std::chrono::system_clock::time_point SyslogMessage::parse_timestamp(
    const std::string& str) {
  std::tm tm_val{};
  std::istringstream ss(str);
  ss >> std::get_time(&tm_val, "%Y-%m-%d %H:%M:%S");
  if (ss.fail()) {
    return std::chrono::system_clock::now();
  }
  tm_val.tm_isdst = -1;
  std::time_t time = std::mktime(&tm_val);
  return std::chrono::system_clock::from_time_t(time);
}

SyslogMessage SyslogMessage::parse_log_line(const std::string& line) {
  SyslogMessage msg;
  msg.timestamp = std::chrono::system_clock::now();
  msg.severity = SyslogSeverity::INFO;
  msg.facility = SyslogFacility::USER;

  // Expected format: Timestamp|Severity|Facility|Source IP|Hostname|Application|Message
  std::vector<std::string> fields;
  std::string field;
  std::istringstream stream(line);

  // Split on '|' but limit to 7 fields (message may contain '|')
  int count = 0;
  while (count < 6 && std::getline(stream, field, '|')) {
    fields.push_back(field);
    count++;
  }
  // Rest of the line is the message (may contain '|')
  if (std::getline(stream, field)) {
    // Read the rest of the stream since message may contain '|'
    std::string rest;
    while (std::getline(stream, rest, '|')) {
      field += "|" + rest;
    }
    fields.push_back(field);
  }

  if (fields.size() >= 7) {
    msg.timestamp = parse_timestamp(fields[0]);
    msg.severity = parse_severity(fields[1]);
    msg.facility = parse_facility(fields[2]);
    msg.source_ip = fields[3];
    msg.hostname = fields[4];
    msg.application = fields[5];
    msg.message = fields[6];
  } else {
    // Not enough fields, treat as plain message
    msg.message = line;
  }

  return msg;
}

SyslogMessage SyslogMessage::parse_dash_log_line(const std::string& line) {
  SyslogMessage msg;
  msg.timestamp = std::chrono::system_clock::now();
  msg.severity = SyslogSeverity::INFO;
  msg.facility = SyslogFacility::USER;

  // Expected format: "YYYY-MM-DD HH:MM:SS ---SEVERITY message text"
  // Find the "---" delimiter
  auto dash_pos = line.find(" ---");
  if (dash_pos == std::string::npos) {
    msg.message = line;
    return msg;
  }

  // Parse timestamp (everything before " ---")
  std::string ts_str = line.substr(0, dash_pos);
  msg.timestamp = parse_timestamp(ts_str);

  // After "---", extract severity and message
  // Skip the " ---" (4 chars)
  std::string after_dashes = line.substr(dash_pos + 4);

  // Severity is the next token, message is the rest
  auto space_pos = after_dashes.find(' ');
  if (space_pos != std::string::npos) {
    msg.severity = parse_severity(after_dashes.substr(0, space_pos));
    msg.message = after_dashes.substr(space_pos + 1);
  } else {
    // Only severity, no message
    msg.severity = parse_severity(after_dashes);
  }

  return msg;
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

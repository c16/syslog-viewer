#include "syslog_message.h"

#include <algorithm>
#include <cctype>
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
  // Trim whitespace and control characters, then normalize to uppercase
  std::string upper = str;
  while (!upper.empty() && (unsigned char)upper.back() <= ' ') {
    upper.pop_back();
  }
  while (!upper.empty() && (unsigned char)upper.front() <= ' ') {
    upper.erase(upper.begin());
  }
  std::transform(upper.begin(), upper.end(), upper.begin(),
                 [](unsigned char c) { return std::toupper(c); });

  if (upper == "EMERG" || upper == "EMERGENCY")
    return SyslogSeverity::EMERGENCY;
  if (upper == "ALERT") return SyslogSeverity::ALERT;
  if (upper == "CRIT" || upper == "CRITICAL")
    return SyslogSeverity::CRITICAL;
  if (upper == "ERR" || upper == "ERROR") return SyslogSeverity::ERROR;
  if (upper == "WARN" || upper == "WARNING")
    return SyslogSeverity::WARNING;
  if (upper == "NOTICE") return SyslogSeverity::NOTICE;
  if (upper == "INFO" || upper == "INFORMATION")
    return SyslogSeverity::INFO;
  if (upper == "DEBUG") return SyslogSeverity::DEBUG;
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
  // Normalize the timestamp: replace 'T' with space, strip fractional
  // seconds and timezone offset so we can parse as "%Y-%m-%d %H:%M:%S"
  std::string normalized = str;

  // Replace 'T' separator with space (ISO 8601)
  auto t_pos = normalized.find('T');
  if (t_pos != std::string::npos && t_pos == 10) {
    normalized[t_pos] = ' ';
  }

  // Strip fractional seconds (.123456) and timezone (+00:00 or Z)
  // After "YYYY-MM-DD HH:MM:SS" (19 chars), truncate
  if (normalized.size() > 19) {
    normalized = normalized.substr(0, 19);
  }

  std::tm tm_val{};
  std::istringstream ss(normalized);
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

  // Read first 6 pipe-delimited fields, then treat the remainder as message
  int count = 0;
  while (count < 6 && std::getline(stream, field, '|')) {
    fields.push_back(field);
    count++;
  }
  // Rest of the line is the message (may contain '|')
  if (std::getline(stream, field)) {
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

  // Expected format: "TIMESTAMP hostname app - - - SEVERITY [message]"
  // Timestamp can be "YYYY-MM-DD HH:MM:SS" or ISO 8601 "YYYY-MM-DDTHH:MM:SS.ffffff+00:00"
  auto marker_pos = line.find(" - - - ");
  if (marker_pos == std::string::npos) {
    msg.message = line;
    return msg;
  }

  // Parse the part before " - - - " by splitting on spaces
  std::string before = line.substr(0, marker_pos);
  std::vector<std::string> tokens;
  std::istringstream bstream(before);
  std::string token;
  while (bstream >> token) {
    tokens.push_back(token);
  }

  if (!tokens.empty()) {
    // Determine if first token is ISO 8601 with 'T' (single-token timestamp)
    // or old format where date and time are separate tokens
    if (tokens[0].find('T') != std::string::npos && tokens[0].size() > 10) {
      // ISO 8601: "2026-01-15T14:33:02.756342+00:00 hostname app"
      msg.timestamp = parse_timestamp(tokens[0]);
      if (tokens.size() > 1) msg.hostname = tokens[1];
      if (tokens.size() > 2) msg.application = tokens[2];
    } else if (tokens.size() >= 2 && tokens[1].find(':') != std::string::npos) {
      // Space-separated: "2026-01-15 14:33:02 hostname app"
      msg.timestamp = parse_timestamp(tokens[0] + " " + tokens[1]);
      if (tokens.size() > 2) msg.hostname = tokens[2];
      if (tokens.size() > 3) msg.application = tokens[3];
    } else {
      // Unknown timestamp format, treat first token as timestamp
      msg.timestamp = parse_timestamp(tokens[0]);
      if (tokens.size() > 1) msg.hostname = tokens[1];
      if (tokens.size() > 2) msg.application = tokens[2];
    }
  }

  // Parse the part after " - - - ": "SEVERITY [message]"
  std::string after = line.substr(marker_pos + 7);  // skip " - - - "

  auto space_pos = after.find(' ');
  if (space_pos != std::string::npos) {
    msg.severity = parse_severity(after.substr(0, space_pos));
    msg.message = after.substr(space_pos + 1);
  } else {
    msg.severity = parse_severity(after);
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

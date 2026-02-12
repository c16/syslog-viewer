#include "syslog_message.h"

#include <gtest/gtest.h>

// --- severity_string ---

TEST(SyslogMessageSeverityString, AllLevels) {
  SyslogMessage msg;
  msg.severity = SyslogSeverity::EMERGENCY;
  EXPECT_EQ(msg.severity_string(), "EMERG");
  msg.severity = SyslogSeverity::ALERT;
  EXPECT_EQ(msg.severity_string(), "ALERT");
  msg.severity = SyslogSeverity::CRITICAL;
  EXPECT_EQ(msg.severity_string(), "CRIT");
  msg.severity = SyslogSeverity::ERROR;
  EXPECT_EQ(msg.severity_string(), "ERROR");
  msg.severity = SyslogSeverity::WARNING;
  EXPECT_EQ(msg.severity_string(), "WARN");
  msg.severity = SyslogSeverity::NOTICE;
  EXPECT_EQ(msg.severity_string(), "NOTICE");
  msg.severity = SyslogSeverity::INFO;
  EXPECT_EQ(msg.severity_string(), "INFO");
  msg.severity = SyslogSeverity::DEBUG;
  EXPECT_EQ(msg.severity_string(), "DEBUG");
}

// --- facility_string ---

TEST(SyslogMessageFacilityString, AllFacilities) {
  SyslogMessage msg;
  msg.facility = SyslogFacility::KERN;
  EXPECT_EQ(msg.facility_string(), "kern");
  msg.facility = SyslogFacility::USER;
  EXPECT_EQ(msg.facility_string(), "user");
  msg.facility = SyslogFacility::MAIL;
  EXPECT_EQ(msg.facility_string(), "mail");
  msg.facility = SyslogFacility::DAEMON;
  EXPECT_EQ(msg.facility_string(), "daemon");
  msg.facility = SyslogFacility::AUTH;
  EXPECT_EQ(msg.facility_string(), "auth");
  msg.facility = SyslogFacility::SYSLOG;
  EXPECT_EQ(msg.facility_string(), "syslog");
  msg.facility = SyslogFacility::LPR;
  EXPECT_EQ(msg.facility_string(), "lpr");
  msg.facility = SyslogFacility::NEWS;
  EXPECT_EQ(msg.facility_string(), "news");
  msg.facility = SyslogFacility::UUCP;
  EXPECT_EQ(msg.facility_string(), "uucp");
  msg.facility = SyslogFacility::CRON;
  EXPECT_EQ(msg.facility_string(), "cron");
  msg.facility = SyslogFacility::AUTHPRIV;
  EXPECT_EQ(msg.facility_string(), "authpriv");
  msg.facility = SyslogFacility::FTP;
  EXPECT_EQ(msg.facility_string(), "ftp");
  msg.facility = SyslogFacility::LOCAL0;
  EXPECT_EQ(msg.facility_string(), "local0");
  msg.facility = SyslogFacility::LOCAL7;
  EXPECT_EQ(msg.facility_string(), "local7");
}

// --- priority ---

TEST(SyslogMessagePriority, Calculation) {
  SyslogMessage msg;
  // Priority = facility * 8 + severity
  // local0(16) * 8 + info(6) = 134
  msg.facility = SyslogFacility::LOCAL0;
  msg.severity = SyslogSeverity::INFO;
  EXPECT_EQ(msg.priority(), 134);

  // kern(0) * 8 + emergency(0) = 0
  msg.facility = SyslogFacility::KERN;
  msg.severity = SyslogSeverity::EMERGENCY;
  EXPECT_EQ(msg.priority(), 0);

  // user(1) * 8 + error(3) = 11
  msg.facility = SyslogFacility::USER;
  msg.severity = SyslogSeverity::ERROR;
  EXPECT_EQ(msg.priority(), 11);

  // auth(4) * 8 + warning(4) = 36
  msg.facility = SyslogFacility::AUTH;
  msg.severity = SyslogSeverity::WARNING;
  EXPECT_EQ(msg.priority(), 36);
}

// --- timestamp_string ---

TEST(SyslogMessageTimestampString, FormatsCorrectly) {
  SyslogMessage msg;
  // Set a known timestamp: 2026-01-15 10:30:00
  std::tm tm_val{};
  tm_val.tm_year = 126;  // 2026 - 1900
  tm_val.tm_mon = 0;     // January
  tm_val.tm_mday = 15;
  tm_val.tm_hour = 10;
  tm_val.tm_min = 30;
  tm_val.tm_sec = 0;
  tm_val.tm_isdst = -1;
  msg.timestamp = std::chrono::system_clock::from_time_t(std::mktime(&tm_val));

  EXPECT_EQ(msg.timestamp_string(), "2026-01-15 10:30:00");
}

// --- parse_severity ---

TEST(ParseSeverity, AllKnownValues) {
  EXPECT_EQ(SyslogMessage::parse_severity("EMERG"), SyslogSeverity::EMERGENCY);
  EXPECT_EQ(SyslogMessage::parse_severity("ALERT"), SyslogSeverity::ALERT);
  EXPECT_EQ(SyslogMessage::parse_severity("CRIT"), SyslogSeverity::CRITICAL);
  EXPECT_EQ(SyslogMessage::parse_severity("ERROR"), SyslogSeverity::ERROR);
  EXPECT_EQ(SyslogMessage::parse_severity("WARN"), SyslogSeverity::WARNING);
  EXPECT_EQ(SyslogMessage::parse_severity("NOTICE"), SyslogSeverity::NOTICE);
  EXPECT_EQ(SyslogMessage::parse_severity("INFO"), SyslogSeverity::INFO);
  EXPECT_EQ(SyslogMessage::parse_severity("DEBUG"), SyslogSeverity::DEBUG);
}

TEST(ParseSeverity, CaseInsensitive) {
  EXPECT_EQ(SyslogMessage::parse_severity("error"), SyslogSeverity::ERROR);
  EXPECT_EQ(SyslogMessage::parse_severity("Error"), SyslogSeverity::ERROR);
  EXPECT_EQ(SyslogMessage::parse_severity("warn"), SyslogSeverity::WARNING);
  EXPECT_EQ(SyslogMessage::parse_severity("Warning"), SyslogSeverity::WARNING);
  EXPECT_EQ(SyslogMessage::parse_severity("info"), SyslogSeverity::INFO);
  EXPECT_EQ(SyslogMessage::parse_severity("debug"), SyslogSeverity::DEBUG);
  EXPECT_EQ(SyslogMessage::parse_severity("crit"), SyslogSeverity::CRITICAL);
  EXPECT_EQ(SyslogMessage::parse_severity("alert"), SyslogSeverity::ALERT);
  EXPECT_EQ(SyslogMessage::parse_severity("emerg"), SyslogSeverity::EMERGENCY);
  EXPECT_EQ(SyslogMessage::parse_severity("notice"), SyslogSeverity::NOTICE);
}

TEST(ParseSeverity, LongForms) {
  EXPECT_EQ(SyslogMessage::parse_severity("EMERGENCY"), SyslogSeverity::EMERGENCY);
  EXPECT_EQ(SyslogMessage::parse_severity("CRITICAL"), SyslogSeverity::CRITICAL);
  EXPECT_EQ(SyslogMessage::parse_severity("WARNING"), SyslogSeverity::WARNING);
  EXPECT_EQ(SyslogMessage::parse_severity("INFORMATION"), SyslogSeverity::INFO);
  EXPECT_EQ(SyslogMessage::parse_severity("ERR"), SyslogSeverity::ERROR);
  // Case-insensitive long forms
  EXPECT_EQ(SyslogMessage::parse_severity("Emergency"), SyslogSeverity::EMERGENCY);
  EXPECT_EQ(SyslogMessage::parse_severity("Critical"), SyslogSeverity::CRITICAL);
  EXPECT_EQ(SyslogMessage::parse_severity("Warning"), SyslogSeverity::WARNING);
}

TEST(ParseSeverity, TrimsWhitespaceAndControlChars) {
  EXPECT_EQ(SyslogMessage::parse_severity("ERROR\r"), SyslogSeverity::ERROR);
  EXPECT_EQ(SyslogMessage::parse_severity("ERROR\n"), SyslogSeverity::ERROR);
  EXPECT_EQ(SyslogMessage::parse_severity("ERROR "), SyslogSeverity::ERROR);
  EXPECT_EQ(SyslogMessage::parse_severity(" ERROR"), SyslogSeverity::ERROR);
  EXPECT_EQ(SyslogMessage::parse_severity(" ERROR "), SyslogSeverity::ERROR);
  EXPECT_EQ(SyslogMessage::parse_severity("WARN\r\n"), SyslogSeverity::WARNING);
  EXPECT_EQ(SyslogMessage::parse_severity("\tDEBUG\t"), SyslogSeverity::DEBUG);
}

TEST(ParseSeverity, UnknownDefaultsToInfo) {
  EXPECT_EQ(SyslogMessage::parse_severity("GARBAGE"), SyslogSeverity::INFO);
  EXPECT_EQ(SyslogMessage::parse_severity(""), SyslogSeverity::INFO);
}

// --- parse_facility ---

TEST(ParseFacility, AllKnownValues) {
  EXPECT_EQ(SyslogMessage::parse_facility("kern"), SyslogFacility::KERN);
  EXPECT_EQ(SyslogMessage::parse_facility("user"), SyslogFacility::USER);
  EXPECT_EQ(SyslogMessage::parse_facility("mail"), SyslogFacility::MAIL);
  EXPECT_EQ(SyslogMessage::parse_facility("daemon"), SyslogFacility::DAEMON);
  EXPECT_EQ(SyslogMessage::parse_facility("auth"), SyslogFacility::AUTH);
  EXPECT_EQ(SyslogMessage::parse_facility("syslog"), SyslogFacility::SYSLOG);
  EXPECT_EQ(SyslogMessage::parse_facility("lpr"), SyslogFacility::LPR);
  EXPECT_EQ(SyslogMessage::parse_facility("news"), SyslogFacility::NEWS);
  EXPECT_EQ(SyslogMessage::parse_facility("uucp"), SyslogFacility::UUCP);
  EXPECT_EQ(SyslogMessage::parse_facility("cron"), SyslogFacility::CRON);
  EXPECT_EQ(SyslogMessage::parse_facility("authpriv"), SyslogFacility::AUTHPRIV);
  EXPECT_EQ(SyslogMessage::parse_facility("ftp"), SyslogFacility::FTP);
  EXPECT_EQ(SyslogMessage::parse_facility("local0"), SyslogFacility::LOCAL0);
  EXPECT_EQ(SyslogMessage::parse_facility("local7"), SyslogFacility::LOCAL7);
}

TEST(ParseFacility, UnknownDefaultsToUser) {
  EXPECT_EQ(SyslogMessage::parse_facility("GARBAGE"), SyslogFacility::USER);
  EXPECT_EQ(SyslogMessage::parse_facility(""), SyslogFacility::USER);
}

// --- parse_timestamp ---

TEST(ParseTimestamp, ValidTimestamp) {
  auto tp = SyslogMessage::parse_timestamp("2026-03-15 14:30:00");
  auto time_t_val = std::chrono::system_clock::to_time_t(tp);
  std::tm tm_val{};
  localtime_r(&time_t_val, &tm_val);

  EXPECT_EQ(tm_val.tm_year, 126);  // 2026 - 1900
  EXPECT_EQ(tm_val.tm_mon, 2);     // March (0-indexed)
  EXPECT_EQ(tm_val.tm_mday, 15);
  EXPECT_EQ(tm_val.tm_hour, 14);
  EXPECT_EQ(tm_val.tm_min, 30);
  EXPECT_EQ(tm_val.tm_sec, 0);
}

TEST(ParseTimestamp, ISO8601WithFractionalAndTimezone) {
  // +00:00 means UTC; verify using gmtime_r for timezone-independent check
  auto tp = SyslogMessage::parse_timestamp("2026-01-15T14:33:02.756342+00:00");
  auto time_t_val = std::chrono::system_clock::to_time_t(tp);
  std::tm tm_val{};
  gmtime_r(&time_t_val, &tm_val);

  EXPECT_EQ(tm_val.tm_year, 126);  // 2026 - 1900
  EXPECT_EQ(tm_val.tm_mon, 0);     // January (0-indexed)
  EXPECT_EQ(tm_val.tm_mday, 15);
  EXPECT_EQ(tm_val.tm_hour, 14);
  EXPECT_EQ(tm_val.tm_min, 33);
  EXPECT_EQ(tm_val.tm_sec, 2);
}

TEST(ParseTimestamp, ISO8601WithPositiveOffset) {
  // +05:30 means local is 5h30m ahead of UTC; 14:33:02+05:30 = 09:03:02 UTC
  auto tp = SyslogMessage::parse_timestamp("2026-01-15T14:33:02.000000+05:30");
  auto time_t_val = std::chrono::system_clock::to_time_t(tp);
  std::tm tm_val{};
  gmtime_r(&time_t_val, &tm_val);

  EXPECT_EQ(tm_val.tm_year, 126);
  EXPECT_EQ(tm_val.tm_mon, 0);
  EXPECT_EQ(tm_val.tm_mday, 15);
  EXPECT_EQ(tm_val.tm_hour, 9);
  EXPECT_EQ(tm_val.tm_min, 3);
  EXPECT_EQ(tm_val.tm_sec, 2);
}

TEST(ParseTimestamp, ISO8601WithNegativeOffset) {
  // -05:00 means local is 5h behind UTC; 14:33:02-05:00 = 19:33:02 UTC
  auto tp = SyslogMessage::parse_timestamp("2026-01-15T14:33:02.000000-05:00");
  auto time_t_val = std::chrono::system_clock::to_time_t(tp);
  std::tm tm_val{};
  gmtime_r(&time_t_val, &tm_val);

  EXPECT_EQ(tm_val.tm_year, 126);
  EXPECT_EQ(tm_val.tm_mon, 0);
  EXPECT_EQ(tm_val.tm_mday, 15);
  EXPECT_EQ(tm_val.tm_hour, 19);
  EXPECT_EQ(tm_val.tm_min, 33);
  EXPECT_EQ(tm_val.tm_sec, 2);
}

TEST(ParseTimestamp, ISO8601WithZulu) {
  // Z means UTC
  auto tp = SyslogMessage::parse_timestamp("2026-01-15T14:33:02Z");
  auto time_t_val = std::chrono::system_clock::to_time_t(tp);
  std::tm tm_val{};
  gmtime_r(&time_t_val, &tm_val);

  EXPECT_EQ(tm_val.tm_year, 126);
  EXPECT_EQ(tm_val.tm_mon, 0);
  EXPECT_EQ(tm_val.tm_mday, 15);
  EXPECT_EQ(tm_val.tm_hour, 14);
  EXPECT_EQ(tm_val.tm_min, 33);
  EXPECT_EQ(tm_val.tm_sec, 2);
}

TEST(ParseTimestamp, ISO8601WithTSeparator) {
  auto tp = SyslogMessage::parse_timestamp("2026-06-20T08:15:30");
  auto time_t_val = std::chrono::system_clock::to_time_t(tp);
  std::tm tm_val{};
  localtime_r(&time_t_val, &tm_val);

  EXPECT_EQ(tm_val.tm_year, 126);
  EXPECT_EQ(tm_val.tm_mon, 5);     // June
  EXPECT_EQ(tm_val.tm_mday, 20);
  EXPECT_EQ(tm_val.tm_hour, 8);
  EXPECT_EQ(tm_val.tm_min, 15);
  EXPECT_EQ(tm_val.tm_sec, 30);
}

TEST(ParseTimestamp, InvalidTimestampReturnsNow) {
  auto before = std::chrono::system_clock::now();
  auto tp = SyslogMessage::parse_timestamp("not-a-timestamp");
  auto after = std::chrono::system_clock::now();

  EXPECT_GE(tp, before);
  EXPECT_LE(tp, after);
}

// --- parse (RFC3164) ---

TEST(ParseRFC3164, FullMessage) {
  auto msg = SyslogMessage::parse(
      "<134>Jan 11 10:30:00 webserver nginx[1234]: GET /index.html 200",
      "192.168.1.100");

  // Priority 134 = local0(16) * 8 + info(6)
  EXPECT_EQ(msg.severity, SyslogSeverity::INFO);
  EXPECT_EQ(msg.facility, SyslogFacility::LOCAL0);
  EXPECT_EQ(msg.hostname, "webserver");
  EXPECT_EQ(msg.application, "nginx");
  EXPECT_EQ(msg.process_id, "1234");
  EXPECT_EQ(msg.message, "GET /index.html 200");
  EXPECT_EQ(msg.source_ip, "192.168.1.100");
}

TEST(ParseRFC3164, WithoutPid) {
  auto msg = SyslogMessage::parse(
      "<38>Jan  5 08:00:00 myhost myapp: Something happened", "10.0.0.1");

  // Priority 38 = auth(4) * 8 + info(6)
  EXPECT_EQ(msg.severity, SyslogSeverity::INFO);
  EXPECT_EQ(msg.facility, SyslogFacility::AUTH);
  EXPECT_EQ(msg.hostname, "myhost");
  EXPECT_EQ(msg.application, "myapp");
  EXPECT_TRUE(msg.process_id.empty());
  EXPECT_EQ(msg.message, "Something happened");
}

TEST(ParseRFC3164, PriorityExtractsCorrectly) {
  // Priority 0 = kern(0) * 8 + emerg(0)
  auto msg = SyslogMessage::parse(
      "<0>Jan  1 00:00:00 host app: kernel emergency", "1.2.3.4");
  EXPECT_EQ(msg.severity, SyslogSeverity::EMERGENCY);
  EXPECT_EQ(msg.facility, SyslogFacility::KERN);

  // Priority 11 = user(1) * 8 + error(3)
  msg = SyslogMessage::parse(
      "<11>Jan  1 00:00:00 host app: user error", "1.2.3.4");
  EXPECT_EQ(msg.severity, SyslogSeverity::ERROR);
  EXPECT_EQ(msg.facility, SyslogFacility::USER);
}

// --- parse (simple format) ---

TEST(ParseSimple, HostnameAndMessage) {
  auto msg = SyslogMessage::parse("myhost some plain message", "10.0.0.5");

  EXPECT_EQ(msg.hostname, "myhost");
  EXPECT_EQ(msg.message, "some plain message");
  EXPECT_EQ(msg.source_ip, "10.0.0.5");
  // Defaults
  EXPECT_EQ(msg.severity, SyslogSeverity::INFO);
  EXPECT_EQ(msg.facility, SyslogFacility::USER);
}

TEST(ParseSimple, EmptyMessage) {
  auto msg = SyslogMessage::parse("", "10.0.0.1");

  EXPECT_EQ(msg.message, "");
  EXPECT_EQ(msg.source_ip, "10.0.0.1");
  EXPECT_EQ(msg.severity, SyslogSeverity::INFO);
  EXPECT_EQ(msg.facility, SyslogFacility::USER);
}

TEST(ParseSimple, NoPriorityNoRFC) {
  auto msg = SyslogMessage::parse("just-a-single-word", "1.1.1.1");
  // Single word with no space => regex won't match simple format either
  // Falls through to msg.message = remaining
  EXPECT_EQ(msg.message, "just-a-single-word");
}

// --- parse_dash_log_line (dash-delimited) ---
// Format: "YYYY-MM-DD HH:MM:SS hostname app - - - SEVERITY [message]"

TEST(ParseDashLogLine, BasicErrorLine) {
  auto msg = SyslogMessage::parse_dash_log_line(
      "2026-01-11 15:30:45 buildroot ssb-mk2 - - - ERROR");

  EXPECT_EQ(msg.severity, SyslogSeverity::ERROR);
  EXPECT_EQ(msg.hostname, "buildroot");
  EXPECT_EQ(msg.application, "ssb-mk2");
  EXPECT_EQ(msg.timestamp_string(), "2026-01-11 15:30:45");
  EXPECT_EQ(msg.facility, SyslogFacility::USER);
  EXPECT_TRUE(msg.message.empty());
}

TEST(ParseDashLogLine, WithMessage) {
  auto msg = SyslogMessage::parse_dash_log_line(
      "2026-03-20 08:00:00 webserver nginx - - - INFO Server started successfully");

  EXPECT_EQ(msg.severity, SyslogSeverity::INFO);
  EXPECT_EQ(msg.hostname, "webserver");
  EXPECT_EQ(msg.application, "nginx");
  EXPECT_EQ(msg.message, "Server started successfully");
  EXPECT_EQ(msg.timestamp_string(), "2026-03-20 08:00:00");
}

TEST(ParseDashLogLine, AllSeverities) {
  auto test = [](const std::string& sev, SyslogSeverity expected) {
    auto msg = SyslogMessage::parse_dash_log_line(
        "2026-01-01 00:00:00 host app - - - " + sev + " test message");
    EXPECT_EQ(msg.severity, expected) << "Failed for severity: " << sev;
    EXPECT_EQ(msg.message, "test message");
    EXPECT_EQ(msg.hostname, "host");
    EXPECT_EQ(msg.application, "app");
  };

  test("EMERG", SyslogSeverity::EMERGENCY);
  test("ALERT", SyslogSeverity::ALERT);
  test("CRIT", SyslogSeverity::CRITICAL);
  test("ERROR", SyslogSeverity::ERROR);
  test("WARN", SyslogSeverity::WARNING);
  test("NOTICE", SyslogSeverity::NOTICE);
  test("INFO", SyslogSeverity::INFO);
  test("DEBUG", SyslogSeverity::DEBUG);
}

TEST(ParseDashLogLine, SeverityOnly) {
  auto msg = SyslogMessage::parse_dash_log_line(
      "2026-01-01 00:00:00 myhost myapp - - - WARN");

  EXPECT_EQ(msg.severity, SyslogSeverity::WARNING);
  EXPECT_EQ(msg.hostname, "myhost");
  EXPECT_EQ(msg.application, "myapp");
  EXPECT_TRUE(msg.message.empty());
}

TEST(ParseDashLogLine, MessageWithSpaces) {
  auto msg = SyslogMessage::parse_dash_log_line(
      "2026-06-15 12:30:00 gateway proxy - - - DEBUG GET /api/users?page=1 HTTP/1.1 200 OK");

  EXPECT_EQ(msg.severity, SyslogSeverity::DEBUG);
  EXPECT_EQ(msg.message, "GET /api/users?page=1 HTTP/1.1 200 OK");
}

TEST(ParseDashLogLine, CaseInsensitiveSeverity) {
  auto msg = SyslogMessage::parse_dash_log_line(
      "2026-01-01 00:00:00 host app - - - error connection lost");
  EXPECT_EQ(msg.severity, SyslogSeverity::ERROR);
  EXPECT_EQ(msg.message, "connection lost");

  msg = SyslogMessage::parse_dash_log_line(
      "2026-01-01 00:00:00 host app - - - Warning disk almost full");
  EXPECT_EQ(msg.severity, SyslogSeverity::WARNING);
  EXPECT_EQ(msg.message, "disk almost full");
}

TEST(ParseDashLogLine, LongFormSeverity) {
  auto msg = SyslogMessage::parse_dash_log_line(
      "2026-01-01 00:00:00 host app - - - CRITICAL system failure");
  EXPECT_EQ(msg.severity, SyslogSeverity::CRITICAL);

  msg = SyslogMessage::parse_dash_log_line(
      "2026-01-01 00:00:00 host app - - - WARNING low memory");
  EXPECT_EQ(msg.severity, SyslogSeverity::WARNING);

  msg = SyslogMessage::parse_dash_log_line(
      "2026-01-01 00:00:00 host app - - - EMERGENCY kernel panic");
  EXPECT_EQ(msg.severity, SyslogSeverity::EMERGENCY);
}

TEST(ParseDashLogLine, TrailingCarriageReturn) {
  // Simulates Windows line endings where \r remains after getline
  auto msg = SyslogMessage::parse_dash_log_line(
      "2026-01-11 15:30:45 buildroot ssb-mk2 - - - ERROR\r");
  EXPECT_EQ(msg.severity, SyslogSeverity::ERROR);
  EXPECT_EQ(msg.hostname, "buildroot");
  EXPECT_EQ(msg.application, "ssb-mk2");
}

TEST(ParseDashLogLine, TrailingWhitespace) {
  auto msg = SyslogMessage::parse_dash_log_line(
      "2026-01-11 15:30:45 buildroot ssb-mk2 - - - WARN  ");
  EXPECT_EQ(msg.severity, SyslogSeverity::WARNING);
}

TEST(ParseDashLogLine, ISO8601Timestamp) {
  auto msg = SyslogMessage::parse_dash_log_line(
      "2026-01-15T14:33:02.756342+00:00 buildroot ssb-mk2 - - - ERROR something failed");

  EXPECT_EQ(msg.severity, SyslogSeverity::ERROR);
  EXPECT_EQ(msg.hostname, "buildroot");
  EXPECT_EQ(msg.application, "ssb-mk2");
  EXPECT_EQ(msg.message, "something failed");

  // Verify UTC time directly (timestamp_string uses local time)
  auto time_t_val = std::chrono::system_clock::to_time_t(msg.timestamp);
  std::tm tm_val{};
  gmtime_r(&time_t_val, &tm_val);
  EXPECT_EQ(tm_val.tm_hour, 14);
  EXPECT_EQ(tm_val.tm_min, 33);
  EXPECT_EQ(tm_val.tm_sec, 2);
}

TEST(ParseDashLogLine, ISO8601TimestampNoMessage) {
  auto msg = SyslogMessage::parse_dash_log_line(
      "2026-01-15T14:33:02.756342+00:00 buildroot ssb-mk2 - - - ERROR");

  EXPECT_EQ(msg.severity, SyslogSeverity::ERROR);
  EXPECT_EQ(msg.hostname, "buildroot");
  EXPECT_EQ(msg.application, "ssb-mk2");
  EXPECT_TRUE(msg.message.empty());

  // Verify UTC time directly
  auto time_t_val = std::chrono::system_clock::to_time_t(msg.timestamp);
  std::tm tm_val{};
  gmtime_r(&time_t_val, &tm_val);
  EXPECT_EQ(tm_val.tm_hour, 14);
  EXPECT_EQ(tm_val.tm_min, 33);
}

TEST(ParseDashLogLine, NoDashDelimiter) {
  auto msg = SyslogMessage::parse_dash_log_line("no dashes here at all");

  // Falls back to storing whole line as message
  EXPECT_EQ(msg.message, "no dashes here at all");
  EXPECT_EQ(msg.severity, SyslogSeverity::INFO);
}

// --- parse_log_line (pipe-delimited) ---

TEST(ParseLogLine, ValidLine) {
  auto msg = SyslogMessage::parse_log_line(
      "2026-01-11 15:30:45|INFO|user|192.168.1.100|webserver|nginx|GET /index.html 200");

  EXPECT_EQ(msg.severity, SyslogSeverity::INFO);
  EXPECT_EQ(msg.facility, SyslogFacility::USER);
  EXPECT_EQ(msg.source_ip, "192.168.1.100");
  EXPECT_EQ(msg.hostname, "webserver");
  EXPECT_EQ(msg.application, "nginx");
  EXPECT_EQ(msg.message, "GET /index.html 200");
  EXPECT_EQ(msg.timestamp_string(), "2026-01-11 15:30:45");
}

TEST(ParseLogLine, MessageContainsPipe) {
  auto msg = SyslogMessage::parse_log_line(
      "2026-06-01 12:00:00|ERROR|daemon|10.0.0.1|host|app|msg with | pipes | inside");

  EXPECT_EQ(msg.severity, SyslogSeverity::ERROR);
  EXPECT_EQ(msg.facility, SyslogFacility::DAEMON);
  EXPECT_EQ(msg.message, "msg with | pipes | inside");
}

TEST(ParseLogLine, AllSeverities) {
  std::string base = "2026-01-01 00:00:00|%s|user|0.0.0.0|h|a|m";
  auto test = [](const std::string& sev, SyslogSeverity expected) {
    auto msg = SyslogMessage::parse_log_line(
        "2026-01-01 00:00:00|" + sev + "|user|0.0.0.0|h|a|m");
    EXPECT_EQ(msg.severity, expected) << "Failed for severity: " << sev;
  };

  test("EMERG", SyslogSeverity::EMERGENCY);
  test("ALERT", SyslogSeverity::ALERT);
  test("CRIT", SyslogSeverity::CRITICAL);
  test("ERROR", SyslogSeverity::ERROR);
  test("WARN", SyslogSeverity::WARNING);
  test("NOTICE", SyslogSeverity::NOTICE);
  test("INFO", SyslogSeverity::INFO);
  test("DEBUG", SyslogSeverity::DEBUG);
}

TEST(ParseLogLine, TooFewFields) {
  auto msg = SyslogMessage::parse_log_line("only|three|fields");

  // Not enough fields; falls back to storing the whole line as message
  EXPECT_EQ(msg.message, "only|three|fields");
  EXPECT_EQ(msg.severity, SyslogSeverity::INFO);
  EXPECT_EQ(msg.facility, SyslogFacility::USER);
}

TEST(ParseLogLine, EmptyLine) {
  auto msg = SyslogMessage::parse_log_line("");

  EXPECT_EQ(msg.message, "");
  EXPECT_EQ(msg.severity, SyslogSeverity::INFO);
}

// --- Roundtrip: severity_string <-> parse_severity ---

TEST(Roundtrip, SeverityStringRoundtrip) {
  SyslogMessage msg;
  SyslogSeverity severities[] = {
      SyslogSeverity::EMERGENCY, SyslogSeverity::ALERT,
      SyslogSeverity::CRITICAL,  SyslogSeverity::ERROR,
      SyslogSeverity::WARNING,   SyslogSeverity::NOTICE,
      SyslogSeverity::INFO,      SyslogSeverity::DEBUG};

  for (auto sev : severities) {
    msg.severity = sev;
    std::string str = msg.severity_string();
    EXPECT_EQ(SyslogMessage::parse_severity(str), sev)
        << "Roundtrip failed for: " << str;
  }
}

TEST(Roundtrip, FacilityStringRoundtrip) {
  SyslogMessage msg;
  SyslogFacility facilities[] = {
      SyslogFacility::KERN,     SyslogFacility::USER,
      SyslogFacility::MAIL,     SyslogFacility::DAEMON,
      SyslogFacility::AUTH,     SyslogFacility::SYSLOG,
      SyslogFacility::LPR,     SyslogFacility::NEWS,
      SyslogFacility::UUCP,    SyslogFacility::CRON,
      SyslogFacility::AUTHPRIV, SyslogFacility::FTP,
      SyslogFacility::LOCAL0,  SyslogFacility::LOCAL1,
      SyslogFacility::LOCAL2,  SyslogFacility::LOCAL3,
      SyslogFacility::LOCAL4,  SyslogFacility::LOCAL5,
      SyslogFacility::LOCAL6,  SyslogFacility::LOCAL7};

  for (auto fac : facilities) {
    msg.facility = fac;
    std::string str = msg.facility_string();
    EXPECT_EQ(SyslogMessage::parse_facility(str), fac)
        << "Roundtrip failed for: " << str;
  }
}

// --- Roundtrip: timestamp_string <-> parse_timestamp ---

TEST(Roundtrip, TimestampRoundtrip) {
  SyslogMessage msg;
  std::tm tm_val{};
  tm_val.tm_year = 126;
  tm_val.tm_mon = 5;
  tm_val.tm_mday = 20;
  tm_val.tm_hour = 8;
  tm_val.tm_min = 15;
  tm_val.tm_sec = 30;
  tm_val.tm_isdst = -1;
  msg.timestamp = std::chrono::system_clock::from_time_t(std::mktime(&tm_val));

  std::string ts_str = msg.timestamp_string();
  EXPECT_EQ(ts_str, "2026-06-20 08:15:30");

  auto parsed_tp = SyslogMessage::parse_timestamp(ts_str);
  auto parsed_time_t = std::chrono::system_clock::to_time_t(parsed_tp);
  auto orig_time_t = std::chrono::system_clock::to_time_t(msg.timestamp);
  EXPECT_EQ(parsed_time_t, orig_time_t);
}

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

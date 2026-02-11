#include "udp_listener.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <condition_variable>
#include <gtest/gtest.h>
#include <mutex>
#include <thread>

// --- Constructor and port config ---

TEST(UdpListener, DefaultPort) {
  UdpListener listener;
  EXPECT_EQ(listener.get_port(), 514);
}

TEST(UdpListener, CustomPort) {
  UdpListener listener(8514);
  EXPECT_EQ(listener.get_port(), 8514);
}

TEST(UdpListener, SetPortWhenStopped) {
  UdpListener listener(1000);
  listener.set_port(2000);
  EXPECT_EQ(listener.get_port(), 2000);
}

TEST(UdpListener, InitiallyNotListening) {
  UdpListener listener;
  EXPECT_FALSE(listener.is_listening());
}

// --- Start/stop lifecycle ---

TEST(UdpListener, StartAndStop) {
  UdpListener listener(0);

  bool received = false;
  listener.start([&](const SyslogMessage&) { received = true; });
  EXPECT_TRUE(listener.is_listening());
  EXPECT_GT(listener.get_port(), 0);

  listener.stop();
  EXPECT_FALSE(listener.is_listening());
}

TEST(UdpListener, StopWhenNotListening) {
  UdpListener listener;
  // Should not crash or throw
  listener.stop();
  EXPECT_FALSE(listener.is_listening());
}

TEST(UdpListener, DoubleStartIgnored) {
  UdpListener listener(0);

  listener.start([](const SyslogMessage&) {});
  EXPECT_TRUE(listener.is_listening());

  // Second start should be a no-op
  listener.start([](const SyslogMessage&) {});
  EXPECT_TRUE(listener.is_listening());

  listener.stop();
}

TEST(UdpListener, SetPortIgnoredWhileListening) {
  UdpListener listener(0);

  listener.start([](const SyslogMessage&) {});
  int bound_port = listener.get_port();
  listener.set_port(9999);
  EXPECT_EQ(listener.get_port(), bound_port);  // Should not change

  listener.stop();
}

// --- Message reception ---

TEST(UdpListener, ReceivesUdpMessage) {
  UdpListener listener(0);

  std::mutex mtx;
  std::condition_variable cv;
  SyslogMessage received_msg;
  bool got_message = false;

  listener.start([&](const SyslogMessage& msg) {
    std::lock_guard<std::mutex> lock(mtx);
    received_msg = msg;
    got_message = true;
    cv.notify_one();
  });

  int bound_port = listener.get_port();

  // Send a UDP message to the listener
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  ASSERT_GE(sock, 0);

  struct sockaddr_in addr {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(bound_port);
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

  std::string raw = "<134>Jan 11 10:30:00 testhost testapp[42]: Hello from test";
  sendto(sock, raw.c_str(), raw.size(), 0, (struct sockaddr*)&addr,
         sizeof(addr));
  close(sock);

  // Wait for the message to be received
  {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait_for(lock, std::chrono::seconds(3), [&] { return got_message; });
  }

  EXPECT_TRUE(got_message);
  EXPECT_EQ(received_msg.hostname, "testhost");
  EXPECT_EQ(received_msg.application, "testapp");
  EXPECT_EQ(received_msg.process_id, "42");
  EXPECT_EQ(received_msg.message, "Hello from test");
  EXPECT_EQ(received_msg.severity, SyslogSeverity::INFO);
  EXPECT_EQ(received_msg.facility, SyslogFacility::LOCAL0);
  EXPECT_EQ(received_msg.source_ip, "127.0.0.1");

  listener.stop();
}

TEST(UdpListener, ReceivesMultipleMessages) {
  UdpListener listener(0);

  std::mutex mtx;
  std::condition_variable cv;
  int count = 0;

  listener.start([&](const SyslogMessage&) {
    std::lock_guard<std::mutex> lock(mtx);
    count++;
    cv.notify_one();
  });

  int bound_port = listener.get_port();

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  ASSERT_GE(sock, 0);

  struct sockaddr_in addr {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(bound_port);
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

  for (int i = 0; i < 5; i++) {
    std::string raw = "<134>Jan 11 10:30:00 host app: message " + std::to_string(i);
    sendto(sock, raw.c_str(), raw.size(), 0, (struct sockaddr*)&addr,
           sizeof(addr));
  }
  close(sock);

  // Wait for all messages
  {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait_for(lock, std::chrono::seconds(3), [&] { return count >= 5; });
  }

  EXPECT_EQ(count, 5);
  listener.stop();
}

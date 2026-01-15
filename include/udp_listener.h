#ifndef UDP_LISTENER_H
#define UDP_LISTENER_H

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "syslog_message.h"

class UdpListener {
 public:
  using MessageCallback = std::function<void(const SyslogMessage&)>;

  UdpListener(int port = 514);
  ~UdpListener();

  // Start listening for messages
  void start(MessageCallback callback);

  // Stop listening
  void stop();

  // Check if listening
  bool is_listening() const;

  // Get current port
  int get_port() const;

  // Set port (only when stopped)
  void set_port(int port);

 private:
  void listen_thread();

  int port_;
  int socket_fd_;
  std::atomic<bool> running_;
  std::unique_ptr<std::thread> thread_;
  MessageCallback callback_;
};

#endif  // UDP_LISTENER_H

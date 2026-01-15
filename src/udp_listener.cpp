#include "udp_listener.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <stdexcept>

UdpListener::UdpListener(int port)
    : port_(port), socket_fd_(-1), running_(false) {}

UdpListener::~UdpListener() { stop(); }

void UdpListener::start(MessageCallback callback) {
  if (running_) {
    return;
  }

  callback_ = std::move(callback);

  // Create UDP socket
  socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_fd_ < 0) {
    throw std::runtime_error("Failed to create socket");
  }

  // Set socket options to reuse address
  int opt = 1;
  if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    close(socket_fd_);
    throw std::runtime_error("Failed to set socket options");
  }

  // Bind to port
  struct sockaddr_in server_addr{};
  std::memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port_);

  if (bind(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) <
      0) {
    close(socket_fd_);
    throw std::runtime_error("Failed to bind to port " + std::to_string(port_));
  }

  // Start listening thread
  running_ = true;
  thread_ = std::make_unique<std::thread>(&UdpListener::listen_thread, this);
}

void UdpListener::stop() {
  if (!running_) {
    return;
  }

  running_ = false;

  // Close socket to unblock recvfrom
  if (socket_fd_ >= 0) {
    shutdown(socket_fd_, SHUT_RDWR);
    close(socket_fd_);
    socket_fd_ = -1;
  }

  // Wait for thread to finish
  if (thread_ && thread_->joinable()) {
    thread_->join();
  }

  thread_.reset();
}

bool UdpListener::is_listening() const { return running_; }

int UdpListener::get_port() const { return port_; }

void UdpListener::set_port(int port) {
  if (!running_) {
    port_ = port;
  }
}

void UdpListener::listen_thread() {
  char buffer[65536];
  struct sockaddr_in client_addr{};
  socklen_t client_len = sizeof(client_addr);

  while (running_) {
    ssize_t bytes_received =
        recvfrom(socket_fd_, buffer, sizeof(buffer) - 1, 0,
                 (struct sockaddr*)&client_addr, &client_len);

    if (bytes_received < 0) {
      if (running_) {
        std::cerr << "Error receiving data\n";
      }
      break;
    }

    if (bytes_received > 0) {
      buffer[bytes_received] = '\0';
      std::string message(buffer);

      // Get source IP
      char source_ip[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &client_addr.sin_addr, source_ip, INET_ADDRSTRLEN);

      // Parse and callback
      if (callback_) {
        SyslogMessage parsed_msg = SyslogMessage::parse(message, source_ip);
        callback_(parsed_msg);
      }
    }
  }
}

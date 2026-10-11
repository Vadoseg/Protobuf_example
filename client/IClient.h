#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>

class IClient {
  public:
    IClient() = default;

  protected:
    void connectClient(){
      sock_fd = socket(AF_INET, SOCK_STREAM, 0);
      if (sock_fd < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return;
      }

      serv_addr.sin_family = AF_INET;
      serv_addr.sin_port = htons(8080);

      if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address" << std::endl;
        return;
      }

      if (connect(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return;
      }
      std::cout << "[Client] Connected to server." << std::endl;
    }

    bool readAll(int socket_fd, void* buffer, size_t size) {
      size_t bytes_read = 0;
      char* ptr = static_cast<char*>(buffer);
      while (bytes_read < size) {
        ssize_t result = read(socket_fd, ptr + bytes_read, size - bytes_read);
        if (result <= 0) return false;
        bytes_read += result;
      }
      return true;
    }

    bool writeAll(int socket_fd, const void* buffer, size_t size) {
      size_t bytes_written = 0;
      const char* ptr = static_cast<const char*>(buffer);
      while (bytes_written < size) {
        ssize_t result = write(socket_fd, ptr + bytes_written, size - bytes_written);
        if (result <= 0) return false;
        bytes_written += result;
      }
      return true;
    }

  protected:
    int sock_fd;
    sockaddr_in serv_addr{};
};

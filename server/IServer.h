#pragma once
#include <unistd.h>
#include <sys/socket.h>

class IServer{
  public:
    virtual void start() = 0;

  protected:
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
};

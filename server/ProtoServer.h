#pragma once
#include "IServer.h"
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "dsp_test_sweep.pb.h"

std::ostream& operator<<(std::ostream& os, MainConfig req){
  os << "\nMainConfig {";
  os << "\n\tDSP_EN = " << req.ctrl().dsp_enabled();
  os << "\n\tDEC_FACTOR_CODE = " << req.ctrl().dec_factor_code();
  os << "\n\tPILOT_EN = " << req.ctrl().pilot_enabled();
  os << "\n\tPROBE_EN = " << req.ctrl().probe_enabled();
  os << "\n\tOVERVIEW_MODE = " << req.ctrl().overview_mode();
  os << "\n\tTX2RX_LAT = " << req.ctrl().tx2rx_latency();
  os << "\n}\n";
  return os;
}

class ProtoServer : public IServer {
  public:
    ProtoServer(){
      server_fd = socket(AF_INET, SOCK_STREAM, 0);
      if (server_fd < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return;
      }

      int opt = 1;
      setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
      address.sin_family = AF_INET;
      address.sin_addr.s_addr = INADDR_ANY;
      address.sin_port = htons(8080);

      if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        return;
      }
    }

    ~ProtoServer(){
      close(server_fd);
    }

    void start() override{
      if (listen(server_fd, 10) < 0) {
        std::cerr << "Listen failed" << std::endl;
        return;
      }

      std::cout << "Server listening on port 8080 (Protobuf)..." << std::endl;

      while (true) {
        sockaddr_in client_addr{};
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd >= 0) {
          std::cout << "[ProtoServer] Client connected from " << inet_ntoa(client_addr.sin_addr) << std::endl;
          handle_client(client_fd); // Block call
        }
      }
    }

  private:
    void handle_client(int client_fd) {
      while (true) {
        req.Clear();
        uint32_t network_size = 0;
        if (!readAll(client_fd, &network_size, sizeof(network_size))) break;
        uint32_t message_size = ntohl(network_size);

        std::vector<char> buffer(message_size);
        auto start = std::chrono::high_resolution_clock::now();
        if (!readAll(client_fd, buffer.data(), message_size)) break;

        if (!req.ParseFromArray(buffer.data(), buffer.size())) {
          std::cerr << "Failed to parse request protobuf." << std::endl;
          break;
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "\nВремя выполнения парсинга Protobuf из сети (Binary): " << duration.count() << " мкс" << std::endl;

        std::cout << "[Server]: " << req;

        BasicResp resp;
        ++id;
        resp.set_id(id);
        resp.set_message("Processed: " + std::to_string(req.ByteSizeLong()) + " bytes");

        int proto_size = resp.ByteSizeLong();
        std::vector<uint8_t> out_buffer(sizeof(uint32_t) + proto_size);
        uint32_t response_size_net = htonl(static_cast<uint32_t>(proto_size));
        std::memcpy(out_buffer.data(), &response_size_net, sizeof(uint32_t));

        resp.SerializeToArray(out_buffer.data() + sizeof(uint32_t), proto_size);
        if (!writeAll(client_fd, out_buffer.data(), out_buffer.size())) break;
      }

      close(client_fd);
      std::cout << "[ProtoServer] Client disconnected." << std::endl;
    }

  private:
    int server_fd;
    sockaddr_in address{};
    MainConfig req;
    uint32_t id = 0;
};

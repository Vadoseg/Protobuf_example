#pragma once
#include "IServer.h"
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <chrono>
#include "dsp_test_sweep.pb.h"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

class RJServer : public IServer {
  public:
    RJServer(){
      // doc.Accept(writer);
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

    ~RJServer(){
      close(server_fd);
    }

    void start() override{
      if (listen(server_fd, 10) < 0) {
        std::cerr << "Listen failed" << std::endl;
        return;
      }

      std::cout << "Server listening on port 8080 (Rapidjson)..." << std::endl;

      while (true) {
        sockaddr_in client_addr{};
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd >= 0) {
          std::cout << "[RJServer] Client connected from " << inet_ntoa(client_addr.sin_addr) << std::endl;
          handle_client(client_fd); // Blo`qck call
        }
      }
    }

  private:
    void handle_client(int client_fd) {
      int i = 0;
      auto start = std::chrono::high_resolution_clock::now();
      std::vector<char> buffer;

      while (true) {
        // sb.Clear();
        // doc.SetNull();
        size_t network_size = 0;
        if (!readAll(client_fd, &network_size, sizeof(network_size))) break;
        size_t message_size = ntohl(network_size);

        if (buffer.size() < message_size + 1)
          buffer.resize(message_size + 1);
        if (!readAll(client_fd, buffer.data(), message_size)) break;

        buffer[message_size] = '\0';
         rapidjson::Document doc;
        doc.Parse(buffer.data());
        if (doc.HasParseError()) {
          std::cerr << "Failed to parse request json." << std::endl;
          break;
        } else{
          if(first_msg) {
            start = std::chrono::high_resolution_clock::now();
            first_msg = false;
          }
        }


        // std::cout << "[Server]: " << req;

        // BasicResp resp;
        // ++id;
        // resp.set_id(id);
        // resp.set_message("Processed: " + std::to_string(req.ByteSizeLong()) + " bytes");

        // int proto_size = resp.ByteSizeLong();
        // std::vector<uint8_t> out_buffer(sizeof(uint32_t) + proto_size);
        // uint32_t response_size_net = htonl(static_cast<uint32_t>(proto_size));
        // std::memcpy(out_buffer.data(), &response_size_net, sizeof(uint32_t));

        // resp.SerializeToArray(out_buffer.data() + sizeof(uint32_t), proto_size);
        // if (!writeAll(client_fd, out_buffer.data(), out_buffer.size())) break;
        ++i;
        if(i >= 10000) break;
      }
      auto end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
      std::cout << "\nВремя выполнения парсинга Json из сети: " << duration.count()/i << " мкс" << std::endl;
      close(client_fd);
      std::cout << "[RJServer] Client disconnected." << std::endl;
    }

  private:
    bool first_msg = true;
    int server_fd;
    sockaddr_in address{};
    // rapidjson::Document doc;
    // rapidjson::StringBuffer sb;
    // rapidjson::Writer<rapidjson::StringBuffer> writer{sb};
    uint32_t id = 0;
};

#pragma once
#include "IClient.h"
#include <string>
#include <sstream>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

class RJClient : public IClient {
  public:

  void connect(){
    connectClient();

    while(1){
      doc.SetNull();
      parseAndSaveDataRJ("../../../dsp_test_sweep.json");

      auto start = std::chrono::high_resolution_clock::now();
      size_t binary_size = htonl(json_str.length());
      writeAll(sock_fd, &binary_size, sizeof(binary_size));
      writeAll(sock_fd, json_str.c_str(), json_str.length());

      auto end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
      std::cout << "\nВремя выполнения отправки Json: " << duration.count() << " мкс" << std::endl;

      // uint32_t response_size_net = 0;
      // if (readAll(sock_fd, &response_size_net, sizeof(response_size_net))) {
      //     uint32_t response_size = ntohl(response_size_net);

      //     std::vector<char> resp_buffer(response_size);
      //     if (readAll(sock_fd, resp_buffer.data(), response_size)) {
      //       BasicResp resp;
      //       if (resp.ParseFromArray(resp_buffer.data(), resp_buffer.size())) {
      //         std::cout << "[RJClient] Response received!";
      //         std::cout << "\nID: " << resp.id();
      //         std::cout << "\nMessage: " << resp.message() << std::endl;
      //       }
      //     }
      //   }

      std::cout << "Нажмите Enter для продолжения..." << std::endl;
      std::cin.get();
    }
  }

  private:
    void parseAndSaveDataRJ(const char* path){
      std::ifstream cfg(path);
      if (!cfg.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return;
      }

      std::stringstream buffer;
      buffer << cfg.rdbuf();
      json_str = buffer.str();
      cfg.close();

      auto start = std::chrono::high_resolution_clock::now();

      if (doc.Parse(json_str.c_str()).HasParseError()) {
        std::cerr << "Ошибка парсинга JSON! Код ошибки: "
          << doc.GetParseError() << std::endl;
        return;
      }
      auto end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
      std::cout << "\nВремя выполнения Json Parsing: " << duration.count() << " мкс" << std::endl;
    }

  private:
    std::string json_str;
    int server_fd;
    sockaddr_in address{};
    rapidjson::Document doc;
    // rapidjson::StringBuffer sb;
    // rapidjson::Writer<rapidjson::StringBuffer> writer{sb};
    uint32_t id = 0;
};

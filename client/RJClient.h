#pragma once
#include "IClient.h"
#include <string>
#include <sstream>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <chrono>

class RJClient : public IClient {
  public:

  void connect(){
    connectClient();

    while(1){
      doc.SetNull();
      parseAndSaveDataRJ("../../../dsp_test_sweep.json");

      size_t binary_size = htonl(json_str.length());

      auto start = std::chrono::high_resolution_clock::now();
      for(int i = 0; i < 10000; ++i){
        writeAll(sock_fd, &binary_size, sizeof(binary_size));
        writeAll(sock_fd, json_str.c_str(), json_str.length());
      }

      auto end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
      std::cout << "\nВремя выполнения отправки Json: " << duration.count()/10000 << " мкс" << std::endl;
      break;

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

      // std::cout << "Нажмите Enter для продолжения..." << std::endl;
      // std::cin.get();
    }
  }

  private:
    void parseAndSaveDataRJ(const char* path){
      std::ifstream cfg(path, std::ios::binary | std::ios::ate);
      if (!cfg.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return;
      }

      std::streamsize size = cfg.tellg();
      cfg.seekg(0, std::ios::beg);

      json_str = std::string(size, '\0');
      if (!cfg.read(&json_str[0], size)) {
        std::cerr << "Ошибка чтения файла!" << std::endl;
        return;
      }
      cfg.close();

      const int iterations = 100;

      auto start = std::chrono::high_resolution_clock::now();
      for (int i = 0; i < iterations; ++i) {
        doc.Clear();
        // rapidjson::MemoryStream ms(json_str.c_str(), json_str.size());
        std::string tmp = json_str;

        if (doc.ParseInsitu(tmp.data()).HasParseError()) {
          std::cerr << "Ошибка парсинга JSON на итерации " << i
                    << "! Код: " << doc.GetParseError() << std::endl;
          return;
        }
      }

      auto end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
      std::cout << "\nСреднее время RapidJSON Parsing (MemoryStream): "
                << duration.count() / iterations << " мкс" << std::endl;
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

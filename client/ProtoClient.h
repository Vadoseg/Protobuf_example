#pragma once
#include "IClient.h"
#include "dsp_test_sweep.pb.h"

class ProtoClient : public IClient {
  public:
    ~ProtoClient(){
      close(sock_fd);
    }

    void connect(){
      connectClient();

      while(1){
        MainConfig req;
        parseAndSaveDataBinaryProto("../../../config_dsp_test_sweep.pb", req);

        int proto_size = req.ByteSizeLong();
        std::vector<uint8_t> out_buffer(sizeof(uint32_t) + proto_size);

        uint32_t request_size_net = htonl(static_cast<uint32_t>(proto_size));
        std::memcpy(out_buffer.data(), &request_size_net, sizeof(uint32_t));

        auto start = std::chrono::high_resolution_clock::now();
        req.SerializeToArray(out_buffer.data() + sizeof(uint32_t), proto_size);
        writeAll(sock_fd, out_buffer.data(), out_buffer.size());

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "\nВремя выполнения отправки Protobuf (Binary): " << duration.count() << " мкс" << std::endl;

        uint32_t response_size_net = 0;
        if (readAll(sock_fd, &response_size_net, sizeof(response_size_net))) {
          uint32_t response_size = ntohl(response_size_net);

          std::vector<char> resp_buffer(response_size);
          if (readAll(sock_fd, resp_buffer.data(), response_size)) {
            BasicResp resp;
            if (resp.ParseFromArray(resp_buffer.data(), resp_buffer.size())) {
              std::cout << "[ProtoClient] Response received!";
              std::cout << "\nID: " << resp.id();
              std::cout << "\nMessage: " << resp.message() << std::endl;
            }
          }
        }

        std::cout << "Нажмите Enter для продолжения..." << std::endl;
        std::cin.get();
      }
    }

  private:
    void parseAndSaveDataBinaryProto(const char* path, MainConfig& main_cfg){
      std::ifstream cfg;
      cfg.open(path, std::ios::binary);
      if (!cfg.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return;
      }
      auto start = std::chrono::high_resolution_clock::now();

      if (!main_cfg.ParseFromIstream(&cfg)) {
        std::cerr << "Ошибка парсинга .pb файла!" << std::endl;
        return;
      }

      auto end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
      std::cout << "\nВремя выполнения Protobuf Parsing (Binary): " << duration.count() << " мкс" << std::endl;
    }

};

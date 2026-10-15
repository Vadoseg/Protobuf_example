#pragma once
#include "IClient.h"
#include "dsp_test_sweep.pb.h"
#include <fcntl.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

class ProtoClient : public IClient {
  public:
    ~ProtoClient(){
      close(sock_fd);
    }

    void connect(){
      connectClient();
      MainConfig req;

      while(1){
        req.Clear();
        parseAndSaveDataBinaryProto("../../../config_dsp_test_sweep.pb", req);

        int proto_size = req.ByteSizeLong();
        size_t total_size = sizeof(uint32_t) + proto_size;
        if (out_buffer.capacity() < total_size)
          out_buffer.reserve(total_size * 2);
        out_buffer.resize(total_size);

        uint32_t request_size_net = htonl(static_cast<uint32_t>(proto_size));
        std::memcpy(out_buffer.data(), &request_size_net, sizeof(uint32_t));
        req.SerializeWithCachedSizesToArray(out_buffer.data() + sizeof(uint32_t));

        auto start = std::chrono::high_resolution_clock::now();
        for(int i = 0; i < 10000; ++i)
         writeAll(sock_fd, out_buffer.data(), total_size);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "\nВремя выполнения отправки Protobuf (Binary): " << duration.count()/10000 << " мкс" << std::endl;
        break;
        uint32_t response_size_net = 0;
        if (readAll(sock_fd, &response_size_net, sizeof(response_size_net))) {
          uint32_t response_size = ntohl(response_size_net);

          std::vector<char> resp_buffer(response_size);
          if (readAll(sock_fd, resp_buffer.data(), response_size)) {
            BasicResp resp;
            resp.ParseFromArray(resp_buffer.data(), resp_buffer.size());
            // if (resp.ParseFromArray(resp_buffer.data(), resp_buffer.size())) {
              // std::cout << "[ProtoClient] Response received!";
              // std::cout << "\nID: " << resp.id();
              // std::cout << "\nMessage: " << resp.message() << std::endl;
            // }
          }
        }

        // std::cout << "Нажмите Enter для продолжения..." << std::endl;
        // std::cin.get();
      }
    }

  private:
    void parseAndSaveDataBinaryProto(const char* path, MainConfig& main_cfg){
      int fd = open(path, O_RDONLY);
      if (fd < 0) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return;
      }

      const int iterations = 100;

      auto start = std::chrono::high_resolution_clock::now();
      for (int i = 0; i < iterations; ++i) {
        main_cfg.Clear();
        lseek(fd, 0, SEEK_SET);

        google::protobuf::io::FileInputStream file_stream(fd);
        if (!main_cfg.ParseFromZeroCopyStream(&file_stream)) {
          std::cerr << "Ошибка парсинга на итерации " << i << std::endl;
          close(fd);
          return;
        }
      }

      auto end = std::chrono::high_resolution_clock::now();
      close(fd);

      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
      std::cout << "\nСреднее время Protobuf Parsing (ZeroCopy): "
                << duration.count() / iterations << " мкс" << std::endl;
    }

    std::vector<uint8_t> out_buffer;

};

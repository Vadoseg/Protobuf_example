#pragma once
#include <fstream>
#include <chrono>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "gen/dsp_test_sweep.pb.h"

class ConfigPbParser {
  public:
    ConfigPbParser(const char* config_path) : path(config_path){}

    void parseAndSaveDataTextProto(){
      cfg.open(path);
      if (!cfg.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return;
      }

      google::protobuf::io::IstreamInputStream stream(&cfg);
      auto start = std::chrono::high_resolution_clock::now();


      if (!google::protobuf::TextFormat::Parse(&stream, &main_cfg)) {
        std::cerr << "Ошибка парсинга .textproto файла!" << std::endl;
        return;
      }

      // auto& ctrl = main_cfg.ctrl();
      // dsp_e = ctrl.dsp_enabled();
      // pilot_e = ctrl.pilot_enabled();
      // probe_e = ctrl.probe_enabled();
      // overview_mode = ctrl.overview_mode();
      // dec_fact_code = ctrl.dec_factor_code();
      // dsp_out_mux = ctrl.dsp_output_mux();
      // sync_pulse_mux = ctrl.sync_pulse_mux();
      // tx_output_mux = ctrl.tx_output_mux();
      // tx2rx_lat = ctrl.tx2rx_latency();
      // for(int i = 0; i < ctrl.input_mux_rx_size(); ++i)
      //   in_mux_rx[i] = ctrl.input_mux_rx(i);


      auto end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
      std::cout << "\nВремя выполнения Protobuf Parsing (TextProto): " << duration.count() << " мкс" << std::endl;

      // std::cout << "\nARRAY: " << in_mux_rx[3];
    }

    void parseAndSaveDataBinaryProto(){
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


  private:
    const char* path;
    std::ifstream cfg;
    MainConfig main_cfg;

    // Showed below only for saving test, use SAME STRUCTURE FROM .PROTO
    bool dsp_e{false};
    bool pilot_e{false};
    bool probe_e{false};
    uint32_t overview_mode = 0;
    uint32_t dec_fact_code = 0;
    uint32_t tx2rx_lat = 0;
    Control_DspOutputMux dsp_out_mux = Control_DspOutputMux_DSP_MUX_UNSPECIFIED;
    Control_SyncPulseMux sync_pulse_mux = Control_SyncPulseMux_SYNC_MUX_UNSPECIFIED;
    Control_TxOutputMux tx_output_mux = Control_TxOutputMux_TX_MUX_UNSPECIFIED;
    std::array<Control_InputMux, 4> in_mux_rx{};
};

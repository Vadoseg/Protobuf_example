#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Заголовочные файлы RapidJSON
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

struct Queue_t {
  uint32_t index;
  uint32_t tag;
  uint32_t sm;
  bool active;
  bool mode_switch;
  uint32_t az_code;
  uint32_t el_code;
  uint32_t fr_code;
};

struct Overview_t {

};

class CfgJsParser {
  public:
    CfgJsParser(const char* config_path){
      std::ifstream cfg(config_path);
      if (!cfg.is_open()) {
        std::cerr << "Не удалось открыть файл!" << std::endl;
        return;
      }

      std::stringstream buffer;
      buffer << cfg.rdbuf();
      std::string json_content = buffer.str();
      cfg.close();

      parseAndSaveData(json_content);
    }

    void parseAndSaveData(std::string& json_content){
      auto start = std::chrono::high_resolution_clock::now();

      rapidjson::Document doc;
      if (doc.Parse(json_content.c_str()).HasParseError()) {
        std::cerr << "Ошибка парсинга JSON! Код ошибки: "
                  << doc.GetParseError() << std::endl;
        return;
      }

      rapidjson::StringBuffer strBuffer;
      rapidjson::Writer<rapidjson::StringBuffer> writer(strBuffer);
      doc.Accept(writer);

      if (doc.IsObject() && doc.HasMember("control"))
      {
        auto& ctrl = doc["control"];
        dsp_e = ctrl["dsp_enabled"].GetBool();
        pilot_e = ctrl["pilot_enabled"].GetBool();
        probe_e = ctrl["probe_enabled"].GetBool();
        overview_mode = ctrl["overview_mode"].GetUint();
        dsp_out_mux = ctrl["dsp_output_mux"].GetString();
        sync_pulse_mux = ctrl["sync_pulse_mux"].GetString();
        tx_output_mux = ctrl["tx_output_mux"].GetString();
        in_mux_rx[0] = ctrl["input_mux_rx0"].GetString();
        in_mux_rx[1] = ctrl["input_mux_rx1"].GetString();
        in_mux_rx[2] = ctrl["input_mux_rx2"].GetString();
        in_mux_rx[3] = ctrl["input_mux_rx3"].GetString();

        auto& tasks = doc["tasks"];
        const auto& que_array = tasks["que"];
        for (auto it = que_array.Begin(); it != que_array.End(); ++it) {
            ques.push_back(Queue_t{
              .index = (*it)["index"].GetUint(),
              .tag = (*it)["tag"].GetUint(),
              .sm = (*it)["sm"].GetUint(),
              .active = (*it)["active"].GetBool(),
              .mode_switch= (*it)["mode_switch"].GetBool(),
              .az_code = (*it)["az_code"].GetUint(),
              .el_code = (*it)["el_code"].GetUint(),
              .fr_code = (*it)["fr_code"].GetUint()
            });
        }
      }

      auto end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
      std::cout << "\nВремя выполнения Json Parsing: " << duration.count() << " мс" << std::endl;
    }

  private:
    bool dsp_e{false};
    bool pilot_e{false};
    bool probe_e{false};
    uint32_t overview_mode = 0;
    uint32_t dec_fact_code = 0;
    std::string dsp_out_mux;
    std::string sync_pulse_mux;
    std::string tx_output_mux;
    std::array<std::string, 4> in_mux_rx{};

    std::vector<Queue_t> ques;

};

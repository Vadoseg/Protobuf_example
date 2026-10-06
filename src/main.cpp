#include <iostream>
#include "config_pb_gen.h"
#include "config_pb_parser.h"
#include "config_js_parser.h"

int main() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  // ConfigPbGen cfg_gen;
  // cfg_gen.createCfg();

  // ConfigPbParser cfg_parser("../../../config_dsp_test_sweep.textproto");
  // cfg_parser.parseAndSaveDataTextProto();

  CfgJsParser cfg_js_parser("../../../dsp_test_sweep.json");

  ConfigPbParser cfg_parser_bin("../../../config_dsp_test_sweep.pb");
  cfg_parser_bin.parseAndSaveDataBinaryProto();

  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}

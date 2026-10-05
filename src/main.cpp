#include <iostream>
#include "config_pb_gen.h"
#include "config_pb_parser.h"
#include "config_js_parser.h"

int main() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  // ConfigPbGen cfg_gen;
  // cfg_gen.createCfg();
  ConfigPbParser cfg_parser("../../../config_dsp_test_sweep.textproto");
  CfgJsParser cfg_js_parser("../../../dsp_test_sweep.json");


  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}

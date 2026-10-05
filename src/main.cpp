#include <iostream>
#include "config_pb_gen.h"
#include "config_pb_parser.h"

int main() {

  // ConfigPbGen cfg_gen;
  // cfg_gen.createCfg();
  ConfigPbParser cfg_parser("../../../config_dsp_test_sweep.textproto");

  return 0;
}

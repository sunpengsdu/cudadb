//============================================================================
// Name        : CudaDB.cpp
// Author      : Sun Peng
// Version     :
// Copyright   : Your copyright notice
// Description : CudaDB in C++, Ansi-style
//============================================================================

#include <string>
#include <iostream>
#include <glog/logging.h>
#include "mog_gpu.h"

int main(int argc, char**argv) {
  mog_gpu *DB;
  DB = new mog_gpu(100);
  DB->use_device(0);
  DB->use_device(1);
  DB->setup();
  for (int32_t i=0; i<1000000; ++i) {
    DB->insert(0, std::to_string(i), "sunpeng_test_mog_gpu_size");
  }
  char* test_get = mog_gpu::malloc(0, 100);
  for (int32_t i=999900; i<1000010; ++i) {
    std::cout << DB->read(0, std::to_string(i), test_get) << "\n";
  }
  DB->close();
  return 0;
}

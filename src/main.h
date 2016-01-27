//============================================================================
// Name        : CudaDB.h
// Author      : Sun Peng
// Version     :
// Copyright   : Your copyright notice
// Description : CudaDB in C++, Ansi-style
//============================================================================

#ifndef SUNPENG_MOG_MAIN_H
#define SUNPENG_MOG_MAIN_H

#include <string>
#include <iostream>
#include "glog/logging.h"

extern char* mog_malloc_gpu(int32_t device_id,
                int32_t size);
#endif

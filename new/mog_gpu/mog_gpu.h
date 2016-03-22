//============================================================================
// Name        : DB.h
// Author      : Sun Peng
// Version     :
// Copyright   : Your copyright notice
// Description : CudaDB in C++, Ansi-style
//============================================================================

#ifndef SUNPENG_CAP_MOG_GPU_H
#define SUNPENG_CAP_MOG_GPU_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <glog/logging.h>
#include "./include/cuda.h"
#include "./GpuCache.h"

typedef char* Page;

class mog_gpu {
public:
	mog_gpu();
	mog_gpu(const int64_t cache_size);
    ~mog_gpu();
    int64_t set_size(const int64_t cache_size);
    int64_t use_device(const int64_t device_id);
    int64_t read(const int64_t device_id, const std::string& key, char *value);
    int64_t insert(const int64_t device_id, const std::string& key, const std::string& value);
    int64_t setup();
    int64_t close();
    static char* malloc(int64_t device_id, int64_t size);

private:
    int64_t get_device_num(int *num);

    int device_num;
    std::vector<int64_t> devices;

    int64_t page_size;
    int64_t page_num;

    std::map<int64_t, GpuCache*> gpu_caches;
};
#endif

//============================================================================
// Name        : DB.h
// Author      : Sun Peng
// Version     :
// Copyright   : Your copyright notice
// Description : CudaDB in C++, Ansi-style
//============================================================================

#ifndef SUNPENG_CAP_MOG_H
#define SUNPENG_CAP_MOG_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include "./include/cuda.h"

#include "glog/logging.h"
#include "yaml-cpp/yaml.h"

#include "./CpuCache.h"
#include "./GpuCache.h"

extern void mog_malloc_gpu(int32_t device_id,
                        int32_t page_size,
                        char** pages,
                        int32_t page_id);

extern void mog_memcpy_cpu_to_gpu(int32_t device_id, char* dst, const char* src, int32_t page_size);

extern void mog_memcpy_gpu_to_cpu(int32_t device_id, char* dst, const char* src, int32_t page_size);

extern void mog_vectorAdd(int32_t device_id, const char *A, const char *B, char *C, int numElements);

namespace cap {

typedef char* Page;
typedef std::vector<Page> Block;
typedef std::vector<Block> Buffer;
typedef std::vector<Page> Cache;

class mog {
public:
    mog();
    mog(const std::string& config_file);
    ~mog();

    int32_t setup(const std::string& config_file);
    int32_t open(const std::string& db_name);
    bool db_exist(const std::string& db_name);

private:
    int32_t initial_para(const std::string& config_file);
    int32_t get_device_num(int32_t *num);
    int32_t mog_malloc_cpu(int32_t page_size, char** pages, int32_t page_id);
    int32_t allocate_memory();

    int32_t create_db();
    int32_t load_db();

    YAML::Node  yaml_config;
    std::string config_file;
    int32_t device_num;
    std::vector<int32_t> devices;
    std::string db_name;

    int32_t page_size;
    //cache is used for read operation
    //buffer is used for write operation
    int32_t cpu_page_num;
    int32_t gpu_page_num;
    int32_t block_page_num;
    int32_t buffer_page_num;

    std::map<int32_t, GpuCache> gpu_caches;
    CpuCache cpu_caches;
};

}

#endif

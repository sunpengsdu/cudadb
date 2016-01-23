//============================================================================
// Name        : DB.h
// Author      : Sun Peng
// Version     :
// Copyright   : Your copyright notice
// Description : CudaDB in C++, Ansi-style
//============================================================================

#ifndef SUNPENG_CAP_MOG_H
#define SUNPENG_CAP_MOG_H

#define MOG_SUCCESS 0

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
#include "./WriteBuffer.h"

#include "./CentraIndex.h"

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
    bool    exist(const std::string& db_name);
    int32_t write(const std::string& key, const char *value, int32_t length);
    int32_t read(const std::string& key, char *value);

private:
    int32_t initial_para(const std::string& config_file);
    int32_t get_device_num(int32_t *num);
    int32_t allocate_memory();

    int32_t create();
    int32_t load();

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
    int32_t page_per_block;;
    int32_t buffer_page_num;
    std::string ssd_path;
    std::string dfs_path;

    std::map<int32_t, GpuCache> gpu_caches;
    CpuCache cpu_caches;
   // WriteBuffer write_buffers;
};

}

#endif

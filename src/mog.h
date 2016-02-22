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
#include "./SSDCache.h"

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

    int64_t setup(const std::string& config_file);
    int64_t open(const std::string& db_name);
    bool    exist(const std::string& db_name);
    int64_t insert_file(const std::string &key, const std::string& file_path);
    int64_t write(const std::string& key, const char *value, int64_t length);
    int64_t read(const std::string& key, char *value);
    int64_t gpu_read(const int64_t device_id, const std::string& key, char *value);
    int64_t sync();
    int64_t close();
    static char* malloc_gpu(int64_t device_id, int64_t size);

private:
    int64_t initial_para(const std::string& config_file);
    int64_t get_device_num(int *num);
    int64_t allocate_memory();

    int64_t create();
    int64_t load();

    YAML::Node  yaml_config;
    std::string config_file;
    int device_num;
    std::vector<int64_t> devices;
    std::string db_name;

    int64_t page_size;
    //cache is used for read operation
    //buffer is used for write operation
    int64_t cpu_page_num;
    int64_t gpu_page_num;
    int64_t SSD_page_num;
    int64_t page_per_block;;
    int64_t buffer_page_num;
    std::string ssd_path;
    std::string dfs_path;

    std::map<int64_t, GpuCache*> gpu_caches;
   // CpuCache cpu_caches;
   // WriteBuffer write_buffers;
};

}

#endif

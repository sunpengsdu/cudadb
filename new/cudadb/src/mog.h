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
#include <glog/logging.h>
#include <yaml-cpp/yaml.h>

#include "./include/cuda.h"
#include "./CpuCache.h"
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
    int64_t insert_file_with_copy(const std::string &key, const std::string& file_path, char* value);
    int64_t read_file(const std::string &path, char *value);
    int64_t write(const std::string& key, const char *value, int64_t length);
    int64_t read(const std::string& key, char *value);
    int64_t sync();
    int64_t close();

private:
    int64_t initial_para(const std::string& config_file);
    int64_t allocate_memory();

    int64_t create();
    int64_t load();

    std::string config_file;
    std::string db_name;

    int64_t page_size;
    int64_t cpu_page_num;
    int64_t SSD_page_num;
    int64_t page_per_block;;
    int64_t buffer_page_num;
    std::string ssd_path;
    std::string hdd_path;
};

}

#endif

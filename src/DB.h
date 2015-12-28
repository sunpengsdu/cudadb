//============================================================================
// Name        : DB.h
// Author      : Sun Peng
// Version     :
// Copyright   : Your copyright notice
// Description : CudaDB in C++, Ansi-style
//============================================================================

#ifndef SUNPENG_CUDADB_DB_H
#define SUNPENG_CUDADB_DB_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <boost/filesystem.hpp>

#include "glog/logging.h"
#include "yaml-cpp/yaml.h"

namespace cudadb {

class DB {
public:
    DB();
    DB(const std::string& config_file);
    ~DB();
    int32_t open(const std::string& db_name);
    bool db_exist(const std::string& db_name);

private:
    int32_t initial_para(const std::string& config_file);
    int32_t create_db();
    int32_t load_db();

    YAML::Node  yaml_config;
    std::string config_file;

    std::string db_name;

    int32_t page_size;
    //cache is used for read operation
    //buffer is used for write operation
    int32_t cache_ram_size;
    int32_t cahe_gpu_size;
    int32_t buffer_ram_size;
};

}

#endif

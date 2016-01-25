/*
 * writebuffers.h
 *
 *  Created on: 5 Jan 2016
 *      Author: sunp
 */

#ifndef SUNPENG_CAP_WRITEBUFFERS_H_
#define SUNPENG_CAP_WRITEBUFFERS_H_

#define WRITEBUFFER_SUCCESS 0

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <thread>
#include <unordered_map>
#include <cmath>
#include <fstream>
#include <atomic>
#include <boost/pool/pool.hpp>

#include "./include/glog/logging.h"
#include "./include/threadpool/boost/threadpool.hpp"
#include "./CentraIndex.h"
#include "./SSDCache.h"

namespace cap {

struct kv_info {
    std::string key;
    char* value;
    int32_t value_length;
    int32_t value_length_type;
    int32_t w_lock;
    int32_t r_lock;
};

class WriteBuffer {
public:
    WriteBuffer();
    virtual ~WriteBuffer();

    int32_t page_size;
    int32_t page_per_block;
    int32_t page_num;
    int32_t max_page_num;
    int32_t initial_flag;
    int32_t flush_thread_num;
    int32_t latest_block_id;
    static const int32_t slab_size[11];//KB

    std::string ssd_path;
    std::string dfs_path;

    boost::pool<> *page[11];

    std::queue<size_t> kv_list;
    std::unordered_map<size_t, kv_info> kv_store;
    std::unordered_map<int32_t, std::vector<kv_info>> blocks;
    std::atomic<int32_t> total_size_without_block;
    std::atomic<int32_t> total_size;
    boost::threadpool::pool *flush_thread_pool;

//    std::vector<>

    //std::vector<std::vector<char*>> buffer;

    int32_t initial(const int32_t page_size,
                const int32_t page_per_block,
                const int32_t page_num);
    int32_t allocate_memory();

    int32_t write(const std::string& key, const char *value, int32_t length);
    int32_t read(const std::string& key, char *value);
    static WriteBuffer& singleton();
    static void flush(int32_t block_id);
};

} /* namespace cap */

#endif /* WRITEBUFFERS_H_ */

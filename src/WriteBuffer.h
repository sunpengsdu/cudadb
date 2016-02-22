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

typedef boost::shared_mutex wbuffer_cache_rwmutex;
typedef boost::shared_lock<ssd_cache_rwmutex> wbuffer_readLock;
typedef boost::unique_lock<ssd_cache_rwmutex> wbuffer_writeLock;

struct kv_info {
    std::string key;
    char* value;
    int64_t value_length;
    int64_t value_length_type;
//    int64_t w_lock;
//    int64_t r_lock;
};

class WriteBuffer {
public:
    WriteBuffer();
    virtual ~WriteBuffer();

    int64_t page_size;
    int64_t page_per_block;
    int64_t page_num;
    int64_t max_page_num;
    int64_t initial_flag;
    int64_t flush_thread_num;
    int64_t latest_block_id;
    static const int64_t slab_size[11];//KB

    std::string ssd_path;
    std::string dfs_path;

    boost::pool<> *page[11];

    wbuffer_cache_rwmutex wbuffer_lock;

    std::queue<size_t> kv_list;
    std::unordered_map<size_t, kv_info> kv_store;
    std::unordered_map<int64_t, std::vector<kv_info>> blocks;
    std::atomic<int64_t> total_size_without_block;
    std::atomic<int64_t> total_size;
    boost::threadpool::pool *flush_thread_pool;

//    std::vector<>

    //std::vector<std::vector<char*>> buffer;

    int64_t initial(const int64_t page_size,
                const int64_t page_per_block,
                const int64_t page_num);
    int64_t allocate_memory();

    int64_t write(const std::string& key, const char *value, int64_t length);
    int64_t read(const std::string& key, char *value);
    static WriteBuffer& singleton();
    static void flush(int64_t block_id);
    int64_t sync();
    int64_t close();
};

} /* namespace cap */

#endif /* WRITEBUFFERS_H_ */

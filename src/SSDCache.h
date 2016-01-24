/*
 * SSDCache.h
 *
 *  Created on: 24 Jan 2016
 *      Author: sunp
 */

#ifndef SUNPENG_CAP_SSDCACHE_H_
#define SUNPENG_CAP_SSDCACHE_H_

#include <string>
#include <queue>
#include <vector>
#include <iostream>
#include <mutex>
#include <atomic>
#include <map>
#include <unordered_map>
#include "./include/threadpool/boost/threadpool.hpp"

namespace cap {

class SSDCache {
public:
    SSDCache();
    virtual ~SSDCache();

    static void flush(int32_t block_id);

    int32_t max_block_num;
    std::atomic<int32_t> block_num;

    boost::threadpool::pool *thread_pool;

    std::string ssd_path;
    std::string dfs_path;
    std::map<int32_t, int32_t> unsynced_block_id;
    std::vector<int32_t> cached_block_id;
    std::vector<int32_t> cached_block_handle_id;

    std::mutex unsynced_block_lock;
    std::mutex cached_block_lock;
    std::mutex cached_block_handle_lock;

    int32_t new_block(int32_t block_id);

    static SSDCache& singleton();
};

} /* namespace cap */

#endif /* SSDCACHE_H_ */

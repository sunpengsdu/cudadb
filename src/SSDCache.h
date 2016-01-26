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
#include <deque>
#include <unordered_map>
#include <stdio.h>

#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS

#include "./CentraIndex.h"
#include "./include/threadpool/boost/threadpool.hpp"
#include "./include/glog/logging.h"

namespace cap {

//struct SSDCachedInfo {
//    int32_t block_id;
//    int32_t file_size;
//};

typedef boost::shared_mutex ssd_cache_rwmutex;
typedef boost::shared_lock<ssd_cache_rwmutex> ssd_cache_readLock;
typedef boost::unique_lock<ssd_cache_rwmutex> ssd_cache_writeLock;

class SSDCache {
public:
    SSDCache();
    virtual ~SSDCache();

    static void flush(int32_t block_id);
    static void fetch(int32_t block_id);

    int32_t max_block_num;
    std::atomic<int32_t> block_num;

    boost::threadpool::pool *thread_pool;

    std::string ssd_path;
    std::string dfs_path;
    std::unordered_map<int32_t, int32_t> unsynced_block_id;
    std::deque<int32_t> cached_block_id;
    std::unordered_map<int32_t, int32_t> cached_block;
    std::vector<int32_t> cached_block_handle_id;

    std::mutex read_record_lock;
    std::map<int32_t, int32_t> read_record;
    std::atomic<int32_t> read_num;

    ssd_cache_rwmutex ssd_cache_rwlock;

    std::mutex unsynced_block_lock;
    std::mutex cached_block_lock;
    std::mutex cached_block_handle_lock;

    int32_t initial();

    int32_t new_block(int32_t block_id);

    int32_t read(const std::string &key, const IndexInfo &key_info, char* value);

    static SSDCache& singleton();
};

} /* namespace cap */

#endif /* SSDCACHE_H_ */

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
#include <thread>
#include <unordered_map>
#include <stdio.h>
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS
#include <glog/logging.h>
#include "./CentraIndex.h"
#include "./include/threadpool/boost/threadpool.hpp"

namespace cap {
//struct SSDCachedInfo {
//    int64_t block_id;
//    int64_t file_size;
//};
typedef boost::shared_mutex ssd_cache_rwmutex;
typedef boost::shared_lock<ssd_cache_rwmutex> ssd_cache_readLock;
typedef boost::unique_lock<ssd_cache_rwmutex> ssd_cache_writeLock;

class SSDCache {
public:
  SSDCache();
  virtual ~SSDCache();
  static void flush(int64_t block_id);
  static void fetch(int64_t block_id);
  int64_t max_block_num;
  std::atomic<int64_t> block_num;
  boost::threadpool::pool *thread_pool;
  std::string ssd_path;
  std::string hdd_path;
  std::unordered_map<int64_t, int64_t> unsynced_block_id;
  std::deque<int64_t> cached_block_id;
  std::unordered_map<int64_t, int64_t> cached_block;
  std::unordered_map<int64_t, int64_t> caching_block;
  std::vector<int64_t> cached_block_handle_id;
  std::mutex read_record_lock;
  std::map<int64_t, int64_t> read_record;
  std::atomic<int64_t> read_num;
  ssd_cache_rwmutex ssd_cache_rwlock;
  std::mutex unsynced_block_lock;
  std::mutex cached_block_lock;
  std::mutex cached_block_handle_lock;
  int64_t initial();
  int64_t new_block(int64_t block_id);
  int64_t sync();
  int64_t close();
  int64_t read(const std::string &key, const IndexInfo &key_info, char* value);
  static SSDCache& singleton();
};

} /* namespace cap */

#endif /* SSDCACHE_H_ */

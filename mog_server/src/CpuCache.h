/*
 * CpuCache.h
 *
 *  Created on: 5 Jan 2016
 *      Author: sunp
 */

#ifndef SUNPENG_CAP_CPUCACHE_H_
#define SUNPENG_CAP_CPUCACHE_H_

#define CPUCACHE_SUCCESS 0

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <glog/logging.h>

#include "./SSDCache.h"

typedef boost::shared_mutex cpu_cache_rwmutex;
typedef boost::shared_lock<cpu_cache_rwmutex> cpu_cache_readLock;
typedef boost::unique_lock<cpu_cache_rwmutex> cpu_cache_writeLock;

namespace cap {

struct CPUCachedItem {
  //int64_t key;
  int64_t length;
  int64_t length_type;
  std::vector<int64_t> slabs;
};

class CpuCache {
public:
  CpuCache();
  virtual ~CpuCache();

  cpu_cache_rwmutex rw_cache_lock;
  int64_t page_size;
  int64_t slab_num;
  int64_t page_num;
  std::atomic<int64_t> free_slab_num;
  //std::mutex cached_item_queue_lock;
  std::queue<int64_t> free_slabs;
  std::unordered_map<std::string, CPUCachedItem> cached_item;
  std::queue<std::string> cached_item_queue;
  char**  slabs;
  int64_t initial_flag = 0;
  static const int64_t slab_size[11];//KB

  static CpuCache& singleton();
  int64_t initial(const int64_t page_size, const int64_t page_num);
  int64_t allocate_memory();
  int64_t read(const std::string& key, char *value);
  int64_t insert(const std::string &key, const IndexInfo &central_index_info, char *value);
  int64_t close();
};

} /* namespace cap */

#endif /* CPUCACHE_H_ */

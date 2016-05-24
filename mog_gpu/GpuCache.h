/*
 * GpuCache.h
 *
 *  Created on: 5 Jan 2016
 *      Author: sunp
 */

#ifndef SUNPENG_CAP_GPUCACHE_H_
#define SUNPENG_CAP_GPUCACHE_H_

#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <glog/logging.h>

typedef boost::shared_mutex gpu_cache_rwmutex;
typedef boost::shared_lock<gpu_cache_rwmutex> gpu_cache_readLock;
typedef boost::unique_lock<gpu_cache_rwmutex> gpu_cache_writeLock;

extern void mog_malloc_gpu(int64_t device_id, int64_t slab_size, char** slabs, int64_t slab_id);
extern void mog_memcpy_cpu_to_gpu(int64_t device_id, char* dst, const char* src, int64_t page_size);
extern void mog_memcpy_gpu_to_cpu(int64_t device_id, char* dst, const char* src, int64_t page_size);
extern void mog_memcpy_gpu_to_gpu(int64_t device_id, char* dst, const char* src, int64_t slabe_size);
extern void mog_free_gpu(int64_t device_id, char* data_p);

struct GPUCachedItem {
  //int64_t key;
  int64_t length;
  int64_t length_type;
  std::vector<int64_t> slabs;
};

class GpuCache {
public:
  GpuCache();
  GpuCache(const int64_t device_id);
  virtual ~GpuCache();
  int64_t device_id;
  gpu_cache_rwmutex rw_cache_lock;
  int64_t page_size;
  int64_t slab_num;
  int64_t page_num;
  int64_t free_slab_num;
  //std::mutex cached_item_queue_lock;
  std::queue<int64_t> free_slabs;
  int64_t read_thread_num;
  char cpu_buffer[10][1024*1024];
  std::map<std::string, GPUCachedItem> cached_item;
  std::queue<std::string> cached_item_queue;
  char**  slabs;
  char** grouped_slabs;
  int64_t initial_flag;
  static const int64_t slab_size[11];//KB
  int64_t initial(const int64_t page_size, const int64_t page_num);
  int64_t allocate_memory();
  int64_t close();
  int64_t read(const std::string& key, char *value);
  int64_t insert(const std::string &key,
                 const int64_t length,
                 const char *value);
};
#endif /* GPUCACHE_H_ */

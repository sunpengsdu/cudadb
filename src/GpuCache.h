/*
 * GpuCache.h
 *
 *  Created on: 5 Jan 2016
 *      Author: sunp
 */

#ifndef SUNPENG_CAP_GPUCACHE_H_
#define SUNPENG_CAP_GPUCACHE_H_

#define GPUCACHE_SUCCESS 0

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

#include "./WriteBuffer.h"
#include "./CentraIndex.h"
#include "./CpuCache.h"
#include "./include/glog/logging.h"

typedef boost::shared_mutex gpu_cache_rwmutex;
typedef boost::shared_lock<gpu_cache_rwmutex> gpu_cache_readLock;
typedef boost::unique_lock<gpu_cache_rwmutex> gpu_cache_writeLock;


extern void mog_malloc_gpu(int32_t device_id,
                           int32_t slab_size,
                           char** slabs,
                           int32_t slab_id);

extern void mog_memcpy_cpu_to_gpu(int32_t device_id, char* dst, const char* src, int32_t page_size);

extern void mog_memcpy_gpu_to_cpu(int32_t device_id, char* dst, const char* src, int32_t page_size);

extern void mog_memcpy_gpu_to_gpu(int32_t device_id, char* dst, const char* src, int32_t slabe_size);

extern void mog_free_gpu(int32_t device_id, char* data_p);

namespace cap {

struct GPUCachedItem {
    //int32_t key;
    int32_t length;
    int32_t length_type;
    std::vector<int32_t> slabs;
};

class GpuCache {
public:
    GpuCache();
    GpuCache(const int32_t device_id);
    virtual ~GpuCache();

    int32_t device_id;

    gpu_cache_rwmutex rw_cache_lock;
    int32_t page_size;
    int32_t slab_num;
    int32_t page_num;
    std::atomic<int32_t> free_slab_num;
    //std::mutex cached_item_queue_lock;

    std::queue<int32_t> free_slabs;

    std::atomic<int32_t> read_thread_num;
    char cpu_buffer[10][1024*1024];


    std::unordered_map<std::string, GPUCachedItem> cached_item;
    std::queue<std::string> cached_item_queue;

    char**  slabs;
    char** grouped_slabs;


    int32_t initial_flag = 0;
    static const int32_t slab_size[11];//KB

    int32_t initial(const int32_t page_size, const int32_t page_num);
    int32_t allocate_memory();

    int32_t close();

    int32_t read(const std::string& key, char *value);
    int32_t insert(const std::string &key,
                   const int32_t length,
                   const int32_t length_type,
                   char *value);
};

} /* namespace cap */

#endif /* GPUCACHE_H_ */

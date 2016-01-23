/*
 * GpuCache.cpp
 *
 *  Created on: 5 Jan 2016
 *      Author: sunp
 */

#include "GpuCache.h"

namespace cap {

GpuCache::GpuCache() {
    this->device_id  = 0;
    this->page_num   = 0;
    this->page_size  = 0;
    this-> pages     = NULL;
    this->initial_flag = -1;
}

GpuCache::GpuCache(const int32_t device_id) {
    this->device_id  = device_id;
    this->page_num   = 0;
    this->page_size  = 0;
    this-> pages     = NULL;
    this->initial_flag = -1;
}

GpuCache::~GpuCache() {
    // TODO Auto-generated destructor stub
}

int32_t GpuCache::initial(const int32_t page_size, const int32_t page_num) {
    this->page_size    = page_size;
    this->page_num     = page_num;
    this->initial_flag = 0;
    this->pages        = new char*[page_num];
    return GPUCACHE_SUCCESS;
}

int32_t GpuCache::allocate_memory() {
    CHECK_EQ(initial_flag, 0);
    for (int32_t page_id = 0; page_id < page_num; ++page_id) {
        mog_malloc_gpu(device_id, page_size, pages, page_id);
    }
    return GPUCACHE_SUCCESS;
}


} /* namespace cap */

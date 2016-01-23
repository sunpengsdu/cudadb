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
#include <iostream>
#include <string>
#include <vector>

#include <glog/logging.h>

extern void mog_malloc_gpu(int32_t device_id, int32_t page_size, char** pages, int32_t page_id);

extern void mog_memcpy_cpu_to_gpu(int32_t device_id, char* dst, const char* src, int32_t page_size);

extern void mog_memcpy_gpu_to_cpu(int32_t device_id, char* dst, const char* src, int32_t page_size);

namespace cap {

class GpuCache {
public:
    GpuCache();
    GpuCache(const int32_t device_id);
    virtual ~GpuCache();

    int32_t device_id;
    int32_t page_size;
    int32_t page_num;
    char**  pages;
    int32_t initial_flag;

    int32_t initial(const int32_t page_size, const int32_t page_num);
    int32_t allocate_memory();
};

} /* namespace cap */

#endif /* GPUCACHE_H_ */

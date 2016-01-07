/*
 * CpuCache.cpp
 *
 *  Created on: 5 Jan 2016
 *      Author: sunp
 */

#include "CpuCache.h"

namespace cap {

CpuCache::CpuCache() {
    page_size = 0;
    page_num  = 0;
    pages     = NULL;
    initial_flag = -1;
}

CpuCache::~CpuCache() {
    // TODO Auto-generated destructor stub
}

int32_t CpuCache::initial(const int32_t page_size, const int32_t page_num) {

    this->page_size = page_size;
    this->page_num  = page_num;
    this->pages     = new char*[page_num];
    this->initial_flag = 0;
    return CPUCACHE_SUCCESS;
}
int32_t CpuCache::allocate_memory() {
    CHECK_EQ(initial_flag, 0);
    for (int32_t page_id = 0; page_id < page_num; ++page_id) {
        pages[page_id] = (char *)malloc(page_size);
        memset(pages[page_id], 0, page_size);
    }
    return CPUCACHE_SUCCESS;
}

} /* namespace cap */

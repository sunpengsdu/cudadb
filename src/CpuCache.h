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
#include <iostream>
#include <string>
#include <vector>

#include <glog/logging.h>

namespace cap {

class CpuCache {
public:
    CpuCache();
    virtual ~CpuCache();

    int32_t page_size;
    int32_t page_num;
    char**  pages;
    int32_t initial_flag = 0;

    int32_t initial(const int32_t page_size, const int32_t page_num);
    int32_t allocate_memory();
};

} /* namespace cap */

#endif /* CPUCACHE_H_ */

/*
 * CpuCache.h
 *
 *  Created on: 5 Jan 2016
 *      Author: sunp
 */

#ifndef SUNPENG_CAP_CPUCACHE_H_
#define SUNPENG_CAP_CPUCACHE_H_

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>

namespace cap {

class CpuCache {
public:
    CpuCache();
    virtual ~CpuCache();

    int32_t page_size;
    int32_t page_num;
    char**  pages;
};

} /* namespace cap */

#endif /* CPUCACHE_H_ */
